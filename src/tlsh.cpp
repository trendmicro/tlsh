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
#include <errno.h>
#include <string.h>

/////////////////////////////////////////////////////
// C++ Implementation

Tlsh::Tlsh()
{
}

Tlsh::~Tlsh()
{
}

void Tlsh::update(const unsigned char* data, unsigned int len)
{
    impl.update(data, len);
}

void Tlsh::final(const unsigned char* data, unsigned int len)
{
    if ( NULL != data && len > 0 )
        impl.update(data, len);
    impl.final();
}

const char* Tlsh::getHash()
{
    return impl.hash();
}

const char* Tlsh::getHash(char *buffer, unsigned int bufSize)  
{
    return impl.hash(buffer, bufSize);
}

void Tlsh::reset()
{
    impl.reset();
}

bool Tlsh::operator==(const Tlsh& other) const
{
    if( this == &other )
        return true;
    else
        return ( 0 == impl.compare(other.impl) );
}

bool Tlsh::operator!=(const Tlsh& other) const 
{
    return !(*this==other);
}

int Tlsh::totalDiff(Tlsh *other, bool len_diff)
{
    if ( this == other )
        return 0;
    else
        return (impl.totalDiff(other->impl, len_diff)+1);
}

int Tlsh::fromTlshStr(const char* str)
{
    if ( NULL == str )
        return -(EINVAL);
    else
        return impl.fromTlshStr(str);
}

