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

<<<<<<< HEAD:third_party/tflite_support/src/tensorflow_lite_support/custom_ops/kernel/_pywrap_whitespace_tokenizer_op_resolver.pyi
def AddWhitespaceTokenizerCustomOp(arg0: int) -> None: ...
=======
declare_args() {
  starboard_path = ""
}

if (starboard_path == "") {
  declare_args() {
    target_platform = "linux-x64x11"
  }

  starboard_path = exec_script("//starboard/build/platforms.py",
                               [ target_platform ],
                               "trim string")
}

assert(defined(starboard_path),
       "Please add your platform to starboard/build/platforms.py")
>>>>>>> 9049b1718b7 (Revert "Revert "Build starboard shared library for linux-x64x11 (#4215)"" (#4247)):starboard/build/platform_path.gni
