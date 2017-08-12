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

#include <ortc/RTPPacket.h>
#include <ortc/internal/ortc.events.h>

#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_RTPUtils.h>
#include <ortc/internal/platform.h>

#include <ortc/services/IHelper.h>

//#include <ortc/services/IHTTP.h>
//
//#include <zsLib/Stringize.h>
//#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/integer.h>

#include <sstream>


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

#define RTP_IS_FLAG_SET(xByte, xBitPos) (0 != ((xByte) & (1 << xBitPos)))
#define RTP_GET_BITS(xByte, xBitPattern, xLowestBit) (((xByte) >> (xLowestBit)) & (xBitPattern))
#define RTP_PACK_BITS(xByte, xBitPattern, xLowestBit) (((xByte) & (xBitPattern)) << (xLowestBit))

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtp_rtcp_packet) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper);
  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper);
  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPUtils, UseRTPUtils);

  using CryptoPP::Integer;

  //  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP)
  //
  //  typedef ortc::services::Hasher<CryptoPP::SHA1> SHA1Hasher;


  namespace internal
  {
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
    static bool requiredExtension(
                                  RTPPacket::HeaderExtension *firstExtension,
                                  BYTE headerExtensionAppBits,
                                  size_t headerExtensionPrepaddedSize,
                                  const BYTE *headerExtensionParseStoppedPos
                                  )
    {
      return (NULL != firstExtension) ||
      (0 != headerExtensionAppBits) ||
      (0 != headerExtensionPrepaddedSize) ||
      (NULL != headerExtensionParseStoppedPos);
    }

    //-------------------------------------------------------------------------
    static void getHeaderExtensionSize(
                                       RTPPacket::HeaderExtension *firstExtension,
                                       bool twoByteHeader,
                                       size_t headerExtensionPrepaddedSize,
                                       size_t headerExtensionParseStoppedSize,
                                       size_t &outSizeInBytes,
                                       size_t &outTotalHeaderExtensions
                                       )
    {
      typedef RTPPacket::HeaderExtension HeaderExtension;

      outSizeInBytes = sizeof(DWORD) + headerExtensionPrepaddedSize + headerExtensionParseStoppedSize;

      outTotalHeaderExtensions = 0;
      for (HeaderExtension *current = firstExtension; NULL != current; current = current->mNext) {
        ++outTotalHeaderExtensions;
        outSizeInBytes += (twoByteHeader ? 2 : 1) + (current->mDataSizeInBytes) + current->mPostPaddingSize;
      }

      size_t modulas = (outSizeInBytes % sizeof(DWORD));

      if (0 != modulas) {
        outSizeInBytes += (sizeof(DWORD) - modulas);
      }
    }
  } // namespace internal

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::HeaderExtension
  #pragma mark

  //---------------------------------------------------------------------------
  void RTPPacket::HeaderExtension::trace(const char *message) const
  {
    ZS_EVENTING_5(x, i, Insane, RTPPacketHeaderExtension, ol, RtpPacket, Info, 
      string, message, message,
      byte, id, mID,
      binary, data, mData, 
      size, dataSizeInBytes, mDataSizeInBytes, 
      size_t, postPaddingSize, mPostPaddingSize
    );
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::ClientToMixerExtension
  #pragma mark

  //---------------------------------------------------------------------------
  RTPPacket::ClientToMixerExtension::ClientToMixerExtension(const HeaderExtension &header)
  {
    mID = header.mID;
    mDataSizeInBytes = header.mDataSizeInBytes;
    mPostPaddingSize = header.mPostPaddingSize;
    if (NULL == header.mData) mDataSizeInBytes = 0;
    if (0 == mDataSizeInBytes) mData = NULL;

    if (sizeof(BYTE) == header.mDataSizeInBytes) {
      memcpy(&mLevelBuffer, header.mData, sizeof(BYTE));
      mData = &mLevelBuffer;
    }
  }

  //---------------------------------------------------------------------------
  RTPPacket::ClientToMixerExtension::ClientToMixerExtension(
                                                            BYTE id,
                                                            bool voiceActivity,
                                                            BYTE level
                                                            )
  {
    mID = id;
    mData = &mLevelBuffer;
    mDataSizeInBytes = sizeof(BYTE);

    mLevelBuffer = static_cast<BYTE>(voiceActivity ? 0x80 : 0x00) | (level & 0x7F);
  }

  //---------------------------------------------------------------------------
  bool RTPPacket::ClientToMixerExtension::voiceActivity() const
  {
    return RTP_IS_FLAG_SET(mLevelBuffer, 7);
  }

  //---------------------------------------------------------------------------
  BYTE RTPPacket::ClientToMixerExtension::level() const
  {
    return RTP_GET_BITS(mLevelBuffer, 0x7F, 0);
  }

  //---------------------------------------------------------------------------
  void RTPPacket::ClientToMixerExtension::trace(const char *message) const
  {
    ZS_EVENTING_7(x, i, Insane, RTPPacketClientToMixerExtension, ol, RtpPacket, Info,
      string, message, message,
      byte, id, mID,
      binary, data, mData,
      size, dataSizeInBytes, mDataSizeInBytes,
      size_t, postPaddingSize, mPostPaddingSize,
      bool, voiceActivity, voiceActivity(),
      byte, level, level()
    );
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::MixerToClientExtension
  #pragma mark

  //---------------------------------------------------------------------------
  RTPPacket::MixerToClientExtension::MixerToClientExtension(const HeaderExtension &header)
  {
    mID = header.mID;
    mDataSizeInBytes = header.mDataSizeInBytes;
    mPostPaddingSize = header.mPostPaddingSize;
    if (NULL == header.mData) mDataSizeInBytes = 0;
    if (0 == mDataSizeInBytes) mData = NULL;

    size_t copySize = header.mDataSizeInBytes;
    if (copySize > sizeof(mLevelBuffer)) copySize = sizeof(mLevelBuffer);

    if (0 != copySize) {
      memcpy(&(mLevelBuffer[0]), header.mData, copySize);
      mData = &(mLevelBuffer[0]);
    }
    mDataSizeInBytes = copySize;
  }

  //---------------------------------------------------------------------------
  RTPPacket::MixerToClientExtension::MixerToClientExtension(
                                                            BYTE id,
                                                            BYTE *levels,
                                                            size_t count
                                                            )
  {
    mID = id;
    mDataSizeInBytes = count * sizeof(BYTE);

    if (mDataSizeInBytes > sizeof(mLevelBuffer)) {
      count = sizeof(mLevelBuffer)/sizeof(BYTE);
    }

    if (0 != mDataSizeInBytes) {
      mData = &(mLevelBuffer[0]);
      memcpy(&(mLevelBuffer[0]), levels, mDataSizeInBytes);
    }
  }

  //---------------------------------------------------------------------------
  size_t RTPPacket::MixerToClientExtension::levelsCount() const
  {
    return mDataSizeInBytes / sizeof(BYTE);
  }

  //---------------------------------------------------------------------------
  BYTE RTPPacket::MixerToClientExtension::unusedBit(size_t index) const
  {
    ASSERT(index < levelsCount())
    return RTP_GET_BITS(mLevelBuffer[index], 0x80, 7);
  }

  //---------------------------------------------------------------------------
  BYTE RTPPacket::MixerToClientExtension::level(size_t index) const
  {
    ASSERT(index < levelsCount())
    return RTP_GET_BITS(mLevelBuffer[index], 0x7F, 0);
  }

  //---------------------------------------------------------------------------
  void RTPPacket::MixerToClientExtension::trace(const char *message) const
  {
    ZS_EVENTING_6(x, i, Insane, RTPPacketMixerToClientExtension, ol, RtpPacket, Info,
      string, message, message,
      byte, id, mID,
      binary, data, mData,
      size, dataSizeInBytes, mDataSizeInBytes,
      size_t, postPaddingSize, mPostPaddingSize,
      size_t, levelsCount, levelsCount()
    );
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::StringHeaderExtension
  #pragma mark

  //---------------------------------------------------------------------------
  RTPPacket::StringHeaderExtension::StringHeaderExtension(const HeaderExtension &header)
  {
    ASSERT(sizeof(char) == sizeof(BYTE))

    mID = header.mID;
    mDataSizeInBytes = header.mDataSizeInBytes;
    mPostPaddingSize = header.mPostPaddingSize;
    if (NULL == header.mData) mDataSizeInBytes = 0;
    if (0 == mDataSizeInBytes) mData = NULL;

    size_t copySize = header.mDataSizeInBytes;
    if (copySize > (sizeof(BYTE)*kMaxStringLength)) copySize = (sizeof(BYTE)*kMaxStringLength);

    if (0 != copySize) {
      memcpy(&(mStringBuffer[0]), header.mData, copySize);
      mData = &(mStringBuffer[0]);
    }
    mDataSizeInBytes = copySize;
  }

  //---------------------------------------------------------------------------
  RTPPacket::StringHeaderExtension::StringHeaderExtension(
                                                          BYTE id,
                                                          const char *mid
                                                          )
  {
    mID = id;
    mData = &(mStringBuffer[0]);
    mDataSizeInBytes = sizeof(char)*(NULL != mid ? strlen(mid) : 0);

    if (mDataSizeInBytes > (sizeof(BYTE)*kMaxStringLength)) mDataSizeInBytes = (sizeof(BYTE)*kMaxStringLength);

    if (0 != mDataSizeInBytes) {
      memcpy(&(mStringBuffer[0]), mid, mDataSizeInBytes);
    }
  }

  //---------------------------------------------------------------------------
  const char *RTPPacket::StringHeaderExtension::str() const
  {
    return reinterpret_cast<const char *>(&mStringBuffer[0]);
  }

  //---------------------------------------------------------------------------
  void RTPPacket::StringHeaderExtension::trace(const char *message) const
  {
    ZS_EVENTING_6(x, i, Insane, RTPPacketStringHeaderExtension, ol, RtpPacket, Info,
      string, message, message,
      byte, id, mID,
      binary, data, mData,
      size, dataSizeInBytes, mDataSizeInBytes,
      size_t, postPaddingSize, mPostPaddingSize,
      string, str, str()
    );
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::NumberHeaderExtension
  #pragma mark

  //---------------------------------------------------------------------------
  RTPPacket::NumberHeaderExtension::NumberHeaderExtension(const HeaderExtension &header)
  {
    ASSERT(sizeof(char) == sizeof(BYTE))

    mID = header.mID;
    mDataSizeInBytes = header.mDataSizeInBytes;
    mPostPaddingSize = header.mPostPaddingSize;
    if (NULL == header.mData) mDataSizeInBytes = 0;
    if (0 == mDataSizeInBytes) mData = NULL;

    size_t copySize = header.mDataSizeInBytes;
    if (copySize > (sizeof(BYTE)*kMaxNumberByteLength)) copySize = (sizeof(BYTE)*kMaxNumberByteLength);

    if (0 != copySize) {
      memcpy(&(mNumberBuffer[0]), header.mData, copySize);
      mData = &(mNumberBuffer[0]);
    }
    mDataSizeInBytes = copySize;
  }

  //---------------------------------------------------------------------------
  RTPPacket::NumberHeaderExtension::NumberHeaderExtension(
                                                          BYTE id,
                                                          const BYTE *number,
                                                          size_t lengthInBytes
                                                          )
  {
    mID = id;
    mData = &(mNumberBuffer[0]);
    mDataSizeInBytes = sizeof(char)*(NULL != number ? lengthInBytes : 0);

    if (mDataSizeInBytes > (sizeof(BYTE)*kMaxNumberByteLength)) mDataSizeInBytes = (sizeof(BYTE)*kMaxNumberByteLength);

    if (0 != mDataSizeInBytes) {
      memcpy(&(mNumberBuffer[0]), number, mDataSizeInBytes);
    }
  }

  //---------------------------------------------------------------------------
  RTPPacket::NumberHeaderExtension::NumberHeaderExtension(
                                                          BYTE id,
                                                          const char *valueInBase10
                                                          )
  {
    mID = id;
    mData = &(mNumberBuffer[0]);
    mDataSizeInBytes = 1;

    String str(valueInBase10);

    try {
      Integer value(str); // convert from base 10 into big number class

      size_t minSizeNeeded = value.MinEncodedSize();
      ORTC_THROW_INVALID_PARAMETERS_IF(minSizeNeeded > kMaxNumberByteLength)

      // this will encode in big endian and pad with most significant "0"
      // values as needed
      value.Encode(&(mNumberBuffer[0]), minSizeNeeded);

      mDataSizeInBytes = minSizeNeeded;

    } catch(...) {
      ORTC_THROW_INVALID_PARAMETERS("unable to convert integer: " + str)
    }
  }

  //---------------------------------------------------------------------------
  const BYTE *RTPPacket::NumberHeaderExtension::number() const
  {
    return &(mNumberBuffer[0]);
  }

  //---------------------------------------------------------------------------
  size_t RTPPacket::NumberHeaderExtension::length() const
  {
    return mDataSizeInBytes;
  }

  //---------------------------------------------------------------------------
  String RTPPacket::NumberHeaderExtension::str() const
  {
    if (mDataSizeInBytes < 1) return String();

    Integer value(&(mNumberBuffer[0]), mDataSizeInBytes);

    std::stringstream output;

    output << value;

    return output.str();
  }

  //---------------------------------------------------------------------------
  void RTPPacket::NumberHeaderExtension::trace(const char *message) const
  {
    ZS_EVENTING_6(x, i, Insane, RTPPacketNumberHeaderExtension, ol, RtpPacket, Info,
      string, message, message,
      byte, id, mID,
      binary, data, mData,
      size, dataSizeInBytes, mDataSizeInBytes,
      size_t, postPaddingSize, mPostPaddingSize,
      string, number, str()
    );
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::MidHeaderExtension
  #pragma mark

  //---------------------------------------------------------------------------
  void RTPPacket::MidHeaderExtension::trace(const char *message) const
  {
    ZS_EVENTING_6(x, i, Insane, RTPPacketMidHeaderExtension, ol, RtpPacket, Info,
      string, message, message,
      byte, id, mID,
      bool, data, mData ? true : false,
      size_t, dataSizeInBytes, mDataSizeInBytes,
      size_t, postPaddingSize, mPostPaddingSize,
      string, mid, mid()
    );
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::RidHeaderExtension
  #pragma mark

  //---------------------------------------------------------------------------
  void RTPPacket::RidHeaderExtension::trace(const char *message) const
  {
    ZS_EVENTING_6(x, i, Insane, RTPPacketRidHeaderExtension, ol, RtpPacket, Info,
      string, message, message,
      byte, id, mID,
      binary, data, mData,
      size, dataSizeInBytes, mDataSizeInBytes,
      size_t, postPaddingSize, mPostPaddingSize,
      string, rid, rid()
    );
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::VideoOrientationHeaderExtension
  #pragma mark

  //---------------------------------------------------------------------------
  RTPPacket::VideoOrientationHeaderExtension::VideoOrientationHeaderExtension(const HeaderExtension &header)
  {
    mID = header.mID;
    mDataSizeInBytes = header.mDataSizeInBytes;
    mPostPaddingSize = header.mPostPaddingSize;
    if (NULL == header.mData) mDataSizeInBytes = 0;

    if (mDataSizeInBytes < sizeof(mEncoded)) return; // cannot decrypt this packet
  }

  //---------------------------------------------------------------------------
  RTPPacket::VideoOrientationHeaderExtension::VideoOrientationHeaderExtension(const VideoOrientation6HeaderExtension &)
  {
    mData = &(mEncoded[0]);
    mDataSizeInBytes = sizeof(mEncoded);
  }

  //---------------------------------------------------------------------------
  RTPPacket::VideoOrientationHeaderExtension::VideoOrientationHeaderExtension(
                                                                              const Clockwise &,
                                                                              bool frontFacingCamera,
                                                                              bool flip,
                                                                              UINT orientation
                                                                              )
  {
    mData = &(mEncoded[0]);
    mDataSizeInBytes = sizeof(mEncoded);

    orientation %= 360;

    // find closest approximation to the video orientation for the 2 bit allowance
    orientation += 45;  // range is now 45 -> 404
    orientation /= 90;
    orientation %= 4;   // range is now 0..3

    // Bit#       7   6   5   4   3   2    1     0 (LSB)
    // Definition 0   0   0   0   C   F   R1    R0

    mEncoded[0] = RTP_PACK_BITS(frontFacingCamera ? 1 : 0, 0x1, 3) |
                  RTP_PACK_BITS(flip ? 1 : 0, 0x1, 2) |
                  RTP_PACK_BITS(static_cast<BYTE>(orientation), 0x3, 0);
  }

  //---------------------------------------------------------------------------
  RTPPacket::VideoOrientationHeaderExtension::VideoOrientationHeaderExtension(
                                                                              const CounterClockwise &,
                                                                              bool frontFacingCamera, // true = front facing, false = backfacing
                                                                              bool flip, // horizontal left-right flip (mirro)
                                                                              UINT orientation
                                                                              ) :
    VideoOrientationHeaderExtension(Clockwise {}, frontFacingCamera, flip, 360-(orientation%360))
  {
  }

  //---------------------------------------------------------------------------
  bool RTPPacket::VideoOrientationHeaderExtension::frontFacing() const
  {
    if (NULL == mData) return false;
    return (0 != RTP_GET_BITS(mData[0], 0x1, 3));
  }

  //---------------------------------------------------------------------------
  bool RTPPacket::VideoOrientationHeaderExtension::backFacing() const
  {
    if (NULL == mData) return false;
    return (0 == RTP_GET_BITS(mData[0], 0x1, 3));
  }

  //---------------------------------------------------------------------------
  bool RTPPacket::VideoOrientationHeaderExtension::flip() const
  {
    if (NULL == mData) return false;
    return (0 == RTP_GET_BITS(mData[0], 0x1, 2));
  }

  //---------------------------------------------------------------------------
  UINT RTPPacket::VideoOrientationHeaderExtension::degreesClockwise() const
  {
    if (NULL == mData) return 0;
    UINT degrees = static_cast<UINT>(RTP_GET_BITS(mData[0], 0x3, 0));

    degrees *= 90;

    return degrees;
  }

  //---------------------------------------------------------------------------
  UINT RTPPacket::VideoOrientationHeaderExtension::degreesCounterClockwise() const
  {
    return (360-degreesClockwise())%360;
  }

  //---------------------------------------------------------------------------
  void RTPPacket::VideoOrientationHeaderExtension::trace(const char *message) const
  {
    ZS_EVENTING_8(x, i, Insane, RTPPacketVideoOrientationHeaderExtension, ol, RtpPacket, Info,
      string, message, message,
      byte, id, mID,
      binary, data, mData,
      size, dataSizeInBytes, mDataSizeInBytes,
      size_t, postPaddingSize, mPostPaddingSize,
      bool, frontFacing, frontFacing(),
      bool, flip, flip(),
      uint64, degrees, degreesClockwise()
    );
  }
    
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket::VideoOrientation6HeaderExtension
  #pragma mark

  //---------------------------------------------------------------------------
  RTPPacket::VideoOrientation6HeaderExtension::VideoOrientation6HeaderExtension(const HeaderExtension &header) :
    VideoOrientationHeaderExtension(header)
  {
  }

  //---------------------------------------------------------------------------
  RTPPacket::VideoOrientation6HeaderExtension::VideoOrientation6HeaderExtension(
                                                                                const Clockwise &,
                                                                                bool frontFacingCamera,
                                                                                bool flip,
                                                                                UINT orientation
                                                                                ) :
    VideoOrientationHeaderExtension(*this)
  {
    // find closest approximation to the video orientation for the 6 bit allowance
    orientation %= 360;

    // r = x * 360
    //     -------
    //        64
    //
    // r * 64 = x * 360
    //
    // x = r * 64
    //     ------
    //      360

    orientation *= 64;
    orientation /= 360;
    orientation %= 64;

    //  Bit#         7   6     5     4     3     2     1     0 (LSB)
    //  Definition  R5  R4    R3    R2     C     F    R1    R0

    // R1	R0	R5	R4	R3	R2
    // 0  0   0   0   0   0   = 0
    // 0  0   0   0   0   1   = 1
    // 1  1   1   1   1   1   = 63

    BYTE ordered = static_cast<BYTE>(orientation) & 0x3F;

    mEncoded[0] = RTP_PACK_BITS(frontFacingCamera ? 1 : 0, 0x1, 3) |
                  RTP_PACK_BITS(flip ? 1 : 0, 0x1, 2) |
                  RTP_PACK_BITS(RTP_GET_BITS(ordered, 0x3, 4), 0x3, 0) |
                  RTP_PACK_BITS(RTP_GET_BITS(ordered, 0xF, 0), 0xF, 4);
  }

  //---------------------------------------------------------------------------
  RTPPacket::VideoOrientation6HeaderExtension::VideoOrientation6HeaderExtension(
                                                                                const CounterClockwise &,
                                                                                bool frontFacingCamera,
                                                                                bool flip,
                                                                                UINT orientation
                                                                                ) :
    VideoOrientation6HeaderExtension(Clockwise{}, frontFacingCamera, flip, 360-(orientation % 360))
  {
  }

  //---------------------------------------------------------------------------
  UINT RTPPacket::VideoOrientation6HeaderExtension::degreesClockwise() const
  {
    if (NULL == mData) return 0;

    BYTE ordered = RTP_PACK_BITS(RTP_GET_BITS(mData[0], 0x3, 0), 0x3, 4) |
                   RTP_PACK_BITS(RTP_GET_BITS(mData[0], 0xF, 4), 0xF, 0);

    UINT degrees = (static_cast<UINT>(ordered)*360)/64;
    return degrees % 360;
  }

  //---------------------------------------------------------------------------
  UINT RTPPacket::VideoOrientation6HeaderExtension::degreesCounterClockwise() const
  {
    return (360-degreesClockwise())%360;
  }
    
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket (public)
  #pragma mark

  //---------------------------------------------------------------------------
  RTPPacket::RTPPacket(
                       const make_private &,
                       MediaChannelID mediaChannelID
                       ) :
    mMediaChannelID(mediaChannelID)
  {
  }

  //---------------------------------------------------------------------------
  RTPPacket::~RTPPacket()
  {
    if (mHeaderExtensions) {
      delete [] mHeaderExtensions;
      mHeaderExtensions = NULL;
    }
  }

  //---------------------------------------------------------------------------
  RTPPacketPtr RTPPacket::create(
                                 const RTPPacket &packet,
                                 MediaChannelID mediaChannelID
                                 )
  {
    RTPPacketPtr pThis(make_shared<RTPPacket>(make_private{}, mediaChannelID));
    pThis->generate(packet);
    return pThis;
  }

  //---------------------------------------------------------------------------
  RTPPacketPtr RTPPacket::create(const CreationParams &params)
  {
    RTPPacketPtr pThis(make_shared<RTPPacket>(make_private{}, params.mMediaChannelID));
    pThis->generate(params);
    return pThis;
  }

  //---------------------------------------------------------------------------
  RTPPacketPtr RTPPacket::create(
                                 const BYTE *buffer,
                                 size_t bufferLengthInBytes,
                                 MediaChannelID mediaChannelID
                                 )
  {
    ORTC_THROW_INVALID_PARAMETERS_IF(!buffer)
    ORTC_THROW_INVALID_PARAMETERS_IF(0 == bufferLengthInBytes)
    return RTPPacket::create(UseServicesHelper::convertToBuffer(buffer, bufferLengthInBytes), mediaChannelID);
  }

  //---------------------------------------------------------------------------
  RTPPacketPtr RTPPacket::create(
                                 const SecureByteBlock &buffer,
                                 MediaChannelID mediaChannelID
                                 )
  {
    return RTPPacket::create(buffer.BytePtr(), buffer.SizeInBytes());
  }

  //---------------------------------------------------------------------------
  RTPPacketPtr RTPPacket::create(
                                 SecureByteBlockPtr buffer,
                                 MediaChannelID mediaChannelID
                                 )
  {
    RTPPacketPtr pThis(make_shared<RTPPacket>(make_private{}, mediaChannelID));
    pThis->mBuffer = buffer;
    if (!pThis->parse()) {
      ZS_EVENTING_2(x, w, Debug, RTPPacketCreate, ol, RtpPacket, Start, puid, mediaChannelId, pThis->mMediaChannelID, string, message, "packet could not be parsed");
      return RTPPacketPtr();
    }
    return pThis;
  }

  //---------------------------------------------------------------------------
  const BYTE *RTPPacket::ptr() const
  {
    return mBuffer->BytePtr();
  }

  //---------------------------------------------------------------------------
  size_t RTPPacket::size() const
  {
    return mBuffer->SizeInBytes();
  }

  //---------------------------------------------------------------------------
  SecureByteBlockPtr RTPPacket::buffer() const
  {
    return mBuffer;
  }

  //---------------------------------------------------------------------------
  DWORD RTPPacket::getCSRC(size_t index) const
  {
    ASSERT(index < cc())
    return UseRTPUtils::getBE32(&((ptr())[internal::kMinRtpPacketLen + (sizeof(DWORD)*index)]));
  }

  //---------------------------------------------------------------------------
  const BYTE *RTPPacket::payload() const
  {
    if (0 == mPayloadSize) return NULL;
    const BYTE *buffer = ptr();
    return &(buffer[mHeaderSize + mHeaderExtensionSize]);
  }

  //---------------------------------------------------------------------------
  RTPPacket::HeaderExtension *RTPPacket::getHeaderExtensionAtIndex(size_t index) const
  {
    if (index >= mTotalHeaderExtensions) return NULL;
    return &(mHeaderExtensions[index]);
  }

  //---------------------------------------------------------------------------
  void RTPPacket::trace(const char *message) const
  {
    if (ZS_EVENTING_IS_LOGGING(Insane)) {
      ZS_EVENTING_20(x, i, Insane, RTPPacketTrace, ol, RtpPacket, Info,
        string, message, message,
        puid, mediaChannelID, mMediaChannelID,
        binary, buffer, mBuffer ? mBuffer->BytePtr() : NULL,
        size, size, mBuffer ? mBuffer->SizeInBytes() : 0,
        byte, version, mVersion,
        size_t, padding, mPadding,
        byte, cc, mCC,
        bool, m, mM,
        byte, pt, mPT,
        word, sequenceNumber, mSequenceNumber,
        dword, timestamp, mTimestamp,
        dword, ssrc, mSSRC,
        size_t, headerSize, mHeaderSize,
        size_t, headerExtensionSize, mHeaderExtensionSize,
        size_t, payloadSize, mPayloadSize,
        size_t, totalHeaderExtensions, mTotalHeaderExtensions,
        byte, headerExtensionAppBits, mHeaderExtensionAppBits,
        size_t, headerExtensionPrepaddedSize, mHeaderExtensionPrepaddedSize,
        binary, headerExtensionParseStoppedPos, mHeaderExtensionParseStoppedPos,
        size, headerExtensionParseStoppedSize, mHeaderExtensionParseStoppedSize
      );

      for (auto current = mHeaderExtensions; NULL != current; current = current->mNext) {
        current->trace(message);
      }
    }
  }

  //---------------------------------------------------------------------------
  static bool requiresTwoByteHeader(
                                    RTPPacket::HeaderExtension *firstExtension,
                                    BYTE headerExtensionAppBits
                                    )
  {
    typedef RTPPacket::HeaderExtension HeaderExtension;

    bool twoByteHeader = (0 != headerExtensionAppBits);
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
    return twoByteHeader;
  }

  //---------------------------------------------------------------------------
  void RTPPacket::changeHeaderExtensions(HeaderExtension *firstExtension)
  {
    bool twoByteHeader = requiresTwoByteHeader(firstExtension, mHeaderExtensionAppBits);

    if (twoByteHeader) {
      ORTC_THROW_INVALID_STATE_IF(NULL != mHeaderExtensionParseStoppedPos)  // requires a 1 byte header to append this data
    }

    bool requiresExtension = internal::requiredExtension(firstExtension, mHeaderExtensionAppBits, mHeaderExtensionPrepaddedSize, mHeaderExtensionParseStoppedPos);

    const BYTE *buffer = ptr();

    ASSERT(NULL != buffer)

    size_t existingHeaderExtensionSize = mHeaderExtensionSize;
    size_t postHeaderExtensionSize = mPayloadSize + mPadding;

    ASSERT(size() == (mHeaderSize + existingHeaderExtensionSize + postHeaderExtensionSize))

    if (!requiresExtension) {
      // going to strip the extension header out entirely

      if (!RTP_HEADER_EXTENSION(buffer)) {
        trace("no extension present (thus no need to strip extension from RTP packet)");
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

      trace("stripped existing extension header");
      return;
    }

    mHeaderExtensionSize = 0;
    mTotalHeaderExtensions = 0;
    internal::getHeaderExtensionSize(firstExtension, twoByteHeader, mHeaderExtensionPrepaddedSize, mHeaderExtensionParseStoppedSize, mHeaderExtensionSize, mTotalHeaderExtensions);

    size_t newSize = mHeaderSize + mHeaderExtensionSize + postHeaderExtensionSize;

    SecureByteBlockPtr oldBuffer = mBuffer; // temporary to keep previous allocation alive during swap

    mBuffer = make_shared<SecureByteBlock>(newSize);

    BYTE *newBuffer = mBuffer->BytePtr();

    // copy the bytes before the extension header to the new buffer
    memcpy(newBuffer, buffer, mHeaderSize);

    // copy the bytes from after the extension header in the older buffer to the new buffer
    memcpy(&(newBuffer[mHeaderSize + mHeaderExtensionSize]), &(buffer[mHeaderSize + existingHeaderExtensionSize]), postHeaderExtensionSize);

    writeHeaderExtensions(firstExtension, twoByteHeader);

    trace("header extension changed");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPPacket (internal)
  #pragma mark

  //---------------------------------------------------------------------------
  bool RTPPacket::parse()
  {
    const BYTE *buffer = mBuffer->BytePtr();
    size_t size = mBuffer->SizeInBytes();

    if (size < internal::kMinRtpPacketLen) {
      ZS_EVENTING_3(x, w, Trace, RTPPacketParseWarningPacketLengthTooShort, ol, RtpPacket, Parse,
        string, message, "packet length is too short",
        puid, mediaChannelID, mMediaChannelID,
        size_t, length, size
      );
      return false;
    }

    mVersion = RTP_HEADER_VERSION(buffer);
    if (mVersion != internal::kRtpVersion) {
      ZS_EVENTING_2(x, w, Trace, RTPPacketParseWarning, ol, RtpPacket, Parse,
        string, message, "not an RTP packet",
        puid, mediaChannelID, mMediaChannelID
      );
      return false;
    }

    if (UseRTPUtils::isRTCPPacketType(buffer, size)) {
      ZS_EVENTING_3(x, w, Trace, RTPPacketParseWarningPacketisRTCP, ol, RtpPacket, Parse,
        string, message, "packet is RTCP not RTP",
        puid, mediaChannelID, mMediaChannelID,
        size_t, length, size
      );
      return false;
    }

    bool hasPadding = RTP_HEADER_PADDING(buffer);
    mCC = RTP_HEADER_CC(buffer);
    mM = RTP_HEADER_M(buffer);
    mPT = RTP_HEADER_PT(buffer);
    mSequenceNumber = UseRTPUtils::getBE16(&(buffer[2]));
    mTimestamp = UseRTPUtils::getBE32(&(buffer[4]));
    mSSRC = UseRTPUtils::getBE32(&(buffer[8]));

    mHeaderSize = internal::kMinRtpPacketLen + (static_cast<size_t>(mCC) * sizeof(DWORD));

    if (size < mHeaderSize) {
      ZS_EVENTING_2(x, w, Trace, RTPPacketParseWarning, ol, RtpPacket, Parse,
        string, message, "illegal RTP packet (packet is not large enough to hold header)",
        puid, mediaChannelID, mMediaChannelID
      );
      return false;
    }

    if (RTP_HEADER_EXTENSION(buffer)) {
      if (size < (mHeaderSize + sizeof(DWORD))) {
        ZS_EVENTING_2(x, w, Trace, RTPPacketParseWarning, ol, RtpPacket, Parse,
          string, message, "illegal RTP packet (packet not large enough to hold data)",
          puid, mediaChannelID, mMediaChannelID
        );
        return false;
      }

      mHeaderExtensionSize = (static_cast<size_t>(UseRTPUtils::getBE16(&(buffer[mHeaderSize + 2]))) * sizeof(DWORD)) + sizeof(DWORD);
      if (size < (mHeaderSize + mHeaderExtensionSize)) {
        ZS_EVENTING_2(x, w, Trace, RTPPacketParseWarning, ol, RtpPacket, Parse,
          string, message, "illegal RTP packet (packet not large enough to hold extensions)",
          puid, mediaChannelID, mMediaChannelID
        );
        return false;
      }
    }

    if (hasPadding) {
      mPadding = static_cast<size_t>(buffer[size-1]);
      if (0 == mPadding) {
        ZS_EVENTING_2(x, w, Trace, RTPPacketParseWarning, ol, RtpPacket, Parse,
          string, message, "illegal RTP packet (no padding size)",
          puid, mediaChannelID, mMediaChannelID
        );
        return false;
      }

      if (size < (mHeaderSize + mHeaderExtensionSize + mPadding)) {
        ZS_EVENTING_2(x, w, Trace, RTPPacketParseWarning, ol, RtpPacket, Parse,
          string, message, "illegal RTP packet (packet not large enough to hold padding)",
          puid, mediaChannelID, mMediaChannelID
        );
        return false;
      }
    }

    mPayloadSize = size - (mHeaderSize + mHeaderExtensionSize + mPadding);

    if (0 == mHeaderExtensionSize) {
      // no extensions present
      trace("parsed");
      return true;
    }

    const BYTE *profilePos = &(buffer[mHeaderSize]);

    bool oneByte = false;

    if ((0xBE == profilePos[0]) &&
        (0xDE == profilePos[1])) {
      oneByte = true;
    } else {
      WORD twoByteHeader = UseRTPUtils::getBE16(profilePos);
      mHeaderExtensionAppBits = (twoByteHeader & 0xF);

      if (0x100 != ((twoByteHeader & 0xFFF0) >> 4)) {
        ZS_EVENTING_3(x, w, Trace, RTPPacketParseWarningHeaderProfileNotUnderstood, ol, RtpPacket, Parse,
          string, message, "illegal RTP packet (packet not large enough to hold padding)",
          puid, mediaChannelID, mMediaChannelID,
          word, profile, twoByteHeader
        );
        return false;
      }
    }

    size_t remaining = mHeaderExtensionSize - sizeof(DWORD);
    if (0 == remaining) {
      trace("parsed");
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
          ZS_EVENTING_4(x, w, Trace, RTPPacketParseWarningExtensionHeaderNotValid, ol, RtpPacket, Parse,
            string, message, "extension header is not valid",
            puid, mediaChannelID, mMediaChannelID,
            size_t, remaining, remaining,
            size_t, length, length
          );
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
        ZS_EVENTING_4(x, w, Trace, RTPPacketParseWarningExtensionHeaderNotValid, ol, RtpPacket, Parse,
          string, message, "extension header is not valid (must have room for two byte header)",
          puid, mediaChannelID, mMediaChannelID,
          size_t, remaining, remaining,
          size_t, length, 0
        );
        return false;
      }

      BYTE id = (pos[0]);
      size_t length = (pos[1]);

      if (remaining < (sizeof(WORD) + length)) {
        ZS_EVENTING_4(x, w, Trace, RTPPacketParseWarningExtensionHeaderNotValid, ol, RtpPacket, Parse,
          string, message, "extension header is not valid (must have room for two byte header and length)",
          puid, mediaChannelID, mMediaChannelID,
          size_t, remaining, remaining,
          size_t, length, length
        );
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

    trace("parsed");
    return true;
  }
    
  //---------------------------------------------------------------------------
  void RTPPacket::writeHeaderExtensions(
                                        HeaderExtension *firstExtension,
                                        bool twoByteHeader
                                        )
  {
    ASSERT((bool)mBuffer)
    ASSERT(0 != mBuffer->SizeInBytes())
    ASSERT(0 != mHeaderSize)
    //ASSERT(mHeaderExtensionAppBits)           // needs to be set (but no way to verify here)
    //ASSERT(mTotalHeaderExtensions)            // needs to be set (but no way to verify here)
    //ASSERT(mHeaderExtensionPrepaddedSize)     // needs to be set (but no way to verify here)
    //ASSSRT(mHeaderExtensionParseStoppedSize)  // needs to be set (but no way to verify here)
    //ASSERT(mHeaderExtensionParseStoppedPos)   // needs to be set (but no way to verify here)
    ASSERT(0 != mHeaderExtensionSize)


    BYTE *newBuffer = mBuffer->BytePtr();

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

    WORD totalDWORDsLength = static_cast<WORD>(((mHeaderExtensionSize - sizeof(DWORD)) / sizeof(DWORD)));
    newProfilePos[2] = static_cast<BYTE>((totalDWORDsLength & 0xFF00) >> 8);
    newProfilePos[3] = static_cast<BYTE>(totalDWORDsLength & 0xFF);

    BYTE *pos = &(newProfilePos[sizeof(DWORD)]) + mHeaderExtensionPrepaddedSize;

    HeaderExtension *newExtensions = NULL;
    if (0 != mTotalHeaderExtensions) {
      newExtensions = new HeaderExtension[mTotalHeaderExtensions] {};
    }

    size_t index = 0;
    for (HeaderExtension *current = firstExtension; NULL != current; current = current->mNext, ++index) {
      HeaderExtension *newCurrent = &(newExtensions[index]);
      if (0 != index) {
        newExtensions[index-1].mNext = newCurrent;
      }

      if (twoByteHeader) {
        pos[0] = current->mID;
        pos[1] = static_cast<BYTE>(current->mDataSizeInBytes);
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

    ASSERT(index == mTotalHeaderExtensions)

    mTotalHeaderExtensions = index;
    if (NULL != mHeaderExtensions) {
      delete [] mHeaderExtensions;
      mHeaderExtensions = NULL;
    }
    mHeaderExtensions = newExtensions;
  }

  //---------------------------------------------------------------------------
  void RTPPacket::generate(const RTPPacket &packet)
  {
    CreationParams params;

    params.mMediaChannelID = packet.mMediaChannelID;
    params.mVersion = packet.version();
    params.mPadding = packet.padding();
    params.mM = packet.m();
    params.mPT = packet.pt();
    params.mSequenceNumber = packet.sequenceNumber();
    params.mTimestamp = packet.timestamp();
    params.mSSRC = packet.ssrc();

    params.mCC = packet.cc();
    params.mCC = (static_cast<BYTE>(packet.cc()) & 0xF);

    DWORD tmpCCList[0xF];
    if (params.mCC > 0) {
      for (decltype(params.mCC) index = 0; index < params.mCC; ++index) {
        tmpCCList[index] = packet.getCSRC(index);
      }
      params.mCSRCList = &(tmpCCList[0]);
    }

    params.mPayload = packet.payload();
    params.mPayloadSize = packet.payloadSize();
    params.mFirstHeaderExtension = packet.firstHeaderExtension();
    params.mHeaderExtensionAppBits = packet.headerExtensionAppBits();

    params.mHeaderExtensionPrepaddedSize = packet.headerExtensionPrepaddedSize();
    params.mHeaderExtensionStopParsePos = packet.headerExtensionParseStopped();
    params.mHeaderExtensionStopParseSize = packet.headerExtensionParseStoppedSize();

    generate(params);
  }

  //---------------------------------------------------------------------------
  void RTPPacket::generate(const CreationParams &params)
  {
    ASSERT(params.mVersion <= 0x3)
    ASSERT(params.mPadding <= 0xFF)
    ASSERT(params.mCC <= 0xF)
    ASSERT(params.mPT <= 0x7F)

    ASSERT(0 == params.mCC ? (NULL == params.mCSRCList) : (NULL != params.mCSRCList))
    ASSERT(0 == params.mPayloadSize ? (NULL == params.mPayload) : (NULL != params.mPayload))

    mVersion = params.mVersion & 0x3;
    mPadding = params.mPadding;
    mCC = (static_cast<BYTE>(params.mCC) & 0xF);
    mM = params.mM;
    mPT = (params.mPT & 0x7F);
    mSequenceNumber = params.mSequenceNumber;
    mTimestamp = params.mTimestamp;
    mSSRC = params.mSSRC;

    mHeaderSize = internal::kMinRtpPacketLen + (static_cast<size_t>(mCC) * sizeof(DWORD));
    //mHeaderExtensionSize = 0; // filled in later
    mPayloadSize = params.mPayloadSize;

    //mTotalHeaderExtensions (later)
    //mHeaderExtensions (later)
    mHeaderExtensionAppBits = params.mHeaderExtensionAppBits;

    mHeaderExtensionPrepaddedSize = params.mHeaderExtensionPrepaddedSize;
    mHeaderExtensionParseStoppedPos = params.mHeaderExtensionStopParsePos;
    mHeaderExtensionParseStoppedSize = params.mHeaderExtensionStopParseSize;

    bool twoByteHeader = requiresTwoByteHeader(params.mFirstHeaderExtension, mHeaderExtensionAppBits);

    if (twoByteHeader) {
      ORTC_THROW_INVALID_STATE_IF(NULL != mHeaderExtensionParseStoppedPos)  // requires a 1 byte header to append this data
    }

    bool requiresExtension = internal::requiredExtension(params.mFirstHeaderExtension, mHeaderExtensionAppBits, mHeaderExtensionPrepaddedSize, mHeaderExtensionParseStoppedPos);

    if (requiresExtension) {
      mHeaderExtensionSize = 0;
      mTotalHeaderExtensions = 0;
      internal::getHeaderExtensionSize(params.mFirstHeaderExtension, twoByteHeader, mHeaderExtensionPrepaddedSize, mHeaderExtensionParseStoppedSize, mHeaderExtensionSize, mTotalHeaderExtensions);
    }

    size_t postHeaderExtensionSize = mPayloadSize + mPadding;

    size_t newSize = mHeaderSize + mHeaderExtensionSize + postHeaderExtensionSize;

    mBuffer = make_shared<SecureByteBlock>(newSize);

    BYTE *newBuffer = mBuffer->BytePtr();

    // fill standard header

    BYTE *pos = newBuffer;
    pos[0] = RTP_PACK_BITS(mVersion, 0x3, 6) |
             RTP_PACK_BITS(0 != mPadding ? 1 : 0, 0x1, 5) |
             RTP_PACK_BITS(requiresExtension ? 1 : 0, 0x1, 4) |
             RTP_PACK_BITS(mCC, 0xF, 0);
    pos[1] = RTP_PACK_BITS(mM ? 1 : 0, 0x1, 7) |
             RTP_PACK_BITS(mPT, 0x7F, 0);

    UseRTPUtils::setBE16(&(pos[2]), mSequenceNumber);
    UseRTPUtils::setBE32(&(pos[4]), mTimestamp);
    UseRTPUtils::setBE32(&(pos[8]), mSSRC);

    // write CSRC list (if any)
    for (size_t index = 0; index < static_cast<size_t>(mCC); ++index) {
      UseRTPUtils::setBE32(&(pos[12+(sizeof(DWORD)*index)]), params.mCSRCList[index]);
    }

    if (requiresExtension) {
      writeHeaderExtensions(params.mFirstHeaderExtension, twoByteHeader);
    }

    if (0 != mPayloadSize) {
      memcpy(&(pos[mHeaderSize+mHeaderExtensionSize]), params.mPayload, mPayloadSize);
    }

    if (0 != mPadding) {
      newBuffer[newSize-1] = static_cast<BYTE>(mPadding);
    }
    trace("generated");
  }

} // namespace ortc
