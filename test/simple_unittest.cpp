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
	const char *str1 = "This is a test for Lili Diao. This is a string. Hello Hello Hello";
	const char *str2 = "This is a test for Jon Oliver. This is a string. Hello Hello Hello                                                                                                                                                                                                       ";
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	t1.final( (const unsigned char*) str1, len1);
	t2.final( (const unsigned char*) str2, len2);
	printf("str1 = '%s'\n", str1 );
	printf("str2 = '%s'\n", str2 );
	printf("hash1 = %s\n", t1.getHash() );
	printf("hash2 = %s\n", t2.getHash() );
	printf("similarity = %d\n", 256 - t1.totalDiff(&t2) );
}
