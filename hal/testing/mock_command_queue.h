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

#ifndef THIRD_PARTY_MLIR_EDGE_IREE_HAL_TESTING_MOCK_COMMAND_QUEUE_H_
#define THIRD_PARTY_MLIR_EDGE_IREE_HAL_TESTING_MOCK_COMMAND_QUEUE_H_

#include "testing/base/public/gmock.h"
#include "third_party/mlir_edge/iree/hal/command_queue.h"

namespace iree {
namespace hal {
namespace testing {

class MockCommandQueue : public ::testing::StrictMock<CommandQueue> {
 public:
  MockCommandQueue(std::string name,
                   CommandCategoryBitfield supported_categories)
      : ::testing::StrictMock<CommandQueue>(std::move(name),
                                            supported_categories) {}

  MOCK_METHOD2(Submit, Status(absl::Span<const SubmissionBatch> batches,
                              FenceValue fence));

  MOCK_METHOD0(Flush, Status());

  MOCK_METHOD1(WaitIdle, Status(absl::Time deadline));
};

}  // namespace testing
}  // namespace hal
}  // namespace iree

#endif  // THIRD_PARTY_MLIR_EDGE_IREE_HAL_TESTING_MOCK_COMMAND_QUEUE_H_
