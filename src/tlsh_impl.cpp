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
#define TLSH_STRING_LEN 70

#ifdef UNITTEST
#define LOCAL_FUNCTION
#else
#define LOCAL_FUNCTION static
#endif

LOCAL_FUNCTION void find_quartile(unsigned int *q1, unsigned int *q2, unsigned int *q3, const unsigned int * a_bucket);
LOCAL_FUNCTION unsigned int partition(unsigned int * buf, unsigned int left, unsigned int right);

////////////////////////////////////////////////////////////////////////////////////////////////

TlshImpl::TlshImpl() 
{
    this->reset();
}

TlshImpl::~TlshImpl()
{
}

void TlshImpl::reset()
{
    memset(this->a_bucket, 0, sizeof this->a_bucket);
    memset(this->slide_window, 0, sizeof this->slide_window);
    memset(&this->lsh_bin, 0, sizeof this->lsh_bin);
    memset(this->lsh_code, 0, sizeof this->lsh_code);
    this->data_len = 0;
}

void TlshImpl::update(const unsigned char* data, unsigned int len) 
{
    #define RNG_SIZE    	SLIDING_WND_SIZE
    #define RNG_IDX(i)	((i+RNG_SIZE)%RNG_SIZE)
	
    int j = (int)(this->data_len % RNG_SIZE);
    unsigned int fed_len = this->data_len;
    
    for( unsigned int i=0; i<len; i++, fed_len++, j=RNG_IDX(j+1) ) {
        slide_window[j] = data[i];
        
        if ( fed_len >= 4 ) {
            //only calculate when input >= 5 bytes
            int j_1 = RNG_IDX(j-1);
            int j_2 = RNG_IDX(j-2);
            int j_3 = RNG_IDX(j-3);
            int j_4 = RNG_IDX(j-4);
            
            unsigned char r;
            this->lsh_bin.checksum = b_mapping(0, slide_window[j], slide_window[j_1], this->lsh_bin.checksum);

            r = b_mapping(2, slide_window[j], slide_window[j_1], slide_window[j_2]);
            this->a_bucket[r]++;
            r = b_mapping(3, slide_window[j], slide_window[j_1], slide_window[j_3]);
            this->a_bucket[r]++;
            r = b_mapping(5, slide_window[j], slide_window[j_2], slide_window[j_3]);
            this->a_bucket[r]++;
            r = b_mapping(7, slide_window[j], slide_window[j_2], slide_window[j_4]);
            this->a_bucket[r]++;
            r = b_mapping(11, slide_window[j], slide_window[j_1], slide_window[j_4]);
            this->a_bucket[r]++;
            r = b_mapping(13, slide_window[j], slide_window[j_3], slide_window[j_4]);
            this->a_bucket[r]++;

        }
    }
    this->data_len += len;
}

/* to signal the class there is no more data to be added */
void TlshImpl::final() 
{
    unsigned int q1, q2, q3;
    find_quartile(&q1, &q2, &q3, this->a_bucket);
    
    for(unsigned int i=0; i<32; i++) {
        unsigned char h=0;
        for(unsigned int j=0; j<4; j++) {
            unsigned int k = this->a_bucket[4*i + j];
            if( q3 < k ) {
                h += 3 << (j*2);    // leave the optimization j*2 = j<<1 or j*2 = j+j for compiler
            } else if( q2 < k ) {
                h += 2 << (j*2);
            } else if( q1 < k ) {
                h += 1 << (j*2);
            }
        }
        this->lsh_bin.tmp_code[i] = h;
    }
    
    this->lsh_bin.Lvalue = l_capturing(this->data_len);
    this->lsh_bin.Q.QR.Q1ratio = (unsigned int) ((float)(q1*100)/(float) q3) % 16;
    this->lsh_bin.Q.QR.Q2ratio = (unsigned int) ((float)(q2*100)/(float) q3) % 16;
	
	//Need to confirm with Jon about the byte order
	lsh_bin_struct tmp;
	tmp.checksum = swap_byte( this->lsh_bin.checksum );
	tmp.Lvalue = swap_byte( this->lsh_bin.Lvalue );
	tmp.Q.QB = swap_byte( this->lsh_bin.Q.QB );
	for( int i=0; i < 32; i++ ){
		tmp.tmp_code[i] = (this->lsh_bin.tmp_code[31-i]);
	}
    to_hex( (unsigned char*)&tmp, sizeof(tmp), this->lsh_code );
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
	memcpy( this->lsh_code, str, TLSH_STRING_LEN );
	
	lsh_bin_struct tmp;
	from_hex( this->lsh_code, 70, (unsigned char*)&tmp );
	
    // Reconstruct checksum, Qrations & lvalue
	//Need to confirm with Jon about the byte order
	this->lsh_bin.checksum = swap_byte(tmp.checksum);
	this->lsh_bin.Lvalue = swap_byte( tmp.Lvalue );
	this->lsh_bin.Q.QB = swap_byte(tmp.Q.QB);
	for( int i=0; i < 32; i++ ){
		this->lsh_bin.tmp_code[i] = (tmp.tmp_code[31-i]);
	}

	return 0;
}


/* to get the hex-encoded hash code */
const char* TlshImpl::hash() {
    return this->lsh_code;
}


// compare
int TlshImpl::compare(const TlshImpl& other) const
{
    int ret = 0;
    ret = memcmp( this->a_bucket, other.a_bucket, sizeof this->a_bucket );
    if ( 0 != ret )
        return ret;
    
    ret = memcmp( this->slide_window, other.slide_window, sizeof this->slide_window );
    if ( 0 != ret )
        return ret;
        
    ret = this->lsh_bin.checksum - other.lsh_bin.checksum;
    if ( 0 != ret )
        return ret;

    ret =  this->data_len - other.data_len;
    if ( 0 != ret )
        return ret;

    return 0;
}

int TlshImpl::totalDiff(const TlshImpl& other) const
{
    int diff = 0;
    int ldiff = mod_diff( this->lsh_bin.Lvalue, other.lsh_bin.Lvalue, RANGE_LVALUE);
    if ( ldiff == 0 )
        diff = 0;
    else if ( ldiff == 1 )
        diff = 1;
    else
        diff += ldiff*12;
    
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
    
    if (this->lsh_bin.checksum != other.lsh_bin.checksum )
        diff ++;
    
	diff += h_distance( this->lsh_bin.tmp_code, other.lsh_bin.tmp_code );

    return (diff - 1);
}



#define SWAP_UINT(x,y) do {\
    unsigned int int_tmp = (x);  \
    (x) = (y); \
    (y) = int_tmp; } while(0)

void find_quartile(unsigned int *q1, unsigned int *q2, unsigned int *q3, const unsigned int * a_bucket) 
{
    unsigned int bucket_copy[128], short_cut_left[128], short_cut_right[128], spl=0, spr=0;

    for(unsigned int i=0; i<128; i++) {
        bucket_copy[i] = a_bucket[i];
    }

    for( unsigned int l=0, r=127; ; ) {
        unsigned int ret = partition( bucket_copy, l, r );
        if( ret > 63 ) {
            r = ret - 1;
            short_cut_right[spr] = ret;
            spr++;
        } else if( ret < 63 ){
            l = ret + 1;
            short_cut_left[spl] = ret;
            spl++;
        } else {
            *q2 = bucket_copy[63];
            break;
        }
    }
    
    short_cut_left[spl] = 62;
    short_cut_right[spr] = 64;

    for( unsigned int i=0, l=0; i<=spl; i++ ) {
        unsigned int r = short_cut_left[i];
        if( r > 31 ) {
            for( ; ; ) {
                unsigned int ret = partition( bucket_copy, l, r );
                if( ret > 31 ) {
                    r = ret-1;
                } else if( ret < 31 ) {
                    l = ret+1;
                } else {
                    *q1 = bucket_copy[31];
                    break;
                }
            }
            break;
        } else if( r < 31 ) {
            l = r;
        } else {
            *q1 = bucket_copy[31];
            break;
        }
    }

    for( unsigned int i=0, r=127; i<=spr; i++ ) {
        unsigned int l = short_cut_right[i];
        if( l < 95 ) {
            for( ; ; ) {
                unsigned int ret = partition( bucket_copy, l, r );
                if( ret > 95 ) {
                    r = ret-1;
                } else if( ret < 95 ) {
                    l = ret+1;
                } else {
                    *q3 = bucket_copy[95];
                    break;
                }
            }
            break;
        } else if( l > 95 ) {
            r = l;
        } else {
            *q3 = bucket_copy[95];
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


