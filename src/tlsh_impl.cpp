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

#include "tlsh.h"
#include "tlsh_impl.h"
#include "tlsh_util.h"

#include <string>
#include <cassert>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <string.h>
#include <errno.h>

#define RANGE_LVALUE 256
#define RANGE_QRATIO 16

#ifdef UNITTEST
#define LOCAL_FUNCTION
#else
#define LOCAL_FUNCTION static
#endif

LOCAL_FUNCTION void find_quartile(unsigned int *q1, unsigned int *q2, unsigned int *q3, const unsigned int * a_bucket);
LOCAL_FUNCTION unsigned int partition(unsigned int * buf, unsigned int left, unsigned int right);

////////////////////////////////////////////////////////////////////////////////////////////////

TlshImpl::TlshImpl() : a_bucket(NULL), data_len(0), lsh_code(NULL), lsh_code_valid(false)
{
    memset(this->slide_window, 0, sizeof this->slide_window);
    memset(&this->lsh_bin, 0, sizeof this->lsh_bin);
}

TlshImpl::~TlshImpl()
{
    delete [] this->a_bucket;
    delete [] this->lsh_code;
}

void TlshImpl::reset()
{
    delete [] this->a_bucket; this->a_bucket = NULL;
    memset(this->slide_window, 0, sizeof this->slide_window);
    delete [] this->lsh_code; this->lsh_code = NULL; 
    memset(&this->lsh_bin, 0, sizeof this->lsh_bin);
    this->data_len = 0;
    this->lsh_code_valid = false;   
}

void TlshImpl::update(const unsigned char* data, unsigned int len) 
{
    #define RNG_SIZE    	SLIDING_WND_SIZE
    #define RNG_IDX(i)	((i+RNG_SIZE)%RNG_SIZE)
	
    int j = (int)(this->data_len % RNG_SIZE);
    unsigned int fed_len = this->data_len;

    if (this->a_bucket == NULL) {
        this->a_bucket = new unsigned int [BUCKETS];
        memset(this->a_bucket, 0, sizeof(int)*BUCKETS);
    }

    for( unsigned int i=0; i<len; i++, fed_len++, j=RNG_IDX(j+1) ) {
        this->slide_window[j] = data[i];
        
        if ( fed_len >= 4 ) {
            //only calculate when input >= 5 bytes
            int j_1 = RNG_IDX(j-1);
            int j_2 = RNG_IDX(j-2);
            int j_3 = RNG_IDX(j-3);
            int j_4 = RNG_IDX(j-4);
           
            for (int k = 0; k < TLSH_CHECKSUM_LEN; k++) {
                 if (k == 0) {
                     this->lsh_bin.checksum[k] = b_mapping(0, this->slide_window[j], this->slide_window[j_1], this->lsh_bin.checksum[k]);
                 }
                 else {
                     // use calculated 1 byte checksums to expand the total checksum to 3 bytes
                     this->lsh_bin.checksum[k] = b_mapping(this->lsh_bin.checksum[k-1], this->slide_window[j], this->slide_window[j_1], this->lsh_bin.checksum[k]);
                 }
            }

            unsigned char r;
            r = b_mapping(2, this->slide_window[j], this->slide_window[j_1], this->slide_window[j_2]);
            this->a_bucket[r]++;
            r = b_mapping(3, this->slide_window[j], this->slide_window[j_1], this->slide_window[j_3]);
            this->a_bucket[r]++;
            r = b_mapping(5, this->slide_window[j], this->slide_window[j_2], this->slide_window[j_3]);
            this->a_bucket[r]++;
            r = b_mapping(7, this->slide_window[j], this->slide_window[j_2], this->slide_window[j_4]);
            this->a_bucket[r]++;
            r = b_mapping(11, this->slide_window[j], this->slide_window[j_1], this->slide_window[j_4]);
            this->a_bucket[r]++;
            r = b_mapping(13, this->slide_window[j], this->slide_window[j_3], this->slide_window[j_4]);
            this->a_bucket[r]++;

        }
    }
    this->data_len += len;
}

/* to signal the class there is no more data to be added */
void TlshImpl::final() 
{
    // incoming data must more than or equal to MIN_DATA_LENGTH bytes
    if (this->data_len < MIN_DATA_LENGTH) {
      // this->lsh_code be empty
      delete [] this->a_bucket; this->a_bucket = NULL;
      return;
    }

    unsigned int q1, q2, q3;
    find_quartile(&q1, &q2, &q3, this->a_bucket);

    // buckets must be more than 50% non-zero
    int nonzero = 0;
    for(unsigned int i=0; i<CODE_SIZE; i++) {
      for(unsigned int j=0; j<4; j++) {
        if (this->a_bucket[4*i + j] > 0) {
          nonzero++;
        }
      }
    }
    if (nonzero <= 4*CODE_SIZE/2) {
      delete [] this->a_bucket; this->a_bucket = NULL;
      return;
    }
    
    for(unsigned int i=0; i<CODE_SIZE; i++) {
        unsigned char h=0;
        for(unsigned int j=0; j<4; j++) {
            unsigned int k = this->a_bucket[4*i + j];
            if( q3 < k ) {
                h += 3 << (j*2);  // leave the optimization j*2 = j<<1 or j*2 = j+j for compiler
            } else if( q2 < k ) {
                h += 2 << (j*2);
            } else if( q1 < k ) {
                h += 1 << (j*2);
            }
        }
        this->lsh_bin.tmp_code[i] = h;
    }

    //Done with a_bucket so deallocate
    delete [] this->a_bucket; this->a_bucket = NULL;
    
    this->lsh_bin.Lvalue = l_capturing(this->data_len);
    this->lsh_bin.Q.QR.Q1ratio = (unsigned int) ((float)(q1*100)/(float) q3) % 16;
    this->lsh_bin.Q.QR.Q2ratio = (unsigned int) ((float)(q2*100)/(float) q3) % 16;
    this->lsh_code_valid = true;   
}

int TlshImpl::fromTlshStr(const char* str)
{
    // Validate input string
    for( int i=0; i < TLSH_STRING_LEN; i++ )
        if (!( 
            (str[i] >= '0' && str[i] <= '9') || 
            (str[i] >= 'A' && str[i] <= 'F') ||
            (str[i] >= 'a' && str[i] <= 'f') ))
        {
            return 1;
        }

    this->reset();
    
    lsh_bin_struct tmp;
    from_hex( str, TLSH_STRING_LEN, (unsigned char*)&tmp );
    
    // Reconstruct checksum, Qrations & lvalue
    for (int k = 0; k < TLSH_CHECKSUM_LEN; k++) {    
        this->lsh_bin.checksum[k] = swap_byte(tmp.checksum[k]);
    }
    this->lsh_bin.Lvalue = swap_byte( tmp.Lvalue );
    this->lsh_bin.Q.QB = swap_byte(tmp.Q.QB);
    for( int i=0; i < CODE_SIZE; i++ ){
        this->lsh_bin.tmp_code[i] = (tmp.tmp_code[CODE_SIZE-1-i]);
    }
    this->lsh_code_valid = true;   

    return 0;
}

const char* TlshImpl::hash(char *buffer, unsigned int bufSize) const
{
    if (bufSize < TLSH_STRING_LEN + 1) {
        strncpy(buffer, "", bufSize);
        return buffer;
    }
    if (this->lsh_code_valid == false) {
        strncpy(buffer, "", bufSize);
        return buffer;
    }

    lsh_bin_struct tmp;
    for (int k = 0; k < TLSH_CHECKSUM_LEN; k++) {    
      tmp.checksum[k] = swap_byte( this->lsh_bin.checksum[k] );
    }
    tmp.Lvalue = swap_byte( this->lsh_bin.Lvalue );
    tmp.Q.QB = swap_byte( this->lsh_bin.Q.QB );
    for( int i=0; i < CODE_SIZE; i++ ){
        tmp.tmp_code[i] = (this->lsh_bin.tmp_code[CODE_SIZE-1-i]);
    }

    to_hex( (unsigned char*)&tmp, sizeof(tmp), buffer);
    return buffer;
}

/* to get the hex-encoded hash code */
const char* TlshImpl::hash() const
{
    if (this->lsh_code != NULL) {
        // lsh_code has been previously calculated, so just return it
        return this->lsh_code;
    }

    this->lsh_code = new char [TLSH_STRING_LEN+1];
    memset(this->lsh_code, 0, TLSH_STRING_LEN+1);
	
    return hash(this->lsh_code, TLSH_STRING_LEN+1);
}


// compare
int TlshImpl::compare(const TlshImpl& other) const
{
    return (memcmp( &(this->lsh_bin), &(other.lsh_bin), sizeof(this->lsh_bin)));
}

int TlshImpl::totalDiff(const TlshImpl& other, bool len_diff) const
{
    int diff = 0;
    
    if (len_diff) {
        int ldiff = mod_diff( this->lsh_bin.Lvalue, other.lsh_bin.Lvalue, RANGE_LVALUE);
        if ( ldiff == 0 )
            diff = 0;
        else if ( ldiff == 1 )
            diff = 1;
        else
           diff += ldiff*12;
    }
    
    int q1diff = mod_diff( this->lsh_bin.Q.QR.Q1ratio, other.lsh_bin.Q.QR.Q1ratio, RANGE_QRATIO);
    if ( q1diff <= 1 )
        diff += q1diff;
    else           
        diff += (q1diff-1)*12;
    
    int q2diff = mod_diff( this->lsh_bin.Q.QR.Q2ratio, other.lsh_bin.Q.QR.Q2ratio, RANGE_QRATIO);
    if ( q2diff <= 1)
        diff += q2diff;
    else
        diff += (q2diff-1)*12;
    
    for (int k = 0; k < TLSH_CHECKSUM_LEN; k++) {    
      if (this->lsh_bin.checksum[k] != other.lsh_bin.checksum[k] ) {
        diff ++;
        break;
      }
    }
    
    diff += h_distance( CODE_SIZE, this->lsh_bin.tmp_code, other.lsh_bin.tmp_code );

    return (diff);
}



#define SWAP_UINT(x,y) do {\
    unsigned int int_tmp = (x);  \
    (x) = (y); \
    (y) = int_tmp; } while(0)

void find_quartile(unsigned int *q1, unsigned int *q2, unsigned int *q3, const unsigned int * a_bucket) 
{
    unsigned int bucket_copy[EFF_BUCKETS], short_cut_left[EFF_BUCKETS], short_cut_right[EFF_BUCKETS], spl=0, spr=0;
    unsigned int p1 = EFF_BUCKETS/4-1;
    unsigned int p2 = EFF_BUCKETS/2-1;
    unsigned int p3 = EFF_BUCKETS-EFF_BUCKETS/4-1;
    unsigned int end = EFF_BUCKETS-1;

    for(unsigned int i=0; i<=end; i++) {
        bucket_copy[i] = a_bucket[i];
    }

    for( unsigned int l=0, r=end; ; ) {
        unsigned int ret = partition( bucket_copy, l, r );
        if( ret > p2 ) {
            r = ret - 1;
            short_cut_right[spr] = ret;
            spr++;
        } else if( ret < p2 ){
            l = ret + 1;
            short_cut_left[spl] = ret;
            spl++;
        } else {
            *q2 = bucket_copy[p2];
            break;
        }
    }
    
    short_cut_left[spl] = p2-1;
    short_cut_right[spr] = p2+1;

    for( unsigned int i=0, l=0; i<=spl; i++ ) {
        unsigned int r = short_cut_left[i];
        if( r > p1 ) {
            for( ; ; ) {
                unsigned int ret = partition( bucket_copy, l, r );
                if( ret > p1 ) {
                    r = ret-1;
                } else if( ret < p1 ) {
                    l = ret+1;
                } else {
                    *q1 = bucket_copy[p1];
                    break;
                }
            }
            break;
        } else if( r < p1 ) {
            l = r;
        } else {
            *q1 = bucket_copy[p1];
            break;
        }
    }

    for( unsigned int i=0, r=end; i<=spr; i++ ) {
        unsigned int l = short_cut_right[i];
        if( l < p3 ) {
            for( ; ; ) {
                unsigned int ret = partition( bucket_copy, l, r );
                if( ret > p3 ) {
                    r = ret-1;
                } else if( ret < p3 ) {
                    l = ret+1;
                } else {
                    *q3 = bucket_copy[p3];
                    break;
                }
            }
            break;
        } else if( l > p3 ) {
            r = l;
        } else {
            *q3 = bucket_copy[p3];
            break;
        }
    }

}

unsigned int partition(unsigned int * buf, unsigned int left, unsigned int right) 
{
    if( left == right ) {
        return left;
    }
    if( left+1 == right ) {
        if( buf[left] > buf[right] ) {
            SWAP_UINT( buf[left], buf[right] );
        }
        return left;
    }
        
    unsigned int ret = left, pivot = (left + right)>>1;
    
    unsigned int val = buf[pivot];
    
    buf[pivot] = buf[right];
    buf[right] = val;
    
    for( unsigned int i = left; i < right; i++ ) {
        if( buf[i] < val ) {
            SWAP_UINT( buf[ret], buf[i] );
            ret++;
        }
    }
    buf[right] = buf[ret];
    buf[ret] = val;
    
    return ret;
}


