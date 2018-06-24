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
  //
  // IRTPTypes
  //
  
  interaction IRTPTypes
  {
    ZS_DECLARE_STRUCT_PTR(Capabilities);
    ZS_DECLARE_STRUCT_PTR(CodecCapability);
    ZS_DECLARE_STRUCT_PTR(OpusCodecCapabilityOptions);
    ZS_DECLARE_STRUCT_PTR(OpusCodecCapabilityParameters);
    ZS_DECLARE_STRUCT_PTR(VP8CodecCapabilityParameters);
    ZS_DECLARE_STRUCT_PTR(H264CodecCapabilityParameters);
    ZS_DECLARE_STRUCT_PTR(RTXCodecCapabilityParameters);
    ZS_DECLARE_STRUCT_PTR(FlexFECCodecCapabilityParameters);
    ZS_DECLARE_STRUCT_PTR(HeaderExtension);
    ZS_DECLARE_STRUCT_PTR(RTCPFeedback);
    ZS_DECLARE_STRUCT_PTR(Parameters);
    ZS_DECLARE_TYPEDEF_PTR(Parameters, ImmutableParameters);
    ZS_DECLARE_STRUCT_PTR(CodecParameters);
    ZS_DECLARE_STRUCT_PTR(OpusCodecParameters);
    ZS_DECLARE_TYPEDEF_PTR(VP8CodecCapabilityParameters, VP8CodecParameters);
    ZS_DECLARE_TYPEDEF_PTR(H264CodecCapabilityParameters, H264CodecParameters);
    ZS_DECLARE_TYPEDEF_PTR(RTXCodecCapabilityParameters, RTXCodecParameters);
    ZS_DECLARE_TYPEDEF_PTR(FlexFECCodecCapabilityParameters, FlexFECCodecParameters);
    ZS_DECLARE_STRUCT_PTR(REDCodecParameters);
    ZS_DECLARE_STRUCT_PTR(HeaderExtensionParameters);
    ZS_DECLARE_STRUCT_PTR(EncodingParameters);
    ZS_DECLARE_STRUCT_PTR(RTCPParameters);
    ZS_DECLARE_STRUCT_PTR(FECParameters);
    ZS_DECLARE_STRUCT_PTR(RTXParameters);

    ZS_DECLARE_TYPEDEF_PTR(std::list<CodecCapability>, CodecCapabilitiesList);
    ZS_DECLARE_TYPEDEF_PTR(std::list<HeaderExtension>, HeaderExtensionsList);
    ZS_DECLARE_TYPEDEF_PTR(std::list<RTCPFeedback>, RTCPFeedbackList);
    ZS_DECLARE_TYPEDEF_PTR(std::list<CodecParameters>, CodecParametersList);
    ZS_DECLARE_TYPEDEF_PTR(std::list<HeaderExtensionParameters>, HeaderExtensionParametersList);
    ZS_DECLARE_TYPEDEF_PTR(std::list<EncodingParameters>, EncodingParametersList);

    typedef String FECMechanism;
    ZS_DECLARE_TYPEDEF_PTR(std::list<FECMechanism>, FECMechanismList);

    typedef BYTE PayloadType;
    typedef DWORD SSRCType;
    ZS_DECLARE_TYPEDEF_PTR(std::list<PayloadType>, PayloadTypeList);
    typedef String EncodingID;
    ZS_DECLARE_TYPEDEF_PTR(std::list<EncodingID>, EncodingIDList);

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::DegradationPreference
    //

    enum DegradationPreferences {
      DegradationPreference_First,

      DegradationPreference_MaintainFramerate     = DegradationPreference_First,
      DegradationPreference_MaintainResolution,
      DegradationPreference_Balanced,

      DegradationPreference_Last                  = DegradationPreference_Balanced,
    };

    static const char *toString(DegradationPreferences preference) noexcept;
    static DegradationPreferences toDegredationPreference(const char *preference) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::Capabilities
    //

    struct Capabilities {
      CodecCapabilitiesList mCodecs;
      HeaderExtensionsList  mHeaderExtensions;
      FECMechanismList      mFECMechanisms;

      Capabilities() noexcept {}
      Capabilities(const Capabilities &op2) noexcept {(*this) = op2;}
      Capabilities(ElementPtr rootEl) noexcept;

      static CapabilitiesPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return CapabilitiesPtr(); return make_shared<Capabilities>(rootEl); }
      ElementPtr createElement(const char *objectName = "capabilities") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::CodecCapability
    //

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

      CodecCapability() noexcept {};
      CodecCapability(const CodecCapability &source) noexcept;
      CodecCapability(ElementPtr elem) noexcept;
      CodecCapability &operator=(const CodecCapability &op2) noexcept;

      ElementPtr createElement(const char *objectName = "codecCapability") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;

    protected:
      void copyAny(const CodecCapability &source) noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::OpusCodecCapabilityOptions
    //

    struct OpusCodecCapabilityOptions : public Any
    {
      enum Signals {
        Signal_First,

        Signal_Auto = Signal_First,
        Signal_Music,
        Signal_Voice,

        Signal_Last = Signal_Voice
      };
      static const char *toString(Signals signal) noexcept;
      static Signals toSignal(const char *signal) noexcept(false);

      enum Applications {
        Application_First,

        Application_VoIP = Application_First,
        Application_Audio,
        Application_LowDelay,

        Application_Last = Application_LowDelay
      };
      static const char *toString(Applications application) noexcept;
      static Applications toApplication(const char *application) noexcept(false);

      // sender capabilities
      Optional<ULONG> mComplexity;
      Optional<Signals> mSignal;
      Optional<Applications> mApplication;
      Optional<ULONG> mPacketLossPerc;
      Optional<bool> mPredictionDisabled;

      OpusCodecCapabilityOptions() noexcept {}
      OpusCodecCapabilityOptions(const OpusCodecCapabilityOptions &op2) noexcept {(*this) = op2;}
      OpusCodecCapabilityOptions(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "codecCapability") const noexcept;

      static OpusCodecCapabilityOptionsPtr create(const OpusCodecCapabilityOptions &capability) noexcept;
      static OpusCodecCapabilityOptionsPtr convert(AnyPtr any) noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::OpusCodecCapabilityParameters
    //

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

      OpusCodecCapabilityParameters() noexcept {}
      OpusCodecCapabilityParameters(const OpusCodecCapabilityParameters &op2) noexcept {(*this) = op2;}
      OpusCodecCapabilityParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "opusCodecCapabilityParameters") const noexcept;

      static OpusCodecCapabilityParametersPtr create(const OpusCodecCapabilityParameters &capability) noexcept;
      static OpusCodecCapabilityParametersPtr convert(AnyPtr any) noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::VP8CodecCapabilityParameters
    //

    // https://tools.ietf.org/html/rfc7741#section-6.1
    struct VP8CodecCapabilityParameters : public Any
    {
      Optional<ULONG> mMaxFR;
      Optional<ULONGLONG> mMaxFS;

      VP8CodecCapabilityParameters() noexcept {}
      VP8CodecCapabilityParameters(const VP8CodecCapabilityParameters &op2) noexcept {(*this) = op2;}
      VP8CodecCapabilityParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "vp8CodecCapabilityParameters") const noexcept;

      static VP8CodecCapabilityParametersPtr create(const VP8CodecCapabilityParameters &capability) noexcept;
      static VP8CodecCapabilityParametersPtr convert(AnyPtr any) noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::H264CodecCapabilityParameters
    //

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

      H264CodecCapabilityParameters() noexcept {}
      H264CodecCapabilityParameters(const H264CodecCapabilityParameters &op2) noexcept {(*this) = op2;}
      H264CodecCapabilityParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "h264CodecCapabilityParameters") const noexcept;

      static H264CodecCapabilityParametersPtr create(const H264CodecCapabilityParameters &capability) noexcept;
      static H264CodecCapabilityParametersPtr convert(AnyPtr any) noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::RTXCodecCapabilityParameters
    //

    // https://tools.ietf.org/html/rfc4588#section-8.1
    struct RTXCodecCapabilityParameters : public Any
    {
      PayloadType mApt {};
      Milliseconds mRTXTime {};

      static RTXCodecCapabilityParametersPtr create(const RTXCodecCapabilityParameters &params) noexcept;
      static RTXCodecCapabilityParametersPtr convert(AnyPtr any) noexcept;

      RTXCodecCapabilityParameters() noexcept {}
      RTXCodecCapabilityParameters(const RTXCodecParameters &op2) noexcept {(*this) = op2;}
      RTXCodecCapabilityParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "rtxCodecCapabilityParameters") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::FlexFECCodecCapabilityParameters
    //

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
      static const char *toString(ToPs top) noexcept;
      static ToPs toToP(const char *top) noexcept;
      static ToPs toToP(ULONG value) noexcept(false);

      Microseconds mRepairWindow {};

      Optional<ULONG> mL;
      Optional<ULONG> mD;
      Optional<ToPs> mToP;

      FlexFECCodecCapabilityParameters() noexcept {}
      FlexFECCodecCapabilityParameters(const FlexFECCodecCapabilityParameters &op2) noexcept {(*this) = op2;}
      FlexFECCodecCapabilityParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "flexFecCodecParameters") const noexcept;

      static FlexFECCodecCapabilityParametersPtr create(const FlexFECCodecCapabilityParameters &params) noexcept;
      static FlexFECCodecCapabilityParametersPtr convert(AnyPtr any) noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::HeaderExtensions
    //

    struct HeaderExtension {
      String mKind;
      String mURI;
      USHORT mPreferredID {};
      bool   mPreferredEncrypt {false};

      HeaderExtension() noexcept {}
      HeaderExtension(const HeaderExtension &op2) noexcept {(*this) = op2;}
      HeaderExtension(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "headerExtensions") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::RTCPFeedback
    //

    struct RTCPFeedback {
      String mType;
      String mParameter;

      RTCPFeedback() noexcept {}
      RTCPFeedback(const RTCPFeedback &op2) noexcept {(*this) = op2;}
      RTCPFeedback(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "rtcpFeedback") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::RTCPParameters
    //

    struct RTCPParameters {
      SSRCType  mSSRC {};
      String    mCName;
      bool      mReducedSize {false};
      bool      mMux {true};

      RTCPParameters() noexcept {}
      RTCPParameters(const RTCPParameters &op2) noexcept {(*this) = op2;}
      RTCPParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "rtcpParameters") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::Parameters
    //

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

        HashOptions() noexcept {}
      };

      Parameters() noexcept {}
      Parameters(const Parameters &op2) noexcept;
      Parameters(ElementPtr rootEl) noexcept;

      Parameters &operator=(const Parameters &op2) noexcept;

      static ParametersPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash(const HashOptions &options = HashOptions()) const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::CodecParameters
    //

    struct CodecParameters {
      String            mName;
      PayloadType       mPayloadType {};
      Optional<ULONG>   mClockRate {};
      Milliseconds      mPTime {};
      Milliseconds      mMaxPTime {};
      Optional<ULONG>   mNumChannels {};
      RTCPFeedbackList  mRTCPFeedback;
      AnyPtr            mParameters;  // see OpusCodecParameters, RTXCodecParameters, REDCodecParameters, FlexFECCodecParameters for definitions

      CodecParameters() noexcept {}
      CodecParameters(const CodecParameters &source) noexcept;
      CodecParameters(ElementPtr elem) noexcept;

      CodecParameters &operator=(const CodecParameters &op2) noexcept;
      
      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;

    protected:
      void copyAny(const CodecParameters &op2) noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::OpusCodecParameters
    //

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

      OpusCodecParameters() noexcept {}
      OpusCodecParameters(const OpusCodecParameters &op2) noexcept {(*this) = op2;}
      OpusCodecParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "opusCodecParameters") const noexcept;

      static OpusCodecParametersPtr create(const OpusCodecParameters &capability) noexcept;
      static OpusCodecParametersPtr convert(AnyPtr any) noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::VP8CodecParameters
    //

    // see VP8CodecCapabilityParameters

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::H264CodecParameters
    //

    // see H264CodecCapabilityParameters

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::RTXCodecParameters
    //

    // see RTXCodecCapabilityParameters

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::REDCodecParameters
    //

    struct REDCodecParameters : public Any
    {
      PayloadTypeList mPayloadTypes;

      REDCodecParameters() noexcept {}
      REDCodecParameters(const REDCodecParameters &op2) noexcept {(*this) = op2;}
      REDCodecParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "redCodecParameters") const noexcept;

      static REDCodecParametersPtr create(const REDCodecParameters &params) noexcept;
      static REDCodecParametersPtr convert(AnyPtr any) noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::HeaderExtensionParameters
    //

    struct HeaderExtensionParameters {
      String  mURI;
      USHORT  mID {};
      bool    mEncrypt {false};

      HeaderExtensionParameters() noexcept {}
      HeaderExtensionParameters(const HeaderExtensionParameters &op2) noexcept {(*this) = op2;}
      HeaderExtensionParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "headerExtensionParameters") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::FECParameters
    //

    struct FECParameters {
      Optional<SSRCType>  mSSRC;
      String              mMechanism;

      FECParameters() noexcept {}
      FECParameters(const FECParameters &op2) noexcept {(*this) = op2;}
      FECParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "fecParameters") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::RTXParameters
    //

    struct RTXParameters {
      Optional<SSRCType>    mSSRC;

      RTXParameters() noexcept {}
      RTXParameters(const RTXParameters &op2) noexcept {(*this) = op2;}
      RTXParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "rtxParameters") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::PriorityTypes
    //

    enum PriorityTypes {
      PriorityType_First,

      PriorityType_VeryLow =        PriorityType_First,
      PriorityType_Low,
      PriorityType_Medium,
      PriorityType_High,

      PriorityType_Last =           PriorityType_High,
    };

    static const char *toString(PriorityTypes type) noexcept;
    static PriorityTypes toPriorityType(const char *type) noexcept(false);

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::EncodingParameters
    //

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

      EncodingParameters() noexcept {}
      EncodingParameters(const EncodingParameters &op2) noexcept {(*this) = op2;}
      EncodingParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "encodingParameters") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::CodecKinds
    //

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

    static const char *toString(CodecKinds kind) noexcept;
    static CodecKinds toCodecKind(const char *kind) noexcept;

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::SupportedCodecs
    //

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

    static const char *toString(SupportedCodecs codec) noexcept;
    static SupportedCodecs toSupportedCodec(const char *codec) noexcept;
    static bool hasCapabilityOptions(SupportedCodecs codec) noexcept;
    static bool hasCapabilityParameters(SupportedCodecs codec) noexcept;
    static bool hasParameters(SupportedCodecs codec) noexcept;
    static bool requiresCapabilityParameters(SupportedCodecs codec) noexcept;
    static bool requiresParameters(SupportedCodecs codec) noexcept;

    static CodecKinds getCodecKind(SupportedCodecs codec) noexcept;
    static bool isSRSTCodec(SupportedCodecs codec) noexcept; // Single RTP streams Single Transport
    static bool isMRSTCodec(SupportedCodecs codec) noexcept; // Multiple RTP streams Single Transport

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::SupportedCodecs
    //

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

    static const char *toString(ReservedCodecPayloadTypes reservedCodec) noexcept;
    static ReservedCodecPayloadTypes toReservedCodec(PayloadType pt) noexcept;
    static ReservedCodecPayloadTypes toReservedCodec(const char *encodingName) noexcept;

    static ULONG getDefaultClockRate(ReservedCodecPayloadTypes reservedCodec) noexcept;
    static Optional<ULONG> getDefaultNumberOfChannels(ReservedCodecPayloadTypes reservedCodec) noexcept;

    static CodecKinds getCodecKind(ReservedCodecPayloadTypes reservedCodec) noexcept;
    static SupportedCodecs toSupportedCodec(ReservedCodecPayloadTypes reservedCodec) noexcept;

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::HeaderExtensionURIs
    //

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

    static const char *toString(HeaderExtensionURIs extension) noexcept;         // converts header enum to URN format
    static HeaderExtensionURIs toHeaderExtensionURI(const char *uri) noexcept;
    static String toKind(HeaderExtensionURIs extension) noexcept;                // returns "audio", "video" or "" for both

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::KnownFECMechanisms
    //

    enum KnownFECMechanisms
    {
      KnownFECMechanism_First,

      KnownFECMechanism_Unknown = KnownFECMechanism_First,

      KnownFECMechanism_RED,                                // "red" https://tools.ietf.org/html/rfc2198 as a redendent FEC mechanism
      KnownFECMechanism_RED_ULPFEC,                         // "red+ulpfec" https://tools.ietf.org/html/rfc5109 as a single SSRC stream using RED
      KnownFECMechanism_FlexFEC,                            // "flexfec" https://tools.ietf.org/html/draft-ietf-payload-flexible-fec-scheme-01

      KnownFECMechanism_Last = KnownFECMechanism_FlexFEC,
    };

    static const char *toString(KnownFECMechanisms mechanism) noexcept;
    static KnownFECMechanisms toKnownFECMechanism(const char *mechanism) noexcept;

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::KnownFeedbackTypes
    //

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

    static const char *toString(KnownFeedbackTypes type) noexcept;
    static KnownFeedbackTypes toKnownFeedbackType(const char *type) noexcept;

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::KnownFeedbackParameters
    //

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

    static const char *toString(KnownFeedbackParameters parameter) noexcept;
    static KnownFeedbackParameters toKnownFeedbackParameter(const char *parameter) noexcept;

    static KnownFeedbackTypesSet getUseableWithFeedbackTypes(KnownFeedbackParameters mechanism) noexcept;

    //-------------------------------------------------------------------------
    //
    // IRTPTypes::KnownRTCP
    //

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

    static const char *toString(SupportedRTCPMechanisms mechanism) noexcept;
    static SupportedRTCPMechanisms toSupportedRTCPMechanism(const char *mechanism) noexcept;
  };
}
