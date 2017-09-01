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
package com.trendmicro.tlsh.example;

import java.io.IOException;
import java.io.InputStream;

import com.trendmicro.tlsh.Tlsh;
import com.trendmicro.tlsh.TlshCreator;

/**
 * Demonstrate example usage of TLSH
 */
public class TlshExample {

	public static void main(String[] args) throws IOException {
		// The sample files used here is taken from Apache Kafka.
		// The old_code.txt file is the original, while new_code.txt
		// is a re-ordering of the original code with some deletions.
		// They should test as being similar.

		TlshCreator tlshCreator = new TlshCreator();

		// Read and hash the old code
		byte[] buf = new byte[1024];
		InputStream oldCodeStream = Thread.currentThread().getContextClassLoader().getResourceAsStream("old_code.txt");
		int bytesRead = oldCodeStream.read(buf, 0, buf.length);
		while (bytesRead >= 0) {
			tlshCreator.update(buf, 0, bytesRead);
			bytesRead = oldCodeStream.read(buf, 0, buf.length);
		}
		oldCodeStream.close();
		Tlsh oldCodeHash = tlshCreator.getHash();
		System.out.println("Old code hash is " + oldCodeHash.getEncoded());
		
		// Reset the hash creator so that it can be used to create a new hash
		tlshCreator.reset();
		
		// Read and hash the new code
		InputStream newCodeStream = Thread.currentThread().getContextClassLoader().getResourceAsStream("new_code.txt");
		bytesRead = newCodeStream.read(buf, 0, buf.length);
		while (bytesRead >= 0) {
			tlshCreator.update(buf, 0, bytesRead);
			bytesRead = newCodeStream.read(buf, 0, buf.length);
		}
		newCodeStream.close();
		Tlsh newCodeHash = tlshCreator.getHash();
		System.out.println("New code hash is " + newCodeHash.getEncoded());
		
		// Compute the difference between the two files
		int diff = oldCodeHash.totalDiff(newCodeHash, true);
		System.out.println("TLSH difference between files is " + diff);
		if (diff == 0) {
			System.out.println("Files are likely identical");
		} else if (diff < 10) {
			System.out.println("Files are extremely similar");
		} else if (diff < 50) {
			System.out.println("Files are quite similar");
		}
		
	}
	
	// Note: the input stream code is duplicated here to make the sample
	// a single method, not because it is the best way to write the code.

}
