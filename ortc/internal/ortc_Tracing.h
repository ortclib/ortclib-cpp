/*

 Copyright (c) 2016, Hookflash Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#pragma once

#include <ortc/internal/types.h>


#ifdef USE_ETW
#include "ortc_ETWTracing.h"
#else

// Comment the following line to test inline versions of the same macros to test compilation
//#define ORTC_USE_NOOP_EVENT_TRACE_MACROS

// NO-OP VERSIONS OF ALL TRACING MACROS
#ifdef ORTC_USE_NOOP_EVENT_TRACE_MACROS

#define EventWriteOrtcCreate(xStr_Method, xPUID)
#define EventWriteOrtcDestroy(xStr_Method, xPUID)


#else

// duplicate testing compilation methods used to verify compilation when macros get defined
namespace ortc
{

inline void EventWriteOrtcCreate(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDestroy(const char *xStr_Method, PUID xPUID) {}

inline void EventWriteOrtcCertificateCreate(const char *xStr_Method, PUID xPUID, const char *xStr_Algorithm, size_t xsize_t_KeyLength, size_t xsize_t_RandomBits, long long xlong_long_LifetimeInSeconds, long long xlong_long_NotBeforeWindowInSeconds, const char *xStr_Expires) {}
inline void EventWriteOrtcCertificateDestroy(const char *xStr_Method, PUID xPUID) {}

}
#endif //ndef ORTC_USE_NOOP_EVENT_TRACE_MACROS

#endif //USE_ETW

