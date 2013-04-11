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

#ifndef _TLSH_IMPL_H
#define _TLSH_IMPL_H
    
#define SLIDING_WND_SIZE    5

class TlshImpl
{
public:
    TlshImpl();
    ~TlshImpl();
public:
    void update(const unsigned char* data, unsigned int len);
    void final();
    void reset();
    const char* hash();
    int compare(const TlshImpl& other) const;
    int totalDiff(const TlshImpl& other) const;
    int fromTlshStr(const char* str);

private:
    unsigned int a_bucket[256];
    unsigned char slide_window[SLIDING_WND_SIZE];
    unsigned int data_len;
    
    struct lsh_bin_struct {
        unsigned char checksum;
        unsigned char Lvalue;
		union {
			unsigned char QB;
			struct{
				unsigned char Q1ratio : 4;
				unsigned char Q2ratio : 4;
			} QR;
		} Q;
        unsigned char tmp_code[32];
    } lsh_bin;
    
    char lsh_code[71];
};


#endif

