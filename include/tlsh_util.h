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

#ifndef _TLSH_UTIL_H
#define _TLSH_UTIL_H

unsigned char b_mapping(unsigned char salt, unsigned char i, unsigned char j, unsigned char k);
unsigned char l_capturing(unsigned int len);
int mod_diff(unsigned int x, unsigned int y, unsigned int R);
int h_distance( int len, const unsigned char x[], const unsigned char y[]);
void to_hex( unsigned char * psrc, int len, char* pdest);
void from_hex( const char* psrc, int len, unsigned char* pdest);
unsigned char swap_byte( const unsigned char in );

#endif

