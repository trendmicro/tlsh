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
    
#define SLIDING_WND_SIZE  5
#define BUCKETS           256
#define Q_BITS            2    // 2 bits; quartile value 0, 1, 2, 3

// BUCKETS_256 & CHECKSUM_3B are compiler switches defined in CMakeLists.txt

#if defined BUCKETS_256
  #define EFF_BUCKETS         256
  #define CODE_SIZE           64   // 256 * 2 bits = 64 bytes
  #if defined CHECKSUM_3B
    #define TLSH_CHECKSUM_LEN 3
    // defined in tlsh.h   #define TLSH_STRING_LEN   138  // 2 + 3 + 64 bytes = 138 hexidecimal chars
  #else
    #define TLSH_CHECKSUM_LEN 1
    // defined in tlsh.h   #define TLSH_STRING_LEN   134  // 2 + 1 + 64 bytes = 134 hexidecimal chars
  #endif
#else
  #define EFF_BUCKETS         128
  #define CODE_SIZE           32   // 128 * 2 bits = 32 bytes
  #if defined CHECKSUM_3B
    #define TLSH_CHECKSUM_LEN 3
    // defined in tlsh.h   #define TLSH_STRING_LEN   74   // 2 + 3 + 32 bytes = 74 hexidecimal chars
  #else
    #define TLSH_CHECKSUM_LEN 1
    // defined in tlsh.h   #define TLSH_STRING_LEN   70   // 2 + 1 + 32 bytes = 70 hexidecimal chars
  #endif
#endif

class TlshImpl
{
public:
    TlshImpl();
    ~TlshImpl();
public:
    void update(const unsigned char* data, unsigned int len);
    void final();
    void reset();
    const char* hash() const;
    const char* hash(char *buffer, unsigned int bufSize) const;  // saves allocating hash string in TLSH instance - bufSize should be TLSH_STRING_LEN + 1
    int compare(const TlshImpl& other) const;
    int totalDiff(const TlshImpl& other, bool len_diff=true) const;
    int fromTlshStr(const char* str);

private:
    unsigned int *a_bucket;  
    unsigned char slide_window[SLIDING_WND_SIZE];
    unsigned int data_len;
    
    struct lsh_bin_struct {
        unsigned char checksum[TLSH_CHECKSUM_LEN];  // 1 to 3 bytes
        unsigned char Lvalue;                       // 1 byte
        union {
        unsigned char QB;
            struct{
                unsigned char Q1ratio : 4;
                unsigned char Q2ratio : 4;
            } QR;
        } Q;                                        // 1 bytes
        unsigned char tmp_code[CODE_SIZE];          // 32/64 bytes
    } lsh_bin;
    
    mutable char *lsh_code;       // allocated when hash() function without buffer is called - 70/134 bytes or 74/138 bytes
    bool lsh_code_valid;  // true iff final() or fromTlshStr complete successfully
};


#endif

