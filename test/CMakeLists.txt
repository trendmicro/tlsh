# Copyright 2013 Trend Micro Incorporated
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_executable(tlsh_version tlsh_version.cpp)
target_link_libraries(tlsh_version tlsh)
set_target_properties(tlsh_version PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(tlsh_version PROPERTIES OUTPUT_NAME tlsh_version${BUILD_POSTFIX})

add_executable(simple_unittest simple_unittest.cpp)
target_link_libraries(simple_unittest tlsh)
set_target_properties(simple_unittest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(simple_unittest PROPERTIES OUTPUT_NAME simple_unittest${BUILD_POSTFIX})

add_executable(tlsh_unittest tlsh_unittest.cpp)
target_link_libraries(tlsh_unittest tlsh)
set_target_properties(tlsh_unittest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(tlsh_unittest PROPERTIES OUTPUT_NAME tlsh_unittest${BUILD_POSTFIX})
