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
	/// <summary>
	/// Computes the TLSH hash for the input data.
	/// </summary>
	public class Tlsh
	{
		private const int SlidingWndSize = 5;
		private const int Buckets = 256;

		/// <summary>
		/// Minimum data length before a hash can be produced
		/// </summary>
		public const int MinDataLength = 256;

		/// <summary>
		/// Minimum data length before a hash can be produced with the force option
		/// </summary>
		public const int MinForceDataLength = 50;

		/// <summary>
		/// The maximum amount of data allowed in a TLSH computation. Slightly less than 4GiB.
		/// </summary>
		public static readonly uint MaxDataLength = TlshUtil.MaxDataLength;

		private readonly uint[] _ABucket;
		private readonly byte[] _SlideWindow;
		private uint _DataLen;

		private readonly VersionOption _Version;
		private readonly int _BucketCount;

		private readonly int _ChecksumLength;
		private byte _Checksum;
		private readonly byte[] _ChecksumArray;
		private readonly int _CodeSize;
		
		/// <summary>
		/// Initializes a new instance of the <see cref="Tlsh"/> class using recommended default values.
		/// </summary>
		public Tlsh(): this(BucketOption.Default, ChecksumOption.OneByte, VersionOption.Version4) { }

	
		/// <summary>
		/// Initializes a new instance of the <see cref="Tlsh"/> class using the specified values.
		/// </summary>
		/// <param name="bucketOption">Specifies the bucket count; more buckets can give better difference calculations but require more space to store the computed hash.</param>
		/// <param name="checksumOption">Specifies the checksum length; longer checksums are more resilient but increase hash creation time.</param>
		/// <param name="versionOption">The version of TLSH to use. It is usually best to use the latest version, unless you need an older version for compatibility reasons.</param>
		public Tlsh(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption)
		{
			_Version = versionOption;
			_BucketCount = (int) bucketOption;
			_CodeSize = _BucketCount >> 2; // Each bucket contributes 2 bits to output code
			_ChecksumLength = (int) checksumOption;

			_SlideWindow = new byte[SlidingWndSize];
			_ABucket = new uint[Buckets];

			_ChecksumArray = _ChecksumLength > 1 ? new byte[_ChecksumLength] : null;
		}


		/// <summary>
		/// Add all the data in the given array to the current hash
		/// </summary>
		/// <param name="data">The data to add.</param>
		public void Update(byte[] data) => Update(data, 0, (uint)data.Length);


		/// <summary>
		/// Add data in the given array to the current hash.
		/// </summary>
		/// <param name="data">The data to add.</param>
		/// <param name="offset">The offset in the array to start reading from.</param>
		/// <param name="length">How many bytes to read from the array.</param>
		/// <exception cref="InvalidOperationException">The data hashed exceeds <see cref="MaxDataLength"/>.</exception>
		public void Update(byte[] data, int offset, uint length)
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

			
			// The variables are all local for performance reasons.
			var fedLen = _DataLen;
			var checksumArray = _ChecksumArray;
			var slideWindow = _SlideWindow;
			var aBucket = _ABucket;
			var checksum = _Checksum;
			var checksumLength = _ChecksumLength;

			for (var i = offset; i < offset + length; i++, fedLen++)
			{
				slideWindow[j] = (byte)(data[i] & 0xFF);

				if (fedLen >= 4)
				{
					// only calculate when input >= 5 bytes

					ref var slj = ref slideWindow[j];
					ref var slj1 = ref slideWindow[j1];
					checksum = TlshUtil.PearsonHash(0, slj, slj1, checksum);
					if (checksumLength > 1)
					{
						checksumArray[0] = checksum;
						for (var k = 1; k < checksumLength; k++)
						{
							// use calculated 1 byte checksums to expand the total checksum to 3 bytes
							checksumArray[k] = TlshUtil.PearsonHash(checksumArray[k - 1], slj, slj1, checksumArray[k]);
						}
					}

					ref var slj2 = ref slideWindow[j2];
					var r = TlshUtil.PearsonHash(2, slj, slj1, slj2);
					aBucket[r]++;
					ref var slj3 = ref slideWindow[j3];
					r = TlshUtil.PearsonHash(3, slj, slj1, slj3);
					aBucket[r]++;
					r = TlshUtil.PearsonHash(5, slj, slj2, slj3);
					aBucket[r]++;
					ref var slj4 = ref slideWindow[j4];
					r = TlshUtil.PearsonHash(7, slj, slj2, slj4);
					aBucket[r]++;
					r = TlshUtil.PearsonHash(11, slj, slj1, slj4);
					aBucket[r]++;
					r = TlshUtil.PearsonHash(13, slj, slj3, slj4);
					aBucket[r]++;
				}

				// rotate the sliding window indexes
				var jTmp = j4;
				j4 = j3;
				j3 = j2;
				j2 = j1;
				j1 = j;
				j = jTmp;
			}

			_Checksum = checksum;
			_DataLen += length;

			if (_DataLen > MaxDataLength)
			{
				throw new InvalidOperationException("Too much data has been hashed");
			}
		}

		/// <summary>
		/// Find quartiles based on current buckets.
		/// </summary>
		/// <returns></returns>
		private (uint, uint, uint) FindQuartile()
		{
			var bucketCopy = new uint[_BucketCount];
			Array.Copy(_ABucket, bucketCopy, _BucketCount);
			var quartile = _BucketCount >> 2;
			var p1 = quartile - 1;
			var p2 = p1 + quartile;
			var p3 = p2 + quartile;

			Array.Sort(bucketCopy);

			return (bucketCopy[p1], bucketCopy[p2], bucketCopy[p3]);
		}


		/// <summary>
		/// Get the computed TLSH structure. This call will succeed only if <see cref="IsValid"/> returns true. Otherwise, an <see cref="InvalidOperationException"/> will be thrown.
		/// </summary>
		/// <param name="force">Attempt to force hash creation even if not enough data has been hashed. This is not guaranteed to produce output.</param>
		/// <exception cref="InvalidOperationException">If a hash cannot be product because too little data has been seen.</exception>
		public TlshValue GetHash(bool force = false)
		{
			if (TryGetHash(out var result, force)) return result;
			throw new InvalidOperationException("TLSH not valid; either not enough data or data has too little variance");
		}

		
		/// <summary>
		/// Get the computed TLSH structure. This call will succeed only if <see cref="IsValid"/> returns true.
		/// </summary>
		/// <param name="result">If the result is <c>true</c>, this variable will contain the computed TLSH value.</param>
		/// <param name="force">attempt to force hash creation even if not enough data has been hashed. This is not guaranteed to produce output.</param>
		/// <returns><c>true</c>, if the TLSH value could be computed. <c>false</c> otherwise.</returns>
		public bool TryGetHash(out TlshValue result, bool force = false)
		{
			if (!IsValid(force))
			{
				result = default;
				return false;
			}

			var (q1, q2, q3) = FindQuartile();

			// issue #79 - divide by 0 if q3 == 0
			// This should already by caught by isValid but an extra check here just in case
			if (q3 == 0)
			{
				result = default;
				return false;
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
				result = new TlshValue(_Version, new[] { _Checksum }, lvalue, q1Ratio, q2Ratio, tmpCode);
				return true;
			}

			var checkSumCopy = new byte[_ChecksumArray.Length];
			_ChecksumArray.CopyTo(checkSumCopy, 0);
			result = new TlshValue(_Version, checkSumCopy, lvalue, q1Ratio, q2Ratio, tmpCode);
			return true;
		}

		/// <summary>
		/// Reset the hasher so that it can be used to create a new hash
		/// </summary>
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

		/// <summary>
		/// Check if enough data has been processed to produce a TLSH structure. TLSH
		/// requires a minimum amount of data, and the data must have a certain amount of
		/// variance, before a hash can be created.
		/// </summary>
		/// <param name="force">Try to force hash creation even if insufficient data has been seen.</param>
		/// <returns><c>true</c>, if enough data has been processed to produce a TLSH structure. <c>false</c>, otherwise.</returns>
		public bool IsValid(bool force = false)
		{
			// incoming data must be more than or equal to MIN_DATA_LENGTH bytes
			if (_DataLen < MinForceDataLength || !force && _DataLen < MinDataLength)
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