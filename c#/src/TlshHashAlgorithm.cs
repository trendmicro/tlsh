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
using System.Security.Cryptography;

namespace TrendMicro.Tlsh
{
	/// <summary>
	/// Computes the TLSH hash for the input data.
	/// </summary>
	public class TlshHashAlgorithm : HashAlgorithm
	{
		private readonly Tlsh _Tlsh;
		private readonly BucketOption _BucketOption;
		private readonly ChecksumOption _ChecksumOption;
		private readonly bool _ForceHashCreation;
		
		/// <summary>
		/// Initializes a new instance of the <see cref="Tlsh"/> class using recommended default values.
		/// </summary>
		public TlshHashAlgorithm(): this(BucketOption.Default, ChecksumOption.OneByte, VersionOption.Version4) { }

		/// <summary>
		/// Initializes a new instance of the <see cref="Tlsh"/> class using the specified values.
		/// </summary>
		/// <param name="bucketOption">Specifies the bucket count; more buckets can give better difference calculations but require more space to store the computed hash.</param>
		/// <param name="checksumOption">Specifies the checksum length; longer checksums are more resilient but increase hash creation time.</param>
		/// <param name="versionOption">The version of TLSH to use. It is usually best to use the latest version, unless you need an older version for compatibility reasons.</param>
		/// <param name="forceHashCreation">Attempt to force hash creation even if not enough data has been hashed. This is not guaranteed to produce output.</param>
		public TlshHashAlgorithm(BucketOption bucketOption, ChecksumOption checksumOption, VersionOption versionOption, bool forceHashCreation = false)
		{
			_Tlsh = new Tlsh(bucketOption, checksumOption, versionOption);
			_BucketOption = bucketOption;
			_ChecksumOption = checksumOption;
			_ForceHashCreation = forceHashCreation;
			HashSize = ((int)_ChecksumOption + 1 + 1 + (int)_BucketOption / 4) * 8;
		}

		/// <inheritdoc />
		public override int HashSize { get; }

		/// <inheritdoc />
		protected override void HashCore(byte[] array, int ibStart, int cbSize) => _Tlsh.Update(array, ibStart, (uint) cbSize);

		/// <inheritdoc />
		protected override byte[] HashFinal() => _Tlsh.TryGetHash(out var hash, _ForceHashCreation) ? hash.ToByteArray() : Array.Empty<byte>();

		/// <inheritdoc />
		public override void Initialize() => _Tlsh.Reset();
	}
}