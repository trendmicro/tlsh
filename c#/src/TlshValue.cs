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
using System.Text;

namespace TrendMicro.Tlsh
{

	/// <summary>
	/// A TLSH structure that can be encoded to a hex string or compared to another TLSH structure.
	/// </summary>
	public readonly struct TlshValue : IEquatable<TlshValue>
	{
		/*
		 * Porting notes:
		 * This code is mostly copied from tlsh_impl.h/tlsh_impl.cpp
		 * In particular, this class is lsh_bin_struct with the diff operation
		 * that only really applies to the struct moved here.
		 * Also, the code dispenses with the C-style union code which
		 * doesn't help us in C# land.
		 */

		private const int RangeLvalue = 256;
		private const int RangeQRatio = 16;


		/// <summary>
		/// Creates a TLSH instance from a string.
		/// </summary>
		/// <param name="input">The input string to parse from.</param>
		/// <returns>The parsed TLSH structure.</returns>
		/// <exception cref="ArgumentException">If the input string cannot be parsed.</exception>
		public static TlshValue Parse(string input)
		{
			(int[] checksum, int[] tmpCode, VersionOption? versionOption) Initialize()
			{
				foreach (BucketOption bucketOption in AllBucketOptions)
				{
					foreach (ChecksumOption checksumOption in AllChecksumOptions)
					{
						if (TryParse(input, bucketOption, checksumOption, VersionOption.Original, out var checksum, out var tmpCode, out var versionOption)) return (checksum, tmpCode, versionOption);
						if (TryParse(input, bucketOption, checksumOption, VersionOption.Version4, out checksum, out tmpCode, out versionOption)) return (checksum, tmpCode, versionOption);
					}
				}

				return (default, default, default);
			}

			static bool TryParse(string value, BucketOption bucketOption, ChecksumOption checksumOption, VersionOption tryVersion, out int[] ints, out int[] tmpCode, out VersionOption? actualVersion)
			{

				if (value.Length != HashStringLength(bucketOption, checksumOption, tryVersion))
				{
					ints = default;
					tmpCode = default;
					actualVersion = default;
					return false;
				}

				ints = new int[(int)checksumOption];
				tmpCode = new int[(int)bucketOption / 4];
				actualVersion = tryVersion;
				return true;

			}

			var (checksum, tmpCode, versionOption) = Initialize();

			if (checksum == null)
			{
				throw new ArgumentException("Invalid hash string, length does not match any known encoding", nameof(input));
			}

			var offset = versionOption.Value.VersionString.Length;
			for (var k = 0; k < checksum.Length; k++)
			{
				checksum[k] = TlshUtil.FromHexSwapped(input, offset);
				offset += 2;
			}

			var lvalue = TlshUtil.FromHexSwapped(input, offset);
			offset += 2;

			var qRatios = TlshUtil.FromHex(input, offset);
			offset += 2;

			for (var i = 0; i < tmpCode.Length; i++)
			{
				// un-reverse the code during encoding
				tmpCode[tmpCode.Length - i - 1] = TlshUtil.FromHex(input, offset);
				offset += 2;
			}

			return new TlshValue(versionOption.Value, checksum, lvalue, qRatios >> 4, qRatios & 0xF, tmpCode);
		}

		private static int HashStringLength(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption) => versionOption.VersionString.Length + ((int)bucketOption / 2) + ((int)checksumOption * 2) + 4;


		/// <summary>
		/// Returns the version of this <see cref="Tlsh"/> value.
		/// </summary>
		public VersionOption Version { get; }
		private readonly int[] _Checksum; // 1 or 3 bytes
		private readonly int _Lvalue; // 1 byte
		private readonly int _Q1Ratio; // 4 bits
		private readonly int _Q2Ratio; // 4 bits
		private readonly int[] _Codes; // 32/64 bytes
		private static readonly Array AllChecksumOptions = Enum.GetValues(typeof(ChecksumOption));
		private static readonly Array AllBucketOptions = Enum.GetValues(typeof(BucketOption));

		internal TlshValue(VersionOption versionOption, int[] checksum, int lvalue, int q1Ratio, int q2Ratio, int[] codes)
		{
			Version = versionOption;
			_Checksum = checksum;
			_Lvalue = lvalue;
			_Q1Ratio = q1Ratio;
			_Q2Ratio = q2Ratio;
			_Codes = codes;
		}

		/// <inheritdoc />
		public override string ToString() => GetEncoded();
		
		private string GetEncoded()
		{
			// The C++ code reverses the order of some of the fields before
			// converting to hex, so copy that behaviour.
			var sb = new StringBuilder(HashStringLength());
			sb.Append(Version.VersionString);

			foreach (var entry in _Checksum)
			{
				TlshUtil.ToHexSwapped(entry, sb);
			}

			TlshUtil.ToHexSwapped(_Lvalue, sb);
			TlshUtil.ToHex(_Q1Ratio << 4 | _Q2Ratio, sb);
			for (var i = 0; i < _Codes.Length; i++)
			{
				// reverse the code during encoding
				TlshUtil.ToHex(_Codes[_Codes.Length - 1 - i], sb);
			}

			return sb.ToString();
		}

		/// <summary>
		/// Calculate how long the output hex string will be
		/// </summary>
		/// <returns>The length of the hex string.</returns>
		private int HashStringLength() => 
			// extra 4 characters come from length and Q1 and Q2 ratio.
			Version.VersionString.Length + _Codes.Length * 2 + _Checksum.Length * 2 + 4;

		/// <summary>
		/// Calculates the difference to another TLSH hash.
		/// </summary>
		/// <param name="other">The hash to compute the difference to.</param>
		/// <param name="includeFileLengthDifference">Specifies if the file length is to be
		///   included in the difference calculation. In general, the length should be
		///       considered in the difference calculation, but there could be
		///   applications where a part of the adversarial activity might be to
		///       add a lot of content. For example to add 1 million zero bytes at
		///   the end of a file. In that case, the caller would want to exclude
		///   the length from the calculation.</param>
		/// <returns>The difference computed as per the TLSH algorithm. Should be between 0 and 2000.</returns>
		/// <exception cref="ArgumentException">If the given TLSH structure was created with different options; hashes can only be compared if they were created with the same bucket and checksum options</exception>
		public int DistanceTo(TlshValue other, bool includeFileLengthDifference)
		{
			if (_Checksum.Length != other._Checksum.Length || _Codes.Length != other._Codes.Length) throw new ArgumentException("Given TLSH structure was created with different options from this hash and cannot be compared", nameof(other));

			var diff = 0;

			if (includeFileLengthDifference)
			{
				var ldiff = TlshUtil.ModDiff(_Lvalue, other._Lvalue, RangeLvalue);
				if (ldiff == 0)
					diff = 0;
				else if (ldiff == 1)
					diff = 1;
				else
					diff += ldiff * 12;
			}

			var q1diff = TlshUtil.ModDiff(_Q1Ratio, other._Q1Ratio, RangeQRatio);
			if (q1diff <= 1)
				diff += q1diff;
			else
				diff += (q1diff - 1) * 12;

			var q2diff = TlshUtil.ModDiff(_Q2Ratio, other._Q2Ratio, RangeQRatio);
			if (q2diff <= 1)
				diff += q2diff;
			else
				diff += (q2diff - 1) * 12;

			for (var k = 0; k < _Checksum.Length; k++)
			{
				if (_Checksum[k] != other._Checksum[k])
				{
					diff++;
					break;
				}
			}

			diff += TlshUtil.HDistance(_Codes, other._Codes);

			return diff;
		}

		/// <inheritdoc />
		public bool Equals(TlshValue other)
		{
			if (!(Equals(_Checksum, other._Checksum) && _Lvalue == other._Lvalue && _Q1Ratio == other._Q1Ratio && _Q2Ratio == other._Q2Ratio && Version.Equals(other.Version) && _Codes.Length == other._Codes.Length)) return false;
			for (var i = 0; i < _Codes.Length; i++)
			{
				if (_Codes[i] != other._Codes[i]) return false;
			}

			return false;
		}

		/// <inheritdoc />
		public override bool Equals(object obj) => obj is TlshValue other && Equals(other);

		/// <inheritdoc />
		public override int GetHashCode()
		{
			unchecked
			{
				var hashCode = (_Checksum != null ? _Checksum.GetHashCode() : 0);
				hashCode = (hashCode * 397) ^ _Lvalue;
				hashCode = (hashCode * 397) ^ _Q1Ratio;
				hashCode = (hashCode * 397) ^ _Q2Ratio;
				hashCode = (hashCode * 397) ^ (_Codes != null ? _Codes.GetHashCode() : 0);
				hashCode = (hashCode * 397) ^ Version.GetHashCode();
				return hashCode;
			}
		}

		/// <inheritdoc />
		public static bool operator ==(TlshValue left, TlshValue right) => left.Equals(right);

		/// <inheritdoc />
		public static bool operator !=(TlshValue left, TlshValue right) => !left.Equals(right);
	}
}