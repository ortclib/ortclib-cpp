/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/types.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IORTC
  //

  interaction IORTC
  {
    typedef zsLib::Log Log;

    static void setup(IMessageQueuePtr defaultDelegateMessageQueue) noexcept;
#ifdef WINUWP
#ifdef __cplusplus_winrt
    static void setup(Windows::UI::Core::CoreDispatcher ^ dispatcher) noexcept;
#endif  //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
    static void setup(winrt::Windows::UI::Core::CoreDispatcher dispatcher) noexcept;
#endif //CPPWINRT_VERSION
#endif //WINUWP
    
    static Milliseconds ntpServerTime() noexcept;
    static void ntpServerTime(const Milliseconds &value) noexcept;

    static void setDefaultLogLevel(Log::Level level) noexcept;
    static void setLogLevel(const char *componenet, Log::Level level) noexcept;

    static void setDefaultEventingLevel(Log::Level level) noexcept;
    static void setEventingLevel(const char *componenet, Log::Level level) noexcept;

    static void startMediaTracing() noexcept;
    static void stopMediaTracing() noexcept;
    static bool isMediaTracing() noexcept;
    static bool saveMediaTrace(String filename) noexcept;
    static bool saveMediaTrace(String host, int port) noexcept;
    static bool isMRPInstalled() noexcept;

    virtual ~IORTC() noexcept {} // make polymorphic
  };
}
