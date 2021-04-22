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

/**
 * A TLSH structure that can be encoded to a hex string or
 * compared to another TLSH structure.
 * <p>
 * TLSH structures are created by {@link TlshCreator} or by calling
 * {@link #fromTlshStr(String)}
 */
public class Tlsh {
	/*
	 * Porting notes:
	 * This code is mostly copied from tlsh_impl.h/tlsh_impl.cpp
	 * In particular, this class is lsh_bin_struct with the diff operation
	 * that only really applies to the struct moved here.
	 * Also, the code dispenses with the C-style union code which
	 * doesn't help us in Java land.
	 */

	private static final int RANGE_LVALUE = 256;
	private static final int RANGE_QRATIO = 16;


	/**
	 * Turn an encoded TLSH string into a Tlsh structure.
	 * 
	 * @param tlshStr
	 *            The encoded TLSH string
	 * @return the decoded Tlsh structure
	 * @throws IllegalArgumentException
	 *             If the given string cannot be parsed correctly
	 */
	public static Tlsh fromTlshStr(String tlshStr) throws IllegalArgumentException {
	    VersionOption versionOption = null;
		int[] checksum = null;
		int[] tmp_code = null;
		for (BucketOption bucketOption : BucketOption.values()) {
			for (ChecksumOption checksumOption : ChecksumOption.values()) {
	            for (VersionOption tryVersion : VersionOption.values()) {
    				if (tlshStr.length() == hashStringLength(bucketOption, checksumOption, tryVersion)) {
    					checksum = new int[checksumOption.getChecksumLength()];
    					tmp_code = new int[bucketOption.getBucketCount() / 4];
    					versionOption = tryVersion;
    					break;
    				}
	            }
			}
		}
		if (checksum == null) {
			throw new IllegalArgumentException("Invalid hash string, length does not match any known encoding");
		}

		int offset = versionOption.getVersionString().length();
		for (int k = 0; k < checksum.length; k++) {
			checksum[k] = TlshUtil.from_hex_swapped(tlshStr, offset);
			offset += 2;
		}
		
		int Lvalue = TlshUtil.from_hex_swapped(tlshStr, offset);
		offset += 2;

		int qRatios = TlshUtil.from_hex(tlshStr, offset);
		offset += 2;

		for (int i = 0; i < tmp_code.length; i++) {
			// un-reverse the code during encoding
			tmp_code[tmp_code.length - i - 1] = TlshUtil.from_hex(tlshStr, offset);
			offset += 2;
		}

		return new Tlsh(versionOption, checksum, Lvalue, qRatios >> 4, qRatios & 0xF, tmp_code);
	}
	
	/**
	 * Get the length of the encoded output string for different
	 * hash creation options 
	 */
	private static int hashStringLength(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption) {
		return versionOption.getVersionString().length() + (bucketOption.getBucketCount() / 2) + (checksumOption.getChecksumLength() * 2) + 4;
	}

	/////////////////////////////////////////////////////////////
	// Instance stuff
	//
	private final VersionOption version;
	private final int[] checksum; // 1 or 3 bytes
	private final int Lvalue; // 1 byte
	private final int Q1ratio; // 4 bits
	private final int Q2ratio; // 4 bits
	private final int[] codes; // 32/64 bytes
	
	Tlsh(VersionOption versionOption, int[] checksum, int lvalue, int q1ratio, int q2ratio, int[] codes) {
	    this.version = versionOption;
		this.checksum = checksum;
		Lvalue = lvalue;
		Q1ratio = q1ratio;
		Q2ratio = q2ratio;
		this.codes = codes;
	}

	/**
	 * Return the version this hash was created with
	 * @return the version this hash was created with
	 */
	public VersionOption getVersion() {
	    return version;
	}

	/**
	 * Convert this object to a string; equivalent to {@link #getEncoded()}
	 * 
	 * @return the hex-encoded string
	 */
	@Override
	public String toString() {
		return getEncoded();
	}

	/**
	 * Convert this TLSH to hex-encoded string
	 * 
	 * @return the hex-encoded string
	 */
	public String getEncoded() {
		// The C++ code reverses the order of some of the fields before
		// converting to hex, so copy that behaviour.
		StringBuilder sb = new StringBuilder(hashStringLength());
		sb.append(version.getVersionString());

		for (int k = 0; k < checksum.length; k++) {
			TlshUtil.to_hex_swapped(checksum[k], sb);
		}
		TlshUtil.to_hex_swapped(Lvalue, sb);
		TlshUtil.to_hex(Q1ratio << 4 | Q2ratio, sb);
		for (int i = 0; i < codes.length; i++) {
			// reverse the code during encoding
			TlshUtil.to_hex(codes[codes.length - 1 - i], sb);
		}

		return sb.toString();
	}

	/**
	 * Calculate how long the output hex string will be
	 */
	private int hashStringLength() {
		// extra 4 characters come from length and Q1 and Q2 ratio.
		return version.getVersionString().length() + codes.length * 2 + checksum.length * 2 + 4;
	}

	/**
	 * Calculate the difference with another TLSH hash.
	 * <p>
	 * There is no hard-and-fast way to interpret the difference number output by
	 * this function, it is not a percentage or probability. Lower scores are more
	 * similar, higher scores are less similar. A value of 0 means the data the
	 * hashes were generated from is likely identical. A value less than 50 means
	 * the data the hashes were generated from is likely quite similar. Scores over
	 * 1000 are possible for very different data.
	 * 
	 * @param other
	 *            the hash to compute the difference from
	 * 
	 * @param len_diff
	 *            The len_diff parameter specifies if the file length is to be
	 *            included in the difference calculation (len_diff=true) or if it is
	 *            to be excluded (len_diff=false). In general, the length should be
	 *            considered in the difference calculation, but there could be
	 *            applications where a part of the adversarial activity might be to
	 *            add a lot of content. For example to add 1 million zero bytes at
	 *            the end of a file. In that case, the caller would want to exclude
	 *            the length from the calculation.
	 * 
	 * @return the difference computed as per the TLSH algorithm
	 * @throws IllegalArgumentException
	 *             If the given TLSH structure was created with different options;
	 *             hashes can only be compared if they were created with the same
	 *             bucket and checksum options.
	 */
	public int totalDiff(Tlsh other, boolean len_diff) throws IllegalArgumentException {
		if (checksum.length != other.checksum.length || codes.length != other.codes.length) {
			throw new IllegalArgumentException(
					"Given TLSH structure was created with different options from this hash and cannot be compared");
		}

		int diff = 0;

		if (len_diff) {
			int ldiff = TlshUtil.mod_diff(Lvalue, other.Lvalue, RANGE_LVALUE);
			if (ldiff == 0)
				diff = 0;
			else if (ldiff == 1)
				diff = 1;
			else
				diff += ldiff * 12;
		}

		int q1diff = TlshUtil.mod_diff(Q1ratio, other.Q1ratio, RANGE_QRATIO);
		if (q1diff <= 1)
			diff += q1diff;
		else
			diff += (q1diff - 1) * 12;

		int q2diff = TlshUtil.mod_diff(Q2ratio, other.Q2ratio, RANGE_QRATIO);
		if (q2diff <= 1)
			diff += q2diff;
		else
			diff += (q2diff - 1) * 12;

		for (int k = 0; k < checksum.length; k++) {
			if (checksum[k] != other.checksum[k]) {
				diff++;
				break;
			}
		}

		diff += TlshUtil.h_distance(codes, other.codes);

		return diff;
	}

}