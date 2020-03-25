/*
 * TLSH is provided for use under two licenses: Apache OR BSD.
 * Users may opt to use either license depending on the license
 * restictions of the systems with which they plan to integrate
 * the TLSH code.
 */ 

/* ==============
 * Apache License
 * ==============
 * Copyright 2013 Trend Micro Incorporated
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
 * Copyright (c) 2013, Trend Micro Incorporated
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


//////////////////////////////////////////////////////////////////////////
//
// (C) Trend Micro
// written Jon Oliver 2009 and 2010
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tlsh.h"

int main(int argc, char *argv[])
{
	int showvers = 1;
	Tlsh t1;
	Tlsh t2;

	const char *str1 = "This is a test for Lili Diao. This is a string. Hello Hello Hello ";
	const char *str2 = "This is a test for Jon Oliver. This is a string. Hello Hello Hello ";
	int len1 = strlen(str1);
	int len2 = strlen(str2);

	char minSizeBuffer1[512];
	for (int i = 0; i < 511; i++) {
		minSizeBuffer1[i] = i % 26 + 'A';
	}
	minSizeBuffer1[511] = 0;
	strncpy(minSizeBuffer1, str1, len1);
	t1.final( (const unsigned char*) minSizeBuffer1, 512);

	char minSizeBuffer2[1024];
	for (int i = 0; i < 1023; i++) {
		minSizeBuffer2[i] = i % 26 + 'A';
	}
	minSizeBuffer2[1023] = 0;
	strncpy(minSizeBuffer2, str2, len2);
	t2.final( (const unsigned char*) minSizeBuffer2, 1024);

	printf("str1 = '%s'\n", minSizeBuffer1 );
	printf("str2 = '%s'\n", minSizeBuffer2 );

	printf("hash1 = %s\n", t1.getHash(showvers) );
	printf("hash2 = %s\n", t2.getHash(showvers) );

	printf("difference (same strings) = %d\n", t1.totalDiff(&t1) );
	printf("difference (with len) = %d\n", t1.totalDiff(&t2) );
	printf("difference (without len) = %d\n", t1.totalDiff(&t2, false) );

	printf("Testing Tlsh with multiple update calls\n");
    Tlsh t3, t4;
    snprintf(minSizeBuffer1, sizeof(minSizeBuffer1), "%s", str1);
	t3.update( (const unsigned char*) minSizeBuffer1, len1);
	for (int i = 0; i < 511; i++) {
		minSizeBuffer1[i] = i % 26 + 'A';
	}
	minSizeBuffer1[511] = 0;
	t3.update( (const unsigned char*) minSizeBuffer1+len1, 512-len1);
	t3.final();
	assert(strcmp(t1.getHash(showvers), t3.getHash(showvers)) == 0);

    snprintf(minSizeBuffer2, sizeof(minSizeBuffer2), "%s", str2);
	t4.update( (const unsigned char*) minSizeBuffer2, len2);
	for (int i = 0; i < 1023; i++) {
		minSizeBuffer2[i] = i % 26 + 'A';
	}
	minSizeBuffer2[1023] = 0;
	t4.final( (const unsigned char*) minSizeBuffer2+len2, 1024-len2);
	assert(strcmp(t2.getHash(showvers), t4.getHash(showvers)) == 0);

	printf("hash3 = %s\n", t3.getHash(showvers) );
	printf("hash4 = %s\n", t4.getHash(showvers) );

	printf("Testing Tlsh.fromTlshStr()\n");
	printf("Recreating tlsh3 from %s\n", t1.getHash(minSizeBuffer1, sizeof(minSizeBuffer1), showvers));
	t3.reset();
	t3.fromTlshStr(minSizeBuffer1);
	printf("hash3 = %s\n", t3.getHash(minSizeBuffer2, sizeof(minSizeBuffer2), showvers));
	assert(strcmp(minSizeBuffer1, minSizeBuffer2) == 0);
	
	printf("Recreating tlsh4 from %s\n", t2.getHash(minSizeBuffer1, sizeof(minSizeBuffer1), showvers));
	t4.reset();
	t4.fromTlshStr(minSizeBuffer1);
	printf("hash4 = %s\n", t4.getHash(minSizeBuffer2, sizeof(minSizeBuffer2), showvers));
	assert(strcmp(minSizeBuffer1, minSizeBuffer2) == 0);
	printf("difference (same strings) = %d\n", t3.totalDiff(&t3) );
	printf("difference (with len) = %d\n", t3.totalDiff(&t4) );
	printf("difference (without len) = %d\n", t3.totalDiff(&t4, false) );
}
