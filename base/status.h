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

#ifndef THIRD_PARTY_MLIR_EDGE_IREE_BASE_STATUS_H_
#define THIRD_PARTY_MLIR_EDGE_IREE_BASE_STATUS_H_

#ifdef IREE_CONFIG_GOOGLE_INTERNAL
#include "third_party/mlir_edge/iree/base/google/status_google.h"
#else
#include "third_party/mlir_edge/iree/base/internal/status.h"
#include "third_party/mlir_edge/iree/base/internal/status_builder.h"
#include "third_party/mlir_edge/iree/base/internal/status_errno.h"
#include "third_party/mlir_edge/iree/base/internal/status_errors.h"
#include "third_party/mlir_edge/iree/base/internal/status_macros.h"
#include "third_party/mlir_edge/iree/base/internal/statusor.h"
#endif  // IREE_CONFIG_GOOGLE_INTERNAL

#endif  // THIRD_PARTY_MLIR_EDGE_IREE_BASE_STATUS_H_
