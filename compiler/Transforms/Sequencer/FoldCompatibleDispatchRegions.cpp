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

#include "third_party/llvm/llvm/include/llvm/ADT/ArrayRef.h"
#include "third_party/llvm/llvm/include/llvm/ADT/DenseMap.h"
#include "third_party/llvm/llvm/include/llvm/ADT/DenseSet.h"
#include "third_party/llvm/llvm/include/llvm/ADT/SetVector.h"
#include "third_party/llvm/llvm/include/llvm/ADT/SmallVector.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/Dialect/StandardOps/Ops.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/Attributes.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/BlockAndValueMapping.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/Builders.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/Location.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/MLIRContext.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/StandardTypes.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/Pass/Pass.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/Pass/PassRegistry.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/Support/LLVM.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/Support/LogicalResult.h"
#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/Transforms/Utils.h"
#include "third_party/mlir_edge/iree/compiler/IR/Ops.h"
#include "third_party/mlir_edge/iree/compiler/Utils/DispatchUtils.h"

namespace mlir {
namespace iree_compiler {

// Identifies dispatch regions that have compatible workloads and folds them.
// This relies on CSE having deduped workloads to simplify the logic to simply
// looking for dispatch regions using the same values.
class FoldCompatibleDispatchRegionsPass
    : public FunctionPass<FoldCompatibleDispatchRegionsPass> {
 public:
  void runOnFunction() override {
    auto func = getFunction();
    for (auto &block : func) {
      if (failed(mergeBlockDispatchRegions(func, &block))) {
        return signalPassFailure();
      }
    }
  }
};

std::unique_ptr<OpPassBase<FuncOp>> createFoldCompatibleDispatchRegionsPass() {
  return std::make_unique<FoldCompatibleDispatchRegionsPass>();
}

static PassRegistration<FoldCompatibleDispatchRegionsPass> pass(
    "iree-fold-compatible-dispatch-regions",
    "Folds dispatch regions that have compatible workloads.");

}  // namespace iree_compiler
}  // namespace mlir
