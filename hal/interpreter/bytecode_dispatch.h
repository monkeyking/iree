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

#ifndef THIRD_PARTY_MLIR_EDGE_IREE_HAL_INTERPRETER_BYTECODE_DISPATCH_H_
#define THIRD_PARTY_MLIR_EDGE_IREE_HAL_INTERPRETER_BYTECODE_DISPATCH_H_

#include "third_party/mlir_edge/iree/base/status.h"
#include "third_party/mlir_edge/iree/hal/allocator.h"
#include "third_party/mlir_edge/iree/hal/interpreter/bytecode_kernels.h"
#include "third_party/mlir_edge/iree/vm/stack.h"
#include "third_party/mlir_edge/iree/vm/stack_frame.h"

namespace iree {
namespace hal {

Status Dispatch(hal::Allocator* allocator,
                kernels::RuntimeState* kernel_runtime_state, vm::Stack* stack,
                vm::StackFrame* entry_stack_frame,
                absl::Span<BufferView> entry_results);

}  // namespace hal
}  // namespace iree

#endif  // THIRD_PARTY_MLIR_EDGE_IREE_HAL_INTERPRETER_BYTECODE_DISPATCH_H_
