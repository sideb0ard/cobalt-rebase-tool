//
//
// Copyright 2015 gRPC authors.
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
#include <grpc/grpc.h>
#include <grpcpp/security/auth_context.h>

#include <memory>

<<<<<<< HEAD:third_party/grpc/source/src/cpp/common/insecure_create_auth_context.cc
namespace grpc {

std::shared_ptr<const AuthContext> CreateAuthContext(grpc_call* call) {
  (void)call;
  return std::shared_ptr<const AuthContext>();
=======
// #include "starboard/android/shared/application_android.h"

SbWindow SbWindowCreate(const SbWindowOptions* options) {
  // return starboard::android::shared::ApplicationAndroid::Get()->CreateWindow(
  //     options);
>>>>>>> f28ac7d8996 (`cobalt_apk` to build dev.cobalt.coat (#4304)):starboard/android/shared/window_create.cc
}

}  // namespace grpc
