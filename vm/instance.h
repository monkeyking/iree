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

#ifndef THIRD_PARTY_MLIR_EDGE_IREE_VM_INSTANCE_H_
#define THIRD_PARTY_MLIR_EDGE_IREE_VM_INSTANCE_H_

#include <memory>

#include "third_party/mlir_edge/iree/hal/device_manager.h"
#include "third_party/mlir_edge/iree/vm/debug/debug_server.h"

namespace iree {
namespace vm {

// Shared runtime instance responsible for routing Context events, enumerating
// and creating hardware device interfaces, and managing thread pools.
//
// A single runtime instance can service multiple contexts and hosting
// applications should try to reuse a runtime as much as possible. This ensures
// that resource allocation across contexts is handled and extraneous device
// interaction is avoided.
class Instance {
 public:
  // Allocates a global unique ID.
  static int NextUniqueId();

  explicit Instance(std::unique_ptr<debug::DebugServer> debug_server = nullptr);
  ~Instance();
  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&) = delete;

  debug::DebugServer* debug_server() const { return debug_server_.get(); }

  hal::DeviceManager* device_manager() const { return device_manager_.get(); }

 private:
  std::unique_ptr<debug::DebugServer> debug_server_;
  std::unique_ptr<hal::DeviceManager> device_manager_;
};

}  // namespace vm
}  // namespace iree

#endif  // THIRD_PARTY_MLIR_EDGE_IREE_VM_INSTANCE_H_
