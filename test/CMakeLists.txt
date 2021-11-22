# TLSH is provided for use under two licenses: Apache OR BSD.
# Users may opt to use either license depending on the license
# restictions of the systems with which they plan to integrate
# the TLSH code.
#
# ==============
# Apache License
# ==============
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
#
# ===========
# BSD License
# ===========
# Copyright (c) 2013, Trend Micro Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.

# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.

add_executable(simple_unittest simple_unittest.cpp)
target_link_libraries(simple_unittest PRIVATE tlsh_static)
target_link_libraries(simple_unittest PRIVATE)
set_target_properties(simple_unittest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(simple_unittest PROPERTIES OUTPUT_NAME simple_unittest${BUILD_POSTFIX})

add_executable(tlsh_unittest tlsh_unittest.cpp)
target_link_libraries(tlsh_unittest PRIVATE tlsh_static)
target_link_libraries(tlsh_unittest PRIVATE)
set_target_properties(tlsh_unittest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(tlsh_unittest PROPERTIES OUTPUT_NAME tlsh_unittest${BUILD_POSTFIX})
set_target_properties(tlsh_unittest PROPERTIES SKIP_BUILD_RPATH TRUE)

add_executable(timing_unittest timing_unittest.cpp)
target_link_libraries(timing_unittest PRIVATE tlsh_static)
target_link_libraries(timing_unittest PRIVATE)
set_target_properties(timing_unittest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(timing_unittest PROPERTIES OUTPUT_NAME timing_unittest${BUILD_POSTFIX})
set_target_properties(timing_unittest PROPERTIES SKIP_BUILD_RPATH TRUE)

add_executable(order_bug order_bug.cpp)
target_link_libraries(order_bug PRIVATE tlsh_static)
target_link_libraries(order_bug PRIVATE)
set_target_properties(order_bug PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(order_bug PROPERTIES OUTPUT_NAME order_bug${BUILD_POSTFIX})
set_target_properties(order_bug PROPERTIES SKIP_BUILD_RPATH TRUE)

add_executable(tlsh_pattern tlsh_pattern.cpp)
target_link_libraries(tlsh_pattern PRIVATE tlsh_static)
target_link_libraries(tlsh_pattern PRIVATE)
set_target_properties(tlsh_pattern PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(tlsh_pattern PROPERTIES OUTPUT_NAME tlsh_pattern${BUILD_POSTFIX})
set_target_properties(tlsh_pattern PROPERTIES SKIP_BUILD_RPATH TRUE)

add_executable(tlsh_parts tlsh_parts.cpp)
target_link_libraries(tlsh_parts PRIVATE tlsh_static)
target_link_libraries(tlsh_parts PRIVATE)
set_target_properties(tlsh_parts PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(tlsh_parts PROPERTIES OUTPUT_NAME tlsh_parts${BUILD_POSTFIX})
set_target_properties(tlsh_parts PROPERTIES SKIP_BUILD_RPATH TRUE)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# issue #115 - CREATE_LINK does not work on CENTOS 7
# file(CREATE_LINK tlsh_unittest ${CMAKE_SOURCE_DIR}/bin/tlsh SYMBOLIC)

install(TARGETS tlsh_unittest DESTINATION bin)
install(CODE "file(CREATE_LINK tlsh_unittest \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/tlsh SYMBOLIC)")
