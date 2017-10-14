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


set(TLSH_SRCS tlsh.cpp tlsh_impl.cpp tlsh_util.cpp)
add_library(tlsh STATIC ${TLSH_SRCS})
set_target_properties(tlsh PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set_target_properties(tlsh PROPERTIES OUTPUT_NAME tlsh${BUILD_POSTFIX})

##########################
# the tlsh library is small - there is no reason to have a shared library
# it was causing problems when compiling / testing tools on Linux
##########################

set(TLSH_SHARED_LIBRARY 0)
if(TLSH_SHARED_LIBRARY EQUAL 1)
    add_library(tlsh_shared SHARED ${TLSH_SRCS})
    set_target_properties(tlsh_shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
    set_target_properties(tlsh_shared PROPERTIES OUTPUT_NAME tlsh${BUILD_POSTFIX})
    set_target_properties(tlsh_shared PROPERTIES VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
                                                 SOVERSION "0")
endif()

include(GNUInstallDirs)
if(TLSH_SHARED_LIBRARY EQUAL 1)
    install(TARGETS tlsh tlsh_shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
else()
    install(TARGETS tlsh             DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

install(FILES ../include/tlsh.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
