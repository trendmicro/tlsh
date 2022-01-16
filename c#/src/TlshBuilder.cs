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

namespace TrendMicro.Tlsh
{

	/**
 * A class for turning bytes into a TLSH structure. Note that TLSH requires a
 * minimum of 256 bytes of input and the input must have a certain amount of
 * variance before a hash can be produced. 
 * <p>
 * Example usage:
 * <pre>
 * TlshBuilder tlshCreator = new TlshBuilder();
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
	public class TlshBuilder
	{
		private const int SlidingWndSize = 5;
		private const int Buckets = 256;

		/** Minimum data length before a hash can be produced */
		public const int MinDataLength = 256;

		/** Minimum data length before a hash can be produced with the force option */
		public const int MinForceDataLength = 50;

		/**
	 * The maximum amount of data allowed in a TLSH computation.
	 * Slightly less than 4GiB.
	 */
		public static readonly long MaxDataLength = TlshUtil.MaxDataLength;

		/**
	 * Accumulator buckets. This uses long accumulators and
	 * differs from the C++ implementation which has the
	 * luxury of unsigned int.
	 */
		private readonly long[] _ABucket;
		private readonly int[] _SlideWindow;
		private long _DataLen;

		private readonly VersionOption _Version;
		private readonly int _BucketCount;

		private readonly int _ChecksumLength;
		private int _Checksum;
		private readonly int[] _ChecksumArray;
		private readonly int _CodeSize;

		/**
	 * Standard constructor for computing TLSH with 128 buckets and 1 byte checksum
	 * using the latest version.
	 */
		public TlshBuilder(): this(BucketOption.Default, ChecksumOption.OneByte, VersionOption.Version4) { }

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
		public TlshBuilder(BucketOption bucketOption, ChecksumOption checksumOption): this(bucketOption, checksumOption, VersionOption.Version4) { }

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
		public TlshBuilder(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption)
		{
			_Version = versionOption;
			_BucketCount = (int) bucketOption;
			_CodeSize = _BucketCount >> 2; // Each bucket contributes 2 bits to output code
			_ChecksumLength = (int) checksumOption;

			_SlideWindow = new int[SlidingWndSize];
			_ABucket = new long[Buckets];

			_ChecksumArray = _ChecksumLength > 1 ? new int[_ChecksumLength] : null;
		}

		/**
	 * Add all the data in the given array to the current hash
	 * 
	 * @param data
	 *  The data to hash
	 */
		public void Update(byte[] data)
		{
			Update(data, 0, data.Length);
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
		public void Update(byte[] data, int offset, int len)
		{
			const int rngSize = SlidingWndSize;
			// #define RNG_IDX(i) ((i+RNG_SIZE)%RNG_SIZE)

			// Indexes into the sliding window. They cycle like
			// 0 4 3 2 1
			// 1 0 4 3 2
			// 2 1 0 4 3
			// 3 2 1 0 4
			// 4 3 2 1 0
			// 0 4 3 2 1
			// and so on
			var j = (int)(_DataLen % rngSize);
			var j1 = (j - 1 + rngSize) % rngSize;
			var j2 = (j - 2 + rngSize) % rngSize;
			var j3 = (j - 3 + rngSize) % rngSize;
			var j4 = (j - 4 + rngSize) % rngSize;

			
			var fedLen = _DataLen;

			for (var i = offset; i < offset + len; i++, fedLen++)
			{
				var slideWindow = _SlideWindow;
				slideWindow[j] = data[i] & 0xFF;

				if (fedLen >= 4)
				{
					// only calculate when input >= 5 bytes

					ref var slj = ref slideWindow[j];
					ref var slj1 = ref slideWindow[j1];
					_Checksum = TlshUtil.PearsonHash(0, slj, slj1, _Checksum);
					if (_ChecksumLength > 1)
					{
						_ChecksumArray[0] = _Checksum;
						for (var k = 1; k < _ChecksumLength; k++)
						{
							// use calculated 1 byte checksums to expand the total checksum to 3 bytes
							_ChecksumArray[k] = TlshUtil.PearsonHash(_ChecksumArray[k - 1], slj,
								slj1, _ChecksumArray[k]);
						}
					}

					ref var slj2 = ref slideWindow[j2];
					var r = TlshUtil.PearsonHash(2, slj, slj1, slj2);
					_ABucket[r]++;
					ref var slj3 = ref slideWindow[j3];
					r = TlshUtil.PearsonHash(3, slj, slj1, slj3);
					_ABucket[r]++;
					r = TlshUtil.PearsonHash(5, slj, slj2, slj3);
					_ABucket[r]++;
					ref var slj4 = ref slideWindow[j4];
					r = TlshUtil.PearsonHash(7, slj, slj2, slj4);
					_ABucket[r]++;
					r = TlshUtil.PearsonHash(11, slj, slj1, slj4);
					_ABucket[r]++;
					r = TlshUtil.PearsonHash(13, slj, slj3, slj4);
					_ABucket[r]++;
				}

				// rotate the sliding window indexes
				var jTmp = j4;
				j4 = j3;
				j3 = j2;
				j2 = j1;
				j1 = j;
				j = jTmp;
			}

			_DataLen += len;

			if (_DataLen > MaxDataLength)
			{
				throw new InvalidOperationException("Too much data has been hashed");
			}
		}

		/**
	 * Find quartiles based on current buckets.
	 */
		private (long, long, long) find_quartile()
		{
			var bucketCopy = new long[_BucketCount];
			Array.Copy(_ABucket, bucketCopy, _BucketCount);
			var quartile = _BucketCount >> 2;
			var p1 = quartile - 1;
			var p2 = p1 + quartile;
			var p3 = p2 + quartile;

			Array.Sort(bucketCopy);

			return (bucketCopy[p1], bucketCopy[p2], bucketCopy[p3]);
		}

		/**
	 * Get the computed TLSH structure, or <code>null</code> if not enough data has
	 * been processed.
	 * 
	 * @return the computed TLSH structure, or <code>null</code> if not enough data
	 *         has been processed.
	 * @see #getHash(boolean)
	 */
		public Tlsh? GetHashNoThrow()
		{
			try
			{
				return GetHash(false);
			}
			catch (InvalidOperationException)
			{
				// completely eat any exceptions
				return default;
			}
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
		public Tlsh GetHash(bool force = false)
		{
			if (!IsValid(force))
			{
				throw new InvalidOperationException("TLSH not valid; either not enough data or data has too little variance");
			}

			var (q1, q2, q3) = find_quartile();

			// issue #79 - divide by 0 if q3 == 0
			// This should already by caught by isValid but an extra check here just in case
			if (q3 == 0)
			{
				throw new InvalidOperationException("TLSH not valid; too little variance in the data");
			}

			var tmpCode = new int[_CodeSize];
			for (var i = 0; i < _CodeSize; i++)
			{
				var h = 0;
				for (var j = 0; j < 4; j++)
				{
					var k = _ABucket[4 * i + j];
					if (q3 < k)
					{
						h += 3 << (j * 2);
					}
					else if (q2 < k)
					{
						h += 2 << (j * 2);
					}
					else if (q1 < k)
					{
						h += 1 << (j * 2);
					}
				}

				tmpCode[i] = h;
			}

			var lvalue = TlshUtil.LCapturing(_DataLen);
			var q1Ratio = (int)(q1 * 100.0f / q3) & 0xF;
			var q2Ratio = (int)(q2 * 100.0f / q3) & 0xF;

			if (_ChecksumLength == 1)
			{
				return new Tlsh(_Version, new[] { _Checksum }, lvalue, q1Ratio, q2Ratio, tmpCode);
			}

			var checkSumCopy = new int[_ChecksumArray.Length];
			_ChecksumArray.CopyTo(checkSumCopy, 0);
			return new Tlsh(_Version, checkSumCopy, lvalue, q1Ratio, q2Ratio, tmpCode);
		}

		/**
	 * Reset the hasher so that it can be used to create a new hash
	 */
		public void Reset()
		{
			Array.Clear(_SlideWindow, 0, _SlideWindow.Length);
			Array.Clear(_ABucket, 0, _ABucket.Length);
			_Checksum = 0;
			if (_ChecksumArray != null)
			{
				Array.Clear(_ChecksumArray, 0, _ChecksumArray.Length);
			}

			_DataLen = 0;
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
		public bool IsValid(bool force = false)
		{
			// incoming data must be more than or equal to MIN_DATA_LENGTH bytes
			if (_DataLen < MinForceDataLength || (!force && _DataLen < MinDataLength))
			{
				return false;
			}

			// buckets must be more than 50% non-zero
			var nonzero = 0;
			for (var i = 0; i < _BucketCount; i++)
			{
				if (_ABucket[i] > 0) nonzero++;
			}

			return nonzero > _BucketCount >> 1 && _DataLen <= MaxDataLength;
		}

	}
}