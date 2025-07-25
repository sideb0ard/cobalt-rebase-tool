//
//
// Copyright 2021 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//

#ifndef GRPCPP_EXT_ADMIN_SERVICES_H
#define GRPCPP_EXT_ADMIN_SERVICES_H

<<<<<<< HEAD:third_party/grpc/source/include/grpcpp/ext/admin_services.h
#include <grpcpp/server_builder.h>
=======
#include <atomic>
>>>>>>> c7bc400a5e0 (Migrate C++ code from starboard/atomic.h to std::atomic (#4199)):starboard/loader_app/pending_restart.cc

namespace grpc {

<<<<<<< HEAD:third_party/grpc/source/include/grpcpp/ext/admin_services.h
// Registers admin services to the given ServerBuilder. This function will add
// admin services based on build time dependencies, for example, it only adds
// CSDS service if xDS is enabled in this binary.
void AddAdminServices(grpc::ServerBuilder* builder);

}  // namespace grpc

#endif  // GRPCPP_EXT_ADMIN_SERVICES_H
=======
namespace {
std::atomic<int32_t> g_pending_restart{0};
}  // namespace

bool IsPendingRestart() {
  return g_pending_restart.load(std::memory_order_relaxed) == 1;
}

void SetPendingRestart(bool value) {
  g_pending_restart.store(value ? 1 : 0, std::memory_order_relaxed);
}

}  // namespace loader_app
}  // namespace starboard
>>>>>>> c7bc400a5e0 (Migrate C++ code from starboard/atomic.h to std::atomic (#4199)):starboard/loader_app/pending_restart.cc
