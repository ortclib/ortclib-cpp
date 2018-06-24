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

#include <ortc/internal/types.h>
#include <ortc/internal/platform.h>
#include <ortc/internal/ortc_RTPUtils.h>

//#include <ortc/services/IHelper.h>
//
//#include <zsLib/Stringize.h>
//#include <zsLib/Log.h>
//#include <zsLib/XML.h>
//
//#include <cryptopp/sha.h>


#ifdef _WIN32
namespace std {
  inline time_t mktime(struct tm *timeptr) { return ::mktime(timeptr); }
}
#endif //_WIN32

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_rtp_types) }

namespace ortc
{
//  ZS_DECLARE_TYPEDEF_PTR(ortc::services::ISettings, UseSettings)
//  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)
//  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP)
//
//  typedef ortc::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (helpers)
    //

    //-------------------------------------------------------------------------
    static const size_t kMinRtpPacketLen = 12;
    //static const size_t kMaxRtpPacketLen = 2048;
    static const size_t kMinRtcpPacketLen = 4;

    static const uint8_t kRtpVersion = 2;
    static const size_t kRtpFlagsOffset = 0;
    static const size_t kRtpPayloadTypeOffset = 1;
    static const size_t kRtpSeqNumOffset = 2;
    static const size_t kRtpTimestampOffset = 4;
    static const size_t kRtpSsrcOffset = 8;
    static const size_t kRtcpPayloadTypeOffset = 1;

    //-------------------------------------------------------------------------
    static void set8(void* memory, size_t offset, BYTE v) noexcept {
      static_cast<BYTE*>(memory)[offset] = v;
    }

    //-------------------------------------------------------------------------
    static BYTE get8(const void* memory, size_t offset) noexcept {
      return static_cast<const BYTE*>(memory)[offset];
    }

    //-------------------------------------------------------------------------
    static bool getUint8(const void* data, size_t offset, int* value) noexcept {
      if (!data || !value) {
        return false;
      }
      *value = *(static_cast<const BYTE*>(data)+offset);
      return true;
    }

    //-------------------------------------------------------------------------
    static bool getUint16(const void* data, size_t offset, int* value) noexcept {
      if (!data || !value) {
        return false;
      }
      *value = static_cast<int>(RTPUtils::getBE16(static_cast<const BYTE*>(data)+offset));
      return true;
    }

    //-------------------------------------------------------------------------
    static bool getUint32(const void* data, size_t offset, DWORD* value) noexcept {
      if (!data || !value) {
        return false;
      }
      *value = RTPUtils::getBE32(static_cast<const BYTE*>(data)+offset);
      return true;
    }

    //-------------------------------------------------------------------------
    static bool setUint8(void* data, size_t offset, uint8_t value) noexcept {
      if (!data) {
        return false;
      }
      set8(data, offset, value);
      return true;
    }

    //-------------------------------------------------------------------------
    static bool setUint16(void* data, size_t offset, uint16_t value) noexcept {
      if (!data) {
        return false;
      }
      RTPUtils::setBE16(static_cast<BYTE*>(data)+offset, value);
      return true;
    }

    //-------------------------------------------------------------------------
    static bool setUint32(void* data, size_t offset, DWORD value) noexcept {
      if (!data) {
        return false;
      }
      RTPUtils::setBE32(static_cast<BYTE*>(data)+offset, value);
      return true;
    }

    //-------------------------------------------------------------------------
    static bool getRtpFlags(const void* data, size_t len, int* value) noexcept {
      if (len < kMinRtpPacketLen) {
        return false;
      }
      return getUint8(data, kRtpFlagsOffset, value);
    }

    //-------------------------------------------------------------------------
    static bool getRtpPayloadType(const void* data, size_t len, int* value) noexcept {
      if (len < kMinRtpPacketLen) {
        return false;
      }
      if (!getUint8(data, kRtpPayloadTypeOffset, value)) {
        return false;
      }
      *value &= 0x7F;
      return true;
    }

    //-------------------------------------------------------------------------
    static bool getRtpSeqNum(const void* data, size_t len, int* value) noexcept {
      if (len < kMinRtpPacketLen) {
        return false;
      }
      return getUint16(data, kRtpSeqNumOffset, value);
    }

    //-------------------------------------------------------------------------
    static bool getRtpTimestamp(const void* data, size_t len, DWORD* value) noexcept {
      if (len < kMinRtpPacketLen) {
        return false;
      }
      return getUint32(data, kRtpTimestampOffset, value);
    }

    //-------------------------------------------------------------------------
    static bool getRtpSsrc(const void* data, size_t len, DWORD* value) noexcept {
      if (len < kMinRtpPacketLen) {
        return false;
      }
      return getUint32(data, kRtpSsrcOffset, value);
    }

    //-------------------------------------------------------------------------
    static bool getRtpHeaderLen(const void* data, size_t len, size_t* value) noexcept {
      if (!data || len < kMinRtpPacketLen || !value) return false;
      const BYTE* header = static_cast<const BYTE*>(data);
      // Get base header size + length of CSRCs (not counting extension yet).
      size_t header_size = kMinRtpPacketLen + (header[0] & 0xF) * sizeof(DWORD);
      if (len < header_size) return false;
      // If there's an extension, read and add in the extension size.
      if (header[0] & 0x10) {
        if (len < header_size + sizeof(DWORD)) return false;
        header_size += ((RTPUtils::getBE16(header + header_size + 2) + 1) *
          sizeof(DWORD));
        if (len < header_size) return false;
      }
      *value = header_size;
      return true;
    }

    //-------------------------------------------------------------------------
    //bool getRtpHeader(const void* data, size_t len, RtpHeader* header) noexcept {
    //  return (GetRtpPayloadType(data, len, &(header->payload_type)) &&
    //    GetRtpSeqNum(data, len, &(header->seq_num)) &&
    //    GetRtpTimestamp(data, len, &(header->timestamp)) &&
    //    GetRtpSsrc(data, len, &(header->ssrc)));
    //}

    //-------------------------------------------------------------------------
    static bool getRtcpType(const void* data, size_t len, int* value) noexcept {
      if (len < kMinRtcpPacketLen) {
        return false;
      }
      return getUint8(data, kRtcpPayloadTypeOffset, value);
    }

    //-------------------------------------------------------------------------
    // This method returns SSRC first of RTCP packet, except if packet is SDES.
    // TODO(mallinath) - Fully implement RFC 5506. This standard doesn't restrict
    // to send non-compound packets only to feedback messages.
    static bool getRtcpSsrc(const void* data, size_t len, DWORD* value) noexcept {
      // Packet should be at least of 8 bytes, to get SSRC from a RTCP packet.
      if (!data || len < kMinRtcpPacketLen + 4 || !value) return false;
      int pl_type;
      if (!getRtcpType(data, len, &pl_type)) return false;
      // SDES packet parsing is not supported.
      if (pl_type == 202) return false; // payload type SDES
      *value = RTPUtils::getBE32(static_cast<const BYTE*>(data)+4);
      return true;
    }

    //-------------------------------------------------------------------------
    static bool setRtpSsrc(void* data, ZS_MAYBE_USED() size_t len, DWORD value) noexcept {
      ZS_MAYBE_USED(len);
      return setUint32(data, kRtpSsrcOffset, value);
    }

    //-------------------------------------------------------------------------
    //// Assumes version 2, no padding, no extensions, no csrcs.
    //bool SetRtpHeader(void* data, size_t len, const RtpHeader& header) noexcept {
    //  if (!IsValidRtpPayloadType(header.payload_type) ||
    //    header.seq_num < 0 || header.seq_num > UINT16_MAX) {
    //    return false;
    //  }
    //  return (SetUint8(data, kRtpFlagsOffset, kRtpVersion << 6) &&
    //    SetUint8(data, kRtpPayloadTypeOffset, header.payload_type & 0x7F) &&
    //    SetUint16(data, kRtpSeqNumOffset,
    //    static_cast<uint16_t>(header.seq_num)) &&
    //    SetUint32(data, kRtpTimestampOffset, header.timestamp) &&
    //    SetRtpSsrc(data, len, header.ssrc));
    //}

    //-------------------------------------------------------------------------
    static bool IsRtpPacket(const void* data, size_t len) noexcept {
      if (len < kMinRtpPacketLen)
        return false;

      return (static_cast<const BYTE*>(data)[0] >> 6) == kRtpVersion;
    }

    //-------------------------------------------------------------------------
    static bool IsValidRtpPayloadType(int payload_type) noexcept {
      return payload_type >= 0 && payload_type <= 127;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Helper
    //

    //-------------------------------------------------------------------------
    WORD RTPUtils::getBE16(const void* memory) noexcept {
      return static_cast<WORD>((get8(memory, 0) << 8) |
                               (get8(memory, 1) << 0));
    }

    //-------------------------------------------------------------------------
    DWORD RTPUtils::getBE32(const void* memory) noexcept {
      return (static_cast<DWORD>(get8(memory, 0)) << 24) |
             (static_cast<DWORD>(get8(memory, 1)) << 16) |
             (static_cast<DWORD>(get8(memory, 2)) << 8) |
             (static_cast<DWORD>(get8(memory, 3)) << 0);
    }

    //-------------------------------------------------------------------------
    void RTPUtils::setBE16(void* memory, WORD v) noexcept {
      set8(memory, 0, static_cast<BYTE>(v >> 8));
      set8(memory, 1, static_cast<BYTE>(v >> 0));
    }

    //-------------------------------------------------------------------------
    void RTPUtils::setBE32(void* memory, DWORD v) noexcept {
      set8(memory, 0, static_cast<BYTE>(v >> 24));
      set8(memory, 1, static_cast<BYTE>(v >> 16));
      set8(memory, 2, static_cast<BYTE>(v >> 8));
      set8(memory, 3, static_cast<BYTE>(v >> 0));
    }

    //-------------------------------------------------------------------------
    int RTPUtils::getRtpPayloadType(const void* data, size_t len) noexcept
    {
      int value {};
      if (ortc::internal::getRtpPayloadType(data, len, &value)) {
        return value;
      }

      ZS_LOG_ERROR(Detail, slog("GetRtpPayloadType failed"))
      return 0;
    }

    //-------------------------------------------------------------------------
    int RTPUtils::getRtpSeqNum(const void* data, size_t len) noexcept
    {
      int value{};
      if (ortc::internal::getRtpSeqNum(data, len, &value)) {
        return value;
      }

      ZS_LOG_ERROR(Detail, slog("GetRtpSeqNum failed"))
      return 0;
    }

    //-------------------------------------------------------------------------
    DWORD RTPUtils::getRtpTimestamp(const void* data, size_t len) noexcept
    {
      DWORD value{};
      if (ortc::internal::getRtpTimestamp(data, len, &value)) {
        return value;
      }

      ZS_LOG_ERROR(Detail, slog("GetRtpTimestamp failed"))
      return 0;
    }

    //-------------------------------------------------------------------------
    DWORD RTPUtils::getRtpSsrc(const void* data, size_t len) noexcept
    {
      DWORD value{};
      if (ortc::internal::getRtpSsrc(data, len, &value)) {
        return value;
      }

      ZS_LOG_ERROR(Detail, slog("GetRtpSsrc failed"))
      return 0;
    }

    //-------------------------------------------------------------------------
    size_t RTPUtils::getRtpHeaderLen(const void* data, size_t len) noexcept
    {
      size_t value{};
      if (ortc::internal::getRtpHeaderLen(data, len, &value)) {
        return value;
      }

      ZS_LOG_ERROR(Detail, slog("GetRtpHeaderLen failed"))
      return 0;
    }

    //-------------------------------------------------------------------------
    int RTPUtils::getRtcpType(const void* data, size_t len) noexcept {
      int value{};
      if (ortc::internal::getRtcpType(data, len, &value)) {
        return value;
      }

      ZS_LOG_ERROR(Detail, slog("GetRtcpType failed"))
      return 0;
    }

    //-------------------------------------------------------------------------
    DWORD RTPUtils::getRtcpSsrc(const void* data, size_t len) noexcept
    {
      DWORD value{};
      if (ortc::internal::getRtcpSsrc(data, len, &value)) {
        return value;
      }

      ZS_LOG_ERROR(Detail, slog("GetRtcpSsrc failed"))
      return 0;
    }

    //-------------------------------------------------------------------------
    bool RTPUtils::setRtpSsrc(void* data, size_t len, DWORD value) noexcept
    {
      return ortc::internal::setRtpSsrc(data, len, value);
    }

    //-------------------------------------------------------------------------
    bool RTPUtils::isRtpPacket(const void* data, size_t len) noexcept
    {
      return ortc::internal::IsRtpPacket(data, len);
    }

    //-------------------------------------------------------------------------
    bool RTPUtils::isRTCPPacketType(const BYTE *data, size_t len) noexcept
    {
      if (len < 2) {
        return false;
      }
      BYTE pt = (data[1] & 0x7F);
      return (63 < pt) && (pt < 96);
    }

    //-------------------------------------------------------------------------
    bool RTPUtils::isValidRtpPayloadType(int payload_type) noexcept
    {
      return ortc::internal::IsValidRtpPayloadType(payload_type);
    }

    //-------------------------------------------------------------------------
    Time RTPUtils::ntpToTime(
                             DWORD ntpMS,
                             DWORD ntpLS
                             ) noexcept
    {
      // 0x3B9ACA00ULL = 1,000,000,000 (microsoeconds)
      // 0xFFFFFFFFULL = 2^32
      QWORD microseconds = (static_cast<QWORD>(ntpLS) * 0x3B9ACA00ULL) / 0xFFFFFFFFULL;

      Microseconds totalMicrosecondsSince(microseconds);

      Seconds secondsSince(static_cast<Seconds::rep>(static_cast<QWORD>(ntpMS)));

      std::tm tmepoch {};

      if (0 == (0x80000000 & ntpMS)) {
        // 7-Feb-2036 @ 06:28:16 UTC
        tmepoch.tm_year = 2036;
        tmepoch.tm_mon = 1; // 2-1
        tmepoch.tm_mday = 7;
        tmepoch.tm_hour = 6;
        tmepoch.tm_min = 28;
        tmepoch.tm_sec = 16;
      } else {
        // 1-Jan-1900 @ 00:00:00
        tmepoch.tm_year = 1900;
        tmepoch.tm_mon = 0; // 1-1
        tmepoch.tm_mday = 1;
        tmepoch.tm_hour = 0;
        tmepoch.tm_min = 0;
        tmepoch.tm_sec = 0;
      }

      time_t tepoch = std::mktime(&tmepoch);

      Time epoch = std::chrono::system_clock::from_time_t(tepoch);

      return (epoch + secondsSince) + totalMicrosecondsSince;
    }

    //-------------------------------------------------------------------------
    Log::Params RTPUtils::slog(const char *message) noexcept
    {
      return Log::Params(message, "ortc::RTPUtils");
    }
  } // namespace internal
}
