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

#ifndef THIRD_PARTY_MLIR_EDGE_IREE_HAL_DEVICE_MANAGER_H_
#define THIRD_PARTY_MLIR_EDGE_IREE_HAL_DEVICE_MANAGER_H_

#include <vector>

#include "third_party/absl/synchronization/mutex.h"
#include "third_party/absl/types/span.h"
#include "third_party/mlir_edge/iree/base/status.h"
#include "third_party/mlir_edge/iree/hal/allocator.h"
#include "third_party/mlir_edge/iree/hal/buffer.h"
#include "third_party/mlir_edge/iree/hal/device.h"
#include "third_party/mlir_edge/iree/hal/device_placement.h"
#include "third_party/mlir_edge/iree/hal/executable_format.h"

namespace iree {
namespace hal {

// Specifies how devices should be resolved to DevicePlacements.
// Most fields are optional and when not included will be ignored.
struct PlacementSpec {
  // TODO(benvanik): other requirements (features/caps, power, etc).

  // A list of executable formats that the placement should support.
  // If more than one format is provided any device satisfying at least one
  // will be considered for placement. The formats can be sorted in descending
  // priority order to prefer the first available format in the case of ties.
  absl::Span<const ExecutableFormat> available_formats;
};

// Manages device lifetime and placement resolution.
//
// Thread-safe. Note that callers must ensure that unregistered devices are kept
// alive for as long as any commands are in-flight that may be using them.
class DeviceManager final {
 public:
  DeviceManager();
  ~DeviceManager();

  // Registers a device with the manager.
  // The device will be used to resolve placements. Any placements resolved
  // prior to the addition of the device will need to be refreshed by the caller
  // if they want to make use of the new device.
  Status RegisterDevice(std::shared_ptr<Device> device);

  // Unregisters a device with the manager.
  // Placements that resolved to the device prior to unregistering will remain
  // valid for that device. Callers will need to refresh the placements to
  // ensure the device stops being used.
  Status UnregisterDevice(Device* device);

  // TODO(benvanik): dispatch info + requirements + etc -> DevicePlacement.

  // Resolves a placement spec to a device placement based on the registered
  // devices.
  // If the placement is not fully specified the device and queue may be chosen
  // at random. See PlacementSpec for more information about resolution and
  // ranking.
  StatusOr<DevicePlacement> ResolvePlacement(
      const PlacementSpec& placement_spec) const;

  // Finds an allocator that can allocate buffers of the given |memory_type| and
  // |buffer_usage| such that the buffers can be used interchangebly.
  // Fails if there is no Allocator that can satisfy that requirement.
  StatusOr<Allocator*> FindCompatibleAllocator(
      MemoryTypeBitfield memory_type, BufferUsageBitfield buffer_usage,
      absl::Span<const DevicePlacement> device_placements) const;

  // Tries to allocate a host-local buffer that _may_ be optimal for use with
  // the given |device_placements| and _may_ be device-visible. The buffer can
  // be used for staging uploads to device-local buffers and is useful for times
  // when the buffer will be used more on the host than the device. If a buffer
  // never needs to be used with a device prefer instead
  // Allocator::host_local()::Allocate.
  //
  // Returns a buffer even if it's not possible to satisfy the requested
  // |buffer_usage| for the |device_placements| at the cost of a run-time
  // performance hit.
  StatusOr<ref_ptr<Buffer>> TryAllocateDeviceVisibleBuffer(
      MemoryTypeBitfield memory_type, BufferUsageBitfield buffer_usage,
      device_size_t allocation_size,
      absl::Span<const DevicePlacement> device_placements);
  StatusOr<ref_ptr<Buffer>> TryAllocateDeviceVisibleBuffer(
      BufferUsageBitfield buffer_usage, device_size_t allocation_size,
      absl::Span<const DevicePlacement> device_placements) {
    return TryAllocateDeviceVisibleBuffer(
        MemoryType::kHostLocal | MemoryType::kDeviceVisible, buffer_usage,
        allocation_size, device_placements);
  }

  // Allocates a host-local buffer that is optimal for use on the host but is
  // usable by the given |device_placements| (at a possible performance
  // penalty). The buffer can be used for staging uploads to device-local
  // buffers and is useful for times when the buffer will be used more on the
  // host than the device. If a buffer never needs to be used with a device
  // prefer instead HeapBuffer::Allocate.
  //
  // Fails if it is not possible to allocate and satisfy all |device_placements|
  // for the requested |buffer_usage|.
  StatusOr<ref_ptr<Buffer>> AllocateDeviceVisibleBuffer(
      MemoryTypeBitfield memory_type, BufferUsageBitfield buffer_usage,
      device_size_t allocation_size,
      absl::Span<const DevicePlacement> device_placements);
  StatusOr<ref_ptr<Buffer>> AllocateDeviceVisibleBuffer(
      BufferUsageBitfield buffer_usage, device_size_t allocation_size,
      absl::Span<const DevicePlacement> device_placements) {
    return AllocateDeviceVisibleBuffer(
        MemoryType::kHostLocal | MemoryType::kDeviceVisible, buffer_usage,
        allocation_size, device_placements);
  }

  // Allocates a device-local buffer that is optimal for use with the given
  // |device_placements|. The buffer will not be host-visible and can only be
  // used from compatible device queues.
  //
  // Fails if it is not possible to allocate and satisfy all |device_placements|
  // for the requested |buffer_usage|.
  StatusOr<ref_ptr<Buffer>> AllocateDeviceLocalBuffer(
      MemoryTypeBitfield memory_type, BufferUsageBitfield buffer_usage,
      device_size_t allocation_size,
      absl::Span<const DevicePlacement> device_placements);
  StatusOr<ref_ptr<Buffer>> AllocateDeviceLocalBuffer(
      BufferUsageBitfield buffer_usage, device_size_t allocation_size,
      absl::Span<const DevicePlacement> device_placements) {
    return AllocateDeviceLocalBuffer(MemoryType::kDeviceLocal, buffer_usage,
                                     allocation_size, device_placements);
  }

 private:
  mutable absl::Mutex device_mutex_;
  std::vector<std::shared_ptr<Device>> devices_ ABSL_GUARDED_BY(device_mutex_);
};

}  // namespace hal
}  // namespace iree

#endif  // THIRD_PARTY_MLIR_EDGE_IREE_HAL_DEVICE_MANAGER_H_
