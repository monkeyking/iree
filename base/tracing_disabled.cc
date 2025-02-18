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

// This file is linked in only when WTF is not enabled. It allows us to keep the
// same flags and functions without needing to do a bunch of ifdef hackery or
// undefok mangling.

#include <cstdint>
#include <string>

#include "third_party/absl/flags/flag.h"
#include "third_party/mlir_edge/iree/base/logging.h"
#include "third_party/mlir_edge/iree/base/tracing.h"

ABSL_FLAG(int32_t, iree_trace_file_period, 0,
          "Flag for tracing. Use --define=GLOBAL_WTF_ENABLE=1 to enable WTF.");
ABSL_FLAG(std::string, iree_trace_file, "",
          "Flag for tracing. Use --define=GLOBAL_WTF_ENABLE=1 to enable WTF.");

namespace iree {

void InitializeTracing() {
  if (!absl::GetFlag(FLAGS_iree_trace_file).empty()) {
    LOG(WARNING) << "WTF trace save requested but WTF is not compiled in. "
                 << "Enable by building with --define=GLOBAL_WTF_ENABLE=1.";
  }
}

void FlushTrace() {}

}  // namespace iree
