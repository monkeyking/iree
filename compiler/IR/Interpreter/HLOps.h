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

#ifndef THIRD_PARTY_MLIR_EDGE_IREE_COMPILER_IR_INTERPRETER_HLOPS_H_
#define THIRD_PARTY_MLIR_EDGE_IREE_COMPILER_IR_INTERPRETER_HLOPS_H_

#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/Dialect/StandardOps/Ops.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/Attributes.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/Dialect.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/OpDefinition.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/StandardTypes.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/TypeUtilities.h"

namespace mlir {
namespace iree_compiler {
namespace IREEInterp {
namespace HL {

#define GET_OP_CLASSES
#include "third_party/mlir_edge/iree/compiler/IR/Interpreter/HLOps.h.inc"

}  // namespace HL
}  // namespace IREEInterp
}  // namespace iree_compiler
}  // namespace mlir

#endif  // THIRD_PARTY_MLIR_EDGE_IREE_COMPILER_IR_INTERPRETER_HLOPS_H_
