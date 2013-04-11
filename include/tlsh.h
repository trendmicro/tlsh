// tlsh.h - TrendLSH  Hash Algorithm

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

#ifndef _TLSH_H
#define _TLSH_H

#define TLSH_SIZE 70

#ifndef NULL
#define NULL 0
#endif

typedef void* TLSH_CTX;


#ifdef __cplusplus
extern "C" {
#endif

int TLSH_Init(TLSH_CTX* c);
int TLSH_Update(TLSH_CTX *c, const unsigned char* data, unsigned int len);
int TLSH_Final(TLSH_CTX *c);
int TLSH_Hash(TLSH_CTX *c, char* lsh);
int TLSH_Compare(TLSH_CTX* c1, TLSH_CTX* c2);
int TLSH_TotalDiff(TLSH_CTX* c1, TLSH_CTX* c2);
int TLSH_FromTlashStr(TLSH_CTX *c, const char*);
void TLSH_Free(TLSH_CTX* c);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class TlshImpl;

class Tlsh{

public:
    Tlsh();

    /* allow the user to add data in multiple iterations */
    void update(const unsigned char* data, unsigned int len);

    /* to signal the class there is no more data to be added */
    void final(const unsigned char* data = NULL, unsigned int len = 0);

    /* to get the hex-encoded hash code */
    const char* getHash();

    /* to bring to object back to the initial state */
    void reset();
    
    /* calculate difference */
    int totalDiff(const Tlsh& other) const;
    int totalDiff(Tlsh *);
    
    /* validate TrendLSH string and reset the hash according to it */
    int fromTlshStr(char* str);

    // operators
    bool operator==(const Tlsh& other) const;
    bool operator!=(const Tlsh& other) const;

    ~Tlsh();

private:
    TlshImpl* impl;
};
#endif

#endif

