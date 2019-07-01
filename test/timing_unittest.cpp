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

#ifndef WINDOWS
#include <sys/time.h>
#endif

#include "tlsh.h"



#define MILLION	1000000
#define MAX_TRY	50

int main(int argc, char *argv[])
{
	int argIdx		= 1;
	int buffer_size		= MILLION;
	while (argc > argIdx) {
                if (strcmp(argv[argIdx], "-version") == 0) {
		        printf("%s\n", Tlsh::version());
			return 0;
		}
                if (strcmp(argv[argIdx], "-size") == 0) {
			char *str = argv[argIdx+1];
			if ( (str[0] >= '0') && (str[0] <= '9')) {
				buffer_size = atoi(argv[argIdx+1]);
			} else {
		        	printf("bad size=%s\n", str);
				return(1);
			}
		}
		argIdx ++;
	}

	Tlsh t1;
	Tlsh t2;

	char *buffer;
	char *buffer2;
	struct timeval tp;
	long before_ms;
	long after_ms;

	if (buffer_size == MILLION) {
		printf("build a buffer with a million bytes...\n" );
	} else {
		printf("build a buffer with a %d bytes...\n", buffer_size );
	}
	buffer = new char[buffer_size];
	for (int i = 0; i < buffer_size; i++) {
		buffer[i] = i % 26 + 'A';
	}
	buffer[buffer_size-1] = '\0';

	gettimeofday(&tp, NULL);
	before_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds

	printf("eval TLSH (%s) %d times...\n", Tlsh::version(), MAX_TRY );
	for (int tries=0; tries<MAX_TRY; tries++) {
		t1.reset();
		t1.final( (const unsigned char*) buffer, buffer_size);
	}

	gettimeofday(&tp, NULL);
	printf("TLSH(buffer) = %s\n", t1.getHash() );

	after_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds
	long diff	= (after_ms - before_ms);
	long per_iter	= diff / MAX_TRY;

	printf("Test 1: Evaluate TLSH digest\n");
	printf("BEFORE	ms=%ld\n", before_ms);
	printf("AFTER	ms=%ld\n", after_ms);
	printf("TIME	ms=%ld\n", diff);
	printf("TIME	ms=%ld	per iteration\n", per_iter);
	printf("\n");

	buffer2 = new char[buffer_size];
	for (int i = 0; i < buffer_size; i++) {
		buffer2[i] = i % 90 + ' ';
	}

	buffer2[buffer_size-1] = '\0';

	t2.reset();
	t2.final( (const unsigned char*) buffer2, buffer_size);

	gettimeofday(&tp, NULL);
	before_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds

	int dist = 0;
	printf("eval TLSH distance %d million times...\n", MAX_TRY );
	for (int tries=0; tries<MAX_TRY * MILLION; tries++) {
		dist = t1.totalDiff(&t2);
	}

	gettimeofday(&tp, NULL);
	after_ms = tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds
	diff		= (after_ms - before_ms);
	per_iter	= diff / MAX_TRY;

	printf("Test 2: Calc distance TLSH digest\n");
	printf("dist=%d\n", dist);
	printf("BEFORE	ms=%ld\n", before_ms);
	printf("AFTER	ms=%ld\n", after_ms);
	printf("TIME	ms=%ld\n", diff);
	printf("TIME	ms=%ld	per million iterations\n", per_iter);
	printf("\n");

	delete[] buffer;
	delete[] buffer2;
}
