/*

 Copyright (c) 2014, Hookflash Inc.
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

// these are the predefines that might be available on various platforms (start out undefined)
#undef HAVE_IPHLPAPI_H
#undef HAVE_SYS_TYPES_H
#undef HAVE_IFADDRS_H
#undef HAVE_NET_IF_H
#undef HAVE_NETINIT6_IN6_VAR_H
#undef HAVE_GMTIME_S
#undef HAVE_SPRINTF_S
#undef HAVE_GETADAPTERADDRESSES
#undef HAVE_GETIFADDRS


#ifdef _WIN32

#ifdef __cplusplus_winrt
#undef WINRT
#define WINRT
#endif //__cplusplus_winrt


// WIN32 platforms have these defined
#define HAVE_IPHLPAPI_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_GMTIME_S 1
#define HAVE_SPRINTF_S 1
#define HAVE_GETADAPTERADDRESSES 1

#ifdef WINRT

// WINRT has these defined

// WINRT does not support these features (but WIN32 does)
#undef HAVE_GETADAPTERADDRESSES

#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP

// WINRT phone has these defined

// WINRT phone odes not support these features (but WINRT does)
#undef HAVE_IPHLPAPI_H
#endif //defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP

#endif //WINRT

#endif //_WIN32


#ifdef __QNX__

// QNX supports these features
#define HAVE_SYS_TYPES_H 1
#define HAVE_IFADDRS_H 1
#define HAVE_GETIFADDRS 1

#endif //__QNX__


#ifdef __APPLE__

#include <TargetConditionals.h>

// Mac / iOS support these features
#define HAVE_SYS_TYPES_H 1
#define HAVE_IFADDRS_H 1
#define HAVE_GETIFADDRS 1
#define HAVE_NET_IF_H 1
#if TARGET_OS_IPHONE
// iphone OS
#else
#define HAVE_NETINIT6_IN6_VAR_H 1
#endif //defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

#endif //__APPPLE__


#ifdef _LINUX

// Linux has these options
#define HAVE_SYS_TYPES_H 1
#define HAVE_IFADDRS_H 1
#define HAVE_NET_IF_H 1
#define HAVE_NETINIT6_IN6_VAR_H 1
#define HAVE_GETIFADDRS 1

#ifdef _ANDROID

// Android supports these additional features

// Android does not support these features
#undef HAVE_IFADDRS_H

#endif //_ANDROID
#endif //_LINUX
