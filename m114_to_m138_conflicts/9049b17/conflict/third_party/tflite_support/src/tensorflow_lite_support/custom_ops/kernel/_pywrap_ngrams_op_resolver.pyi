# Copyright 2023 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

<<<<<<< HEAD:third_party/tflite_support/src/tensorflow_lite_support/custom_ops/kernel/_pywrap_ngrams_op_resolver.pyi
def AddNgramsCustomOp(arg0: int) -> None: ...
=======
import("//starboard/content/ssl/certs.gni")

copy("copy_ssl_certificates") {
  if (enable_install_targets) {
    install_content = true
  }
  sources = network_certs
  outputs =
      [ "$sb_static_contents_output_data_dir/ssl/certs/{{source_file_part}}" ]
}
>>>>>>> 9049b1718b7 (Revert "Revert "Build starboard shared library for linux-x64x11 (#4215)"" (#4247)):starboard/content/ssl/BUILD.gn
