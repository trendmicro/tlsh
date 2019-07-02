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

cmake_minimum_required(VERSION 2.8)

project(TLSH)

set(VERSION_MAJOR 3)
set(VERSION_MINOR 13)
set(VERSION_PATCH 1)

# Standard shared library flag is supported, enable to build a shared library
#set(BUILD_SHARED_LIBS OFF)

# Edit those values or call cmake with options defined below
set(TLSH_BUCKETS_DEFAULT "TLSH_BUCKETS_128")
set(TLSH_CHECKSUM_DEFAULT "TLSH_CHECKSUM_1B")

# TLSH uses only half the counting buckets.
# It can use all the buckets now.
option(TLSH_BUCKETS_48 "Use 48 bits buckets" OFF)
option(TLSH_BUCKETS_128 "Use 128 bits buckets" OFF)
option(TLSH_BUCKETS_256 "Use 256 bits buckets" OFF)

if(NOT TLSH_BUCKETS_48 AND NOT TLSH_BUCKETS_128 AND NOT TLSH_BUCKETS_256)
  set(${TLSH_BUCKETS_DEFAULT} ON)
endif()

if(TLSH_BUCKETS_48)
  set(TLSH_HASH "min hash")
  set(TLSH_BUCKETS_SIZE "BUCKETS_48")
elseif(TLSH_BUCKETS_128)
  set(TLSH_HASH "compact hash")
  set(TLSH_BUCKETS_SIZE "BUCKETS_128")
elseif(TLSH_BUCKETS_256)
  set(TLSH_HASH "full hash")
  set(TLSH_BUCKETS_SIZE "BUCKETS_256")
endif()

message(STATUS "Hash type: ${TLSH_HASH}")

option(TLSH_CHECKSUM_0B "Do not use checksum" OFF)
option(TLSH_CHECKSUM_1B "Use 1 byte checksum (collision rate: 1/24)" OFF)
option(TLSH_CHECKSUM_3B "Use 3 bytes checksum (collision rate: 1/5000)" OFF)

if(NOT TLSH_CHECKSUM_0B AND NOT TLSH_CHECKSUM_1B AND NOT TLSH_CHECKSUM_3B)
  set(${TLSH_CHECKSUM_DEFAULT} ON)
endif()

if(TLSH_CHECKSUM_0B)
  set(TLSH_CHECKSUM "no checksum")
  set(TLSH_CHECKSUM_TYPE "CHECKSUM_0B")
elseif(TLSH_CHECKSUM_1B)
  set(TLSH_CHECKSUM "1 byte checksum")
  set(TLSH_CHECKSUM_TYPE "CHECKSUM_1B")
elseif(TLSH_CHECKSUM_3B)
  set(TLSH_CHECKSUM "3 bytes checksum")
  set(TLSH_CHECKSUM_TYPE "CHECKSUM_3B")
endif()

message(STATUS "Checksum: ${TLSH_CHECKSUM}")

# setting TLSH_DISTANCE_PARAMETERS to 'ON' allows you to set command line
# arguments to set - and hence experiment with the distance parameters
option(TLSH_DISTANCE_PARAMETERS "Enable cli control of the distance parameters" OFF)

configure_file(cmake/VERSION.in ${CMAKE_CURRENT_SOURCE_DIR}/VERSION)
configure_file(cmake/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TLSH_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TLSH_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TLSH_SOURCE_DIR}/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TLSH_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${TLSH_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${TLSH_SOURCE_DIR}/lib)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-g")
    endif()
else(CMAKE_BUILD_TYPE STREQUAL Debug)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-O2 -fvisibility=internal")  ## Remove TlshImpl symbols
    endif()
    if(MSVC)
        set(CMAKE_CXX_FLAGS "/O2")        ## Optimize
    endif()
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

# user can override CXX; make sure tests link and load properly regardless of LD_LIBRARY_PATH
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(utils)
add_subdirectory(Testing)

if(MSVC)
    add_subdirectory(Windows)
endif()

set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "TLSH-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES
"~$"
"^${PROJECT_SOURCE_DIR}/bin/"
"^${PROJECT_SOURCE_DIR}/lib/"
"^${PROJECT_SOURCE_DIR}/build/"
"^${PROJECT_SOURCE_DIR}/py_ext/data/"
"^${PROJECT_SOURCE_DIR}/py_ext/build/"
"^${PROJECT_SOURCE_DIR}/Testing/tmp/"
"^${PROJECT_SOURCE_DIR}/CMakeFiles/"
)
include(CPack)

