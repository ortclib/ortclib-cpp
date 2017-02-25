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
#include <set>

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
    ZS_DECLARE_STRUCT_PTR(OpusCodecCapabilityOptions)
    ZS_DECLARE_STRUCT_PTR(OpusCodecCapabilityParameters)
    ZS_DECLARE_STRUCT_PTR(VP8CodecCapabilityParameters)
    ZS_DECLARE_STRUCT_PTR(H264CodecCapabilityParameters)
    ZS_DECLARE_STRUCT_PTR(RTXCodecCapabilityParameters)
    ZS_DECLARE_STRUCT_PTR(FlexFECCodecCapabilityParameters)
    ZS_DECLARE_STRUCT_PTR(HeaderExtension)
    ZS_DECLARE_STRUCT_PTR(RTCPFeedback)
    ZS_DECLARE_STRUCT_PTR(Parameters)
    ZS_DECLARE_STRUCT_PTR(CodecParameters)
    ZS_DECLARE_STRUCT_PTR(OpusCodecParameters)
    ZS_DECLARE_TYPEDEF_PTR(VP8CodecCapabilityParameters, VP8CodecParameters)
    ZS_DECLARE_TYPEDEF_PTR(H264CodecCapabilityParameters, H264CodecParameters)
    ZS_DECLARE_TYPEDEF_PTR(RTXCodecCapabilityParameters, RTXCodecParameters)
    ZS_DECLARE_TYPEDEF_PTR(FlexFECCodecCapabilityParameters, FlexFECCodecParameters)
    ZS_DECLARE_STRUCT_PTR(REDCodecParameters)
    ZS_DECLARE_STRUCT_PTR(HeaderExtensionParameters)
    ZS_DECLARE_STRUCT_PTR(EncodingParameters)
    ZS_DECLARE_STRUCT_PTR(RTCPParameters)
    ZS_DECLARE_STRUCT_PTR(FECParameters)
    ZS_DECLARE_STRUCT_PTR(RTXParameters)

    ZS_DECLARE_TYPEDEF_PTR(std::list<CodecCapability>, CodecCapabilitiesList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<HeaderExtension>, HeaderExtensionsList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<RTCPFeedback>, RTCPFeedbackList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<CodecParameters>, CodecParametersList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<HeaderExtensionParameters>, HeaderExtensionParametersList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<EncodingParameters>, EncodingParametersList)

    typedef String FECMechanism;
    ZS_DECLARE_TYPEDEF_PTR(std::list<FECMechanism>, FECMechanismList)

    typedef BYTE PayloadType;
    typedef DWORD SSRCType;
    ZS_DECLARE_TYPEDEF_PTR(std::list<PayloadType>, PayloadTypeList)
    typedef String EncodingID;
    ZS_DECLARE_TYPEDEF_PTR(std::list<EncodingID>, EncodingIDList)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::DegradationPreference
    #pragma mark

    enum DegradationPreferences {
      DegradationPreference_First,

      DegradationPreference_MaintainFramerate     = DegradationPreference_First,
      DegradationPreference_MaintainResolution,
      DegradationPreference_Balanced,

      DegradationPreference_Last                  = DegradationPreference_Balanced,
    };

    static const char *toString(DegradationPreferences preference);
    static DegradationPreferences toDegredationPreference(const char *preference) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::Capabilities
    #pragma mark

    struct Capabilities {
      CodecCapabilitiesList mCodecs;
      HeaderExtensionsList  mHeaderExtensions;
      FECMechanismList      mFECMechanisms;

      Capabilities() {}
      Capabilities(const Capabilities &op2) {(*this) = op2;}
      Capabilities(ElementPtr rootEl);

      static CapabilitiesPtr create(ElementPtr rootEl) { if (!rootEl) return CapabilitiesPtr(); return make_shared<Capabilities>(rootEl); }
      ElementPtr createElement(const char *objectName = "capabilities") const;

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
      Optional<ULONG>   mClockRate {};
      PayloadType       mPreferredPayloadType {};
      Milliseconds      mPTime {};
      Milliseconds      mMaxPTime {};
      Optional<ULONG>   mNumChannels {};
      RTCPFeedbackList  mRTCPFeedback;
      AnyPtr            mParameters;              // OpusCodecCapabilityParameters, VP8CodecCapabilityParameters, H264CodecCapabilityParameters, RTXCodecCapabilityParameters, FlexFECCodecCapabilityParameters
      AnyPtr            mOptions;                 // OpusCodecCapabilityOptions
      USHORT            mMaxTemporalLayers {0};
      USHORT            mMaxSpatialLayers {0};
      bool              mSVCMultiStreamSupport {};

      CodecCapability() {};
      CodecCapability(const CodecCapability &source);
      CodecCapability(ElementPtr elem);

      CodecCapability &operator=(const CodecCapability &op2) = delete;

      ElementPtr createElement(const char *objectName = "codecCapability") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::OpusCodecCapabilityOptions
    #pragma mark

    struct OpusCodecCapabilityOptions : public Any
    {
      enum Signals {
        Signal_First,

        Signal_Auto = Signal_First,
        Signal_Music,
        Signal_Voice,

        Signal_Last = Signal_Voice
      };
      static const char *toString(Signals signal);
      static Signals toSignal(const char *signal);

      enum Applications {
        Application_First,

        Application_VoIP = Application_First,
        Application_Audio,
        Application_LowDelay,

        Application_Last = Application_LowDelay
      };
      static const char *toString(Applications application);
      static Applications toApplication(const char *application);

      // sender capabilities
      Optional<ULONG> mComplexity;
      Optional<Signals> mSignal;
      Optional<Applications> mApplication;
      Optional<ULONG> mPacketLossPerc;
      Optional<bool> mPredictionDisabled;

      OpusCodecCapabilityOptions() {}
      OpusCodecCapabilityOptions(const OpusCodecCapabilityOptions &op2) {(*this) = op2;}
      OpusCodecCapabilityOptions(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "codecCapability") const;

      static OpusCodecCapabilityOptionsPtr create(const OpusCodecCapabilityOptions &capability);
      static OpusCodecCapabilityOptionsPtr convert(AnyPtr any);

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::OpusCodecCapabilityParameters
    #pragma mark

    // https://tools.ietf.org/html/rfc7587#section-6.1
    struct OpusCodecCapabilityParameters : public Any
    {
      // receiver capability parameters
      Optional<ULONG> mMaxPlaybackRate;
      Optional<ULONG> mMaxAverageBitrate;
      Optional<bool> mStereo;
      Optional<bool> mCBR;
      Optional<bool> mUseInbandFEC;
      Optional<bool> mUseDTX;

      // sender capability parameters
      Optional<ULONG> mSPropMaxCaptureRate;
      Optional<bool> mSPropStereo;

      OpusCodecCapabilityParameters() {}
      OpusCodecCapabilityParameters(const OpusCodecCapabilityParameters &op2) {(*this) = op2;}
      OpusCodecCapabilityParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "opusCodecCapabilityParameters") const;

      static OpusCodecCapabilityParametersPtr create(const OpusCodecCapabilityParameters &capability);
      static OpusCodecCapabilityParametersPtr convert(AnyPtr any);

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::VP8CodecCapabilityParameters
    #pragma mark

    // https://tools.ietf.org/html/rfc7741#section-6.1
    struct VP8CodecCapabilityParameters : public Any
    {
      Optional<ULONG> mMaxFR;
      Optional<ULONGLONG> mMaxFS;

      VP8CodecCapabilityParameters() {}
      VP8CodecCapabilityParameters(const VP8CodecCapabilityParameters &op2) {(*this) = op2;}
      VP8CodecCapabilityParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "vp8CodecCapabilityParameters") const;

      static VP8CodecCapabilityParametersPtr create(const VP8CodecCapabilityParameters &capability);
      static VP8CodecCapabilityParametersPtr convert(AnyPtr any);

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::H264CodecCapabilityParameters
    #pragma mark

    // https://tools.ietf.org/html/rfc7742#section-6.2
    struct H264CodecCapabilityParameters : public Any
    {
      typedef std::list<USHORT>  PacketizationModeList;

      Optional<ULONG> mProfileLevelID {};
      PacketizationModeList mPacketizationModes;

      Optional<ULONGLONG> mMaxMBPS {};
      Optional<ULONGLONG> mMaxSMBPS {};
      Optional<ULONGLONG> mMaxFS {};
      Optional<ULONGLONG> mMaxCPB {};
      Optional<ULONGLONG> mMaxDPB {};
      Optional<ULONGLONG> mMaxBR {};

      H264CodecCapabilityParameters() {}
      H264CodecCapabilityParameters(const H264CodecCapabilityParameters &op2) {(*this) = op2;}
      H264CodecCapabilityParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "h264CodecCapabilityParameters") const;

      static H264CodecCapabilityParametersPtr create(const H264CodecCapabilityParameters &capability);
      static H264CodecCapabilityParametersPtr convert(AnyPtr any);

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::RTXCodecCapabilityParameters
    #pragma mark

    // https://tools.ietf.org/html/rfc4588#section-8.1
    struct RTXCodecCapabilityParameters : public Any
    {
      PayloadType mApt {};
      Milliseconds mRTXTime {};

      static RTXCodecCapabilityParametersPtr create(const RTXCodecCapabilityParameters &params);
      static RTXCodecCapabilityParametersPtr convert(AnyPtr any);

      RTXCodecCapabilityParameters() {}
      RTXCodecCapabilityParameters(const RTXCodecParameters &op2) {(*this) = op2;}
      RTXCodecCapabilityParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "rtxCodecCapabilityParameters") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::FlexFECCodecCapabilityParameters
    #pragma mark

    // https://tools.ietf.org/html/draft-ietf-payload-flexible-fec-scheme-02
    struct FlexFECCodecCapabilityParameters : public Any
    {
      enum ToPs {
        ToP_First                           = 0,

        ToP_1DInterleavedFEC                = ToP_First,  // 1d-interleaved-fec
        ToP_1DNonInterleavedFEC             = 1,          // 1d-non-interleaved-fec
        ToP_2DParityFEEC                    = 2,          // 2d-parity-fec
        ToP_Reserved                        = 3,

        ToP_Last = ToP_Reserved,
      };
      static const char *toString(ToPs top);
      static ToPs toToP(const char *top);
      static ToPs toToP(ULONG value);

      Microseconds mRepairWindow {};

      Optional<ULONG> mL;
      Optional<ULONG> mD;
      Optional<ToPs> mToP;

      FlexFECCodecCapabilityParameters() {}
      FlexFECCodecCapabilityParameters(const FlexFECCodecCapabilityParameters &op2) {(*this) = op2;}
      FlexFECCodecCapabilityParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "flexFecCodecParameters") const;

      static FlexFECCodecCapabilityParametersPtr create(const FlexFECCodecCapabilityParameters &params);
      static FlexFECCodecCapabilityParametersPtr convert(AnyPtr any);

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::HeaderExtensions
    #pragma mark

    struct HeaderExtension {
      String mKind;
      String mURI;
      USHORT mPreferredID {};
      bool   mPreferredEncrypt {false};

      HeaderExtension() {}
      HeaderExtension(const HeaderExtension &op2) {(*this) = op2;}
      HeaderExtension(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "headerExtensions") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::RTCPFeedback
    #pragma mark

    struct RTCPFeedback {
      String mType;
      String mParameter;

      RTCPFeedback() {}
      RTCPFeedback(const RTCPFeedback &op2) {(*this) = op2;}
      RTCPFeedback(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "rtcpFeedback") const;

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

      RTCPParameters() {}
      RTCPParameters(const RTCPParameters &op2) {(*this) = op2;}
      RTCPParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "rtcpParameters") const;

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
      EncodingParametersList        mEncodings;
      RTCPParameters                mRTCP;
      DegradationPreferences        mDegredationPreference {DegradationPreference_Balanced};

      struct HashOptions
      {
        bool mMuxID {true};
        bool mCodecs {true};
        bool mHeaderExtensions {true};
        bool mEncodingParameters {true};
        bool mRTCP {true};
        bool mDegredationPreference {true};

        HashOptions() {}
      };

      Parameters() {}
      Parameters(const Parameters &op2);
      Parameters(ElementPtr rootEl);

      Parameters &operator=(const Parameters &op2);

      static ParametersPtr create(ElementPtr rootEl) { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash(const HashOptions &options = HashOptions()) const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::CodecParameters
    #pragma mark

    struct CodecParameters {
      String            mName;
      PayloadType       mPayloadType {};
      Optional<ULONG>   mClockRate {};
      Milliseconds      mPTime {};
      Milliseconds      mMaxPTime {};
      Optional<ULONG>   mNumChannels {};
      RTCPFeedbackList  mRTCPFeedback;
      AnyPtr            mParameters;  // see OpusCodecParameters, RTXCodecParameters, REDCodecParameters, FlexFECCodecParameters for definitions

      CodecParameters() {}
      CodecParameters(const CodecParameters &source);
      CodecParameters(ElementPtr elem);

      CodecParameters &operator=(const CodecParameters &op2) = delete;

      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::OpusCodecParameters
    #pragma mark

    struct OpusCodecParameters : public Any
    {
      typedef OpusCodecCapabilityOptions::Signals Signals;
      typedef OpusCodecCapabilityOptions::Applications Applications;

      // sender parameters
      Optional<ULONG> mMaxPlaybackRate;
      Optional<ULONG> mMaxAverageBitrate;
      Optional<bool> mStereo;
      Optional<bool> mCBR;
      Optional<bool> mUseInbandFEC;
      Optional<bool> mUseDTX;

      Optional<ULONG> mComplexity;
      Optional<Signals> mSignal;
      Optional<Applications> mApplication;
      Optional<ULONG> mPacketLossPerc;
      Optional<bool> mPredictionDisabled;

      // receiver parameters
      Optional<ULONG> mSPropMaxCaptureRate;
      Optional<bool> mSPropStereo;

      OpusCodecParameters() {}
      OpusCodecParameters(const OpusCodecParameters &op2) {(*this) = op2;}
      OpusCodecParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "opusCodecParameters") const;

      static OpusCodecParametersPtr create(const OpusCodecParameters &capability);
      static OpusCodecParametersPtr convert(AnyPtr any);

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::VP8CodecParameters
    #pragma mark

    // see VP8CodecCapabilityParameters

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::H264CodecParameters
    #pragma mark

    // see H264CodecCapabilityParameters

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::RTXCodecParameters
    #pragma mark

    // see RTXCodecCapabilityParameters

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::REDCodecParameters
    #pragma mark

    struct REDCodecParameters : public Any
    {
      PayloadTypeList mPayloadTypes;

      REDCodecParameters() {}
      REDCodecParameters(const REDCodecParameters &op2) {(*this) = op2;}
      REDCodecParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "redCodecParameters") const;

      static REDCodecParametersPtr create(const REDCodecParameters &params);
      static REDCodecParametersPtr convert(AnyPtr any);

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

      HeaderExtensionParameters() {}
      HeaderExtensionParameters(const HeaderExtensionParameters &op2) {(*this) = op2;}
      HeaderExtensionParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "headerExtensionParameters") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::FECParameters
    #pragma mark

    struct FECParameters {
      Optional<SSRCType>  mSSRC;
      String              mMechanism;

      FECParameters() {}
      FECParameters(const FECParameters &op2) {(*this) = op2;}
      FECParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "fecParameters") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::RTXParameters
    #pragma mark

    struct RTXParameters {
      Optional<SSRCType>    mSSRC;

      RTXParameters() {}
      RTXParameters(const RTXParameters &op2) {(*this) = op2;}
      RTXParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "rtxParameters") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::PriorityTypes
    #pragma mark

    enum PriorityTypes {
      PriorityType_First,

      PriorityType_VeryLow =        PriorityType_First,
      PriorityType_Low,
      PriorityType_Medium,
      PriorityType_High,

      PriorityType_Last =           PriorityType_High,
    };

    static const char *toString(PriorityTypes type);
    static PriorityTypes toPriorityType(const char *type);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::EncodingParameters
    #pragma mark

    struct EncodingParameters {
      Optional<SSRCType>      mSSRC {};
      Optional<PayloadType>   mCodecPayloadType;
      Optional<FECParameters> mFEC;
      Optional<RTXParameters> mRTX;
      Optional<PriorityTypes> mPriority;
      Optional<ULONGLONG>     mMaxBitrate {};
      Optional<double>        mMinQuality {0};
      Optional<double>        mResolutionScale {};
      Optional<double>        mFramerateScale {};
      bool                    mActive {true};
      EncodingID              mEncodingID;
      EncodingIDList          mDependencyEncodingIDs;

      EncodingParameters() {}
      EncodingParameters(const EncodingParameters &op2) {(*this) = op2;}
      EncodingParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "encodingParameters") const;

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

      CodecKind_AudioSupplemental,  // e.g. CN, telephone-event

      CodecKind_Data,               // e.g. fax, teletype

      CodecKind_Last = CodecKind_FEC,
    };

    static const char *toString(CodecKinds kind);
    static CodecKinds toCodecKind(const char *kind);

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
      SupportedCodec_FlexFEC,         // flexfec

      SupportedCodec_CN,              // cn

      SupportedCodec_TelephoneEvent,  // telephone-event

      SupportedCodec_Last = SupportedCodec_TelephoneEvent
    };

    static const char *toString(SupportedCodecs codec);
    static SupportedCodecs toSupportedCodec(const char *codec);
    static bool hasCapabilityOptions(SupportedCodecs codec);
    static bool hasCapabilityParameters(SupportedCodecs codec);
    static bool hasParameters(SupportedCodecs codec);
    static bool requiresCapabilityParameters(SupportedCodecs codec);
    static bool requiresParameters(SupportedCodecs codec);

    static CodecKinds getCodecKind(SupportedCodecs codec);
    static bool isSRSTCodec(SupportedCodecs codec); // Single RTP streams Single Transport
    static bool isMRSTCodec(SupportedCodecs codec); // Multiple RTP streams Single Transport

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
    static ReservedCodecPayloadTypes toReservedCodec(PayloadType pt);
    static ReservedCodecPayloadTypes toReservedCodec(const char *encodingName);

    static ULONG getDefaultClockRate(ReservedCodecPayloadTypes reservedCodec);
    static Optional<ULONG> getDefaultNumberOfChannels(ReservedCodecPayloadTypes reservedCodec);

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

      HeaderExtensionURI_RID,                                           // https://tools.ietf.org/html/draft-pthatcher-mmusic-rid-00
                                                                        // urn:ietf:params:rtp-hdrext:rid

      HeaderExtensionURI_3gpp_VideoOrientation,                         //  urn:3gpp:video-orientation
      HeaderExtensionURI_3gpp_VideoOrientation6,                        //  urn:3gpp:video-orientation:6

      HeaderExtensionURI_TransmissionTimeOffsets,                       // https://tools.ietf.org/html/rfc5450
                                                                        // urn:ietf:params:rtp-hdrext:toffset
      
      HeaderExtensionURI_AbsoluteSendTime,                              // http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time

      HeaderExtensionURI_TransportSequenceNumber,                       // http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01

      HeaderExtensionURI_Last = HeaderExtensionURI_TransportSequenceNumber
    };

    static const char *toString(HeaderExtensionURIs extension);         // converts header enum to URN format
    static HeaderExtensionURIs toHeaderExtensionURI(const char *uri);
    static String toKind(HeaderExtensionURIs extension);                // returns "audio", "video" or "" for both

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::KnownFECMechanisms
    #pragma mark

    enum KnownFECMechanisms
    {
      KnownFECMechanism_First,

      KnownFECMechanism_Unknown = KnownFECMechanism_First,

      KnownFECMechanism_RED,                                // "red" https://tools.ietf.org/html/rfc2198 as a redendent FEC mechanism
      KnownFECMechanism_RED_ULPFEC,                         // "red+ulpfec" https://tools.ietf.org/html/rfc5109 as a single SSRC stream using RED
      KnownFECMechanism_FlexFEC,                            // "flexfec" https://tools.ietf.org/html/draft-ietf-payload-flexible-fec-scheme-01

      KnownFECMechanism_Last = KnownFECMechanism_FlexFEC,
    };

    static const char *toString(KnownFECMechanisms mechanism);
    static KnownFECMechanisms toKnownFECMechanism(const char *mechanism);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::KnownFeedbackTypes
    #pragma mark

    enum KnownFeedbackTypes
    {
      KnownFeedbackType_First,

      KnownFeedbackType_Unknown = KnownFeedbackType_First,

      // http://www.iana.org/assignments/sdp-parameters/sdp-parameters.xhtml#sdp-parameters-14
      KnownFeedbackType_ACK,                                // ack
      KnownFeedbackType_APP,                                // app
      KnownFeedbackType_CCM,                                // ccm
      KnownFeedbackType_NACK,                               // nack
      KnownFeedbackType_TRR_INT,                            // trr-int
      KnownFeedbackType_3gpp_roi_arbitrary,                 // 3gpp-roi-arbitrary
      KnownFeedbackType_3gpp_roi_predefined,                // 3gpp-roi-predefined

      // https://tools.ietf.org/html/draft-alvestrand-rmcat-remb-03
      KnownFeedbackType_REMB,                               // goog-remb

      KnownFeedbackType_transport_cc,                       // transport congestion control

      KnownFeedbackType_Last = KnownFeedbackType_transport_cc,
    };

    typedef std::set<KnownFeedbackTypes> KnownFeedbackTypesSet;

    static const char *toString(KnownFeedbackTypes type);
    static KnownFeedbackTypes toKnownFeedbackType(const char *type);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::KnownFeedbackParameters
    #pragma mark

    enum KnownFeedbackParameters
    {
      KnownFeedbackParameter_First,

      KnownFeedbackParameter_Unknown = KnownFeedbackParameter_First,

      // http://www.iana.org/assignments/sdp-parameters/sdp-parameters.xhtml#sdp-parameters-15
      KnownFeedbackParameter_SLI,                                 // sli
      KnownFeedbackParameter_PLI,                                 // pli
      KnownFeedbackParameter_RPSI,                                // rpsi
      KnownFeedbackParameter_APP,                                 // app
      KnownFeedbackParameter_RAI,                                 // rai
      KnownFeedbackParameter_TLLEI,                               // tllei
      KnownFeedbackParameter_PSLEI,                               // pslei

      // http://www.iana.org/assignments/sdp-parameters/sdp-parameters.xhtml#sdp-parameters-19
      KnownFeedbackParameter_FIR,                                 // fir
      KnownFeedbackParameter_TMMBR,                               // tmmbr
      KnownFeedbackParameter_TSTR,                                // tstr
      KnownFeedbackParameter_VBCM,                                // vbcm
      KnownFeedbackParameter_PAUSE,                               // pause

      KnownFeedbackParameter_Last = KnownFeedbackParameter_PAUSE,
    };

    static const char *toString(KnownFeedbackParameters parameter);
    static KnownFeedbackParameters toKnownFeedbackParameter(const char *parameter);

    static KnownFeedbackTypesSet getUseableWithFeedbackTypes(KnownFeedbackParameters mechanism);

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
