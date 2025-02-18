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

//===- IREEIndexComputation.h ----------------------------------*- C++//-*-===//
//
// Index Propagation for IREE statements that are used in dispatch functions.
//
//===----------------------------------------------------------------------===//
#ifndef IREE_COMPILER_TRANSLATION_SPIRV_H
#define IREE_COMPILER_TRANSLATION_SPIRV_H

#include "third_party/llvm/llvm/projects/google_mlir/include/mlir/IR/Function.h"
#include "third_party/mlir_edge/iree/compiler/IR/Ops.h"
#include "third_party/mlir_edge/iree/compiler/IR/StructureOps.h"
#include "third_party/mlir_edge/iree/compiler/Translation/SPIRV/XLAIndexPropagation.h"

namespace mlir {
namespace iree_compiler {

/// Gets the launch size associated with the dispatch function that this op is
/// part of.
inline LogicalResult getLaunchSize(Operation *op,
                                   SmallVectorImpl<int64_t> &launchSize) {
  auto funcOp = op->getParentOfType<FuncOp>();
  if (!funcOp || !funcOp.getAttr("iree.executable.export")) {
    return op->emitError(
        "expected operation to be in dispatch function to get launch size");
  }
  auto workloadAttr =
      funcOp.getAttrOfType<DenseElementsAttr>("iree.executable.workload");
  if (!workloadAttr) {
    op->emitError(
        "unable to find workload size, missing attribute "
        "iree.executable.workload in dispatch function");
  }
  launchSize.clear();
  for (auto value : workloadAttr.getValues<APInt>()) {
    launchSize.push_back(value.getSExtValue());
  }
  // Drop trailing ones.
  auto dropFrom = launchSize.size() - 1;
  while (dropFrom > 0 && launchSize[dropFrom] == 1) {
    --dropFrom;
  }
  if (dropFrom > 0) {
    launchSize.erase(std::next(launchSize.begin(), dropFrom + 1),
                     launchSize.end());
  }
  return success();
}

/// Index propagation for iree.load_input operation. This operation is
/// essentially a copy from a memref to a tensor. So just copy the index map to
/// the memref operand from the result tensor.
class IREELoadIndexPropagation final
    : public IndexPropagationOp<IREE::LoadInputOp> {
 public:
  using IndexPropagationOp<IREE::LoadInputOp>::IndexPropagationOp;

  LogicalResult propagateIndexMap(
      Operation *operation, IndexComputationCache &indexMap) const override;
};

/// Index propagation for iree.store_output operation. The launch size is
/// assumed to match the shape of the tensor that is being stored. This
/// operation acts as a seed for the index propogation. Each workitem is assumed
/// to compute a single element of this tensor. The range of the index map is
/// the reverse of the launch dimension.
class IREEStoreIndexPropagation final
    : public IndexPropagationOp<IREE::StoreOutputOp> {
 public:
  using IndexPropagationOp<IREE::StoreOutputOp>::IndexPropagationOp;

  LogicalResult propagateIndexMap(
      Operation *operation, IndexComputationCache &indexMap) const override;
};

}  // namespace iree_compiler
}  // namespace mlir

#endif  // IREE_COMPILER_TRANSLATION_SPIRV_H
