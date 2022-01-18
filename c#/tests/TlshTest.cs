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
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Shouldly;

namespace TrendMicro.Tlsh;

[TestClass]
public class TlshTestS
{
	private static string? _SourceRoot;

	[ClassInitialize]
	public static void InitClass(TestContext context)
	{
		_SourceRoot = ExampleDataUtilities.GetSourceRoot();
	}

	[ClassCleanup]
	public static void TeardownClass()
	{
		ExampleDataUtilities.ClearFileCache();
	}

	[TestMethod]
	public void Test_hash_128_checksum_1()
	{
		Test_hash(BucketOption.Default, ChecksumOption.OneByte, VersionOption.Version4);
	}

	[TestMethod]
	public void Test_hash_128_checksum_3()
	{
		Test_hash(BucketOption.Default, ChecksumOption.ThreeBytes);
	}

	[TestMethod]
	public void Test_hash_256_checksum_1()
	{
		Test_hash(BucketOption.Extended, ChecksumOption.OneByte);
	}

	[TestMethod]
	public void Test_hash_256_checksum_3()
	{
		Test_hash(BucketOption.Extended, ChecksumOption.ThreeBytes);
	}

	private void Test_hash(BucketOption bucketOption, ChecksumOption checksumOption)
	{
		Test_hash(bucketOption, checksumOption, VersionOption.Original);
	}

	private void Test_hash(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption)
	{
		var expectedHashes = ExampleDataUtilities.GetExpectedHashes(_SourceRoot, (int)bucketOption, (int)checksumOption);
		expectedHashes.ShouldNotBeNull();
		expectedHashes.ShouldNotBeEmpty();

		var tlsh = new Tlsh(bucketOption, checksumOption, versionOption);
		foreach (var (file, expectedHash) in expectedHashes)
		{
			var wholeFile = ExampleDataUtilities.GetFileBytes(file);
			var smallSource = wholeFile.Length < Tlsh.MinDataLength;
			tlsh.Update(wholeFile);
			tlsh.IsValid(smallSource).ShouldBeTrue();
			var hash = tlsh.GetHash(smallSource);
			var hashString = hash.ToString();

			hashString.ShouldBe(expectedHash, $"Hashes do not match for file {file}");
			tlsh.Reset();
			tlsh.IsValid().ShouldBeFalse();
		}
	}

	[TestMethod]
	public void Test_diff_128_checksum_1_with_length()
	{
		Test_diff(BucketOption.Default, ChecksumOption.OneByte, true);
	}

	[TestMethod]
	public void Test_diff_128_checksum_1_without_length()
	{
		Test_diff(BucketOption.Default, ChecksumOption.OneByte, false);
	}

	[TestMethod]
	public void Test_diff_128_checksum_3_with_length()
	{
		Test_diff(BucketOption.Default, ChecksumOption.ThreeBytes, true);
	}

	[TestMethod]
	public void Test_diff_128_checksum_3_without_length()
	{
		Test_diff(BucketOption.Default, ChecksumOption.ThreeBytes, false);
	}

	[TestMethod]
	public void Test_diff_256_checksum_1_with_length()
	{
		Test_diff(BucketOption.Extended, ChecksumOption.OneByte, true);
	}

	[TestMethod]
	public void Test_diff_256_checksum_1_without_length()
	{
		Test_diff(BucketOption.Extended, ChecksumOption.OneByte, false);
	}

	[TestMethod]
	public void Test_diff_256_checksum_3_with_length()
	{
		Test_diff(BucketOption.Extended, ChecksumOption.ThreeBytes, true);
	}

	[TestMethod]
	public void Test_diff_256_checksum_3_without_length()
	{
		Test_diff(BucketOption.Extended, ChecksumOption.ThreeBytes, false);
	}

	private void Test_diff(BucketOption bucketOption, ChecksumOption checksumOption, bool includeLength)
	{
		var expectedDiffs = ExampleDataUtilities.GetExpectedDiffScores(_SourceRoot,
			(int)bucketOption, (int)checksumOption, includeLength);
		expectedDiffs.ShouldNotBeNull();
		expectedDiffs.ShouldNotBeEmpty();

		foreach (var filesAndDiff in expectedDiffs)
		{
			var sourceFile = ExampleDataUtilities.GetFileBytes(filesAndDiff.SourceFile);
			var targetFile = ExampleDataUtilities.GetFileBytes(filesAndDiff.TargetFile);
			var smallSource = sourceFile.Length < Tlsh.MinDataLength;
			var smallTarget = targetFile.Length < Tlsh.MinDataLength;

			var sourceTlshCreator = new Tlsh(bucketOption, checksumOption, VersionOption.Version4);
			sourceTlshCreator.Update(sourceFile);
			sourceTlshCreator.IsValid(smallSource).ShouldBeTrue($"TLSH not valid for source file {filesAndDiff.SourceFile} (target file {filesAndDiff.TargetFile})");
			var sourceTlsh = sourceTlshCreator.GetHash(smallSource);

			var targetTlshCreator = new Tlsh(bucketOption, checksumOption, VersionOption.Version4);
			targetTlshCreator.Update(targetFile);
			targetTlshCreator.IsValid(smallTarget).ShouldBeTrue($"TLSH not valid for target file {filesAndDiff.TargetFile} (source file {filesAndDiff.SourceFile})");
			var targetTlsh = targetTlshCreator.GetHash(smallTarget);
			sourceTlsh.DistanceTo(targetTlsh, includeLength).ShouldBe(filesAndDiff.ExpectedDiff, $"Incorrect diff for {filesAndDiff.SourceFile} and {filesAndDiff.TargetFile}");
			// Make sure diff is symmetric
			targetTlsh.DistanceTo(sourceTlsh, includeLength).ShouldBe(filesAndDiff.ExpectedDiff, $"Incorrect diff for {filesAndDiff.SourceFile} and {filesAndDiff.TargetFile}");
		}
	}

	/**
	 * Test diff when checksums have different lengths
	 */
	[TestMethod]
	public void Test_diff_different_checksum()
	{
		var buf = new byte[1024];
		new Random().NextBytes(buf);
		var checksum1 = new Tlsh(BucketOption.Default, ChecksumOption.OneByte, VersionOption.Version4);
		checksum1.Update(buf);
		var tlsh1 = checksum1.GetHash();

		var checksum3 = new Tlsh(BucketOption.Default, ChecksumOption.ThreeBytes, VersionOption.Version4);
		checksum3.Update(buf);
		var tlsh3 = checksum3.GetHash();

		Should.Throw<ArgumentException>(() => tlsh1.DistanceTo(tlsh3, true));
	}

	/**
	 * Test diff with different bucket counts
	 */
	[TestMethod]
	public void Test_diff_different_buckets()
	{
		var buf = new byte[1024];
		new Random().NextBytes(buf);
		var buckets128 = new Tlsh(BucketOption.Default, ChecksumOption.OneByte, VersionOption.Version4);
		buckets128.Update(buf);
		var tlsh128 = buckets128.GetHash();

		var buckets256 = new Tlsh(BucketOption.Extended, ChecksumOption.OneByte, VersionOption.Version4);
		buckets256.Update(buf);
		var tlsh256 = buckets256.GetHash();

		Should.Throw<ArgumentException>(() => tlsh128.DistanceTo(tlsh256, true));
	}

	/**
	 * Test force flag in isValid call
	 */
	[TestMethod]
	public void Test_isValid()
	{
		var buf = new byte[Tlsh.MinDataLength];
		new Random().NextBytes(buf);
		var tlshCreator = new Tlsh();
		// not quite enough, even when forcing
			
		tlshCreator.Update(buf, 0, Tlsh.MinForceDataLength - 1);

		tlshCreator.IsValid().ShouldBeFalse();
		tlshCreator.IsValid(true).ShouldBeFalse();

		// one more byte should do it for forcing
		tlshCreator.Update(buf, Tlsh.MinForceDataLength - 1, 1);

		tlshCreator.IsValid().ShouldBeFalse();
		tlshCreator.IsValid(true).ShouldBeTrue();

		// add all but one of the remaining bytes, should not change validity
		tlshCreator.Update(buf, Tlsh.MinForceDataLength, Tlsh.MinDataLength - Tlsh.MinForceDataLength - 1);
		tlshCreator.IsValid().ShouldBeFalse();
		tlshCreator.IsValid(true).ShouldBeTrue();

		// add the last byte
		tlshCreator.Update(buf, Tlsh.MinDataLength - 1, 1);
		tlshCreator.IsValid().ShouldBeTrue();
		tlshCreator.IsValid(true).ShouldBeTrue();
	}

	/**
	 * Test input that is not varied enough
	 */
	[TestMethod]
	public void Test_isValid_too_little_variance()
	{
		var buf = new byte[1000];
		for (var i = 0; i < buf.Length;)
		{
			buf[i] = (byte)'a';
			buf[i + 1] = (byte)'b';
			buf[i + 2] = (byte)'c';
			buf[i + 3] = (byte)'d';
			buf[i + 4] = (byte)'e';
			i += 5;
		}

		var tlshCreator = new Tlsh();
		tlshCreator.Update(buf);

		// Should not be able to force validi.ShouldBeFalse();
		tlshCreator.IsValid().ShouldBeFalse();
		tlshCreator.IsValid(true).ShouldBeFalse();
	}

	/**
	 * Test force flag in getHash call
	 */
	[TestMethod]
	public void Test_getHash()
	{
		var buf = new byte[Tlsh.MinDataLength];
		new Random().NextBytes(buf);
		var tlshCreator = new Tlsh();
		// not quite enough, even when forcing
		tlshCreator.Update(buf, 0, Tlsh.MinForceDataLength - 1);

		Should.Throw<InvalidOperationException>(() => tlshCreator.GetHash());

		Should.Throw<InvalidOperationException>(() => tlshCreator.GetHash(true));

		// one more byte should do it for forcing
		tlshCreator.Update(buf, Tlsh.MinForceDataLength - 1, 1);
		tlshCreator.GetHash(true);
		Should.Throw<InvalidOperationException>(() => tlshCreator.GetHash());

		// add the last rest
		tlshCreator.Update(buf, Tlsh.MinForceDataLength, Tlsh.MinDataLength - Tlsh.MinForceDataLength);
		tlshCreator.GetHash(true);
		tlshCreator.GetHash();
	}

	/**
	 * Test input that is not varied enough
	 */
	[TestMethod]
	public void Test_getHashNoThrow()
	{
		var buf = new byte[Tlsh.MinDataLength];
		new Random().NextBytes(buf);
		var tlshCreator = new Tlsh();
		// not quite enough, even when forcing
		tlshCreator.Update(buf, 0, Tlsh.MinDataLength - 1);

		tlshCreator.TryGetHash(out _).ShouldBeFalse();

		// add the final required byte
		tlshCreator.Update(buf, Tlsh.MinDataLength - 1, 1);
		tlshCreator.TryGetHash(out _).ShouldBeTrue();
	}

	/**
	 * Test that multiple calls to the update(byte, int, int) method have the
	 * same result as one call to the update(byte) method
	 */
	[TestMethod]
	public void Test_multiple_vs_single_updates()
	{
		var buf = new byte[Tlsh.MinDataLength];
		new Random().NextBytes(buf);

		var singleUpdate = new Tlsh();
		singleUpdate.Update(buf);
		var singleUpdateHash = singleUpdate.GetHash();

		var multipleUpdates = new Tlsh();
		for (var i = 0; i < buf.Length; ++i) multipleUpdates.Update(buf, i, 1);

		var multipleUpdatesHash = multipleUpdates.GetHash();
		singleUpdateHash.ShouldNotBeSameAs(multipleUpdatesHash);

		singleUpdateHash.ToString().ShouldBe(multipleUpdatesHash.ToString());
	}

	[TestMethod]
	public void Test_from_encoded_string_128_1()
	{
		Test_from_encoded_string(BucketOption.Default, ChecksumOption.OneByte);
	}

	[TestMethod]
	public void Test_from_encoded_string_128_3()
	{
		Test_from_encoded_string(BucketOption.Default, ChecksumOption.ThreeBytes);
	}

	[TestMethod]
	public void Test_from_encoded_string_256_1()
	{
		Test_from_encoded_string(BucketOption.Extended, ChecksumOption.OneByte);
	}

	[TestMethod]
	public void Test_from_encoded_string_256_3()
	{
		Test_from_encoded_string(BucketOption.Extended, ChecksumOption.ThreeBytes);
	}

	/**
	 * Tests that encoded strings can be successfully decoded
	 */
	private void Test_from_encoded_string(BucketOption bucketOption, ChecksumOption checksumOption)
	{
		var buf = new byte[Tlsh.MinDataLength];
		new Random().NextBytes(buf);

		var tlshCreator = new Tlsh(bucketOption, checksumOption, VersionOption.Version4);
		tlshCreator.Update(buf);
		var tlsh = tlshCreator.GetHash();

		var encoded = tlsh.ToString();

		var decoded = TlshValue.Parse(encoded);

		tlsh.DistanceTo(decoded, true).ShouldBe(0);

		// make sure re-encoding is identical to original encoding
		var reEncoded = decoded.ToString();

		reEncoded.ShouldBe(encoded);
	}

	/**
	 * Tests that lower-case strings can be successfully decoded
	 */
	[TestMethod]
	public void Test_fromTlshStr_lowercase()
	{
		var encoded = "14d02b0987d87fa9f74228e362144b556ac8f02705130a5551476442a453e929c8842d";

		var decoded = TlshValue.Parse(encoded);

		// make sure re-encoding is identical to original encoding
		var reEncoded = decoded.ToString();

		reEncoded.ToLower().ShouldBe(encoded);
	}

	/**
	 * Test a string of wrong length is not decoded
	 */
	[TestMethod]
	public void Test_fromTlshStr_bad_length()
	{
		const string truncated = "14d02b0987d87fa9f74228e362144b556ac8f02705130a5551476442a453";

		Should.Throw<ArgumentException>(() => TlshValue.Parse(truncated));
	}

	/**
	 * Test a string of wrong length is not decoded
	 */
	[TestMethod]
	public void Test_fromTlshStr_not_hex()
	{
		// right length, but last character not hex
		const string notQuiteHex = "14D02B0987D87FA9F74228E362144B556AC8F02705130A5551476442A453E929C8842G";

		Should.Throw<ArgumentException>(() => TlshValue.Parse(notQuiteHex));
	}

	/**
	 * Lengthy test for input length above 2GB - Issue #84
	 */
	[TestMethod]
	public void Test_large_input()
	{
		var b = new byte[1 << 10];
		new Random().NextBytes(b);
		const long desiredInputSize = 0x9000_0000L;
		long bytesHashed = 0;
		var creator = new Tlsh();
		while (bytesHashed < desiredInputSize)
		{
			creator.Update(b);
			bytesHashed += b.Length;
		}

		creator.IsValid().ShouldBeTrue();
	}

	/**
	 * Test bounds for LCapturing function
	 */
	[TestMethod]
	public void Test_l_capturing()
	{
		TlshUtil.LCapturing(Tlsh.MaxDataLength - 1).ShouldBe((byte) 169);
		TlshUtil.LCapturing(Tlsh.MaxDataLength).ShouldBe((byte) 169);
		Should.Throw<ArgumentException>(() => TlshUtil.LCapturing(Tlsh.MaxDataLength + 1));
	}
}