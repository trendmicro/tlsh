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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

/**
 * Utilities to extract the known answers from the example data
 */
public class ExampleDataUtilities {
	
	/**
	 * Return the root of the tlsh source project. Can be overridden
	 * by setting the tlsh.root system property.
	 */
	public static File getSourceRoot() throws FileNotFoundException {
		// TODO - allow overriding this via a system property
		String override = System.getProperty("tlsh.root");
		if (override != null) {
			File sourceRoot = new File(override);
			if (!sourceRoot.exists() && !sourceRoot.isDirectory()) {
				throw new FileNotFoundException("tlsh.root property set to '" + override + "' but this does not exist");
			}
			return sourceRoot;
		}
		
		File sourceRoot = new File(".").getAbsoluteFile();
		do {
			// test for existence of expected file. Not bullet-proof, but okay.
			File testingFolder = new File(sourceRoot, "Testing");
			File license = new File(sourceRoot, "LICENSE");
			if (testingFolder.exists() && license.exists()) {
				return sourceRoot;
			}
			
		} while ((sourceRoot = sourceRoot.getParentFile()) != null);
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
	 * @throws IOException 
	 * @throws FileNotFoundException 
	 */
	public static Map<File, String> getExpectedHashes(File sourceRoot, int hashLength, int checksumLength) throws FileNotFoundException, IOException {
		Map<File, String> hashes = new TreeMap<>();
		final File expectedFile = new File(sourceRoot.getPath() + File.separator + "Testing" + File.separator + "exp" + File.separator + "example_data." + hashLength + "." + checksumLength + ".len.out_EXP");
		// Files referenced in the expected outputs files are relative to this folder,
		// not the data file itself
		final File relativeTo = expectedFile.getParentFile().getParentFile();
		try (BufferedReader br = new BufferedReader(new FileReader(expectedFile))) {
			String line = br.readLine();
			while (line != null) {
				String[] parts = line.trim().split("\\s+");
				if (parts.length == 2) {
					String expectedHash = parts[0];
					String relativeFile = parts[1];
					File absoluteFile = new File(relativeTo, relativeFile).getCanonicalFile();
					if (!absoluteFile.exists()) {
						throw new FileNotFoundException("File " + absoluteFile + " is referenced in data file but cannot be found");
					}
					hashes.put(absoluteFile, expectedHash);
				}
				line = br.readLine();
			}
		}
		return hashes;
	}

	/**
	 * A basic struct-type class
	 */
	public static class FilesAndDiff {
		
		final File sourceFile;
		final File targetFile;
		final int expectedDiff;
		public FilesAndDiff(File sourceFile, File targetFile, int expectedDiff) {
			this.sourceFile = sourceFile;
			this.targetFile = targetFile;
			this.expectedDiff = expectedDiff;
		}

		// My kingdom for Scala's tuples
	}

	public static List<FilesAndDiff> getExpectedDiffScores(File sourceRoot, int hashLength, int checksumLength, boolean includeLength) throws FileNotFoundException, IOException {
		List<FilesAndDiff> diffs = new ArrayList<>();
		// Example file name: example_data.128.1.xlen.xref.scores_EXP
		final File expectedFile = new File(sourceRoot.getPath() + File.separator + "Testing" + File.separator + "exp"
				+ File.separator + "example_data." + hashLength + "." + checksumLength + "."
				+ (includeLength ? "len" : "xlen") + ".xref.scores_EXP");
		// Files referenced in the expected outputs files are relative to this folder,
		// not the data file itself
		final File relativeTo = expectedFile.getParentFile().getParentFile();
		try (BufferedReader br = new BufferedReader(new FileReader(expectedFile))) {
			String line = br.readLine();
			while (line != null) {
				String[] parts = line.trim().split("\\s+");
				if (parts.length == 3) {
					String sourceFile = parts[0];
					String targetFile = parts[1];
					String diff = parts[2];
					File absoluteSourceFile = new File(relativeTo, sourceFile).getCanonicalFile();
					if (!absoluteSourceFile.exists()) {
						throw new FileNotFoundException("File " + absoluteSourceFile + " is referenced in data file but cannot be found");
					}
					File absoluteTargetFile = new File(relativeTo, targetFile).getCanonicalFile();
					if (!absoluteTargetFile.exists()) {
						throw new FileNotFoundException("File " + absoluteTargetFile + " is referenced in data file but cannot be found");
					}
					
					diffs.add(new FilesAndDiff(absoluteSourceFile, absoluteTargetFile, Integer.parseInt(diff)));
				}
				line = br.readLine();
			}
		}
		return diffs;
	}
	
	private static final Map<File, byte[]> fileCache = new HashMap<>();

	/**
	 * Read an entire file into a byte array and cache it.
	 * Not safe for large files.
	 */
	public static byte[] getFileBytes(File exampleFile) {
		// Would be nice to use Map.computeIfAbsent but that requires 1.8-level
		// source compatibility
		byte[] bytes = fileCache.get(exampleFile);
		if (bytes == null) {
			try {
				bytes = Files.readAllBytes(exampleFile.toPath());
			} catch (IOException e) {
				throw new RuntimeException("Cannot read file " + exampleFile, e);
			}
			fileCache.put(exampleFile, bytes);
			
		}
		return bytes;
	}

	public static void clearFileCache() {
		fileCache.clear();
	}

}
