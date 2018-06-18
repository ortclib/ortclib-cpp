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

#include <ortc/internal/types.h>

#include <ortc/IICETypes.h>

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPHelpers
    //

    class RTPUtils
    {
    public:
      //RTP Utils
      static WORD getBE16(const void* memory) noexcept;
      static DWORD getBE32(const void* memory) noexcept;

      static void setBE16(void* memory, WORD v) noexcept;
      static void setBE32(void* memory, DWORD v) noexcept;

      static int getRtpPayloadType(const void* data, size_t len) noexcept;
      static int getRtpSeqNum(const void* data, size_t len) noexcept;
      static DWORD getRtpTimestamp(const void* data, size_t len) noexcept;
      static DWORD getRtpSsrc(const void* data, size_t len) noexcept;
      static size_t getRtpHeaderLen(const void* data, size_t len) noexcept;
      static int getRtcpType(const void* data, size_t len) noexcept;
      static DWORD getRtcpSsrc(const void* data, size_t len) noexcept;
      //bool getRtpHeader(const void* data, size_t len, RtpHeader* header) noexcept;

      static bool setRtpSsrc(void* data, size_t len, DWORD value) noexcept;
      // Assumes version 2, no padding, no extensions, no csrcs.
      //bool setRtpHeader(void* data, size_t len, const RtpHeader& header) noexcept;

      static bool isRtpPacket(const void* data, size_t len) noexcept;
      static bool isRTCPPacketType(const BYTE *data, size_t len) noexcept;

      // True if |payload type| is 0-127.
      static bool isValidRtpPayloadType(int payload_type) noexcept;

      static Time ntpToTime(
                            DWORD ntpMS,
                            DWORD ntpLS
                            ) noexcept;

      static Log::Params slog(const char *message) noexcept;
    };

  }
}
