// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "third_party/mlir_edge/iree/vm/bytecode_printer.h"

#include <iomanip>
#include <sstream>

#include "third_party/absl/base/macros.h"
#include "third_party/absl/container/inlined_vector.h"
#include "third_party/absl/strings/str_join.h"
#include "third_party/absl/strings/string_view.h"
#include "third_party/absl/types/span.h"
#include "third_party/mlir_edge/iree/base/status.h"
#include "third_party/mlir_edge/iree/schemas/bytecode/bytecode_v0.h"
#include "third_party/mlir_edge/iree/schemas/source_map_def_generated.h"
#include "third_party/mlir_edge/iree/vm/bytecode_util.h"
#include "third_party/mlir_edge/iree/vm/module.h"
#include "third_party/mlir_edge/iree/vm/type.h"

namespace iree {
namespace vm {

namespace {

template <typename T>
StatusOr<T> ReadValue(absl::Span<const uint8_t> data, int* offset) {
  if (*offset + sizeof(T) > data.size()) {
    return OutOfRangeErrorBuilder(ABSL_LOC) << "Bytecode data underrun";
  }
  auto value = *reinterpret_cast<const T*>(&data[*offset]);
  *offset = *offset + sizeof(T);
  return value;
}

StatusOr<const Type> ReadType(absl::Span<const uint8_t> data, int* offset) {
  ASSIGN_OR_RETURN(uint8_t type_index, ReadValue<uint8_t>(data, offset));
  return Type::FromTypeIndex(type_index);
}

StatusOr<uint8_t> ReadCount(absl::Span<const uint8_t> data, int* offset) {
  return ReadValue<uint8_t>(data, offset);
}

StatusOr<uint16_t> ReadValueSlot(absl::Span<const uint8_t> data, int* offset) {
  return ReadValue<uint16_t>(data, offset);
}

absl::string_view ConstantEncodingToString(ConstantEncoding encoding) {
  switch (encoding) {
#define GET_NAME(ordinal, enum_name, str, ...) \
  case ConstantEncoding::enum_name:            \
    return str;
    IREE_CONSTANT_ENCODING_LIST(GET_NAME)
#undef GET_NAME
    default:
      return "unknown";
  }
}

template <typename T>
std::string TypedDataToString(absl::Span<const uint8_t> bytes) {
  auto typed_data = absl::Span<const T>{
      reinterpret_cast<const T*>(bytes.data()), bytes.size() / sizeof(T)};
  return absl::StrJoin(typed_data, ",");
}

std::string ConstantToString(const Type& type,
                             absl::Span<const uint8_t> bytes) {
  if (!type.is_builtin()) {
    return absl::StrJoin(bytes, ",");
  }
  switch (type.builtin_type()) {
    case BuiltinType::kI8:
      return TypedDataToString<uint8_t>(bytes);
    case BuiltinType::kI16:
      return TypedDataToString<uint16_t>(bytes);
    case BuiltinType::kI32:
      return TypedDataToString<uint32_t>(bytes);
    case BuiltinType::kI64:
      return TypedDataToString<uint64_t>(bytes);
    case BuiltinType::kF16:
      return TypedDataToString<uint16_t>(bytes);
    case BuiltinType::kF32:
      return TypedDataToString<float>(bytes);
    case BuiltinType::kF64:
      return TypedDataToString<double>(bytes);
    default:
      return "<unsupported>";
  }
}

}  // namespace

// static
std::string BytecodePrinter::ToString(
    OpcodeTable opcode_table, const FunctionTable& function_table,
    const ExecutableTable& executable_table,
    const SourceMapResolver& source_map_resolver,
    const BytecodeDef& bytecode_def) {
  BytecodePrinter printer(opcode_table, function_table, executable_table,
                          source_map_resolver);
  auto result = printer.Print(bytecode_def);
  if (!result.ok()) {
    return result.status().ToString();
  }
  return result.ValueOrDie();
}

StatusOr<std::string> BytecodePrinter::Print(
    const BytecodeDef& bytecode_def) const {
  std::ostringstream stream;
  RETURN_IF_ERROR(PrintToStream(bytecode_def, &stream)) << stream.str();
  return stream.str();
}

Status BytecodePrinter::PrintToStream(const BytecodeDef& bytecode_def,
                                      std::ostream* stream) const {
  if (!bytecode_def.contents()) {
    return OkStatus();
  }
  auto data = absl::MakeSpan(
      reinterpret_cast<const uint8_t*>(bytecode_def.contents()->data()),
      bytecode_def.contents()->size());
  return PrintToStream(data, stream);
}

Status BytecodePrinter::PrintToStream(absl::Span<const uint8_t> data,
                                      std::ostream* stream) const {
  // TODO(benvanik): scan and find all branch offsets to insert labels

  int offset = 0;
  absl::optional<SourceLocation> previous_location;
  while (offset < data.length()) {
    auto source_location = source_map_resolver_.ResolveBytecodeOffset(offset);
    if (source_location.has_value()) {
      if (previous_location != source_location) {
        *stream << std::setw(10) << "; " << source_location.value() << "\n";
      }
      previous_location = source_location;
    }

    *stream << std::setw(6) << offset << ": ";

    uint8_t opcode = data[offset++];
    const auto& opcode_info = GetOpcodeInfo(opcode_table_, opcode);
    if (!opcode_info.mnemonic) {
      return UnimplementedErrorBuilder(ABSL_LOC)
             << "Unhandled opcode " << opcode << " at offset " << (offset - 1);
    }
    int payload_offset = offset;

    // Print out return values, if any.
    int base_result_index = 0;
    int printed_result_count = 0;
    for (int i = base_result_index; i < ABSL_ARRAYSIZE(opcode_info.operands);
         ++i) {
      if (opcode_info.operands[i] == OperandEncoding::kNone) break;
      if (printed_result_count > 0) {
        *stream << ", ";
      }
      switch (opcode_info.operands[i]) {
        default:
        case OperandEncoding::kNone:
          return UnimplementedErrorBuilder(ABSL_LOC)
                 << "Unhandled op encoding "
                 << static_cast<int>(opcode_info.operands[i]) << " at offset "
                 << (offset - 1);
        case OperandEncoding::kInputSlot:
        case OperandEncoding::kOutputSlot: {
          // Printing handled below.
          offset += sizeof(uint16_t);
          break;
        }
        case OperandEncoding::kVariadicInputSlots:
        case OperandEncoding::kVariadicOutputSlots: {
          // Printing handled below.
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          offset += count * sizeof(uint16_t);
          break;
        }
        case OperandEncoding::kResultSlot: {
          ++printed_result_count;
          ASSIGN_OR_RETURN(uint16_t slot_ordinal, ReadValueSlot(data, &offset));
          *stream << "%" << slot_ordinal;
          break;
        }
        case OperandEncoding::kVariadicResultSlots: {
          ++printed_result_count;
          *stream << "[";
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          for (int j = 0; j < count; ++j) {
            ASSIGN_OR_RETURN(uint16_t slot_ordinal,
                             ReadValueSlot(data, &offset));
            if (j > 0) *stream << ", ";
            *stream << "%" << slot_ordinal;
          }
          *stream << "]";
          break;
        }
        case OperandEncoding::kVariadicTransferSlots: {
          // Printing handled below.
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          offset += count * 2 * sizeof(uint16_t);
          break;
        }
        case OperandEncoding::kConstant: {
          // Printing handled below.
          ASSIGN_OR_RETURN(auto type, ReadType(data, &offset));
          ASSIGN_OR_RETURN(int rank, ReadCount(data, &offset));
          int element_count = 1;
          for (int j = 0; j < rank; ++j) {
            ASSIGN_OR_RETURN(int dim, ReadValue<int32_t>(data, &offset));
            element_count *= dim;
          }
          offset += sizeof(ConstantEncoding);
          offset += element_count * type.element_size();
          break;
        }
        case OperandEncoding::kFunctionOrdinal: {
          // Printing handled below.
          offset += sizeof(uint32_t);
          break;
        }
        case OperandEncoding::kDispatchOrdinal: {
          // Printing handled below.
          offset += sizeof(uint32_t) + sizeof(uint16_t);
          break;
        }
        case OperandEncoding::kBlockOffset: {
          // Printing handled below.
          offset += sizeof(uint32_t);
          break;
        }
        case OperandEncoding::kTypeIndex: {
          // Printing handled below.
          offset += sizeof(uint8_t);
          break;
        }
        case OperandEncoding::kIndex: {
          // Printing handled below.
          offset += sizeof(int32_t);
          break;
        }
        case OperandEncoding::kIndexList: {
          // Printing handled below.
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          offset += count * sizeof(int32_t);
          break;
        }
        case OperandEncoding::kCmpIPredicate:
        case OperandEncoding::kCmpFPredicate: {
          // Printing handled below.
          offset += sizeof(uint8_t);
          break;
        }
      }
    }
    if (printed_result_count > 0) {
      *stream << " = ";
    }
    offset = payload_offset;

    *stream << opcode_info.mnemonic;

    // Print out operands.
    int base_operand_index = 0;
    int printed_operand_count = 0;
    for (int i = base_operand_index; i < ABSL_ARRAYSIZE(opcode_info.operands);
         ++i) {
      if (opcode_info.operands[i] == OperandEncoding::kNone) break;
      if (opcode_info.operands[i] != OperandEncoding::kResultSlot &&
          opcode_info.operands[i] != OperandEncoding::kVariadicResultSlots) {
        if (i == base_operand_index) {
          *stream << " ";
        } else if (printed_operand_count > 0) {
          *stream << ", ";
        }
      }
      switch (opcode_info.operands[i]) {
        default:
        case OperandEncoding::kNone:
          return UnimplementedErrorBuilder(ABSL_LOC)
                 << "Unhandled op encoding "
                 << static_cast<int>(opcode_info.operands[i]) << " at offset "
                 << (offset - 1);
        case OperandEncoding::kInputSlot: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(uint16_t slot_ordinal, ReadValueSlot(data, &offset));
          *stream << "%" << slot_ordinal;
          break;
        }
        case OperandEncoding::kVariadicInputSlots: {
          ++printed_operand_count;
          *stream << "[";
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          for (int j = 0; j < count; ++j) {
            ASSIGN_OR_RETURN(uint16_t slot_ordinal,
                             ReadValueSlot(data, &offset));
            if (j > 0) *stream << ", ";
            *stream << "%" << slot_ordinal;
          }
          *stream << "]";
          break;
        }
        case OperandEncoding::kOutputSlot: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(uint16_t slot_ordinal, ReadValueSlot(data, &offset));
          *stream << "&"
                  << "%" << slot_ordinal;
          break;
        }
        case OperandEncoding::kVariadicOutputSlots: {
          ++printed_operand_count;
          *stream << "[";
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          for (int j = 0; j < count; ++j) {
            ASSIGN_OR_RETURN(uint16_t slot_ordinal,
                             ReadValueSlot(data, &offset));
            if (j > 0) *stream << ", ";
            *stream << "&"
                    << "%" << slot_ordinal;
          }
          *stream << "]";
          break;
        }
        case OperandEncoding::kResultSlot: {
          // Printing handled above.
          offset += sizeof(uint16_t);
          break;
        }
        case OperandEncoding::kVariadicResultSlots: {
          // Printing handled above.
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          offset += count * sizeof(uint16_t);
          break;
        }
        case OperandEncoding::kVariadicTransferSlots: {
          ++printed_operand_count;
          *stream << "[";
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          for (int j = 0; j < count; ++j) {
            ASSIGN_OR_RETURN(uint16_t src_slot_ordinal,
                             ReadValueSlot(data, &offset));
            ASSIGN_OR_RETURN(uint16_t dst_slot_ordinal,
                             ReadValueSlot(data, &offset));
            if (j > 0) *stream << ", ";
            *stream << "%" << src_slot_ordinal << "=>%" << dst_slot_ordinal;
          }
          *stream << "]";
          break;
        }
        case OperandEncoding::kConstant: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(auto type, ReadType(data, &offset));
          ASSIGN_OR_RETURN(int rank, ReadCount(data, &offset));
          absl::InlinedVector<int32_t, 4> shape(rank);
          int element_count = 1;
          for (int j = 0; j < rank; ++j) {
            ASSIGN_OR_RETURN(int dim, ReadValue<int32_t>(data, &offset));
            shape[j] = dim;
            element_count *= dim;
          }
          ASSIGN_OR_RETURN(auto encoding,
                           ReadValue<ConstantEncoding>(data, &offset));
          *stream << ConstantEncodingToString(encoding);
          int serialized_element_count = 1;
          switch (encoding) {
            case ConstantEncoding::kDense:
              serialized_element_count = element_count;
              break;
            case ConstantEncoding::kSplat:
              serialized_element_count = 1;
              break;
            default:
              return UnimplementedErrorBuilder(ABSL_LOC)
                     << "Unimplemented constant encoding "
                     << static_cast<int>(encoding);
          }
          *stream << " buffer_view<";
          if (!shape.empty()) {
            *stream << absl::StrJoin(shape, "x") << "x";
          }
          *stream << type << ">{";
          size_t element_size = type.element_size();
          auto bytes = data.subspan(
              offset, std::min(serialized_element_count, 1024) * element_size);
          *stream << ConstantToString(type, bytes);
          if (serialized_element_count > 1024) *stream << "...";
          offset += serialized_element_count * element_size;
          *stream << "}";
          break;
        }
        case OperandEncoding::kFunctionOrdinal: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(auto function_ordinal,
                           ReadValue<uint32_t>(data, &offset));
          ASSIGN_OR_RETURN(auto function,
                           function_table_.LookupFunction(function_ordinal));
          *stream << "@" << function_ordinal << " " << function.name();
          break;
        }
        case OperandEncoding::kDispatchOrdinal: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(auto dispatch_ordinal,
                           ReadValue<uint32_t>(data, &offset));
          ASSIGN_OR_RETURN(auto export_ordinal,
                           ReadValue<uint16_t>(data, &offset));
          // TODO(benvanik): lookup in executable table.
          *stream << "@" << dispatch_ordinal << ":" << export_ordinal;
          break;
        }
        case OperandEncoding::kImportOrdinal: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(auto import_ordinal,
                           ReadValue<uint32_t>(data, &offset));
          ASSIGN_OR_RETURN(auto* function,
                           function_table_.LookupImport(import_ordinal));
          *stream << "@i" << import_ordinal << " ";
          switch (function->link_type()) {
            default:
              *stream << "??";
              break;
            case ImportFunction::LinkType::kNativeFunction:
              *stream << "<native>";
              break;
            case ImportFunction::LinkType::kModule:
              *stream << function->linked_function().module().name() << ":"
                      << function->linked_function().name();
              break;
          }
          break;
        }
        case OperandEncoding::kBlockOffset: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(uint32_t block_offset,
                           ReadValue<uint32_t>(data, &offset));
          *stream << ":" << block_offset;
          break;
        }
        case OperandEncoding::kTypeIndex: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(auto type, ReadType(data, &offset));
          *stream << type;
          break;
        }
        case OperandEncoding::kIndex: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(auto index, ReadValue<int32_t>(data, &offset));
          *stream << "#" << index;
          break;
        }
        case OperandEncoding::kIndexList: {
          ++printed_operand_count;
          *stream << "{";
          ASSIGN_OR_RETURN(int count, ReadCount(data, &offset));
          for (int j = 0; j < count; ++j) {
            ASSIGN_OR_RETURN(auto dim, ReadValue<int32_t>(data, &offset));
            if (j > 0) *stream << ",";
            *stream << dim;
          }
          *stream << "}";
          break;
        }
        case OperandEncoding::kCmpIPredicate: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(auto predicate_value,
                           ReadValue<uint8_t>(data, &offset));
          *stream << "<"
                  << PredicateToString(
                         static_cast<CmpIPredicate>(predicate_value))
                  << ">";
          break;
        }
        case OperandEncoding::kCmpFPredicate: {
          ++printed_operand_count;
          ASSIGN_OR_RETURN(auto predicate_value,
                           ReadValue<uint8_t>(data, &offset));
          *stream << "<"
                  << PredicateToString(
                         static_cast<CmpFPredicate>(predicate_value))
                  << ">";
          break;
        }
      }
    }

    *stream << "\n";
  }

  return OkStatus();
}

}  // namespace vm
}  // namespace iree
