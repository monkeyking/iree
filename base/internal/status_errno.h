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

#ifndef THIRD_PARTY_MLIR_EDGE_IREE_BASE_INTERNAL_STATUS_ERRNO_H_
#define THIRD_PARTY_MLIR_EDGE_IREE_BASE_INTERNAL_STATUS_ERRNO_H_

#include "third_party/absl/strings/string_view.h"
#include "third_party/absl/types/source_location.h"
#include "third_party/mlir_edge/iree/base/internal/status.h"
#include "third_party/mlir_edge/iree/base/internal/statusor.h"

namespace iree {

// Returns the code for `error_number`, which should be an `errno` value.
// See https://en.cppreference.com/w/cpp/error/errno_macros and similar refs.
StatusCode ErrnoToCanonicalCode(int error_number);

// Returns a Status, using a code of `ErrnoToCode(error_number)`, and a
// `message` with the result of `StrError(error_number)` appended.
Status ErrnoToCanonicalStatus(int error_number, absl::string_view message);

// Returns a StatusBuilder using a status of
// `ErrnoToCanonicalStatus(error_number, message)` and `location`.
StatusBuilder ErrnoToCanonicalStatusBuilder(int error_number,
                                            absl::string_view message,
                                            absl::SourceLocation location);

}  // namespace iree

#endif  // THIRD_PARTY_MLIR_EDGE_IREE_BASE_INTERNAL_STATUS_ERRNO_H_
