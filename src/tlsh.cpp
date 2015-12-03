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
#include "stdio.h"
#include "version.h"
#include <errno.h>
#include <string.h>

/////////////////////////////////////////////////////
// C++ Implementation

Tlsh::Tlsh():impl(NULL)
{
    impl = new TlshImpl();
}

Tlsh::~Tlsh()
{
    delete impl;
}

const char *Tlsh::version()
{
    static char versionBuf[256];
    if (versionBuf[0] == '\0')
        snprintf(versionBuf, sizeof(versionBuf), "%d.%d.%d %s %s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, TLSH_HASH, TLSH_CHECKSUM);
    return versionBuf;
}

void Tlsh::update(const unsigned char* data, unsigned int len)
{
    // The python module does not call the Tlsh constructor, so allocate impl if NULL
    if ( NULL == impl )
        impl = new TlshImpl();

    if ( NULL != impl )
        impl->update(data, len);
}

void Tlsh::final(const unsigned char* data, unsigned int len)
{
    // The python module does not call the Tlsh constructor, so allocate impl if NULL
    if ( NULL == impl )
        impl = new TlshImpl();

    if ( NULL != impl ){
        if ( NULL != data && len > 0 )
            impl->update(data, len);
        impl->final();
    }
}

const char* Tlsh::getHash() const
{
    if ( NULL != impl )
        return impl->hash();
    else
        return "";
}

const char* Tlsh::getHash (char *buffer, unsigned int bufSize) const
{
    if ( NULL != impl )
        return impl->hash(buffer, bufSize);
    else {
        buffer[0] = '\0';
        return buffer;
    }
}

void Tlsh::reset()
{
    if ( NULL != impl )
        impl->reset();
}

bool Tlsh::operator==(const Tlsh& other) const
{
    if( this == &other )
        return true;
    else if( NULL == impl || NULL == other.impl )
        return false;
    else
        return ( 0 == impl->compare(*other.impl) );
}

bool Tlsh::operator!=(const Tlsh& other) const 
{
    return !(*this==other);
}

int Tlsh::totalDiff(const Tlsh *other, bool len_diff) const
{
    if( NULL==impl || NULL == other || NULL == other->impl )
        return -(EINVAL);
    else if ( this == other )
        return 0;
    else
        return (impl->totalDiff(*other->impl, len_diff));
}

int Tlsh::fromTlshStr(const char* str)
{
    // The python module does not call the Tlsh constructor, so allocate impl if NULL
    if ( NULL == impl )
        impl = new TlshImpl();

    if ( NULL == impl )
        return -(ENOMEM);
    else if ( NULL == str )
        return -(EINVAL);
    else
        return impl->fromTlshStr(str);
}

