/*
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

//////////////////////////////////////////////////////////////////////////
//
// (C) Trend Micro
// written Jon Oliver 2009 and 2010

//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tlsh.h"

int main(int argc, char *argv[])
{
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

	printf("hash1 = %s\n", t1.getHash() );
	printf("hash2 = %s\n", t2.getHash() );

	printf("difference (same strings) = %d\n", t1.totalDiff(&t1) );
	printf("difference (with len) = %d\n", t1.totalDiff(&t2) );
	printf("difference (without len) = %d\n", t1.totalDiff(&t2, false) );

	Tlsh t3, t4;
	t3.fromTlshStr(t1.getHash());
	t4.fromTlshStr(t2.getHash());
	printf("difference = %d\n", t3.totalDiff(&t4) );

}
