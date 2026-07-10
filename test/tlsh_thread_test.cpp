/*
 * TLSH is provided for use under two licenses: Apache OR BSD.
 * Users may opt to use either license depending on the license
 * restictions of the systems with which they plan to integrate
 * the TLSH code.
 */

/* ==============
 * Apache License
 * ==============
 * Copyright 2013 Trend Micro Incorporated
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* ===========
 * BSD License
 * ===========
 * Copyright (c) 2013, Trend Micro Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.

 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Reentrancy test for TLSH threaded hash options.
 *
 * TLSH_OPTION_THREADED and TLSH_OPTION_THREADED4 internally spawn worker
 * threads to parallelise bucket computation.  If the per-thread work
 * packages are stored as static globals (the pre-fix state) rather than
 * as stack-local variables, concurrent callers stomp on each other's
 * data and produce wrong hashes.
 *
 * This test:
 *   1. Computes TLSH hashes sequentially for N distinct buffers (ground truth).
 *   2. Computes the same hashes again with N outer threads running simultaneously.
 *   3. Compares the results.
 *
 * With the old static-global implementation step 3 fails; with stack-local
 * variables it passes.
 */

#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>
#include <vector>

#include "tlsh.h"

static const int NUM_HASHES  = 8;
static const int BUF_SIZE_T4 = 25000;  /* >= 20000 triggers TLSH_OPTION_THREADED4 */
static const int BUF_SIZE_T2 = 15000;  /* >= 10000 triggers TLSH_OPTION_THREADED  */

static void fill_buf(unsigned char *buf, int size, int seed)
{
	for (int i = 0; i < size; i++)
		buf[i] = (unsigned char)((i * 7 + seed * 131) & 0xff);
}

static int run_test(int tlsh_option, int buf_size, const char *label)
{
	std::vector<std::vector<unsigned char>> bufs(NUM_HASHES,
	                                             std::vector<unsigned char>(buf_size));
	for (int i = 0; i < NUM_HASHES; i++)
		fill_buf(bufs[i].data(), buf_size, i);

	/* Sequential ground-truth hashes */
	std::string expected[NUM_HASHES];
	for (int i = 0; i < NUM_HASHES; i++) {
		Tlsh t;
		t.final(bufs[i].data(), buf_size, tlsh_option);
		expected[i] = t.getHash(1);
	}

	/* Concurrent hashes — all NUM_HASHES outer threads run simultaneously,
	 * each triggering the internal worker threads via tlsh_option.        */
	std::string actual[NUM_HASHES];
	{
		std::vector<std::thread> threads;
		for (int i = 0; i < NUM_HASHES; i++) {
			threads.emplace_back([&bufs, &actual, i, buf_size, tlsh_option]() {
				Tlsh t;
				t.final(bufs[i].data(), buf_size, tlsh_option);
				actual[i] = t.getHash(1);
			});
		}
		// Wait for all threads to finish execution
		for (auto &th : threads)
			th.join();
	}

	int failures = 0;
	for (int i = 0; i < NUM_HASHES; i++) {
		if (expected[i] != actual[i]) {
			fprintf(stderr, "FAIL [%s] hash[%d]: expected %s, got %s\n",
			        label, i, expected[i].c_str(), actual[i].c_str());
			failures++;
		}
	}

	if (failures == 0) {
		printf("passed %s reentrancy test (%d concurrent hashes)\n", label, NUM_HASHES);
		return 0;
	}
	fprintf(stderr, "FAILED [%s]: %d/%d hashes incorrect under concurrent computation\n",
	        label, failures, NUM_HASHES);
	return 1;
}

int main()
{
	int ret = 0;
	ret |= run_test(TLSH_OPTION_THREADED,  BUF_SIZE_T2, "TLSH_OPTION_THREADED");
	ret |= run_test(TLSH_OPTION_THREADED4, BUF_SIZE_T4, "TLSH_OPTION_THREADED4");
	return ret;
}
