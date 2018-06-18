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
#include <ortc/IICETypes.h>
#include <ortc/IMediaStreamTrack.h>

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //
  // RTPPacket
  //

  class RTPPacket
  {
  protected:
    struct make_private {};

  public:
    struct HeaderExtension;
    struct VideoOrientationHeaderExtension;
    struct VideoOrientation6HeaderExtension;
    typedef IMediaStreamTrackTypes::MediaChannelID MediaChannelID;

  public:
    //-----------------------------------------------------------------------
    //
    // RTPPacket::CreationParams
    //

    struct CreationParams
    {
      MediaChannelID mMediaChannelID {};
      BYTE mVersion {2};
      size_t mPadding {};
      bool mM {};
      BYTE mPT {};
      WORD mSequenceNumber {};
      DWORD mTimestamp {};
      DWORD mSSRC {};

      size_t mCC {};
      DWORD *mCSRCList {};

      const BYTE *mPayload {};
      size_t mPayloadSize {};

      HeaderExtension *mFirstHeaderExtension {};
      BYTE mHeaderExtensionAppBits {};

      size_t mHeaderExtensionPrepaddedSize {};
      const BYTE *mHeaderExtensionStopParsePos {};
      size_t mHeaderExtensionStopParseSize {};
    };

    //-----------------------------------------------------------------------
    //
    // RTPPacket::Extension
    //

    struct HeaderExtension
    {
      // see https://tools.ietf.org/html/rfc5285

      BYTE mID {};

      const BYTE *mData {};
      size_t mDataSizeInBytes {};
      size_t mPostPaddingSize {};

      HeaderExtension *mNext {};

      virtual void trace(
                         const char *func = NULL,
                         const char *message = NULL
                         ) const noexcept;
    };

    //-----------------------------------------------------------------------
    //
    // RTPPacket::ClientToMixerExtension
    //

    struct ClientToMixerExtension : public HeaderExtension
    {
      // see https://tools.ietf.org/html/rfc6464

      ClientToMixerExtension(const HeaderExtension &header) noexcept;
      ClientToMixerExtension(
                              BYTE id,
                              bool voiceActivity,
                              BYTE level
                              ) noexcept;

      bool voiceActivity() const noexcept;
      BYTE level() const noexcept;

      virtual void trace(
                         const char *func = NULL,
                         const char *message = NULL
                         ) const noexcept override;

    public:
      BYTE mLevelBuffer {};
    };

    //-----------------------------------------------------------------------
    //
    // RTPPacket::MixerToClientExtension
    //

    struct MixerToClientExtension : public HeaderExtension
    {
      // see https://tools.ietf.org/html/rfc6464

      static const size_t kMaxLevelCount {0xF};

      MixerToClientExtension(const HeaderExtension &header) noexcept;
      MixerToClientExtension(
                              BYTE id,
                              BYTE *levels,
                              size_t count
                              ) noexcept;

      size_t levelsCount() const noexcept;

      BYTE unusedBit(size_t index) const noexcept;
      BYTE level(size_t index) const noexcept;

      virtual void trace(
                         const char *func = NULL,
                         const char *message = NULL
                         ) const noexcept override;

    public:
      BYTE mLevelBuffer[kMaxLevelCount] {};
    };

    //-----------------------------------------------------------------------
    //
    // RTPPacket::StringHeaderExtension
    //

    struct StringHeaderExtension : public HeaderExtension
    {
      static const size_t kMaxStringLength {0xFF};

      StringHeaderExtension(const HeaderExtension &header) noexcept;
      StringHeaderExtension(
                            BYTE id,
                            const char *str
                            ) noexcept;

      const char *str() const noexcept;

      void trace(
                 const char *func = NULL,
                 const char *message = NULL
                 ) const noexcept override;

    public:
      BYTE mStringBuffer[kMaxStringLength+sizeof(char)] {};
    };
      
    //-----------------------------------------------------------------------
    //
    // RTPPacket::NumberHeaderExtension
    //

    struct NumberHeaderExtension : public HeaderExtension
    {
      static const size_t kMaxNumberByteLength {0xFF};

      NumberHeaderExtension(const HeaderExtension &header) noexcept;
      NumberHeaderExtension(
                            BYTE id,
                            const BYTE *number,
                            size_t lengthInBytes
                            ) noexcept;
      NumberHeaderExtension(
                            BYTE id,
                            const char *valueInBase10
                            ) noexcept(false); // throws InvalidParameters

      const BYTE *number() const noexcept;
      size_t length() const noexcept;

      String str() const noexcept;

      void trace(
                 const char *func = NULL,
                 const char *message = NULL
                 ) const noexcept override;

    public:
      BYTE mNumberBuffer[kMaxNumberByteLength] {};
    };
      
    //-----------------------------------------------------------------------
    //
    // RTPPacket::MidHeaderExtension
    //

    struct MidHeaderExtension : public StringHeaderExtension
    {
      // https://tools.ietf.org/html/draft-ietf-mmusic-sdp-bundle-negotiation-23#section-14.3

      static const size_t kMaxMidLength = kMaxStringLength;

      MidHeaderExtension(const HeaderExtension &header) noexcept : StringHeaderExtension(header) {}
      MidHeaderExtension(
                        BYTE id,
                        const char *mid
                        ) noexcept :                              StringHeaderExtension(id, mid) {}

      const char *mid() const noexcept {return str();}

      void trace(
                 const char *func = NULL,
                 const char *message = NULL
                 ) const noexcept override;

    public:
    };

    //-----------------------------------------------------------------------
    //
    // RTPPacket::RidHeaderExtension
    //

    struct RidHeaderExtension : public StringHeaderExtension
    {
      // https://tools.ietf.org/html/draft-pthatcher-mmusic-rid-00

      static const size_t kMaxMidLength = kMaxStringLength;

      RidHeaderExtension(const HeaderExtension &header) noexcept : StringHeaderExtension(header) {}
      RidHeaderExtension(
                          BYTE id,
                          const char *rid
                          ) noexcept :                              StringHeaderExtension(id, rid) {}

      const char *rid() const noexcept {return str();}

      void trace(
                 const char *func = NULL,
                 const char *message = NULL
                 ) const noexcept override;

    public:
    };

    //-----------------------------------------------------------------------
    //
    // RTPPacket::VideoOrientationHeaderExtension
    //

    struct VideoOrientationHeaderExtension : public HeaderExtension
    {
      struct Clockwise {};
      struct CounterClockwise {};

      VideoOrientationHeaderExtension(const HeaderExtension &header) noexcept;
      VideoOrientationHeaderExtension(const VideoOrientation6HeaderExtension &) noexcept;
      VideoOrientationHeaderExtension(
                                      const Clockwise &,
                                      bool frontFacingCamera, // true = front facing, false = backfacing
                                      bool flip, // horizontal left-right flip (mirro)
                                      UINT orientation
                                      ) noexcept;
      VideoOrientationHeaderExtension(
                                      const CounterClockwise &,
                                      bool frontFacingCamera, // true = front facing, false = backfacing
                                      bool flip, // horizontal left-right flip (mirro)
                                      UINT orientation
                                      ) noexcept;

      bool frontFacing() const noexcept;
      bool backFacing() const noexcept;
      bool flip() const noexcept;
      virtual UINT degreesClockwise() const noexcept;
      virtual UINT degreesCounterClockwise() const noexcept;

      virtual void trace(
                         const char *func = NULL,
                         const char *message = NULL
                         ) const noexcept override;

    public:
      BYTE mEncoded[1] {};
    };

    //-----------------------------------------------------------------------
    //
    // RTPPacket::VideoOrientationHeaderExtension
    //

    struct VideoOrientation6HeaderExtension : public VideoOrientationHeaderExtension
    {
      VideoOrientation6HeaderExtension(const HeaderExtension &header) noexcept;
      VideoOrientation6HeaderExtension(
                                        const Clockwise &,
                                        bool frontFacingCamera, // true = front facing, false = backfacing
                                        bool flip, // horizontal left-right flip (mirro)
                                        UINT orientation
                                        ) noexcept;
      VideoOrientation6HeaderExtension(
                                        const CounterClockwise &,
                                        bool frontFacingCamera, // true = front facing, false = backfacing
                                        bool flip, // horizontal left-right flip (mirro)
                                        UINT orientation
                                        ) noexcept;

      virtual UINT degreesClockwise() const noexcept override;
      virtual UINT degreesCounterClockwise() const noexcept override;

    public:
    };

  public:
    //-----------------------------------------------------------------------
    //
    // (public)
    //

    RTPPacket(
              const make_private &,
              MediaChannelID mediaChannelID
              ) noexcept;
    ~RTPPacket() noexcept;

    static RTPPacketPtr create(
                               const RTPPacket &packet,
                               MediaChannelID mediaChannelID  = 0
                               ) noexcept;
    static RTPPacketPtr create(const CreationParams &params) noexcept;
    static RTPPacketPtr create(
                               const BYTE *buffer,
                               size_t bufferLengthInBytes,
                               MediaChannelID mediaChannelID = 0
                               ) noexcept;
    static RTPPacketPtr create(
                               const SecureByteBlock &buffer,
                               MediaChannelID mediaChannelID = 0
                               ) noexcept;
    static RTPPacketPtr create(
                               SecureByteBlockPtr buffer,
                               MediaChannelID mediaChannelID = 0
                               ) noexcept;  // NOTE: exclusive ownership of buffer is taken / assumed

    MediaChannelID mediaChannelID() const noexcept {return mMediaChannelID;}

    const BYTE *ptr() const noexcept;
    size_t size() const noexcept;
    SecureByteBlockPtr buffer() const noexcept;

    BYTE version() const noexcept {return mVersion;}
    size_t padding() const noexcept {return mPadding;}
    size_t cc() const noexcept {return static_cast<size_t>(mCC);}
    bool m() const noexcept {return mM;}
    BYTE pt() const noexcept {return mPT;}
    WORD sequenceNumber() const noexcept {return mSequenceNumber;}
    DWORD timestamp() const noexcept {return mTimestamp;}
    DWORD ssrc() const noexcept {return mSSRC;}

    size_t headerSize() noexcept {return mHeaderSize;}
    size_t headerExtensionSize() noexcept {return mHeaderExtensionSize;}
    size_t payloadSize() noexcept {return mPayloadSize;}

    DWORD getCSRC(size_t index) const noexcept;
    const BYTE *payload() const noexcept;
    size_t payloadSize() const noexcept {return mPayloadSize;}

    size_t totalHeaderExtensions() const noexcept {return mTotalHeaderExtensions;}
    HeaderExtension *firstHeaderExtension() const noexcept {return mHeaderExtensions;}
    HeaderExtension *getHeaderExtensionAtIndex(size_t index) const noexcept;
    BYTE headerExtensionAppBits() const noexcept {return mHeaderExtensionAppBits;}

    size_t headerExtensionPrepaddedSize() const noexcept {return mHeaderExtensionPrepaddedSize;}
    const BYTE *headerExtensionParseStopped() const noexcept {return mHeaderExtensionParseStoppedPos;}
    size_t headerExtensionParseStoppedSize() const noexcept {return mHeaderExtensionParseStoppedSize;}

    void changeHeaderExtensions(HeaderExtension *firstExtension) noexcept(false); // throws InvalidState

    void trace(
               const char *func = NULL,
               const char *message = NULL
               ) const noexcept;

  protected:
    //-----------------------------------------------------------------------
    //
    // (internal)
    //

    bool parse() noexcept;

    void writeHeaderExtensions(
                                HeaderExtension *firstExtension,
                                bool twoByteHeader
                                ) noexcept;
    void generate(const RTPPacket &params) noexcept;
    void generate(const CreationParams &params) noexcept(false); // throws InvalidState

  public:
    SecureByteBlockPtr mBuffer;

    MediaChannelID mMediaChannelID {};

    BYTE mVersion {};
    size_t mPadding {};
    BYTE mCC {};
    bool mM {};
    BYTE mPT {};
    WORD mSequenceNumber {};
    DWORD mTimestamp {};
    DWORD mSSRC {};

    size_t mHeaderSize {};
    size_t mHeaderExtensionSize {};
    size_t mPayloadSize {};

    size_t mTotalHeaderExtensions {};
    HeaderExtension *mHeaderExtensions {};
    BYTE mHeaderExtensionAppBits {};

    size_t mHeaderExtensionPrepaddedSize {};
    const BYTE *mHeaderExtensionParseStoppedPos {};
    size_t mHeaderExtensionParseStoppedSize {};
  };
}

