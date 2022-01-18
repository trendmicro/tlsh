/*
 * TLSH is provided for use under two licenses: Apache OR BSD.
 * Users may opt to use either license depending on the license
 * restictions of the systems with which they plan to integrate
 * the TLSH code.
 */

/* ==============
 * Apache License
 * ==============
 * Copyright 2017 Trend Micro Incorporated
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
 * Copyright (c) 2017, Trend Micro Incorporated
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

using System;
using System.Diagnostics;
using System.Security.Cryptography;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace TrendMicro.Tlsh;

/**
 * Some methods to see the performance of the TLSH implementation.
 *
 */
[TestClass]
public class TlshPerformanceTests
{
	// TODO: This kind of micro-benchmarking is probably better done
	// in a framework like JMH

	/**
     * Attempt to test the performance of the getHash method
     */
	[TestMethod]
	public void test_performance_getHash()
	{
		var tlsh = new Tlsh();
		var b = new byte[Tlsh.MinDataLength];
		using var rng = RandomNumberGenerator.Create();
		rng.GetBytes(b);

		var numIters = 250000;
		var start = Stopwatch.StartNew();
		for (var i = 0; i < numIters; ++i)
		{
			tlsh.Update(b);
			tlsh.GetHash(false);
			tlsh.Reset();
		}

		var diff = start.ElapsedMilliseconds;
		var timePerIter = (float) diff / numIters;
		Console.WriteLine("It took " + diff + "ms to run " + numIters + " iterations, for " + timePerIter + "ns/call");

	}

	[TestMethod]
	public void test_performance_update_128_1()
	{
		test_performance_update(BucketOption.Default, ChecksumOption.OneByte);
	}

	[TestMethod]
	public void test_performance_update_128_3()
	{
		test_performance_update(BucketOption.Default, ChecksumOption.ThreeBytes);
	}

	[TestMethod]
	public void test_performance_update_256_1()
	{
		test_performance_update(BucketOption.Extended, ChecksumOption.OneByte);
	}

	[TestMethod]

	public void test_performance_update_256_3()
	{
		test_performance_update(BucketOption.Extended, ChecksumOption.ThreeBytes);
	}

	/**
     * Attempt to test the performance of the update method
     */
	private void test_performance_update(BucketOption bucketOption, ChecksumOption checksumOption)
	{
		var tlsh = new Tlsh(bucketOption, checksumOption, VersionOption.Version4);
		var b = new byte[16384];
		using var rng = RandomNumberGenerator.Create();
		rng.GetBytes(b);

		var numIters = 10000;
		var start = Stopwatch.StartNew();
		for (var i = 0; i < numIters; ++i)
		{
			tlsh.Update(b);
		}

		tlsh.GetHash(false);
		var diff = start.ElapsedMilliseconds;
		var totalHashed = (long)numIters * b.Length;
		Console.WriteLine("With {0} buckets and {1} byte checksum it took {2}ms to hash {3} bytes, for {4} bytes hashed/s\n",
			(int) bucketOption, (int) checksumOption, (diff), totalHashed,
			(totalHashed * 1000000000L) / (diff));

	}


}