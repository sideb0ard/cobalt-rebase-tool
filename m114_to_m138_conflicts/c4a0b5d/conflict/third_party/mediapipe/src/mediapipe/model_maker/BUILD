# Copyright 2022 The MediaPipe Authors.
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

package(default_visibility = ["//visibility:public"])

<<<<<<< HEAD:third_party/mediapipe/src/mediapipe/model_maker/BUILD
package_group(
    name = "internal",
    packages = [
        "//mediapipe/model_maker/...",
    ],
)
=======
copy("copy_ssl_certificates") {
  if (enable_install_targets) {
    install_content = true
  }
  sources = network_certs
  outputs =
      [ "$sb_static_contents_output_data_dir/ssl/certs/{{source_file_part}}" ]
}
>>>>>>> c4a0b5d0a8c (Build starboard shared library for linux-x64x11 (#4215)):starboard/content/ssl/BUILD.gn
