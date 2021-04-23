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
 * Static utility methods used by {@link TlshCreator}
 */
final class TlshUtil {
	/*
	 * Implementation notes: this code is mostly copied from tlsh_util.cpp
	 * with modifications as required for Java. 
	 */

	/** Pearson's sample random table */
	private static final int[] v_table = {
	    1, 87, 49, 12, 176, 178, 102, 166, 121, 193, 6, 84, 249, 230, 44, 163,
	    14, 197, 213, 181, 161, 85, 218, 80, 64, 239, 24, 226, 236, 142, 38, 200,
	    110, 177, 104, 103, 141, 253, 255, 50, 77, 101, 81, 18, 45, 96, 31, 222,
	    25, 107, 190, 70, 86, 237, 240, 34, 72, 242, 20, 214, 244, 227, 149, 235,
	    97, 234, 57, 22, 60, 250, 82, 175, 208, 5, 127, 199, 111, 62, 135, 248,
	    174, 169, 211, 58, 66, 154, 106, 195, 245, 171, 17, 187, 182, 179, 0, 243,
	    132, 56, 148, 75, 128, 133, 158, 100, 130, 126, 91, 13, 153, 246, 216, 219,
	    119, 68, 223, 78, 83, 88, 201, 99, 122, 11, 92, 32, 136, 114, 52, 10,
	    138, 30, 48, 183, 156, 35, 61, 26, 143, 74, 251, 94, 129, 162, 63, 152,
	    170, 7, 115, 167, 241, 206, 3, 150, 55, 59, 151, 220, 90, 53, 23, 131,
	    125, 173, 15, 238, 79, 95, 89, 16, 105, 137, 225, 224, 217, 160, 37, 123,
	    118, 73, 2, 157, 46, 116, 9, 145, 134, 228, 207, 212, 202, 215, 69, 229,
	    27, 188, 67, 124, 168, 252, 42, 4, 29, 108, 21, 247, 19, 205, 39, 203,
	    233, 40, 186, 147, 198, 192, 155, 33, 164, 191, 98, 204, 165, 180, 117, 76,
	    140, 36, 210, 172, 41, 54, 159, 8, 185, 232, 113, 196, 231, 47, 146, 120,
	    51, 65, 28, 144, 254, 221, 93, 189, 194, 139, 112, 43, 71, 109, 184, 209
	};

	/** Pearson's hash function. All inputs must be integers in the range [0, 255],
	 * and the output will also be an integer in the range [0, 255] */
	static int b_mapping(int salt, int i, int j, int k) {
		int h;
		
		h = v_table[salt];
		h = v_table[h ^ i];
		h = v_table[h ^ j];
		h = v_table[h ^ k];
		return h;
	}

//	/** Natural logarithm of 1.5 */
//	private static final double LOG_1_5 = 0.4054651;
//	/** Natural logarithm of 1.3 */
//	private static final double LOG_1_3 = 0.26236426;
//	/** Natural logarithm of 1.1 */
//	private static final double LOG_1_1 = 0.095310180;

	/* Compute length portion of TLSH */
	/*
	static int l_capturing(long len) {
		int i;
		if (len <= 656) {
			i = (int) Math.floor(Math.log(len) / LOG_1_5);
		} else if (len <= 3199) {
			i = (int) Math.floor(Math.log(len) / LOG_1_3 - 8.72777);
		} else {
			i = (int) Math.floor(Math.log(len) / LOG_1_1 - 62.5472);
		}

		return i & 0xFF;
	}
	*/

	private static final long topval[] = {
		1,
		2,
		3,
		5,
		7,
		11,
		17,
		25,
		38,
		57,
		86,
		129,
		194,
		291,
		437,
		656,
		854,
		1110,
		1443,
		1876,
		2439,
		3171,
		3475,
		3823,
		4205,
		4626,
		5088,
		5597,
		6157,
		6772,
		7450,
		8195,
		9014,
		9916,
		10907,
		11998,
		13198,
		14518,
		15970,
		17567,
		19323,
		21256,
		23382,
		25720,
		28292,
		31121,
		34233,
		37656,
		41422,
		45564,
		50121,
		55133,
		60646,
		66711,
		73382,
		80721,
		88793,
		97672,
		107439,
		118183,
		130002,
		143002,
		157302,
		173032,
		190335,
		209369,
		230306,
		253337,
		278670,
		306538,
		337191,
		370911,
		408002,
		448802,
		493682,
		543050,
		597356,
		657091,
		722800,
		795081,
		874589,
		962048,
		1058252,
		1164078,
		1280486,
		1408534,
		1549388,
		1704327,
		1874759,
		2062236,
		2268459,
		2495305,
		2744836,
		3019320,
		3321252,
		3653374,
		4018711,
		4420582,
		4862641,
		5348905,
		5883796,
		6472176,
		7119394,
		7831333,
		8614467,
		9475909,
		10423501,
		11465851,
		12612437,
		13873681,
		15261050,
		16787154,
		18465870,
		20312458,
		22343706,
		24578077,
		27035886,
		29739474,
		32713425,
		35984770,
		39583245,
		43541573,
		47895730,
		52685306,
		57953837,
		63749221,
		70124148,
		77136564,
		84850228,
		93335252,
		102668779,
		112935659,
		124229227,
		136652151,
		150317384,
		165349128,
		181884040,
		200072456,
		220079703,
		242087671,
		266296456,
		292926096,
		322218735,
		354440623,
		389884688,
		428873168,
		471760495,
		518936559,
		570830240,
		627913311,
		690704607,
		759775136,
		835752671,
		919327967,
		1011260767,
		1112386880,
		1223623232,
		1345985727,
		1480584256,
		1628642751,
		1791507135,
		1970657856,
		2167723648L,
		2384496256L,
		2622945920L,
		2885240448L,
		3173764736L,
		3491141248L,
		3840255616L,
		4224281216L
	};
	
	/**
	 * The maximum amount of data allowed in a TLSH computation.
	 * Slightly less than 4GiB.
	 */
	static final long MAX_DATA_LENGTH = topval[topval.length-1];
	
	/** Compute length portion of TLSH */
	static int l_capturing(long len) {
		int bottom = 0;
		int top = topval.length;
		int idx = top >> 1;

		while (idx < topval.length) {
			if (idx == 0) {
				return idx;
			}
			if ((len <= topval[idx]) && (len > topval[idx-1])) {
				return idx;
			}
			if (len < topval[idx]) {
				top = idx - 1;
			} else {
				bottom = idx + 1;
			}
			idx = (bottom + top) >> 1;
		}
		throw new IllegalArgumentException("Can only compute length portion of TLSH for data lengths up to " + MAX_DATA_LENGTH + " bytes");
	}

	static int mod_diff(int x, int y, int R) {
		int dl = 0;
		int dr = 0;
		if (y > x) {
			dl = y - x;
			dr = x + R - y;
		} else {
			dl = x - y;
			dr = y + R - x;
		}
		return (dl > dr ? dr : dl);
	}

	static int h_distance(int[] x, int[] y) {
		int diff = 0;
		for (int i = 0; i < x.length; i++) {
			diff += DiffTable.bit_pairs_diff_table[x[i]][y[i]];
		}
		return diff;
	}

	/**
	 * Diff table is wrapped in a class to defer cost of generating
	 * the bit pairs diff table until required
	 */
	private static class DiffTable {
		static final int[][] bit_pairs_diff_table = generateTable();

		private static int[][] generateTable() {
			int[][] result = new int[256][256];
			for (int i = 0; i < 256; i++) {
				for (int j = 0; j < 256; j++) {
					int x = i;
					int y = j;
					int d;
					int diff = 0;
					d = Math.abs(x % 4 - y % 4);
					diff += (d == 3 ? 6 : d);
					x /= 4;
					y /= 4;
					d = Math.abs(x % 4 - y % 4);
					diff += (d == 3 ? 6 : d);
					x /= 4;
					y /= 4;
					d = Math.abs(x % 4 - y % 4);
					diff += (d == 3 ? 6 : d);
					x /= 4;
					y /= 4;
					d = Math.abs(x % 4 - y % 4);
					diff += (d == 3 ? 6 : d);
					result[i][j] = diff;
				}
			}
			return result;
		}
	}

	private static final char[] HEX_CHARS = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}; 

	/**
	 * Convert an input integer assumed to contain a value in [0, 255] to two hex characters
	 * and write them to the given string buffer.
	 */
	static void to_hex(int src, StringBuilder dest) {
		dest.append(HEX_CHARS[(src >> 4) & 0xF]);
		dest.append(HEX_CHARS[src & 0xF]);
	}

	/**
	 * Convert an input integer assumed to contain a value in [0, 255] to two hex characters
	 * and write them to the given string buffer in swapped order, e.g. the value 1 will be
	 * written as '10', not '01'
	 */
	static void to_hex_swapped(int src, StringBuilder dest) {
		dest.append(HEX_CHARS[src & 0xF]);
		dest.append(HEX_CHARS[(src >> 4) & 0xF]);
	}

	/**
	 * Convert two hex characters in the given string starting at the given offset
	 * to an integer
	 */
	static int from_hex(String src, int offset) {
		int result = hexCharToInt(src.charAt(offset)) << 4;
		result |= hexCharToInt(src.charAt(offset + 1));
		return result;
	}

	/**
	 * Convert two hex characters in the given string starting at the given offset
	 * to an integer, assuming they were encoded with to_hex_swapped
	 */
	static int from_hex_swapped(String src, int offset) {
		int result = hexCharToInt(src.charAt(offset + 1)) << 4;
		result |= hexCharToInt(src.charAt(offset));
		return result;
	}

	static int hexCharToInt(char hexChar) {
		if ('0' <= hexChar && hexChar <= '9') {
			return hexChar - '0';
		} else if ('A' <= hexChar && hexChar <= 'F') {
			return hexChar - 'A' + 10;
		} else if ('a' <= hexChar && hexChar <= 'f') {
			return hexChar - 'a' + 10;
		} else {
			throw new IllegalArgumentException("Invalid hex character '" + hexChar + "'");
		}
	}

}
