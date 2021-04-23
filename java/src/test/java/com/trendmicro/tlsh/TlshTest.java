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
package com.trendmicro.tlsh;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

/** Unit tests for TLSH creation and diff operations */
public class TlshTest {
	
	private static File sourceRoot;
	
	@BeforeClass
	public static void initClass() throws FileNotFoundException {
		sourceRoot = ExampleDataUtilities.getSourceRoot();
	}
	
	@AfterClass
	public static void teardownClass() throws FileNotFoundException {
		ExampleDataUtilities.clearFileCache();
	}
	
	@Test
	public void test_hash_128_checksum_1() throws IOException {
		test_hash(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_1B, VersionOption.VERSION_4);
	}

	@Test
	public void test_hash_128_checksum_3() throws IOException {
		test_hash(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_3B);
	}

	@Test
	public void test_hash_256_checksum_1() throws IOException {
		test_hash(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_1B);
	}

	@Test
	public void test_hash_256_checksum_3() throws IOException {
		test_hash(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_3B);
	}

	private void test_hash(BucketOption bucketOption, ChecksumOption checksumOption) throws IOException {
		test_hash(bucketOption, checksumOption, VersionOption.ORIGINAL);
	}

	private void test_hash(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption) throws IOException {
		Map<File, String> expectedHashes = ExampleDataUtilities.getExpectedHashes(sourceRoot, bucketOption.getBucketCount(), checksumOption.getChecksumLength());
		assertNotNull(expectedHashes);
		assertFalse(expectedHashes.isEmpty());
		
		TlshCreator tlsh = new TlshCreator(bucketOption, checksumOption, versionOption);
		for (Map.Entry<File, String> fileAndHash : expectedHashes.entrySet()) {
			byte[] wholeFile = ExampleDataUtilities.getFileBytes(fileAndHash.getKey());
			boolean smallSource = wholeFile.length < TlshCreator.MIN_DATA_LENGTH;
			tlsh.update(wholeFile);
			assertTrue(tlsh.isValid(smallSource));
			String hash = tlsh.getHash(smallSource).toString();
			assertEquals("Hashes do not match for file " + fileAndHash.getKey().getAbsolutePath(), fileAndHash.getValue(), hash);
			tlsh.reset();
			assertFalse(tlsh.isValid(false));
		}
	}

	@Test
	public void test_diff_128_checksum_1_with_length() throws IOException {
		test_diff(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_1B, true);
	}

	@Test
	public void test_diff_128_checksum_1_without_length() throws IOException {
		test_diff(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_1B, false);
	}

	@Test
	public void test_diff_128_checksum_3_with_length() throws IOException {
		test_diff(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_3B, true);
	}

	@Test
	public void test_diff_128_checksum_3_without_length() throws IOException {
		test_diff(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_3B, false);
	}

	@Test
	public void test_diff_256_checksum_1_with_length() throws IOException {
		test_diff(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_1B, true);
	}

	@Test
	public void test_diff_256_checksum_1_without_length() throws IOException {
		test_diff(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_1B, false);
	}

	@Test
	public void test_diff_256_checksum_3_with_length() throws IOException {
		test_diff(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_3B, true);
	}

	@Test
	public void test_diff_256_checksum_3_without_length() throws IOException {
		test_diff(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_3B, false);
	}

	private void test_diff(BucketOption bucketOption, ChecksumOption checksumOption, boolean includeLength) throws FileNotFoundException, IOException
			{
		List<ExampleDataUtilities.FilesAndDiff> expectedDiffs = ExampleDataUtilities.getExpectedDiffScores(sourceRoot,
				bucketOption.getBucketCount(), checksumOption.getChecksumLength(), includeLength);
		assertNotNull(expectedDiffs);
		assertFalse(expectedDiffs.isEmpty());

		for (ExampleDataUtilities.FilesAndDiff filesAndDiff : expectedDiffs) {
			byte[] sourceFile = ExampleDataUtilities.getFileBytes(filesAndDiff.sourceFile);
			byte[] targetFile = ExampleDataUtilities.getFileBytes(filesAndDiff.targetFile);
			boolean smallSource = sourceFile.length < TlshCreator.MIN_DATA_LENGTH;
			boolean smallTarget = targetFile.length < TlshCreator.MIN_DATA_LENGTH;

			TlshCreator sourceTlshCreator = new TlshCreator(bucketOption, checksumOption);
			sourceTlshCreator.update(sourceFile);
			assertTrue("TLSH not valid for source file " + filesAndDiff.sourceFile + " (target file " + filesAndDiff.targetFile + ")",
				sourceTlshCreator.isValid(smallSource));
			Tlsh sourceTlsh = sourceTlshCreator.getHash(smallSource);

			TlshCreator targetTlshCreator = new TlshCreator(bucketOption, checksumOption);
			targetTlshCreator.update(targetFile);
			assertTrue("TLSH not valid for target file " + filesAndDiff.targetFile + " (source file " + filesAndDiff.sourceFile + ")",
				targetTlshCreator.isValid(smallTarget));
			Tlsh targetTlsh = targetTlshCreator.getHash(smallTarget);

			assertEquals("Incorrect diff for " + filesAndDiff.sourceFile + " and " + filesAndDiff.targetFile,
					filesAndDiff.expectedDiff, sourceTlsh.totalDiff(targetTlsh, includeLength));
			// Make sure diff is symmetric
			assertEquals("Incorrect diff for " + filesAndDiff.sourceFile + " and " + filesAndDiff.targetFile,
					filesAndDiff.expectedDiff, targetTlsh.totalDiff(sourceTlsh, includeLength));
		}
		
	}

	/** Test diff when checksums have different lengths */
	@Test(expected=IllegalArgumentException.class)
	public void test_diff_different_checksum() {
		byte[] buf = new byte[1024];
		new Random().nextBytes(buf);
		TlshCreator checksum1 = new TlshCreator(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_1B);
		checksum1.update(buf);
		Tlsh tlsh1 = checksum1.getHash();

		TlshCreator checksum3 = new TlshCreator(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_3B);
		checksum3.update(buf);
		Tlsh tlsh3 = checksum3.getHash();
		
		tlsh1.totalDiff(tlsh3, true);
	}

	/** Test diff with different bucket counts */
	@Test(expected=IllegalArgumentException.class)
	public void test_diff_different_buckets() {
		byte[] buf = new byte[1024];
		new Random().nextBytes(buf);
		TlshCreator buckets128 = new TlshCreator(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_1B);
		buckets128.update(buf);
		Tlsh tlsh128 = buckets128.getHash();

		TlshCreator buckets256 = new TlshCreator(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_1B);
		buckets256.update(buf);
		Tlsh tlsh256 = buckets256.getHash();
		
		tlsh128.totalDiff(tlsh256, true);
	}

	/**
	 * Test force flag in isValid call
	 */
	@Test
	public void test_isValid() {
		byte[] buf = new byte[TlshCreator.MIN_DATA_LENGTH];
		new Random().nextBytes(buf);
		TlshCreator tlshCreator = new TlshCreator();
		// not quite enough, even when forcing
		tlshCreator.update(buf, 0, TlshCreator.MIN_FORCE_DATA_LENGTH - 1);

		assertFalse(tlshCreator.isValid(false));
		assertFalse(tlshCreator.isValid(true));

		// one more byte should do it for forcing
		tlshCreator.update(buf, TlshCreator.MIN_FORCE_DATA_LENGTH - 1, 1);

		assertFalse(tlshCreator.isValid(false));
		assertTrue(tlshCreator.isValid(true));

		// add all but one of the remaining bytes, should not change validity
		tlshCreator.update(buf, TlshCreator.MIN_FORCE_DATA_LENGTH, (TlshCreator.MIN_DATA_LENGTH - TlshCreator.MIN_FORCE_DATA_LENGTH - 1));
		assertFalse(tlshCreator.isValid(false));
		assertTrue(tlshCreator.isValid(true));

		// add the last byte
		tlshCreator.update(buf, TlshCreator.MIN_DATA_LENGTH - 1, 1);
		assertTrue(tlshCreator.isValid(false));
		assertTrue(tlshCreator.isValid(true));
	}

	/**
	 * Test input that is not varied enough
	 */
	@Test
	public void test_isValid_too_little_variance() {
		byte[] buf = new byte[1000];
		for (int i = 0; i < buf.length; ) {
			buf[i] = 'a';
			buf[i+1] = 'b';
			buf[i+2] = 'c';
			buf[i+3] = 'd';
			buf[i+4] = 'e';
			i+=5;
		}

		TlshCreator tlshCreator = new TlshCreator();
		tlshCreator.update(buf);

		// Should not be able to force validity
		assertFalse(tlshCreator.isValid(false));
		assertFalse(tlshCreator.isValid(true));
	}

	/**
	 * Test force flag in getHash call
	 */
	@Test
	public void test_getHash() {
		byte[] buf = new byte[TlshCreator.MIN_DATA_LENGTH];
		new Random().nextBytes(buf);
		TlshCreator tlshCreator = new TlshCreator();
		// not quite enough, even when forcing
		tlshCreator.update(buf, 0, TlshCreator.MIN_FORCE_DATA_LENGTH - 1);

		try {
			tlshCreator.getHash();
			fail();
		} catch(IllegalStateException e) {
			// expected
		}
		try {
			tlshCreator.getHash(true);
			fail();
		} catch(IllegalStateException e) {
			// expected
		}

		// one more byte should do it for forcing
		tlshCreator.update(buf, TlshCreator.MIN_FORCE_DATA_LENGTH - 1, 1);
		assertNotNull(tlshCreator.getHash(true));
		try {
			tlshCreator.getHash();
			fail();
		} catch(IllegalStateException e) {
			// expected
		}

		// add the last rest
		tlshCreator.update(buf, TlshCreator.MIN_FORCE_DATA_LENGTH, TlshCreator.MIN_DATA_LENGTH - TlshCreator.MIN_FORCE_DATA_LENGTH);
		assertNotNull(tlshCreator.getHash(true));
		assertNotNull(tlshCreator.getHash(false));
	}

	/**
	 * Test input that is not varied enough
	 */
	@Test
	public void test_getHashNoThrow() {
		byte[] buf = new byte[TlshCreator.MIN_DATA_LENGTH];
		new Random().nextBytes(buf);
		TlshCreator tlshCreator = new TlshCreator();
		// not quite enough, even when forcing
		tlshCreator.update(buf, 0, TlshCreator.MIN_DATA_LENGTH - 1);

		assertNull(tlshCreator.getHashNoThrow());

		// add the final required byte
		tlshCreator.update(buf, TlshCreator.MIN_DATA_LENGTH - 1, 1);
		assertNotNull(tlshCreator.getHashNoThrow());
	}

	/**
	 * Test that multiple calls to the update(byte, int, int) method have the
	 * same result as one call to the update(byte) method
	 */
	@Test
	public void test_multiple_vs_single_updates() {
		byte[] buf = new byte[TlshCreator.MIN_DATA_LENGTH];
		new Random().nextBytes(buf);

		TlshCreator singleUpdate = new TlshCreator();
		singleUpdate.update(buf);
		Tlsh singleUpdateHash = singleUpdate.getHash();

		TlshCreator multipleUpdates = new TlshCreator();
		for (int i = 0; i < buf.length; ++i) {
			multipleUpdates.update(buf, i, 1);
		}
		Tlsh multipleUpdatesHash = multipleUpdates.getHash();
		assertNotSame(singleUpdateHash, multipleUpdatesHash);

		assertEquals(singleUpdateHash.getEncoded(), multipleUpdatesHash.getEncoded());

	}

	@Test
	public void test_from_encoded_string_128_1() {
		test_from_encoded_string(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_1B);
	}

	@Test
	public void test_from_encoded_string_128_3() {
		test_from_encoded_string(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_3B);
	}

	@Test
	public void test_from_encoded_string_256_1() {
		test_from_encoded_string(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_1B);
	}

	@Test
	public void test_from_encoded_string_256_3() {
		test_from_encoded_string(BucketOption.BUCKETS_256, ChecksumOption.CHECKSUM_3B);
	}

	/**
	 * Tests that encoded strings can be successfully decoded
	 */
	private void test_from_encoded_string(BucketOption bucketOption, ChecksumOption checksumOption) {
		byte[] buf = new byte[TlshCreator.MIN_DATA_LENGTH];
		new Random().nextBytes(buf);

		TlshCreator tlshCreator = new TlshCreator(bucketOption, checksumOption);
		tlshCreator.update(buf);
		Tlsh tlsh = tlshCreator.getHash();
		
		String encoded = tlsh.getEncoded();
		
		Tlsh decoded = Tlsh.fromTlshStr(encoded);
		
		assertEquals(0, tlsh.totalDiff(decoded, true));
		
		// make sure re-encoding is identical to original encoding
		String reEncoded = decoded.toString();
		
		assertEquals(encoded, reEncoded);
	}

	/**
	 * Tests that lower-case strings can be successfully decoded
	 */
	@Test
	public void test_fromTlshStr_lowercase() {
		String encoded = "14d02b0987d87fa9f74228e362144b556ac8f02705130a5551476442a453e929c8842d";

		Tlsh decoded = Tlsh.fromTlshStr(encoded);
		
		// make sure re-encoding is identical to original encoding
		String reEncoded = decoded.toString();
		
		assertEquals(encoded, reEncoded.toLowerCase());
	}

	/**
	 * Test a string of wrong length is not decoded
	 */
	@Test(expected=IllegalArgumentException.class)
	public void test_fromTlshStr_bad_length() {
		String truncated = "14d02b0987d87fa9f74228e362144b556ac8f02705130a5551476442a453";

		Tlsh.fromTlshStr(truncated);
	}

	/**
	 * Test a string of wrong length is not decoded
	 */
	@Test(expected=IllegalArgumentException.class)
	public void test_fromTlshStr_not_hex() {
		// right length, but last character not hex
		String notQuiteHex = "14D02B0987D87FA9F74228E362144B556AC8F02705130A5551476442A453E929C8842G";

		Tlsh.fromTlshStr(notQuiteHex);
	}

	/**
	 * Lengthy test for input length above 2GB - Issue #84
	 */
	//@Test
	public void test_large_input() {
		byte[] b = new byte[1 << 10];
		ThreadLocalRandom.current().nextBytes(b);
		final long desiredInputSize = 0x9000_0000L;
		long bytesHashed = 0;
		TlshCreator creator = new TlshCreator();
		while (bytesHashed < desiredInputSize) {
			creator.update(b);
			bytesHashed += b.length;
		}
		assertTrue(creator.isValid());
		
	}
	
	/**
	 * Test bounds for l_capturing function
	 */
	@Test
	public void test_l_capturing() {
		assertEquals(169, TlshUtil.l_capturing(TlshCreator.MAX_DATA_LENGTH-1));
		assertEquals(169, TlshUtil.l_capturing(TlshCreator.MAX_DATA_LENGTH));
		try {
			TlshUtil.l_capturing(TlshCreator.MAX_DATA_LENGTH+1);
			fail("Expected exception for max length");
		} catch (IllegalArgumentException e) {
			// expected
		}
		
	}
	
}
