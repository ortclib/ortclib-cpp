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
      SSRCType  mSSRC {};

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::EncodingParameters
    #pragma mark

    struct EncodingParameters {
      SSRCType                mSSRC {};
      PayloadType             mCodecPayloadType {};
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
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes
    #pragma mark

    enum HeaderExtensionURIs
    {
      HeaderExtensionURI_Unknown,

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
      //                                      as copied outside the encryped
      //                                      paylod (for mixers)
      // Associated SSRC (32 bits) - the SSSRC of the original SSRC to which
      //                             this RTX stream applies.
      //
      // When a mixer is present, this header can be carried non-encryopted to
      // the mixer indicating the important information about the RTX packet
      // so the mixer can make intellegent decisions about where to forward the
      // RTX packet.
      //
      // When no mixer is present, this header need only be transmitted by
      // the sending party until the receiver has successfully acknoledged
      // receipt of any RTP packet carrying this data though any means (e.g.
      // RTCP receiver report or RTCP ACK)

      //  0                   1                   2                   3
      //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      // |ID=1   |      Reserved       |A|            Reserved           |
      // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      // |      Associated synchronization source (SSRC) identifier      |
      //
      // ID=2 (4 bits) Extended FEC/RED information
      //
      // A flag (1 bit) - the Asssociated SSRC field is valid.
      //
      // Associated SSRC (32 bits)- the SSSRC of the original SSRC to which
      //                            this FEC/RED applies (i.e. needed where
      //                            ambiguity may exist).
    };

    static const char *toString(HeaderExtensionURIs extension);         // converts header enum to URN format
    static HeaderExtensionURIs toHeaderExtensionURI(const char *uri);
  };
}
