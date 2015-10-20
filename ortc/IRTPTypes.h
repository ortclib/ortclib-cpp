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

#include <list>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes
  #pragma mark
  
  interaction IRTPTypes
  {
    ZS_DECLARE_STRUCT_PTR(Capabilities)
    ZS_DECLARE_STRUCT_PTR(CodecCapability)
    ZS_DECLARE_STRUCT_PTR(HeaderExtensions)
    ZS_DECLARE_STRUCT_PTR(RtcpFeedback)
    ZS_DECLARE_STRUCT_PTR(Parameters)
    ZS_DECLARE_STRUCT_PTR(CodecParameters)
    ZS_DECLARE_STRUCT_PTR(HeaderExtensionParameters)
    ZS_DECLARE_STRUCT_PTR(EncodingParameters)
    ZS_DECLARE_STRUCT_PTR(RTCPParameters)
    ZS_DECLARE_STRUCT_PTR(FECParameters)
    ZS_DECLARE_STRUCT_PTR(RTXParameters)

    ZS_DECLARE_TYPEDEF_PTR(std::list<CodecCapability>, CodecCapabilitiesList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<HeaderExtensions>, HeaderExtensionsList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<RtcpFeedback>, RtcpFeedbackList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<CodecParameters>, CodecParametersList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<HeaderExtensionParameters>, HeaderExtensionParametersList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<EncodingParameters>, EncodingParametersList)

    typedef String FECMechanism;
    ZS_DECLARE_TYPEDEF_PTR(std::list<FECMechanism>, FECMechanismList)

    typedef BYTE PayloadType;
    typedef DWORD SSRCType;
    typedef String EncodingID;
    ZS_DECLARE_TYPEDEF_PTR(std::list<EncodingID>, EncodingIDList)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::Capabilities
    #pragma mark

    struct Capabilities {
      CodecCapabilitiesList mCodecs;
      HeaderExtensionsList  mHeaderExtensions;
      FECMechanismList      mFECMechanisms;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::CodecCapability
    #pragma mark

    struct CodecCapability {
      String            mName;
      String            mKind;
      ULONG             mClockRate {};
      PayloadType       mPreferredPayloadType {};
      ULONG             mMaxPTime {};
      ULONG             mNumChannels {};
      RtcpFeedbackList  mFeedback;
      AnyPtr            mParameters;
      AnyPtr            mOptions;
      USHORT            mMaxTemporalLayers {0};
      USHORT            mMaxSpatialLayers {0};
      bool              mSVCMultiStreamSupport {};

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::HeaderExtensions
    #pragma mark

    struct HeaderExtensions {
      String mKind;
      String mURI;
      USHORT mPreferredID {};
      bool   mPreferredEncrypt {false};

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::RtcpFeedback
    #pragma mark

    struct RtcpFeedback {
      String mType;
      String mParameter;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::RTCPParameters
    #pragma mark

    struct RTCPParameters {
      SSRCType  mSSRC {};
      String    mCName;
      bool      mReducedSize {false};
      bool      mMux {true};

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::Parameters
    #pragma mark

    struct Parameters {
      String                        mMuxID;
      CodecParametersList           mCodecs;
      HeaderExtensionParametersList mHeaderExtensions;
      EncodingParametersList        mEncodingParameters;
      RTCPParameters                mRTCP;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::CodecParameters
    #pragma mark

    struct CodecParameters {
      String            mName;
      PayloadType       mPayloadType {};
      ULONG             mClockRate {};
      ULONG             mMaxPTime {};
      ULONG             mNumChannels {};
      RtcpFeedbackList  mRTCPFeedback;
      AnyPtr            mParameters;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::HeaderExtensionParameters
    #pragma mark

    struct HeaderExtensionParameters {
      String  mURI;
      USHORT  mID {};
      bool    mEncrypt {false};

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::FECParameters
    #pragma mark

    struct FECParameters {
      SSRCType  mSSRC {};
      String    mMechanism;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::RTXParameters
    #pragma mark

    struct RTXParameters {
      SSRCType mSSRC {};

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::EncodingParameters
    #pragma mark

    struct EncodingParameters {
      Optional<SSRCType>      mSSRC {};
      Optional<PayloadType>   mCodecPayloadType;
      Optional<FECParameters> mFEC;
      Optional<RTXParameters> mRTX;
      double                  mPriority {1.0};
      ULONGLONG               mMaxBitrate {};
      double                  mMinQuality {0};
      double                  mFramerateBias {0.5};
      bool                    mActive {true};
      EncodingID              mEncodingID;
      EncodingIDList          mDependencyEncodingIDs;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::CodecKinds
    #pragma mark

    enum CodecKinds
    {
      CodecKind_First,

      CodecKind_Unknown = CodecKind_First,

      CodecKind_Audio,
      CodecKind_Video,
      CodecKind_AV,

      CodecKind_RTX,
      CodecKind_FEC,

      CodecKind_Last = CodecKind_FEC,
    };

    static const char *toString(CodecKinds kind);
    static CodecKinds toCodecKind(const char *kind);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::MediaTypes
    #pragma mark

#define TODO_REMOVE_THIS_PLEASE 1
#define TODO_REMOVE_THIS_PLEASE 2
    enum MediaTypes
    {
      MediaType_First,

      MediaType_Unknown = MediaType_First,

      MediaType_Any,
      MediaType_Audio,
      MediaType_Video,

      MediaType_Last = MediaType_Video,
    };

    static const char *toString(MediaTypes mediaType);
    static MediaTypes toMediaType(const char *mediaType);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::PacketReceiver
    #pragma mark

    class PacketReceiver {
    public:

      //-------------------------------------------------------------------------
      #pragma mark
      #pragma mark IRTPTypes::PacketReceiver::DeliveryStatuses
      #pragma mark

      enum DeliveryStatuses {
        DeliveryStatus_First,

        DeliveryStatus_Unknown = MediaType_First,

        DeliveryStatus_OK,
        DeliveryStatus_UnknownSSRC,
        DeliveryStatus_PacketError,

        DeliveryStatus_Last = DeliveryStatus_PacketError,
      };

      static const char *toString(DeliveryStatuses status);
      static DeliveryStatuses toDeliveryStatus(const char *status);

      virtual DeliveryStatuses DeliverPacket(
                                             MediaTypes media_type,
                                             const uint8_t* packet,
                                             size_t length,
                                             int64_t timestamp
                                             ) = 0;

    protected:
      virtual ~PacketReceiver() {}
    };


    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::SupportedCodecs
    #pragma mark

    enum SupportedCodecs
    {
      SupportedCodec_First,

      SupportedCodec_Unknown = SupportedCodec_First,

      // audio codecs
      SupportedCodec_Opus,            // opus
      SupportedCodec_Isac,            // isac
      SupportedCodec_G722,            // g722
      SupportedCodec_ILBC,            // ilbc
      SupportedCodec_PCMU,            // pcmu
      SupportedCodec_PCMA,            // pcma

      // video codecs
      SupportedCodec_VP8,             // VP8
      SupportedCodec_VP9,             // VP9
      SupportedCodec_H264,            // H264

      // RTX
      SupportedCodec_RTX,             // rtx

      // FEC
      SupportedCodec_RED,             // red
      SupportedCodec_ULPFEC,          // ulpfec

      SupportedCodec_CN,              // cn

      SupportedCodec_TelephoneEvent,  // telephone-event

      SupportedCodec_Last = SupportedCodec_TelephoneEvent
    };

    static const char *toString(SupportedCodecs codec);
    static SupportedCodecs toSupportedCodec(const char *codec);

    static CodecKinds getCodecKind(SupportedCodecs codec);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::SupportedCodecs
    #pragma mark

    enum ReservedCodecPayloadTypes
    {
      // http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml

      ReservedCodecPayloadType_First          = 0,

      ReservedCodecPayloadType_Unknown        = 0xFF,

      ReservedCodecPayloadType_PCMU_8000      = 0,

      ReservedCodecPayloadType_GSM_8000       = 3,
      ReservedCodecPayloadType_G723_8000      = 4,
      ReservedCodecPayloadType_DVI4_8000      = 5,
      ReservedCodecPayloadType_DVI4_16000     = 6,
      ReservedCodecPayloadType_LPC_8000       = 7,
      ReservedCodecPayloadType_PCMA_8000      = 8,
      ReservedCodecPayloadType_G722_8000      = 9,
      ReservedCodecPayloadType_L16_44100_2    = 10,
      ReservedCodecPayloadType_L16_44100_1    = 11,
      ReservedCodecPayloadType_QCELP_8000     = 12,
      ReservedCodecPayloadType_CN_8000        = 13,
      ReservedCodecPayloadType_MPA_90000      = 14,
      ReservedCodecPayloadType_G728_8000      = 15,
      ReservedCodecPayloadType_DVI4_11025     = 16,
      ReservedCodecPayloadType_DVI4_22050     = 17,
      ReservedCodecPayloadType_G729_8000      = 18,

      ReservedCodecPayloadType_CelB_90000     = 25,
      ReservedCodecPayloadType_JPEG_90000     = 26,

      ReservedCodecPayloadType_nv_90000       = 28,

      ReservedCodecPayloadType_H261_90000     = 31,
      ReservedCodecPayloadType_MPV_90000      = 32,
      ReservedCodecPayloadType_MP2T_90000     = 33,
      ReservedCodecPayloadType_H263_90000     = 34,

      ReservedCodecPayloadType_Last = ReservedCodecPayloadType_H263_90000
    };

    static const char *toString(ReservedCodecPayloadTypes reservedCodec);
    static ReservedCodecPayloadTypes toReservedCodec(const char *encodingName);

    static ULONG getDefaultClockRate(ReservedCodecPayloadTypes reservedCodec);

    static CodecKinds getCodecKind(ReservedCodecPayloadTypes reservedCodec);
    static SupportedCodecs toSupportedCodec(ReservedCodecPayloadTypes reservedCodec);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::HeaderExtensionURIs
    #pragma mark

    enum HeaderExtensionURIs
    {
      HeaderExtensionURI_First,

      HeaderExtensionURI_Unknown = HeaderExtensionURI_First,

      HeaderExtensionURI_MuxID,                                         // https://tools.ietf.org/html/draft-ietf-avtext-sdes-hdr-ext-02
      HeaderExtensionURI_MID = HeaderExtensionURI_MuxID,                // urn:ietf:params:rtp-hdrext:sdes:mid

      HeaderExtensionURI_ClienttoMixerAudioLevelIndication,             // https://tools.ietf.org/html/rfc6464
                                                                        // urn:ietf:params:rtp-hdrext:ssrc-audio-level
      HeaderExtensionURI_MixertoClientAudioLevelIndication,             // https://tools.ietf.org/html/rfc6465
                                                                        // urn:ietf:params:rtp-hdrext:csrc-audio-level

      HeaderExtensionURI_FrameMarking,                                  // https://tools.ietf.org/html/draft-berger-avtext-framemarking-01
                                                                        // urn:ietf:params:rtp-hdrext:framemarkinginfo

      HeaderExtensionURI_ExtendedSourceInformation,                     // extended information about the encoded packet
                                                                        // urn:example:params:rtp-hdrext:extended-ssrc-info

      //  0                   1                   2                   3
      //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      // |ID=1   |        Reserved       |   Original sequence number    |
      // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      // |      Associated synchronization source (SSRC) identifier      |
      //
      // ID=1 (4 bits) Extended SSRC RTX packet information
      //
      // Original sequence number (16 bits) - original sequence number field
      //                                      as copied outside the encrypted
      //                                      payload (for mixers)
      // Associated SSRC (32 bits) - the SSSRC of the original SSRC to which
      //                             this RTX stream applies.
      //
      // When a mixer is present, this header can be carried encrypted to
      // the mixer indicating the important information about the RTX packet
      // so the mixer can make intelligent decisions about where to forward the
      // RTX packet.
      //
      // When no mixer is present, this header need only be transmitted by
      // the sending party until the receiver has successfully acknowledged
      // receipt of any RTP packet carrying this data though any means (e.g.
      // RTCP receiver report or RTCP ACK)

      //  0                   1                   2                   3
      //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      // |ID=2   |      Reserved       |A|            Reserved           |
      // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      // |      Associated synchronization source (SSRC) identifier      |
      //
      // ID=2 (4 bits) Extended FEC/RED information
      //
      // A flag (1 bit) - the Associated SSRC field is valid.
      //
      // Associated SSRC (32 bits)- the SSSRC of the original SSRC to which
      //                            this FEC/RED applies (i.e. needed where
      //                            ambiguity may exist).

      //  0                   1                   2                   3
      //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      // |ID=3   |      Reserved       |A|            Reserved           |
      // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      // |      Associated synchronization source (SSRC) identifier      |
      //
      // ID=3 (4 bits) MST SVC packet layer
      //
      // A flag (1 bit) - the Associated SSRC field is valid.
      //
      // Associated SSRC (32 bits)- the SSSRC of the base layer SSRC to which
      //                            this MST applies (i.e. needed where
      //                            ambiguity may exist).

      HeaderExtensionURI_3gpp_VideoOrientation,                         //  urn:3gpp:video-orientation
      HeaderExtensionURI_3gpp_VideoOrientation6,                        //  urn:3gpp:video-orientation:6

      HeaderExtensionURI_Last = HeaderExtensionURI_3gpp_VideoOrientation6
    };

    static const char *toString(HeaderExtensionURIs extension);         // converts header enum to URN format
    static HeaderExtensionURIs toHeaderExtensionURI(const char *uri);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::KnownFeedbackMechanisms
    #pragma mark

    enum KnownFeedbackMechanisms
    {
      KnownFeedbackMechanism_NACK,          // nack
      KnownFeedbackMechanism_PLI,           // pli
      KnownFeedbackMechanism_REMB,          // goog-remb
    };

    static const char *toSting(KnownFeedbackMechanisms mechanism);
    static KnownFeedbackMechanisms toFeedbackMechanism(const char *mechanism);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::KnownRTCP
    #pragma mark

    enum SupportedRTCPMechanisms
    {
      // http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml#rtp-parameters-4

      SupportedRTCPMechanism_First,

      SupportedRTCPMechanism_Unknown = SupportedRTCPMechanism_First,

      SupportedRTCPMechanism_SR,               // https://tools.ietf.org/html/rfc3550#section-6.4.1
      SupportedRTCPMechanism_RR,               // https://tools.ietf.org/html/rfc3550#section-6.4.2
      SupportedRTCPMechanism_SDES,             // https://tools.ietf.org/html/rfc3550#section-6.5
      SupportedRTCPMechanism_BYE,              // https://tools.ietf.org/html/rfc3550#section-6.6
      SupportedRTCPMechanism_RTPFB,            // https://tools.ietf.org/html/rfc4585#section-6.2
      SupportedRTCPMechanism_PSFB,             // https://tools.ietf.org/html/rfc4585#section-6.3

      SupportedRTCPMechanism_Last = SupportedRTCPMechanism_PSFB
    };

    static const char *toString(SupportedRTCPMechanisms mechanism);
    static SupportedRTCPMechanisms toSupportedRTCPMechanism(const char *mechanism);
  };
}
