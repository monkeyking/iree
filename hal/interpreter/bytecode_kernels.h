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

// Defines kernel functions and provides their implementation via one (or more)
// included files.
//
// Kernels should do the simplest possible operation. Buffer validation is
// handled by the dispatch logic and need not be checked. Kernels may optionally
// accept arguments beyond just the buffers, depending on the required state
// and attributes.
//
// Kernels may optionally have runtime state. This is state that is allocated
// once for the entire Runtime (and stored on RuntimeState) and shared across
// all fibers. This enables kernels that may require thread pools or device
// handles to be shared while kernels that require transient storage to be safe
// to use from multiple fibers concurrently.
//
// All kernels are templated to enable specialization of particular types or
// type combinations. By default the bytecode_kernels_generic.h will provide C++
// semantics as reference and platform-specific versions can be implemented
// as needed.

#ifndef THIRD_PARTY_MLIR_EDGE_IREE_HAL_INTERPRETER_BYTECODE_KERNELS_H_
#define THIRD_PARTY_MLIR_EDGE_IREE_HAL_INTERPRETER_BYTECODE_KERNELS_H_

#include <cstdint>

#include "third_party/absl/types/span.h"
#include "third_party/mlir_edge/iree/base/shape.h"
#include "third_party/mlir_edge/iree/base/status.h"

namespace iree {
namespace hal {
namespace kernels {

struct CompareEQ {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<uint8_t> dst_buffer);
};
struct CompareNE {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<uint8_t> dst_buffer);
};
struct CompareLT {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<uint8_t> dst_buffer);
};
struct CompareLE {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<uint8_t> dst_buffer);
};
struct CompareGT {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<uint8_t> dst_buffer);
};
struct CompareGE {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<uint8_t> dst_buffer);
};

struct Copy {
  template <int element_size>
  static Status Execute(absl::Span<const uint8_t> src_buffer,
                        const Shape& src_shape,
                        absl::Span<const int32_t> src_indices,
                        absl::Span<uint8_t> dst_buffer, const Shape& dst_shape,
                        absl::Span<const int32_t> dst_indices,
                        absl::Span<const int32_t> lengths);
};

struct Select {
  template <typename T>
  static Status Execute(absl::Span<const uint8_t> cond_buffer,
                        absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Transpose {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer, const Shape& src_shape,
                        absl::Span<const int32_t> perm);
};

struct Pad {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<const T> padding_value,
                        absl::Span<T> dst_buffer, const Shape& src_shape,
                        const Shape& dst_shape,
                        absl::Span<const int32_t> edge_padding_low,
                        absl::Span<const int32_t> edge_padding_high,
                        absl::Span<const int32_t> interior_padding);
};

struct Reverse {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer, const Shape& src_shape,
                        absl::Span<const int32_t> dimensions);
};

struct Broadcast {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Tile {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer, const Shape& src_shape,
                        const Shape& dst_shape);
};

struct Not {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct And {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Or {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Xor {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct ShiftLeft {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct ShiftRight {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Add {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Sub {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Abs {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Mul {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Div {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

// a + (b * c)
struct MulAdd {
  template <typename T>
  static Status Execute(absl::Span<const T> a_buffer,
                        absl::Span<const T> b_buffer,
                        absl::Span<const T> c_buffer, absl::Span<T> dst_buffer);
};

struct Exp {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Log {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Rsqrt {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Cos {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Sin {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Tanh {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Atan2 {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Min {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Max {
  template <typename T>
  static Status Execute(absl::Span<const T> lhs_buffer,
                        absl::Span<const T> rhs_buffer,
                        absl::Span<T> dst_buffer);
};

struct Clamp {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<const T> min_buffer,
                        absl::Span<const T> max_buffer,
                        absl::Span<T> dst_buffer);
};

struct Floor {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Ceil {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<T> dst_buffer);
};

struct Convert {
  template <typename SRC, typename DST>
  static Status Execute(absl::Span<const SRC> src_buffer,
                        absl::Span<DST> dst_buffer);
};

struct MatMul {
  struct RuntimeState;

  static std::unique_ptr<RuntimeState> CreateRuntimeState();

  template <typename T, typename ACC>
  struct Buffers {
    Shape lhs_shape;
    absl::Span<const T> lhs_buffer;
    Shape rhs_shape;
    absl::Span<const T> rhs_buffer;
    Shape dst_shape;
    absl::Span<T> dst_buffer;

    // Optional bias buffer.
    absl::Span<const ACC> bias_buffer;

    // Fixed-point multiplier mantissa/exponent. May be a single value (for
    // uniform quantization) or one element per row of the destination matrix
    // for per-channel.
    absl::Span<const ACC> multiplier_mantissa_buffer;
    absl::Span<const int32_t> multiplier_exponent_buffer;
  };

  template <typename T, typename ACC>
  static Status Execute(RuntimeState* runtime_state,
                        const Buffers<T, ACC>& buffers);
};

struct RuntimeState {
  std::unique_ptr<MatMul::RuntimeState> mat_mul_state =
      MatMul::CreateRuntimeState();
};

struct ReduceSum {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<const T> init_buffer,
                        absl::Span<T> dst_buffer, int32_t dimension,
                        const Shape& src_shape, const Shape& dst_shape);
};

struct ReduceMin {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<const T> init_buffer,
                        absl::Span<T> dst_buffer, int32_t dimension,
                        const Shape& src_shape, const Shape& dst_shape);
};

struct ReduceMax {
  template <typename T>
  static Status Execute(absl::Span<const T> src_buffer,
                        absl::Span<const T> init_buffer,
                        absl::Span<T> dst_buffer, int32_t dimension,
                        const Shape& src_shape, const Shape& dst_shape);
};

}  // namespace kernels
}  // namespace hal
}  // namespace iree

#include "third_party/mlir_edge/iree/hal/interpreter/bytecode_kernels_generic.h"  // IWYU pragma: export
#include "third_party/mlir_edge/iree/hal/interpreter/bytecode_kernels_ruy.h"  // IWYU pragma: export

#endif  // THIRD_PARTY_MLIR_EDGE_IREE_HAL_INTERPRETER_BYTECODE_KERNELS_H_
