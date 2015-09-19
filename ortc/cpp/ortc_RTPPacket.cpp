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

#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/IHelper.h>
//#include <openpeer/services/IHTTP.h>
//
//#include <zsLib/Stringize.h>
//#include <zsLib/Log.h>
#include <zsLib/XML.h>
//
//#include <cryptopp/sha.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

#define RTP_HEADER_EXTENSION_BIT (0x10)

#define RTP_HEADER_VERSION(buffer) (((buffer[0]) & 0xC0) >> 6)
#define RTP_HEADER_PADDING(buffer) (0 != ((buffer[0]) & 0x20))
#define RTP_HEADER_EXTENSION(buffer) (0 != ((buffer[0] & RTP_HEADER_EXTENSION_BIT)))
#define RTP_HEADER_CC(buffer) ((buffer[0]) & 0xF)
#define RTP_HEADER_M(buffer) (0 != ((buffer[1]) & 0x80))
#define RTP_HEADER_PT(buffer) ((buffer[1]) & 0x7F)

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
//  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)
//
//  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    static const size_t kMinRtpPacketLen = 12;
    static const BYTE kRtpVersion = 2;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPPacket (public)
    #pragma mark

    //-------------------------------------------------------------------------
    RTPPacket::RTPPacket(const make_private &)
    {
    }

    //-------------------------------------------------------------------------
    RTPPacket::~RTPPacket()
    {
      if (mHeaderExtensions) {
        delete [] mHeaderExtensions;
        mHeaderExtensions = NULL;
      }
    }

    //-------------------------------------------------------------------------
    RTPPacketPtr RTPPacket::create(const BYTE *buffer, size_t bufferLengthInBytes)
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!buffer)
      ORTC_THROW_INVALID_PARAMETERS_IF(0 == bufferLengthInBytes)
      return RTPPacket::create(UseServicesHelper::convertToBuffer(buffer, bufferLengthInBytes));
    }

    //-------------------------------------------------------------------------
    RTPPacketPtr RTPPacket::create(const SecureByteBlock &buffer)
    {
      return RTPPacket::create(buffer.BytePtr(), buffer.SizeInBytes());
    }

    //-------------------------------------------------------------------------
    RTPPacketPtr RTPPacket::create(SecureByteBlockPtr buffer)
    {
      RTPPacketPtr pThis(make_shared<RTPPacket>(make_private{}));
      pThis->mBuffer = buffer;
      if (!pThis->parse()) {
        ZS_LOG_WARNING(Debug, pThis->log("packet could not be parsed"))
        return RTPPacketPtr();
      }
      return pThis;
    }

    //-------------------------------------------------------------------------
    const BYTE *RTPPacket::ptr() const
    {
      return mBuffer->BytePtr();
    }

    //-------------------------------------------------------------------------
    size_t RTPPacket::size() const
    {
      return mBuffer->SizeInBytes();
    }

    //-------------------------------------------------------------------------
    SecureByteBlockPtr RTPPacket::buffer() const
    {
      return mBuffer;
    }

    //-------------------------------------------------------------------------
    DWORD RTPPacket::getCSRC(size_t index) const
    {
      ASSERT(index < cc())
      return UseHelper::getBE32(&((ptr())[kMinRtpPacketLen + (sizeof(DWORD)*index)]));
    }

    //-------------------------------------------------------------------------
    const BYTE *RTPPacket::payload() const
    {
      const BYTE *buffer = ptr();
      return &(buffer[mHeaderSize + mHeaderExtensionSize]);
    }

    //-------------------------------------------------------------------------
    RTPPacket::HeaderExtension *RTPPacket::getHeaderExtensionAtIndex(size_t index) const
    {
      if (index >= mTotalHeaderExtensions) return NULL;
      return &(mHeaderExtensions[index]);
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPPacket::toDebug() const
    {
      ElementPtr objectEl = Element::create("ortc::RTPPacket");

      UseServicesHelper::debugAppend(objectEl, "buffer", mBuffer ? mBuffer->SizeInBytes() : 0);

      UseServicesHelper::debugAppend(objectEl, "version", mVersion);
      UseServicesHelper::debugAppend(objectEl, "padding", mPadding);
      UseServicesHelper::debugAppend(objectEl, "cc", mCC);
      UseServicesHelper::debugAppend(objectEl, "m", mM);
      UseServicesHelper::debugAppend(objectEl, "pt", mPT);
      UseServicesHelper::debugAppend(objectEl, "sequence number", mSequenceNumber);
      UseServicesHelper::debugAppend(objectEl, "timestamp", mTimestamp);
      UseServicesHelper::debugAppend(objectEl, "ssrc", mSSRC);

      UseServicesHelper::debugAppend(objectEl, "header size", mHeaderSize);
      UseServicesHelper::debugAppend(objectEl, "extension header size", mHeaderExtensionSize);
      UseServicesHelper::debugAppend(objectEl, "payload size", mPayloadSize);

      UseServicesHelper::debugAppend(objectEl, "total header extensions", mTotalHeaderExtensions);

      for (auto current = mHeaderExtensions; NULL != current; current = current->mNext)
      {
        ElementPtr extensionEl = Element::create("extension");
        UseServicesHelper::debugAppend(extensionEl, "id", current->mID);
        UseServicesHelper::debugAppend(extensionEl, "size", current->mDataSizeInBytes);
        UseServicesHelper::debugAppend(extensionEl, "data", (NULL != current->mData) ? UseServicesHelper::convertToHex(current->mData, current->mDataSizeInBytes) : String());
        UseServicesHelper::debugAppend(extensionEl, "padding", current->mPostPaddingSize);
        UseServicesHelper::debugAppend(objectEl, extensionEl);
      }

      UseServicesHelper::debugAppend(objectEl, "header extension appbits", mHeaderExtensionAppBits);

      UseServicesHelper::debugAppend(objectEl, "header extension prepadding size", mHeaderExtensionPrepaddedSize);
      UseServicesHelper::debugAppend(objectEl, "header extension parse stopped pos", (NULL != mHeaderExtensionParseStoppedPos) ? ((PTRNUMBER)(mHeaderExtensionParseStoppedPos - ptr())) : 0);
      UseServicesHelper::debugAppend(objectEl, "header extension parse stopped size", mHeaderExtensionParseStoppedSize);

      return objectEl;
    }

    //-------------------------------------------------------------------------
    void RTPPacket::changeHeaderExtensions(HeaderExtension *firstExtension)
    {
      bool twoByteHeader = (0 != mHeaderExtensionAppBits);
      if (!twoByteHeader) {
        for (HeaderExtension *current = firstExtension; NULL != current; current = current->mNext) {
          ASSERT(0 != current->mID) // not legal

          if (current->mID > 14) {
            twoByteHeader = true;
            break;
          }
          if (0 == current->mDataSizeInBytes) {
            twoByteHeader = true;
            break;
          }
          if (current->mDataSizeInBytes > 16) {
            ASSERT(current->mDataSizeInBytes < 255)
            twoByteHeader = true;
            break;
          }
        }
      }

      if (twoByteHeader) {
        ORTC_THROW_INVALID_STATE_IF(NULL != mHeaderExtensionParseStoppedPos)  // requires a 1 byte header to append this data
      }

      bool requiresExtension = (NULL != firstExtension) ||
                               (0 != mHeaderExtensionAppBits) ||
                               (0 != mHeaderExtensionPrepaddedSize) ||
                               (NULL != mHeaderExtensionParseStoppedPos);

      const BYTE *buffer = ptr();

      ASSERT(NULL != buffer)

      size_t existingHeaderExtensionSize = mHeaderExtensionSize;
      size_t postHeaderExtensionSize = mPayloadSize + mPadding;

      ASSERT(size() == (mHeaderSize + existingHeaderExtensionSize + postHeaderExtensionSize))

      if (!requiresExtension) {
        // going to strip the extension header out entirely

        if (RTP_HEADER_EXTENSION(buffer)) {
          ZS_LOG_INSANE(log("no extension present (thus no need to strip extension from RTP packet)"))
          return;
        }

        ASSERT(size() == (mHeaderSize + existingHeaderExtensionSize + postHeaderExtensionSize))

        size_t newSize = mHeaderSize + postHeaderExtensionSize;

        SecureByteBlockPtr tempBuffer(make_shared<SecureByteBlock>(newSize));

        BYTE *newBuffer = tempBuffer->BytePtr();

        // copy the bytes before the extension header to the new buffer
        memcpy(newBuffer, buffer, mHeaderSize);

        // copy the bytes from after the extension header in the older buffer to the new buffer
        memcpy(&(newBuffer[mHeaderSize]), &(buffer[mHeaderSize + existingHeaderExtensionSize]), postHeaderExtensionSize);

        // strip the extension bit
        newBuffer[0] = newBuffer[0] & (0xFF ^ RTP_HEADER_EXTENSION_BIT);

        mBuffer = tempBuffer;

        mHeaderExtensionSize = 0;

        mTotalHeaderExtensions = 0;
        if (mHeaderExtensions) {
          delete [] mHeaderExtensions;
          mHeaderExtensions = NULL;
        }
        mHeaderExtensionAppBits = 0;
        mHeaderExtensionPrepaddedSize = 0;
        mHeaderExtensionParseStoppedPos = NULL;
        mHeaderExtensionParseStoppedSize = 0;

        ZS_LOG_INSANE(debug("stipped existing extension header"))
        return;
      }

      size_t newHeaderExtensionSize = sizeof(DWORD) + mHeaderExtensionPrepaddedSize + mHeaderExtensionParseStoppedSize;

      size_t newTotalExtensions = 0;
      for (HeaderExtension *current = firstExtension; NULL != current; current = current->mNext) {
        ++newTotalExtensions;
        newHeaderExtensionSize += (twoByteHeader ? 2 : 1) + (current->mDataSizeInBytes) + current->mPostPaddingSize;
      }

      size_t modulas = (newHeaderExtensionSize % sizeof(DWORD));

      if (0 != modulas) {
        newHeaderExtensionSize += (sizeof(DWORD) - modulas);
      }

      size_t newSize = mHeaderSize + newHeaderExtensionSize + postHeaderExtensionSize;

      SecureByteBlockPtr tempBuffer(make_shared<SecureByteBlock>(newSize));

      BYTE *newBuffer = tempBuffer->BytePtr();

      // copy the bytes before the extension header to the new buffer
      memcpy(newBuffer, buffer, mHeaderSize);

      // copy the bytes from after the extension header in the older buffer to the new buffer
      memcpy(&(newBuffer[mHeaderSize + newHeaderExtensionSize]), &(buffer[mHeaderSize + existingHeaderExtensionSize]), postHeaderExtensionSize);

      // set the extension bit
      newBuffer[0] = newBuffer[0] | RTP_HEADER_EXTENSION_BIT;

      BYTE *newProfilePos = &(newBuffer[mHeaderSize]);

      if (twoByteHeader) {
        WORD profileType = (0x100 << 4) | (mHeaderExtensionAppBits & 0xF);
        newProfilePos[0] = static_cast<BYTE>((profileType & 0xFF00) >> 8);
        newProfilePos[1] = static_cast<BYTE>(profileType & 0xFF);
      } else {
        newProfilePos[0] = 0xBE;
        newProfilePos[1] = 0xDE;
      }

      WORD totalDWORDsLength = ((newHeaderExtensionSize - sizeof(DWORD)) / sizeof(DWORD));
      newProfilePos[2] = static_cast<BYTE>((totalDWORDsLength & 0xFF00) >> 8);
      newProfilePos[3] = static_cast<BYTE>(totalDWORDsLength & 0xFF);

      BYTE *pos = &(newProfilePos[sizeof(DWORD)]) + mHeaderExtensionPrepaddedSize;

      HeaderExtension *newExtensions = NULL;
      if (0 != newTotalExtensions) {
        newExtensions = new HeaderExtension[newTotalExtensions] {};
      }

      size_t index = 0;
      for (HeaderExtension *current = firstExtension; NULL != current; current = current->mNext, ++index) {
        HeaderExtension *newCurrent = &(newExtensions[index]);
        if (0 != index) {
          newExtensions[index-1].mNext = newCurrent;
        }

        if (twoByteHeader) {
          pos[0] = current->mID;
          pos[1] = current->mDataSizeInBytes;
          pos += 2;
        } else {
          pos[0] = ((current->mID) << 4) | ((current->mDataSizeInBytes - 1) & 0xF);
          ++pos;
        }

        newCurrent->mID = current->mID;
        newCurrent->mData = pos;
        newCurrent->mDataSizeInBytes = current->mDataSizeInBytes;
        newCurrent->mPostPaddingSize = current->mPostPaddingSize;

        if (0 != current->mDataSizeInBytes) {
          memcpy(pos, current->mData, current->mDataSizeInBytes);
          pos += current->mDataSizeInBytes;
        }
        pos += current->mPostPaddingSize;
      }

      if (0 != mHeaderExtensionParseStoppedSize) {
        ASSERT(NULL != mHeaderExtensionParseStoppedPos)

        memcpy(pos, mHeaderExtensionParseStoppedPos, mHeaderExtensionParseStoppedSize);
        mHeaderExtensionParseStoppedPos = pos;
      } else {
        mHeaderExtensionParseStoppedPos = NULL;
      }

      mTotalHeaderExtensions = index;
      if (NULL != mHeaderExtensions) {
        delete [] mHeaderExtensions;
        mHeaderExtensions = NULL;
      }
      mHeaderExtensions = newExtensions;
      mHeaderExtensionSize = newHeaderExtensionSize;

      mBuffer = tempBuffer; // replace existing buffer with new buffer

      ZS_LOG_INSANE(debug("header extension changed"))
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPPacket (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPPacket::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::RTPPacket");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPPacket::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPPacket");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPPacket::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    bool RTPPacket::parse()
    {
      const BYTE *buffer = mBuffer->BytePtr();
      size_t size = mBuffer->SizeInBytes();

      if (size < kMinRtpPacketLen) {
        ZS_LOG_WARNING(Trace, log("packet length is too short") + ZS_PARAM("length", size))
        return false;
      }

      mVersion = RTP_HEADER_VERSION(buffer);
      if (mVersion != kRtpVersion) {
        ZS_LOG_WARNING(Trace, log("not an RTP packet"))
        return false;
      }

      if (UseHelper::isRTCPPacketType(buffer, size)) {
        ZS_LOG_WARNING(Trace, log("packet is RTCP not RTP") + ZS_PARAM("length", size))
        return false;
      }

      bool hasPadding = RTP_HEADER_PADDING(buffer);
      mCC = RTP_HEADER_CC(buffer);
      mM = RTP_HEADER_M(buffer);
      mPT = RTP_HEADER_PT(buffer);
      mSequenceNumber = UseHelper::getBE16(&(buffer[2]));
      mTimestamp = UseHelper::getBE32(&(buffer[4]));
      mSSRC = UseHelper::getBE32(&(buffer[8]));

      mHeaderSize = kMinRtpPacketLen + (static_cast<size_t>(mCC) * sizeof(DWORD));

      if (size < mHeaderSize) {
        ZS_LOG_WARNING(Trace, debug("illegal RTP packet"))
        return false;
      }

      if (RTP_HEADER_EXTENSION(buffer)) {
        if (size < (mHeaderSize + sizeof(DWORD))) {
          ZS_LOG_WARNING(Trace, debug("illegal RTP packet"))
          return false;
        }

        mHeaderExtensionSize = (static_cast<size_t>(UseHelper::getBE16(&(buffer[mHeaderSize + 2]))) * sizeof(DWORD)) + sizeof(DWORD);
        if (size < (mHeaderSize + mHeaderExtensionSize)) {
          ZS_LOG_WARNING(Trace, debug("illegal RTP packet"))
          return false;
        }
      }

      if (hasPadding) {
        mPadding = static_cast<size_t>(buffer[size-1]);
        if (0 == mPadding) {
          ZS_LOG_WARNING(Trace, debug("illegal RTP packet (no padding size)"))
          return false;
        }

        if (size < (mHeaderSize + mHeaderExtensionSize + mPadding)) {
          ZS_LOG_WARNING(Trace, debug("illegal RTP packet"))
          return false;
        }
      }

      mPayloadSize = size - (mHeaderSize + mHeaderExtensionSize + mPadding);

      if (0 == mHeaderExtensionSize) {
        // no extensions present
        ZS_LOG_INSANE(debug("parsed"))
        return true;
      }

      const BYTE *profilePos = &(buffer[mHeaderSize]);

      bool oneByte = false;

      if ((0xBE == profilePos[0]) &&
          (0xDE == profilePos[1])) {
        oneByte = true;
      } else {
        WORD twoByteHeader = UseHelper::getBE16(profilePos);
        mHeaderExtensionAppBits = (twoByteHeader & 0xF);

        if (0x100 != ((twoByteHeader & 0xFFF0) >> 4)) {
          ZS_LOG_WARNING(Trace, log("header extension profile is not understood") + ZS_PARAM("profile", twoByteHeader))
          return false;
        }
      }

      size_t remaining = mHeaderExtensionSize - sizeof(DWORD);
      if (0 == remaining) {
        ZS_LOG_INSANE(debug("parsed"))
        return true;
      }

      const BYTE *pos = &(profilePos[4]);

      size_t totalPossibleHeaderExtensions = (remaining / sizeof(WORD)) + ((0 != (remaining % sizeof(WORD))) ? 1 : 0);

      mHeaderExtensions = new HeaderExtension[totalPossibleHeaderExtensions] {};

      size_t totalFound = 0;

      while ((remaining > 0) &&
             (totalFound < totalPossibleHeaderExtensions)) {

        HeaderExtension *current = &(mHeaderExtensions[totalFound]);

        if (0 == pos[0]) {
          // see https://tools.ietf.org/html/rfc5285 4.1

          // A sequence of extension elements, possibly with padding, forms the
          // header extension defined in the RTP specification.  There are as many
          // extension elements as fit into the length as indicated in the RTP
          // header extension length.  Since this length is signaled in full 32-
          // bit words, padding bytes are used to pad to a 32-bit boundary.  The
          // entire extension is parsed byte-by-byte to find each extension
          // element (no alignment is required), and parsing stops at the earlier
          // of the end of the entire header extension, or, in one-byte headers,
          // on encountering an identifier with the reserved value of 15.

          // In both forms, padding bytes have the value of 0 (zero).  They may be
          // placed between extension elements, if desired for alignment, or after
          // the last extension element, if needed for padding.  A padding byte
          // does not supply the ID of an element, nor the length field.  When a
          // padding byte is found, it is ignored and the parser moves on to
          // interpreting the next byte.

          // skip padding byte
          ++pos;
          --remaining;
          if (0 == totalFound) {
            ++mHeaderExtensionPrepaddedSize;
          } else {
            ++(mHeaderExtensions[totalFound-1].mPostPaddingSize);
          }
          continue;
        }

        if (oneByte) {
          BYTE id = ((pos[0] & 0xF0) >> 4);
          if (id == 0xF) {
            // see https://tools.ietf.org/html/rfc5285 4.2

            // The local identifier value 15 is reserved for future extension and
            // MUST NOT be used as an identifier.  If the ID value 15 is
            // encountered, its length field should be ignored, processing of the
            // entire extension should terminate at that point, and only the
            // extension elements present prior to the element with ID 15
            // considered.
            mHeaderExtensionParseStoppedPos = pos;
            mHeaderExtensionParseStoppedSize = remaining;
            break;
          }

          // The 4-bit length is the number minus one of data bytes of this header
          // extension element following the one-byte header.  Therefore, the
          // value zero in this field indicates that one byte of data follows, and
          // a value of 15 (the maximum) indicates element data of 16 bytes.
          // (This permits carriage of 16-byte values, which is a common length of
          // labels and identifiers, while losing the possibility of zero-length
          // values -- which would often be padded anyway.)

          size_t length = static_cast<size_t>((pos[0] & 0x0F) + 1);

          if (remaining < (1 + length)) {
            ZS_LOG_WARNING(Trace, log("extension header is not valid") + ZS_PARAM("id", id) + ZS_PARAM("remaining", remaining) + ZS_PARAM("length", length))
            return false;
          }

          current->mID = id;
          current->mDataSizeInBytes = length;
          current->mData = &(pos[1]);
          if (0 != totalFound) {
            mHeaderExtensions[totalFound-1].mNext = current;
          }

          remaining -= (1 + length);
          pos += (1 + length);
          ++totalFound;
          continue;
        }

        // must be a two byte header format

        if (remaining < sizeof(WORD)) {
          ZS_LOG_WARNING(Trace, log("extension header is not valid") + ZS_PARAM("remaining", remaining))
          return false;
        }

        BYTE id = (pos[0]);
        size_t length = (pos[1]);

        if (remaining < (sizeof(WORD) + length)) {
          ZS_LOG_WARNING(Trace, log("extension header is not valid") + ZS_PARAM("id", id) + ZS_PARAM("remaining", remaining) + ZS_PARAM("length", length))
          return false;
        }

        current->mID = id;
        current->mDataSizeInBytes = length;
        if (0 != length) {
          current->mData = &(pos[2]);
        }
        if (0 != totalFound) {
          mHeaderExtensions[totalFound-1].mNext = current;
        }

        remaining -= (2 + length);
        pos += (2 + length);
        ++totalFound;
        continue;
      }

      mTotalHeaderExtensions = totalFound;
      
      ZS_LOG_INSANE(debug("parsed"))
      return true;
    }
    
  }

}
