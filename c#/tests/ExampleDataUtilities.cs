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
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;

namespace TrendMicro.Tlsh
{

	/**
 * Utilities to extract the known answers from the example data
 */
	public class ExampleDataUtilities
	{

		/**
	 * Return the root of the tlsh source project. Can be overridden
	 * by setting the tlsh.root system property.
	 */
		public static string? GetSourceRoot()
		{
			var sourceRoot = Directory.GetCurrentDirectory();
			do
			{
				// test for existence of expected file. Not bullet-proof, but okay.
				var testingFolder = Path.Combine(sourceRoot, "Testing");
				var license = Path.Combine(sourceRoot, "LICENSE");
				if (Directory.Exists(testingFolder) && File.Exists(license))
				{
					return sourceRoot;
				}

			} while ((sourceRoot = Path.GetDirectoryName(sourceRoot)) != null);

			throw new FileNotFoundException("Cannot find source root");
		}

		/**
	 * Get a map from absolute canonical file path to expected TLSH hash value for
	 * the file at that path
	 * @param sourceRoot
	 *    The root of the TLSH project. Files are expected to be found
	 *    relative to this path.
	 * @param hashLength
	 *    The length of the TLSH hash
	 * @param checksumLength
	 *    The size of the TLSH checksum
	 * @return
	 */
		public static Dictionary<string, string> GetExpectedHashes(string? sourceRoot, int hashLength, int checksumLength)
		{
			var hashes = new Dictionary<string, string>();
			var expectedFile = Path.Combine(sourceRoot, "Testing", "exp", $"example_data.{hashLength}.{checksumLength}.len.out_EXP");
			// Files referenced in the expected outputs files are relative to this folder,
			// not the data file itself
			var relativeTo = Path.GetDirectoryName(Path.GetDirectoryName(expectedFile));
			using var br = new StreamReader(expectedFile);
			var line = br.ReadLine();
			while (line != null)
			{
				var parts = Regex.Split(line.Trim(), "\\s+");
				if (parts.Length == 2)
				{
					var expectedHash = parts[0];
					var relativeFile = parts[1];
					var absoluteFile = Path.GetFullPath(relativeFile, relativeTo);
					if (!File.Exists(absoluteFile))
					{
						throw new FileNotFoundException($"File {absoluteFile} is referenced in data file but cannot be found");
					}

					hashes[absoluteFile] = expectedHash;
				}

				line = br.ReadLine();
			}

			return hashes;
		}

		/**
	 * A basic struct-type class
	 */
		public readonly struct FilesAndDiff
		{
			public string SourceFile { get; }
			public string TargetFile { get; }
			public int ExpectedDiff { get; }

			public FilesAndDiff(string sourceFile, string targetFile, int expectedDiff)
			{
				SourceFile = sourceFile;
				TargetFile = targetFile;
				ExpectedDiff = expectedDiff;
			}
		}

		public static List<FilesAndDiff> GetExpectedDiffScores(string? sourceRoot, int hashLength, int checksumLength, bool includeLength)
		{
			var diffs = new List<FilesAndDiff>();
			// Example file name: example_data.128.1.xlen.xref.scores_EXP
			var expectedFile = Path.Combine(sourceRoot, "Testing", "exp", $"example_data.{hashLength}.{checksumLength}.{(includeLength ? "len" : "xlen")}.xref.scores_EXP");
			// Files referenced in the expected outputs files are relative to this folder,
			// not the data file itself
			var relativeTo = Path.GetDirectoryName(Path.GetDirectoryName(expectedFile));
			using var br = new StreamReader(expectedFile);
			var line = br.ReadLine();
			while (line != null)
			{
				var parts = Regex.Split(line.Trim(), "\\s+");
				if (parts.Length == 3)
				{
					var sourceFile = parts[0];
					var targetFile = parts[1];
					var diff = parts[2];
					var absoluteSourceFile = Path.GetFullPath(sourceFile, relativeTo);
					if (!File.Exists(absoluteSourceFile))
					{
						throw new FileNotFoundException($"File {absoluteSourceFile} is referenced in data file but cannot be found");
					}

					var absoluteTargetFile = Path.GetFullPath(targetFile, relativeTo);
					if (!File.Exists(absoluteTargetFile))
					{
						throw new FileNotFoundException($"File {absoluteTargetFile} is referenced in data file but cannot be found");
					}

					diffs.Add(new FilesAndDiff(absoluteSourceFile, absoluteTargetFile, int.Parse(diff)));
				}

				line = br.ReadLine();
			}

			return diffs;
		}

		private static readonly Dictionary<string, byte[]> FileCache = new();

		/**
	 * Read an entire file into a byte array and cache it.
	 * Not safe for large files.
	 */
		public static byte[] GetFileBytes(string exampleFile)
		{
			// Would be nice to use Map.computeIfAbsent but that requires 1.8-level
			// source compatibility
			if (FileCache.TryGetValue(exampleFile, out var bytes)) return bytes;
			try
			{
				bytes = File.ReadAllBytes(exampleFile);
			}
			catch (IOException e)
			{
				throw new InvalidOperationException($"{exampleFile}Cannot read file ", e);
			}

			FileCache[exampleFile] = bytes;

			return bytes;
		}

		public static void ClearFileCache() => FileCache.Clear();
	}
}