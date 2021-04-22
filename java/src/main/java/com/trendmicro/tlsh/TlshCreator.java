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

import java.util.Arrays;

/**
 * A class for turning bytes into a TLSH structure. Note that TLSH requires a
 * minimum of 256 bytes of input and the input must have a certain amount of
 * variance before a hash can be produced. 
 * <p>
 * Example usage:
 * <pre>
 * TlshCreator tlshCreator = new TlshCreator();
 * byte[] buf = new byte[1024];
 * InputStream is = ...; // however you get your input
 * int bytesRead = is.read(buf, 0, buf.length);
 * while (bytesRead &gt;= 0) {
 *     tlshCreator.update(buf, 0, bytesRead);
 *     bytesRead = is.read(buf, 0, buf.length);
 * }
 * is.close();
 * Tlsh hash = tlshCreator.getHash();
 * System.out.println("Hash is " + hash.getEncoded());
 * </pre>
 */
public class TlshCreator {
	private static final int SLIDING_WND_SIZE = 5;
	private static final int BUCKETS = 256;

	/** Minimum data length before a hash can be produced */
	public static final int MIN_DATA_LENGTH = 256;
	/** Minimum data length before a hash can be produced with the force option */
	public static final int MIN_FORCE_DATA_LENGTH	 = 50;
	/**
	 * The maximum amount of data allowed in a TLSH computation.
	 * Slightly less than 4GiB.
	 */
	static final long MAX_DATA_LENGTH = TlshUtil.MAX_DATA_LENGTH;

	/**
	 * Accumulator buckets. This uses long accumulators and
	 * differs from the C++ implementation which has the
	 * luxury of unsigned int.
	 */
	private final long[] a_bucket;
	private final int[] slide_window;
	private long data_len;

	private final VersionOption version;
	private final int bucketCount;
	
	private final int checksumLength;
	private int checksum;
	private final int[] checksumArray;
	private final int codeSize;

	/**
	 * Standard constructor for computing TLSH with 128 buckets and 1 byte checksum
	 * using the latest version.
	 */
	public TlshCreator() {
		this(BucketOption.BUCKETS_128, ChecksumOption.CHECKSUM_1B, VersionOption.VERSION_4);
	}

	/**
	 * Constructor that allows specifying bucket count and checksum length.
	 * Uses the latest available version of TLSH.
	 * 
	 * @param bucketOption
	 *            specify the bucket count; more buckets can give better difference
	 *            calculations but require more space to store the computed hash
	 * @param checksumOption
	 *            specify the checksum length; longer checksums are more resilient
	 *            but increase hash creation time
	 */
	public TlshCreator(BucketOption bucketOption, ChecksumOption checksumOption) {
		this(bucketOption, checksumOption, VersionOption.VERSION_4);
	}

	/**
	 * Constructor that allows specifying bucket count and checksum length.
	 * 
	 * @param bucketOption
	 *            specify the bucket count; more buckets can give better difference
	 *            calculations but require more space to store the computed hash
	 * @param checksumOption
	 *            specify the checksum length; longer checksums are more resilient
	 *            but increase hash creation time
	 * @param versionOption
	 *            what version of TLSH is being generated. It is usually best to
	 *            use the latest versionOption unless you need an older versionOption for
	 *            compatibility reasons.
	 */
	public TlshCreator(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption) {
		version = versionOption;
		bucketCount = bucketOption.getBucketCount();
		codeSize = bucketCount >> 2; // Each bucket contributes 2 bits to output code
		checksumLength = checksumOption.getChecksumLength();

		slide_window = new int[SLIDING_WND_SIZE];
		a_bucket = new long[BUCKETS];

		if (checksumLength > 1) {
			checksumArray = new int[checksumLength];
		} else {
			checksumArray = null;
		}
	}

	/**
	 * Add all the data in the given array to the current hash
	 * 
	 * @param data
	 *  The data to hash
	 */
	public void update(byte[] data) {
		update(data, 0, data.length);
	}

	/**
	 * Add data in the given array to the current hash.
	 * 
	 * @param data
	 *    that data to add
	 * @param offset
	 *    the offset in the array to start reading from
	 * @param len
	 *    how many bytes to read from the array
	 *    
	 * @throws IllegalStateException
	 *    if more than {@link #MAX_DATA_LENGTH} bytes have been hashed.
	 *    TLSH is not intended for use on huge files.
	 */
	public void update(byte[] data, int offset, int len) {
		final int RNG_SIZE = SLIDING_WND_SIZE;
		// #define RNG_IDX(i) ((i+RNG_SIZE)%RNG_SIZE)

		// Indexes into the sliding window. They cycle like
		// 0 4 3 2 1
		// 1 0 4 3 2
		// 2 1 0 4 3
		// 3 2 1 0 4
		// 4 3 2 1 0
		// 0 4 3 2 1
		// and so on
		int j = (int)(data_len % RNG_SIZE);
		int j_1 = (j - 1 + RNG_SIZE) % RNG_SIZE;
		int j_2 = (j - 2 + RNG_SIZE) % RNG_SIZE;
		int j_3 = (j - 3 + RNG_SIZE) % RNG_SIZE;
		int j_4 = (j - 4 + RNG_SIZE) % RNG_SIZE;
		
		long fed_len = data_len;

		for (int i = offset; i < offset + len; i++, fed_len++) {
			slide_window[j] = data[i] & 0xFF;

			if (fed_len >= 4) {
				// only calculate when input >= 5 bytes

				checksum = TlshUtil.b_mapping(0, slide_window[j], slide_window[j_1], checksum);
				if (checksumLength > 1) {
					checksumArray[0] = checksum;
					for (int k = 1; k < checksumLength; k++) {
						// use calculated 1 byte checksums to expand the total checksum to 3 bytes
						checksumArray[k] = TlshUtil.b_mapping(checksumArray[k - 1], slide_window[j],
								slide_window[j_1], checksumArray[k]);
					}
				}

				int r;
				r = TlshUtil.b_mapping(2, slide_window[j], slide_window[j_1], slide_window[j_2]);
				a_bucket[r]++;
				r = TlshUtil.b_mapping(3, slide_window[j], slide_window[j_1], slide_window[j_3]);
				a_bucket[r]++;
				r = TlshUtil.b_mapping(5, slide_window[j], slide_window[j_2], slide_window[j_3]);
				a_bucket[r]++;
				r = TlshUtil.b_mapping(7, slide_window[j], slide_window[j_2], slide_window[j_4]);
				a_bucket[r]++;
				r = TlshUtil.b_mapping(11, slide_window[j], slide_window[j_1], slide_window[j_4]);
				a_bucket[r]++;
				r = TlshUtil.b_mapping(13, slide_window[j], slide_window[j_3], slide_window[j_4]);
				a_bucket[r]++;
			}
			// rotate the sliding window indexes
			int j_tmp = j_4;
			j_4 = j_3;
			j_3 = j_2;
			j_2 = j_1;
			j_1 = j;
			j = j_tmp;
		}
		data_len += len;
		
		if (data_len > MAX_DATA_LENGTH) {
			throw new IllegalStateException("Too much data has been hashed");
		}
	}

	/**
	 * Find quartiles based on current buckets.
	 */
	private long[] find_quartile() {
		long[] bucket_copy = Arrays.copyOf(a_bucket, bucketCount);
		int[] short_cut_left = new int[bucketCount];
		int[] short_cut_right = new int[bucketCount];
		int spl = 0;
		int spr = 0;
		final int quartile = bucketCount >> 2;
		final int p1 = quartile - 1;
		final int p2 = p1 + quartile;
		final int p3 = p2 + quartile;
		final int end = p3 + quartile;

		long q1 = 0;
		long q2 = 0;
		long q3 = 0;
		
		// Implementation note: a much simpler implementation would be
		// to do Arrays.sort(bucket_copy), which only adds a microsecond
		// or two to the execution time. 

		// performs a partial quicksort on the bucket copy, until the
		// the chosen pivot point matches equals the p2 quartile boundary
		for (int l = 0, r = end;;) {
			int ret = partition(bucket_copy, l, r);
			if (ret > p2) {
				r = ret - 1;
				short_cut_right[spr] = ret;
				spr++;
			} else if (ret < p2) {
				l = ret + 1;
				short_cut_left[spl] = ret;
				spl++;
			} else {
				q2 = bucket_copy[p2];
				break;
			}
		}

		short_cut_left[spl] = p2 - 1;
		short_cut_right[spr] = p2 + 1;

		// repeat the partial quicksort process on the lower half of the
		// array until the p1 boundary is found
		for (int i = 0, l = 0; i <= spl; i++) {
			int r = short_cut_left[i];
			if (r > p1) {
				for (;;) {
					int ret = partition(bucket_copy, l, r);
					if (ret > p1) {
						r = ret - 1;
					} else if (ret < p1) {
						l = ret + 1;
					} else {
						q1 = bucket_copy[p1];
						break;
					}
				}
				break;
			} else if (r < p1) {
				l = r;
			} else {
				q1 = bucket_copy[p1];
				break;
			}
		}

		// repeat the partial quicksort process on the upper half of the
		// array until the p3 boundary is found
		for (int i = 0, r = end; i <= spr; i++) {
			int l = short_cut_right[i];
			if (l < p3) {
				for (;;) {
					int ret = partition(bucket_copy, l, r);
					if (ret > p3) {
						r = ret - 1;
					} else if (ret < p3) {
						l = ret + 1;
					} else {
						q3 = bucket_copy[p3];
						break;
					}
				}
				break;
			} else if (l > p3) {
				r = l;
			} else {
				q3 = bucket_copy[p3];
				break;
			}
		}

		return new long[] { q1, q2, q3 };
	}

	private int partition(long[] buf, int left, int right) {
		// #define SWAP_UINT(x,y) do {\
		// unsigned int int_tmp = (x); \
		// (x) = (y); \
		// (y) = int_tmp; } while(0)
		if (left == right) {
			return left;
		}
		if (left + 1 == right) {
			if (buf[left] > buf[right]) {
				// SWAP_UINT( buf[left], buf[right] );
				long int_tmp = buf[left];
				buf[left] = buf[right];
				buf[right] = int_tmp;
			}
			return left;
		}

		int ret = left, pivot = (left + right) >> 1;

		long val = buf[pivot];
		buf[pivot] = buf[right];
		buf[right] = val;

		for (int i = left; i < right; i++) {
			if (buf[i] < val) {
				// SWAP_UINT( buf[ret], buf[i] );
				long int_tmp = buf[ret];
				buf[ret] = buf[i];
				buf[i] = int_tmp;

				ret++;
			}
		}
		buf[right] = buf[ret];
		buf[ret] = val;

		return ret;
	}

	/**
	 * Get the computed TLSH structure, or <code>null</code> if not enough data has
	 * been processed.
	 * 
	 * @return the computed TLSH structure, or <code>null</code> if not enough data
	 *         has been processed.
	 * @see #getHash(boolean)
	 */
	public Tlsh getHashNoThrow() {
		try {
			return getHash(false);
		} catch (IllegalStateException e) {
			// completely eat any exceptions
			return null;
		}
	}

	/**
	 * Get the computed TLSH structure. This call will succeed if and only if
	 * {@link #isValid()} returns true
	 * 
	 * @return the computed TLSH structure
	 *         
	 * @throws IllegalStateException
	 *             If a hash cannot be product because too little data has been
	 *             seen.
	 *             
	 * @see #getHash(boolean)
	 */
	public Tlsh getHash() {
		return getHash(false);
	}

	/**
	 * Get the computed TLSH structure. This call will succeed if and only if
	 * {@link #isValid(boolean)} returns true.
	 * <p>
	 * Note that this call does not automatically call {@link #reset()}, so any
	 * subsequent calls to {@link #update(byte[], int, int)} will be incrementally
	 * added to the current hash computation and not a new hash.
	 * 
	 * @param force
	 *            attempt to force hash creation even if not enough data has been
	 *            hashed. This is not guaranteed to produce output.
	 * 
	 * @return the computed TLSH structure
	 * 
	 * @throws IllegalStateException
	 *             If a hash cannot be product because too little data has been
	 *             seen.
	 */
	public Tlsh getHash(boolean force) {
		if (!isValid(force)) {
			throw new IllegalStateException("TLSH not valid; either not enough data or data has too little variance");
		}

		long q1, q2, q3;
		long[] quartiles = find_quartile();
		q1 = quartiles[0];
		q2 = quartiles[1];
		q3 = quartiles[2];
		
		// issue #79 - divide by 0 if q3 == 0
		// This should already by caught by isValid but an extra check here just in case
		if (q3 == 0) {
			throw new IllegalStateException("TLSH not valid; too little variance in the data");
		}

		int[] tmp_code = new int[codeSize];
		for (int i = 0; i < codeSize; i++) {
			int h = 0;
			for (int j = 0; j < 4; j++) {
				long k = a_bucket[4 * i + j];
				if (q3 < k) {
					h += 3 << (j * 2);
				} else if (q2 < k) {
					h += 2 << (j * 2);
				} else if (q1 < k) {
					h += 1 << (j * 2);
				}
			}
			tmp_code[i] = h;
		}

		int lvalue = TlshUtil.l_capturing(data_len);
		int q1ratio = (int) ((float) (q1 * 100.0f) / (float) q3) & 0xF;
		int q2ratio = (int) ((float) (q2 * 100.0f) / (float) q3) & 0xF;

		if (checksumLength == 1) {
			return new Tlsh(version, new int[] {checksum}, lvalue, q1ratio, q2ratio, tmp_code);
		} else {
			return new Tlsh(version, checksumArray.clone(), lvalue, q1ratio, q2ratio, tmp_code);
		}
	}

	/**
	 * Reset the hasher so that it can be used to create a new hash
	 */
	public void reset() {
		Arrays.fill(slide_window, 0);
		Arrays.fill(a_bucket, 0);
		checksum = 0;
		if (checksumArray != null) {
			Arrays.fill(checksumArray, 0);
		}
		data_len = 0;
	}

	/**
	 * Check if enough data has been processed to produce a TLSH structure. TLSH
	 * requires a minimum amount of data, and the data must have a certain amount of
	 * variance, before a hash can be created.
	 * 
	 * @return if enough data has been processed to produce a TLSH structure
	 */
	public boolean isValid() {
		return isValid(false);
	}

	/**
	 * Check if enough data has been processed to produce a TLSH structure. TLSH
	 * requires a minimum amount of data, and the data must have a certain amount of
	 * variance, before a hash can be created.
	 * 
	 * @param force
	 *            Optional parameter to try to force hash creation even if
	 *            insufficient data has been seen.
	 * 
	 * @return if enough data has been processed to produce a TLSH structure
	 */
	public boolean isValid(boolean force) {
		// incoming data must be more than or equal to MIN_DATA_LENGTH bytes
		if (data_len < MIN_FORCE_DATA_LENGTH || (!force && data_len < MIN_DATA_LENGTH)) {
			return false;
		}

		// buckets must be more than 50% non-zero
		int nonzero = 0;
		for (int i = 0; i < bucketCount; i++) {
			if (a_bucket[i] > 0) nonzero++;
		}
		if (nonzero <= (bucketCount >> 1)) {
			return false;
		}
		if (data_len > MAX_DATA_LENGTH) {
			return false;
		}

		return true;
	}

}
