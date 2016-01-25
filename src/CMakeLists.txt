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

set(TLSH_SRCS tlsh.cpp tlsh_impl.cpp tlsh_util.cpp)
add_library(tlsh STATIC ${TLSH_SRCS})
set_target_properties(tlsh PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set_target_properties(tlsh PROPERTIES OUTPUT_NAME tlsh${BUILD_POSTFIX})
add_library(tlsh_shared SHARED ${TLSH_SRCS})
set_target_properties(tlsh_shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set_target_properties(tlsh_shared PROPERTIES OUTPUT_NAME tlsh${BUILD_POSTFIX})
set_target_properties(tlsh_shared PROPERTIES VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
                                             SOVERSION "0")
