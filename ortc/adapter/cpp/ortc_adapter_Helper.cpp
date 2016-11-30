/*

 Copyright (c) 2016, Hookflash Inc. / Hookflash Inc.
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


#include <ortc/adapter/internal/ortc_adapter_Helper.h>

#include <ortc/internal/ortc_RTPTypes.h>

#include <ortc/IHelper.h>

//
//#include <zsLib/Log.h>
//#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
//#include <zsLib/XML.h>


namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortclib_adapter) } }

namespace ortc
{
  namespace adapter
  {
    ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPTypesHelper, UseRTPTypesHelper);

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Helper
      #pragma mark

    }  // namespace internal

    //-------------------------------------------------------------------------
    const char *IHelper::toString(IDPreferences pref)
    {
      switch (pref)
      {
        case IDPreference_Local:  return "local";
        case IDPreference_Remote: return "remote";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    IRTPTypes::SSRCType IHelper::getRandomSSRC()
    {
      auto random = IHelper::random(sizeof(IRTPTypes::SSRCType));
      IRTPTypes::SSRCType tempSSRC{};
      memcpy(&tempSSRC, random->BytePtr(), random->SizeInBytes());
      return tempSSRC;
    }

    //-----------------------------------------------------------------------
    IHelper::SSRCType IHelper::peekNextSSRC(
                                            IMediaStreamTrackTypes::Kinds kind,
                                            SSRCQueue &audioSSRCQueue,
                                            SSRCQueue &videoSSRCQueue
                                            )
    {
      SSRCQueue *ssrcQueue = NULL;

      switch (kind)
      {
        case ortc::IMediaStreamTrackTypes::Kind_Audio:  ssrcQueue = &audioSSRCQueue; break;
        case ortc::IMediaStreamTrackTypes::Kind_Video:  ssrcQueue = &videoSSRCQueue; break;
      }

      ZS_THROW_INVALID_ASSUMPTION_IF(!ssrcQueue);

      if (ssrcQueue->size() > 0) return ssrcQueue->front();

      auto ssrc = getRandomSSRC();
      ssrcQueue->push(ssrc);
      return ssrc;
    }

    //-----------------------------------------------------------------------
    IHelper::SSRCType IHelper::getNextSSRC(
                                           IMediaStreamTrackTypes::Kinds kind,
                                           SSRCQueue &audioSSRCQueue,
                                           SSRCQueue &videoSSRCQueue
                                           )
    {
      auto result = peekNextSSRC(kind, audioSSRCQueue, videoSSRCQueue);
      switch (kind)
      {
        case ortc::IMediaStreamTrackTypes::Kind_Audio:  audioSSRCQueue.pop(); break;
        case ortc::IMediaStreamTrackTypes::Kind_Video:  videoSSRCQueue.pop(); break;
      }

      return result;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Negotiation (conversion)
    #pragma mark

    //-------------------------------------------------------------------------
    IHelper::RTPParametersPtr IHelper::capabilitiesToParameters(const RTPCapabilities &capabilities)
    {
      auto result = make_shared<RTPParameters>();

      result->mCodecs = *capabilitiesToParameters(capabilities.mCodecs);
      result->mHeaderExtensions = *capabilitiesToParameters(capabilities.mHeaderExtensions);
      // mFECMechanisms -- not applicable; mechanisms are set on encodings

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPCapabilitiesPtr IHelper::parametersToCapabilities(const RTPParameters &parameters)
    {
      auto result = make_shared<RTPCapabilities>();

      result->mCodecs = *parametersToCapabilities(parameters.mCodecs);
      result->mHeaderExtensions = *parametersToCapabilities(parameters.mHeaderExtensions);

      bool foundRED = false;
      bool foundULPFEC = false;
      bool foundFlexFEC = false;

      String usingKind;

      for (auto iter = parameters.mCodecs.begin(); iter != parameters.mCodecs.end(); ++iter) {
        auto &codec = (*iter);

        auto supportedCodec = IRTPTypes::toSupportedCodec(codec.mName);

        switch (supportedCodec)
        {
          case IRTPTypes::SupportedCodec_Unknown:         break;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:            break;
          case IRTPTypes::SupportedCodec_Isac:            break;
          case IRTPTypes::SupportedCodec_G722:            break;
          case IRTPTypes::SupportedCodec_ILBC:            break;
          case IRTPTypes::SupportedCodec_PCMU:            break;
          case IRTPTypes::SupportedCodec_PCMA:            break;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:             break;
          case IRTPTypes::SupportedCodec_VP9:             break;
          case IRTPTypes::SupportedCodec_H264:            break;

          // RTX
          case IRTPTypes::SupportedCodec_RTX:             break;

          // FEC
          case IRTPTypes::SupportedCodec_RED:             foundRED = true;  break;
          case IRTPTypes::SupportedCodec_ULPFEC:          foundULPFEC = true; break;
          case IRTPTypes::SupportedCodec_FlexFEC:         foundFlexFEC = true; break;

          case IRTPTypes::SupportedCodec_CN:              break;

          case IRTPTypes::SupportedCodec_TelephoneEvent:  break;
        }

        auto codecKind = IRTPTypes::getCodecKind(supportedCodec);

        switch (codecKind)
        {
          case IRTPTypes::CodecKind_AudioSupplemental:  usingKind = IRTPTypes::toString(IRTPTypes::CodecKind_Audio); break;
          case IRTPTypes::CodecKind_Audio:              usingKind = IRTPTypes::toString(codecKind); break;
          case IRTPTypes::CodecKind_Video:              usingKind = IRTPTypes::toString(codecKind); break;

          case IRTPTypes::CodecKind_Unknown:
          case IRTPTypes::CodecKind_AV:
          case IRTPTypes::CodecKind_RTX:
          case IRTPTypes::CodecKind_FEC:
          case IRTPTypes::CodecKind_Data:               {
            break;
          }
        }
      }

      if (foundRED) {
        if (foundULPFEC) {
          result->mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC));
        } else {
          result->mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED));
        }
      }
      if (foundFlexFEC) {
        result->mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC));
      }

      for (auto iter = result->mHeaderExtensions.begin(); iter != result->mHeaderExtensions.end(); ++iter) {
        auto &ext = (*iter);
        ext.mKind = IRTPTypes::toKind(IRTPTypes::toHeaderExtensionURI(ext.mURI));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPCodecParametersListPtr IHelper::capabilitiesToParameters(const RTPCodecCapabilitiesList &capabiliites)
    {
      auto result = make_shared<RTPCodecParametersList>();

      for (auto iter = capabiliites.begin(); iter != capabiliites.end(); ++iter) {
        auto &capability = (*iter);

        result->push_back(*capabilitiesToParameters(capability));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPCodecCapabilitiesListPtr IHelper::parametersToCapabilities(const RTPCodecParametersList &parameters)
    {
      auto result = make_shared<RTPCodecCapabilitiesList>();

      for (auto iter = parameters.begin(); iter != parameters.end(); ++iter) {
        auto &capability = (*iter);

        result->push_back(*parametersToCapabilities(capability));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPCodecParametersPtr IHelper::capabilitiesToParameters(const RTPCodecCapability &capabilities)
    {
      auto result = make_shared<RTPCodecParameters>();

      result->mName = capabilities.mName;
      result->mPayloadType = capabilities.mPreferredPayloadType;
      result->mClockRate = capabilities.mClockRate;
      result->mPTime = capabilities.mPTime;
      result->mMaxPTime = capabilities.mMaxPTime;
      result->mNumChannels = capabilities.mNumChannels;
      result->mRTCPFeedback = capabilities.mRTCPFeedback;

      if (capabilities.mParameters) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(capabilities.mName);

        switch (supportedCodec)
        {
          case IRTPTypes::SupportedCodec_Unknown:         break;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:            {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPOpusCodecCapabilityParameters, capabilities.mParameters);
            if (input) {
              result->mParameters = opusCodecCapabilityParametersToParameters(*input);
            }
            break;
          }
          case IRTPTypes::SupportedCodec_Isac:            break;
          case IRTPTypes::SupportedCodec_G722:            break;
          case IRTPTypes::SupportedCodec_ILBC:            break;
          case IRTPTypes::SupportedCodec_PCMU:            break;
          case IRTPTypes::SupportedCodec_PCMA:            break;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:             {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPVP8CodecCapabilityParameters, capabilities.mParameters);
            if (input) {
              result->mParameters = vp8CodecCapabilityParametersToParameters(*input);
            }
            break;
          }
          case IRTPTypes::SupportedCodec_VP9:             break;
          case IRTPTypes::SupportedCodec_H264:            {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPH264CodecCapabilityParameters, capabilities.mParameters);
            if (input) {
              result->mParameters = h264CodecCapabilityParametersToParameters(*input);
            }
            break;
          }

          // RTX
          case IRTPTypes::SupportedCodec_RTX:             {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPRTXCodecCapabilityParameters, capabilities.mParameters);
            if (input) {
              result->mParameters = rtxCodecCapabilityParametersToParameters(*input);
            }
            break;
          }

          // FEC
          case IRTPTypes::SupportedCodec_RED:             break;
          case IRTPTypes::SupportedCodec_ULPFEC:          break;
          case IRTPTypes::SupportedCodec_FlexFEC:         {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPFlexFECCodecCapabilityParameters, capabilities.mParameters);
            if (input) {
              result->mParameters = flexFECCodecCapabilityParametersToParameters(*input);
            }
            break;
          }

          case IRTPTypes::SupportedCodec_CN:              break;

          case IRTPTypes::SupportedCodec_TelephoneEvent:  break;
        }
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPCodecCapabilityPtr IHelper::parametersToCapabilities(const RTPCodecParameters &parameters)
    {
      auto result = make_shared<RTPCodecCapability>();

      result->mName = parameters.mName;
      auto supportedCodec = IRTPTypes::toSupportedCodec(parameters.mName);
      auto codecKind = IRTPTypes::getCodecKind(supportedCodec);

      switch (codecKind)
      {
        case IRTPTypes::CodecKind_AudioSupplemental:  result->mKind = IRTPTypes::toString(IRTPTypes::CodecKind_Audio); break;
        case IRTPTypes::CodecKind_Audio:              result->mKind = IRTPTypes::toString(codecKind); break;
        case IRTPTypes::CodecKind_Video:              result->mKind = IRTPTypes::toString(codecKind); break;

        case IRTPTypes::CodecKind_Unknown:
        case IRTPTypes::CodecKind_AV:
        case IRTPTypes::CodecKind_RTX:
        case IRTPTypes::CodecKind_FEC:
        case IRTPTypes::CodecKind_Data:               {
          break;
        }
      }

      result->mClockRate = parameters.mClockRate;
      result->mPreferredPayloadType = parameters.mPayloadType;
      result->mPTime = parameters.mPTime;
      result->mMaxPTime = parameters.mMaxPTime;
      result->mNumChannels = parameters.mNumChannels;
      result->mRTCPFeedback = parameters.mRTCPFeedback;
      
      if (parameters.mParameters) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(parameters.mName);

        switch (supportedCodec)
        {
          case IRTPTypes::SupportedCodec_Unknown:         break;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:            {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPOpusCodecParameters, parameters.mParameters);
            if (input) {
              result->mParameters = opusCodecParametersToCapabilityParameters(*input);
            }
            break;
          }
          case IRTPTypes::SupportedCodec_Isac:            break;
          case IRTPTypes::SupportedCodec_G722:            break;
          case IRTPTypes::SupportedCodec_ILBC:            break;
          case IRTPTypes::SupportedCodec_PCMU:            break;
          case IRTPTypes::SupportedCodec_PCMA:            break;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:             {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPVP8CodecParameters, parameters.mParameters);
            if (input) {
              result->mParameters = vp8CodecParametersToCapabilityParameters(*input);
            }
            break;
          }
          case IRTPTypes::SupportedCodec_VP9:             break;
          case IRTPTypes::SupportedCodec_H264:            {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPH264CodecParameters, parameters.mParameters);
            if (input) {
              result->mParameters = h264CodecParametersToCapabilityParameters(*input);
            }
            break;
          }

          // RTX
          case IRTPTypes::SupportedCodec_RTX:             {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPRTXCodecParameters, parameters.mParameters);
            if (input) {
              result->mParameters = rtxCodecParametersToCapabilityParameters(*input);
            }
            break;
          }

          // FEC
          case IRTPTypes::SupportedCodec_RED:             break;
          case IRTPTypes::SupportedCodec_ULPFEC:          break;
          case IRTPTypes::SupportedCodec_FlexFEC:         {
            auto input = ZS_DYNAMIC_PTR_CAST(RTPFlexFECCodecParameters, parameters.mParameters);
            if (input) {
              result->mParameters = flexFECCodecParametersToCapabilityParameters(*input);
            }
            break;
          }

          case IRTPTypes::SupportedCodec_CN:              break;

          case IRTPTypes::SupportedCodec_TelephoneEvent:  break;
        }
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPOpusCodecParametersPtr IHelper::opusCodecCapabilityParametersToParameters(const RTPOpusCodecCapabilityParameters &capabilityParameters)
    {
      auto result = make_shared<RTPOpusCodecParameters>();

      result->mMaxPlaybackRate = capabilityParameters.mMaxPlaybackRate;
      result->mMaxAverageBitrate = capabilityParameters.mMaxAverageBitrate;
      result->mStereo = capabilityParameters.mStereo;
      result->mCBR = capabilityParameters.mCBR;
      result->mUseInbandFEC = capabilityParameters.mUseInbandFEC;
      result->mUseDTX = capabilityParameters.mUseDTX;
      result->mSPropMaxCaptureRate = capabilityParameters.mSPropMaxCaptureRate;
      result->mSPropStereo = capabilityParameters.mSPropStereo;

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPOpusCodecCapabilityParametersPtr IHelper::opusCodecParametersToCapabilityParameters(const RTPOpusCodecParameters &parameters)
    {
      auto result = make_shared<RTPOpusCodecCapabilityParameters>();

      result->mMaxPlaybackRate = parameters.mMaxPlaybackRate;
      result->mMaxAverageBitrate = parameters.mMaxAverageBitrate;
      result->mStereo = parameters.mStereo;
      result->mCBR = parameters.mCBR;
      result->mUseInbandFEC = parameters.mUseInbandFEC;
      result->mUseDTX = parameters.mUseDTX;
      result->mSPropMaxCaptureRate = parameters.mSPropMaxCaptureRate;
      result->mSPropStereo = parameters.mSPropStereo;

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPVP8CodecParametersPtr IHelper::vp8CodecCapabilityParametersToParameters(const RTPVP8CodecCapabilityParameters &capabilityParameters)
    {
      return make_shared<RTPVP8CodecParameters>(capabilityParameters);
    }

    //-------------------------------------------------------------------------
    IHelper::RTPVP8CodecCapabilityParametersPtr IHelper::vp8CodecParametersToCapabilityParameters(const RTPVP8CodecParameters &parameters)
    {
      return make_shared<RTPVP8CodecCapabilityParameters>(parameters);
    }

    //-------------------------------------------------------------------------
    IHelper::RTPH264CodecParametersPtr IHelper::h264CodecCapabilityParametersToParameters(const RTPH264CodecCapabilityParameters &capabilityParameters)
    {
      return make_shared<RTPH264CodecParameters>(capabilityParameters);
    }

    //-------------------------------------------------------------------------
    IHelper::RTPH264CodecCapabilityParametersPtr IHelper::h264CodecParametersToCapabilityParameters(const RTPH264CodecParameters &parameters)
    {
      return make_shared<RTPH264CodecCapabilityParameters>(parameters);
    }

    //-------------------------------------------------------------------------
    IHelper::RTPRTXCodecParametersPtr IHelper::rtxCodecCapabilityParametersToParameters(const RTPRTXCodecCapabilityParameters &capabilityParameters)
    {
      return make_shared<RTPRTXCodecParameters>(capabilityParameters);
    }

    //-------------------------------------------------------------------------
    IHelper::RTPRTXCodecCapabilityParametersPtr IHelper::rtxCodecParametersToCapabilityParameters(const RTPRTXCodecParameters &parameters)
    {
      return make_shared<RTPRTXCodecCapabilityParameters>(parameters);
    }

    //-------------------------------------------------------------------------
    IHelper::RTPFlexFECCodecParametersPtr IHelper::flexFECCodecCapabilityParametersToParameters(const RTPFlexFECCodecCapabilityParameters &capabilityParameters)
    {
      return make_shared<RTPFlexFECCodecParameters>(capabilityParameters);
    }

    //-------------------------------------------------------------------------
    IHelper::RTPFlexFECCodecCapabilityParametersPtr IHelper::flexFECCodecParametersToCapabilityParameters(const RTPFlexFECCodecParameters &parameters)
    {
      return make_shared<RTPFlexFECCodecCapabilityParameters>(parameters);
    }

    //-------------------------------------------------------------------------
    IHelper::RTPHeaderExtensionParametersListPtr IHelper::capabilitiesToParameters(const RTPHeaderExtensionsList &capabilities)
    {
      auto result = make_shared<RTPHeaderExtensionParametersList>();

      for (auto iter = capabilities.begin(); iter != capabilities.end(); ++iter) {
        auto &value = (*iter);
        result->push_back(*capabilitiesToParameters(value));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPHeaderExtensionsListPtr IHelper::parametersToCapabilities(const RTPHeaderExtensionParametersList &parameters)
    {
      auto result = make_shared<RTPHeaderExtensionsList>();

      for (auto iter = parameters.begin(); iter != parameters.end(); ++iter) {
        auto &value = (*iter);
        result->push_back(*parametersToCapabilities(value));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPHeaderExtensionParametersPtr IHelper::capabilitiesToParameters(const RTPHeaderExtension &capabilities)
    {
      auto result = make_shared<RTPHeaderExtensionParameters>();

      result->mID = capabilities.mPreferredID;
      result->mURI = capabilities.mURI;
      result->mEncrypt = capabilities.mPreferredEncrypt;

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPHeaderExtensionPtr IHelper::parametersToCapabilities(const RTPHeaderExtensionParameters &parameters)
    {
      auto result = make_shared<RTPHeaderExtension>();

      result->mPreferredID = parameters.mID;
      result->mURI = parameters.mURI;
      result->mPreferredEncrypt = parameters.mEncrypt;

      return result;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Negotiation (unions)
    #pragma mark

    //-------------------------------------------------------------------------
    IHelper::RTPCapabilitiesPtr IHelper::createUnion(
                                                     const RTPCapabilities &localCapabilities,
                                                     const RTPCapabilities &remoteCapabilities,
                                                     IDPreferences preference
                                                     )
    {
      auto result = make_shared<RTPCapabilities>();
      result->mCodecs = *createUnion(localCapabilities.mCodecs, remoteCapabilities.mCodecs, preference);
      result->mHeaderExtensions = *createUnion(localCapabilities.mHeaderExtensions, remoteCapabilities.mHeaderExtensions, preference);
      result->mFECMechanisms = *createUnion(localCapabilities.mFECMechanisms, remoteCapabilities.mFECMechanisms, preference);
      return result;
    }

    //-------------------------------------------------------------------------
    static const IRTPTypes::CodecCapability *findCodecByPayloadType(
                                                                    const IRTPTypes::CodecCapabilitiesList &codecs,
                                                                    IRTPTypes::PayloadType payloadType
                                                                    )
    {
      for (auto iter = codecs.begin(); iter != codecs.end(); ++iter) {
        auto &codec = (*iter);
        if (payloadType != codec.mPreferredPayloadType) continue;
        return &codec;
      }

      return nullptr;
    }

    //-------------------------------------------------------------------------
    static const IRTPTypes::CodecParameters *findCodecByPayloadType(
                                                                    const IRTPTypes::CodecParametersList &codecs,
                                                                    IRTPTypes::PayloadType payloadType
                                                                    )
    {
      for (auto iter = codecs.begin(); iter != codecs.end(); ++iter) {
        auto &codec = (*iter);
        if (payloadType != codec.mPayloadType) continue;
        return &codec;
      }

      return nullptr;
    }

    //-------------------------------------------------------------------------
    static bool isMatch(
                        const IRTPTypes::CodecCapability &primaryCodec,
                        const IRTPTypes::CodecCapability &secondaryCodec,
                        bool &outExact
                        )
    {
      outExact = true;

      if (0 != primaryCodec.mName.compareNoCase(secondaryCodec.mName)) return false;

      if (primaryCodec.mClockRate.hasValue()) {
        if (secondaryCodec.mClockRate.hasValue()) {
          if (primaryCodec.mClockRate.value() != secondaryCodec.mClockRate.value()) return false;
        } else {
          outExact = false;
        }
      } else {
        if (secondaryCodec.mClockRate.hasValue()) outExact = false;
      }

      if (primaryCodec.mNumChannels.hasValue()) {
        if (secondaryCodec.mNumChannels.hasValue()) {
          if (primaryCodec.mNumChannels.value() != secondaryCodec.mNumChannels.value()) return false;
        } else {
          outExact = false;
        }
      } else {
        if (secondaryCodec.mNumChannels.hasValue()) outExact = false;
      }
      return true;
    }

    //-------------------------------------------------------------------------
    static bool isMatch(
                        const IRTPTypes::CodecParameters &primaryCodec,
                        const IRTPTypes::CodecCapability &secondaryCodec,
                        bool &outExact
                        )
    {
      outExact = true;

      {
        if (0 != primaryCodec.mName.compareNoCase(secondaryCodec.mName)) goto not_a_match;

        if (primaryCodec.mClockRate.hasValue()) {
          if (secondaryCodec.mClockRate.hasValue()) {
            if (primaryCodec.mClockRate.value() != secondaryCodec.mClockRate.value()) goto not_a_match;
          } else {
            outExact = false;
          }
        } else {
          if (secondaryCodec.mClockRate.hasValue()) outExact = false;
        }

        if (primaryCodec.mNumChannels.hasValue()) {
          if (secondaryCodec.mNumChannels.hasValue()) {
            if (primaryCodec.mNumChannels.value() != secondaryCodec.mNumChannels.value()) goto not_a_match;
          } else {
            outExact = false;
          }
        } else {
          if (secondaryCodec.mNumChannels.hasValue()) outExact = false;
        }
        goto match;
      }

    not_a_match:
      {
        outExact = false;
        return false;
      }
    match: {}

      return true;
    }

    //-------------------------------------------------------------------------
    static bool isMatch(
                        const IRTPTypes::CodecCapability &primaryCodec,
                        const IRTPTypes::CodecCapability &secondaryCodec,
                        const IRTPTypes::CodecCapabilitiesList &primaryList,
                        const IRTPTypes::CodecCapabilitiesList &secondaryList,
                        bool &outExact,
                        bool &outIsRTX
                        )
    {
      outIsRTX = false;
      outExact = false;

      {
        if (!isMatch(primaryCodec, secondaryCodec, outExact)) goto not_a_match;

        auto supportedType = IRTPTypes::toSupportedCodec(primaryCodec.mName);
      
        switch (supportedType) {
          case IRTPTypes::SupportedCodec_Unknown:         goto not_a_match;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:            break;
          case IRTPTypes::SupportedCodec_Isac:            break;
          case IRTPTypes::SupportedCodec_G722:            break;
          case IRTPTypes::SupportedCodec_ILBC:            break;
          case IRTPTypes::SupportedCodec_PCMU:            break;
          case IRTPTypes::SupportedCodec_PCMA:            break;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:             break;
          case IRTPTypes::SupportedCodec_VP9:             break;
          case IRTPTypes::SupportedCodec_H264:            break;

          // RTX
          case IRTPTypes::SupportedCodec_RTX:             {
            outIsRTX = true;

            auto primaryParams = ZS_DYNAMIC_PTR_CAST(IRTPTypes::RTXCodecCapabilityParameters, primaryCodec.mParameters);
            auto secondaryParams = ZS_DYNAMIC_PTR_CAST(IRTPTypes::RTXCodecCapabilityParameters, secondaryCodec.mParameters);
            if ((!primaryParams) ||
                (!secondaryParams)) goto not_a_match;

            auto primaryRTXedCodec = findCodecByPayloadType(primaryList, primaryParams->mApt);
            auto secondaryRTXedCodec = findCodecByPayloadType(secondaryList, secondaryParams->mApt);
            if ((!primaryRTXedCodec) ||
                (!secondaryRTXedCodec)) goto not_a_match;

            bool exactRTXed = true;
            if (!isMatch(*primaryRTXedCodec, *secondaryRTXedCodec, exactRTXed)) goto not_a_match;
            if (!exactRTXed) outExact = exactRTXed;
            break;
          }

          // FEC
          case IRTPTypes::SupportedCodec_RED:             break;
          case IRTPTypes::SupportedCodec_ULPFEC:          break;
          case IRTPTypes::SupportedCodec_FlexFEC:         break;

          case IRTPTypes::SupportedCodec_CN:              break;

          case IRTPTypes::SupportedCodec_TelephoneEvent:  break;
        }
        goto match;
      }

    not_a_match:
      {
        outIsRTX = false;
        outExact = false;
        return false;
      }

    match: {}

      return true;
    }

    //-------------------------------------------------------------------------
    static bool isMatch(
                        const IRTPTypes::CodecParameters &primaryCodec,
                        const IRTPTypes::CodecCapability &secondaryCodec,
                        const IRTPTypes::CodecParametersList &primaryList,
                        const IRTPTypes::CodecCapabilitiesList &secondaryList,
                        bool &outExact,
                        bool &outIsRTX
                        )
    {
      outIsRTX = false;
      outExact = false;

      {
        if (!isMatch(primaryCodec, secondaryCodec, outExact)) goto not_a_match;

        auto supportedType = IRTPTypes::toSupportedCodec(primaryCodec.mName);
      
        switch (supportedType) {
          case IRTPTypes::SupportedCodec_Unknown:         goto not_a_match;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:            break;
          case IRTPTypes::SupportedCodec_Isac:            break;
          case IRTPTypes::SupportedCodec_G722:            break;
          case IRTPTypes::SupportedCodec_ILBC:            break;
          case IRTPTypes::SupportedCodec_PCMU:            break;
          case IRTPTypes::SupportedCodec_PCMA:            break;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:             break;
          case IRTPTypes::SupportedCodec_VP9:             break;
          case IRTPTypes::SupportedCodec_H264:            break;

          // RTX
          case IRTPTypes::SupportedCodec_RTX:             {
            outIsRTX = true;

            auto primaryParams = ZS_DYNAMIC_PTR_CAST(IRTPTypes::RTXCodecCapabilityParameters, primaryCodec.mParameters);
            auto secondaryParams = ZS_DYNAMIC_PTR_CAST(IRTPTypes::RTXCodecCapabilityParameters, secondaryCodec.mParameters);
            if ((!primaryParams) ||
                (!secondaryParams)) goto not_a_match;

            auto primaryRTXedCodec = findCodecByPayloadType(primaryList, primaryParams->mApt);
            auto secondaryRTXedCodec = findCodecByPayloadType(secondaryList, secondaryParams->mApt);
            if ((!primaryRTXedCodec) ||
                (!secondaryRTXedCodec)) goto not_a_match;

            bool exactRTXed = true;
            if (!isMatch(*primaryRTXedCodec, *secondaryRTXedCodec, exactRTXed)) goto not_a_match;
            if (!exactRTXed) outExact = exactRTXed;
            break;
          }

          // FEC
          case IRTPTypes::SupportedCodec_RED:             break;
          case IRTPTypes::SupportedCodec_ULPFEC:          break;
          case IRTPTypes::SupportedCodec_FlexFEC:         break;

          case IRTPTypes::SupportedCodec_CN:              break;

          case IRTPTypes::SupportedCodec_TelephoneEvent:  break;
        }
        goto match;
      }

    not_a_match:
      {
        outIsRTX = false;
        outExact = false;
        return false;
      }

    match: {}

      return true;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPCodecCapabilitiesListPtr IHelper::createUnion(
                                                              const RTPCodecCapabilitiesList &local,
                                                              const RTPCodecCapabilitiesList &remote,
                                                              IDPreferences preference
                                                              )
    {
      auto result = make_shared<RTPCodecCapabilitiesList>();

      auto &primaryList = (useLocal(preference) ? local : remote);
      auto &secondaryList = (useLocal(preference) ? remote : local);

      // codec ordering matters thus we need to use the primary/secondary list to ensure the correct ordering ends up being the result
      for (auto iterPrimary = primaryList.begin(); iterPrimary != primaryList.end(); ++iterPrimary) {
        auto &primaryCodec = (*iterPrimary);

        auto supportedType = IRTPTypes::toSupportedCodec(primaryCodec.mName);

        const RTPCodecCapability *foundSecondaryExact = nullptr;
        const RTPCodecCapability *foundSecondaryInexact = nullptr;

        bool isRTX = false;

        for (auto iterSecondary = secondaryList.begin(); iterSecondary != secondaryList.end(); ++iterSecondary) {
          auto &secondaryCodec = (*iterSecondary);
          bool exact = true;

          // scope: find a remote matching codec
          {
            if (!isMatch(primaryCodec, secondaryCodec, primaryList, secondaryList, exact, isRTX)) goto not_a_match;
            goto match_found;
          }

        match_found:
          {
            if (exact) {
              foundSecondaryExact = &secondaryCodec;
              break;
            }
            foundSecondaryInexact = &secondaryCodec;
            continue;
          }

        not_a_match:
          {
            continue;
          }
        }

        if (foundSecondaryExact) {
          foundSecondaryInexact = foundSecondaryExact;
        } else {
          foundSecondaryExact = foundSecondaryInexact;
        }
        if (!foundSecondaryExact) continue;

        auto &localCodec = useLocal(preference) ? primaryCodec : *foundSecondaryExact;
        auto &remoteCodec = useLocal(preference) ? *foundSecondaryExact : primaryCodec;

        // still copy from the local codec (not the remote)
        IRTPTypes::CodecCapability temp(localCodec);
        temp.mRTCPFeedback = *createUnion(localCodec.mRTCPFeedback, remoteCodec.mRTCPFeedback, preference);
        temp.mPreferredPayloadType = primaryCodec.mPreferredPayloadType;
        if (useRemote(preference)) {
          if (isRTX) {
            temp.mParameters = remoteCodec.mParameters; // a copy will be made
          }
          result->push_back(temp);
        } else {
          result->push_back(temp);
        }
      }
      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPHeaderExtensionsListPtr IHelper::createUnion(
                                                             const RTPHeaderExtensionsList &local,
                                                             const RTPHeaderExtensionsList &remote,
                                                             IDPreferences preference
                                                             )
    {
      auto result = make_shared<RTPHeaderExtensionsList>();
      for (auto iterLocal = local.begin(); iterLocal != local.end(); ++iterLocal) {
        auto &localExt = (*iterLocal);

        for (auto iterRemote = remote.begin(); iterRemote != remote.end(); ++iterRemote) {
          auto &remoteExt = (*iterRemote);

          if (0 != localExt.mURI.compareNoCase(remoteExt.mURI)) continue;

          RTPHeaderExtension newExt(useLocal(preference) ? localExt : remoteExt);
          result->push_back(newExt);
          break;
        }
      }
      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPFECMechanismListPtr IHelper::createUnion(
                                                         const RTPFECMechanismList &local,
                                                         const RTPFECMechanismList &remote,
                                                         IDPreferences preference
                                                         )
    {
      auto result = make_shared<RTPFECMechanismList>();
      for (auto iterLocal = local.begin(); iterLocal != local.end(); ++iterLocal) {
        auto &localMechanism = (*iterLocal);

        for (auto iterRemote = remote.begin(); iterRemote != remote.end(); ++iterRemote) {
          auto &remoteMechanism = (*iterRemote);

          if (0 != localMechanism.compareNoCase(remoteMechanism)) continue;

          RTPFECMechanism newMechanism(useLocal(preference) ? localMechanism : remoteMechanism);
          result->push_back(newMechanism);
          break;
        }
      }
      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPRTCPFeedbackListPtr IHelper::createUnion(
                                                         const RTPRTCPFeedbackList &local,
                                                         const RTPRTCPFeedbackList &remote,
                                                         IDPreferences preference
                                                         )
    {
      auto result = make_shared<RTPRTCPFeedbackList>();
      for (auto iterLocal = local.begin(); iterLocal != local.end(); ++iterLocal) {
        auto &localFeedback = (*iterLocal);

        for (auto iterRemote = remote.begin(); iterRemote != remote.end(); ++iterRemote) {
          auto &remoteFeedback = (*iterRemote);

          if (0 != localFeedback.mType.compareNoCase(remoteFeedback.mType)) continue;
          if (0 != localFeedback.mParameter.compareNoCase(remoteFeedback.mParameter)) continue;

          RTPRTCPFeedback newFeedback(useLocal(preference) ? localFeedback : remoteFeedback);
          result->push_back(newFeedback);
          break;
        }
      }
      return result;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Negotiation (filter)
    #pragma mark

    //-------------------------------------------------------------------------
    IHelper::RTPParametersPtr IHelper::filterParameters(
                                                        const RTPParameters &parameters,
                                                        const RTPCapabilities &capabilities
                                                        )
    {
      auto result = make_shared<RTPParameters>(parameters);

      result->mCodecs = *filterParameters(parameters.mCodecs, capabilities.mCodecs);
      result->mHeaderExtensions = *filterParameters(parameters.mHeaderExtensions, capabilities.mHeaderExtensions);

      for (auto iter = result->mEncodings.begin(); iter != result->mEncodings.end(); ++iter) {
        auto &encoding = (*iter);
        if (!encoding.mCodecPayloadType.hasValue()) continue;

        auto found = findCodecByPayloadType(result->mCodecs, encoding.mCodecPayloadType);
        if (found) continue;

        // disallow codec payload type being used that is no longer present
        encoding.mCodecPayloadType = Optional<IRTPTypes::PayloadType>();

#define TODO_FILTER_RTX_FEC 1
#define TODO_FILTER_RTX_FEC 2
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPCodecParametersListPtr IHelper::filterParameters(
                                                                 const RTPCodecParametersList &codecParameters,
                                                                 const RTPCodecCapabilitiesList &codecCapabilities
                                                                 )
    {
      auto result = make_shared<RTPCodecParametersList>();

      for (auto iterCodecParams = codecParameters.begin(); iterCodecParams != codecParameters.end(); ++iterCodecParams) {
        auto &codecParam = (*iterCodecParams);

        auto supportedType = IRTPTypes::toSupportedCodec(codecParam.mName);

        const RTPCodecCapability *foundSecondaryExact = nullptr;
        const RTPCodecCapability *foundSecondaryInexact = nullptr;

        bool isRTX = false;

        for (auto iterCodecCaps = codecCapabilities.begin(); iterCodecCaps != codecCapabilities.end(); ++iterCodecCaps) {
          auto &codecCap = (*iterCodecCaps);
          bool exact = true;

          // scope: find a remote matching codec
          {
            if (!isMatch(codecParam, codecCap, codecParameters, codecCapabilities, exact, isRTX)) goto not_a_match;
            goto match_found;
          }

        match_found:
          {
            if (exact) {
              foundSecondaryExact = &codecCap;
              break;
            }
            foundSecondaryInexact = &codecCap;
            continue;
          }

        not_a_match:
          {
            continue;
          }
        }

        if (foundSecondaryExact) {
          foundSecondaryInexact = foundSecondaryExact;
        } else {
          foundSecondaryExact = foundSecondaryInexact;
        }
        if (!foundSecondaryExact) continue;

        IRTPTypes::CodecParameters temp(codecParam);
        temp.mRTCPFeedback = *createUnion(codecParam.mRTCPFeedback, foundSecondaryExact->mRTCPFeedback, IDPreference_Local);
        result->push_back(temp);
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IHelper::RTPHeaderExtensionParametersListPtr IHelper::filterParameters(
                                                                           const RTPHeaderExtensionParametersList &headerParameters,
                                                                           const RTPHeaderExtensionsList &headerCapabilities
                                                                           )
    {
      auto result = make_shared<RTPHeaderExtensionParametersList>();

      for (auto iterParams = headerParameters.begin(); iterParams != headerParameters.end(); ++iterParams) {
        auto &headerParam = (*iterParams);

        bool found = false;

        for (auto iterCaps = headerCapabilities.begin(); iterCaps != headerCapabilities.end(); ++iterCaps) {
          auto &headerCap = (*iterCaps);

          if (0 != headerParam.mURI.compareNoCase(headerCap.mURI)) continue;
          found = true;
          break;
        }
        if (!found) continue;

        result->push_back(headerParam);
      }

      return result;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Negotiation (validation)
    #pragma mark

    //-------------------------------------------------------------------------
    bool IHelper::isCompatible(
                               const RTPCapabilities &capabilities,
                               const RTPParameters &parameters
                               )
    {
      for (auto iterOuter = parameters.mCodecs.begin(); iterOuter != parameters.mCodecs.end(); ++iterOuter) {
        auto &codecOuter = *(iterOuter);

        bool foundMatch = false;

        for (auto iterInner = capabilities.mCodecs.begin(); iterInner != capabilities.mCodecs.end(); ++iterInner) {
          auto &codecInner = *(iterInner);

          bool exact = false;
          bool isRTX = false;

          if (codecOuter.mPayloadType != codecInner.mPreferredPayloadType) continue;
          if (!isMatch(codecOuter, codecInner, parameters.mCodecs, capabilities.mCodecs, exact, isRTX)) return false;

          foundMatch = true;
          break;
        }
        if (!foundMatch) return false;
      }

      for (auto iterOuter = parameters.mHeaderExtensions.begin(); iterOuter != parameters.mHeaderExtensions.end(); ++iterOuter) {
        auto &extOuter = (*iterOuter);

        bool foundMatch = false;

        for (auto iterInner = capabilities.mHeaderExtensions.begin(); iterInner != capabilities.mHeaderExtensions.end(); ++iterInner) {
          auto &extInner = *(iterInner);

          if (extOuter.mID != extInner.mPreferredID) continue;
          if (0 != extOuter.mURI.compareNoCase(extInner.mURI)) continue;

          foundMatch = true;
          break;
        }
        if (!foundMatch) return false;
      }

      for (auto iter = parameters.mEncodings.begin(); iter != parameters.mEncodings.end(); ++iter) {
        auto &encoding = (*iter);
        if (!encoding.mCodecPayloadType.hasValue()) continue;

        if (!findCodecByPayloadType(capabilities.mCodecs, encoding.mCodecPayloadType.value())) return false;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool IHelper::isCompatible(
                               const RTPCapabilities &existingCapabilities,
                               const RTPCapabilities &proposedCapabilities
                               )
    {
      for (auto iterOuter = proposedCapabilities.mCodecs.begin(); iterOuter != proposedCapabilities.mCodecs.end(); ++iterOuter) {
        auto &codecOuter = *(iterOuter);

        bool foundMatch = false;

        for (auto iterInner = existingCapabilities.mCodecs.begin(); iterInner != existingCapabilities.mCodecs.end(); ++iterInner) {
          auto &codecInner = *(iterInner);

          bool exact = false;
          bool isRTX = false;

          if (codecOuter.mPreferredPayloadType != codecInner.mPreferredPayloadType) continue;
          if (!isMatch(codecOuter, codecInner, proposedCapabilities.mCodecs, existingCapabilities.mCodecs, exact, isRTX)) return false;

          foundMatch = true;
          break;
        }
        if (!foundMatch) return false;
      }

      for (auto iterOuter = existingCapabilities.mHeaderExtensions.begin(); iterOuter != existingCapabilities.mHeaderExtensions.end(); ++iterOuter) {
        auto &extOuter = (*iterOuter);

        bool foundMatch = false;

        for (auto iterInner = proposedCapabilities.mHeaderExtensions.begin(); iterInner != proposedCapabilities.mHeaderExtensions.end(); ++iterInner) {
          auto &extInner = *(iterInner);

          if (extOuter.mPreferredID != extInner.mPreferredID) continue;
          if (0 != extOuter.mURI.compareNoCase(extInner.mURI)) return false;

          foundMatch = true;
          break;
        }
        if (!foundMatch) return false;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    static bool isMediaCodec(const String &codecName)
    {
      auto supportedCodec = IRTPTypes::toSupportedCodec(codecName);
      auto codecKind = IRTPTypes::getCodecKind(supportedCodec);

      switch (codecKind) {
        case IRTPTypes::CodecKind_Audio:
        case IRTPTypes::CodecKind_Video:
        case IRTPTypes::CodecKind_AV:                 return true;

        case IRTPTypes::CodecKind_Unknown:
        case IRTPTypes::CodecKind_RTX:
        case IRTPTypes::CodecKind_FEC:
        case IRTPTypes::CodecKind_AudioSupplemental: 
        case IRTPTypes::CodecKind_Data:               break;
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool IHelper::hasSupportedMediaCodec(const RTPCapabilities &capabilities)
    {
      for (auto iterOuter = capabilities.mCodecs.begin(); iterOuter != capabilities.mCodecs.end(); ++iterOuter) {
        auto &codec = *(iterOuter);
        if (isMediaCodec(codec.mName)) return true;
      }
      return false;
    }

    //-------------------------------------------------------------------------
    bool IHelper::hasSupportedMediaCodec(const RTPParameters &parameters)
    {
      for (auto iterOuter = parameters.mCodecs.begin(); iterOuter != parameters.mCodecs.end(); ++iterOuter) {
        auto &codec = *(iterOuter);
        if (isMediaCodec(codec.mName)) return true;
      }
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Negotiation (fill)
    #pragma mark

    //-------------------------------------------------------------------------
    void IHelper::fillParameters(
                                 RTPParameters &parameters,
                                 const RTPCapabilities &capabilities,
                                 FillParametersOptions *options
                                 )
    {
      if (!parameters.mRTCP.mCName.hasData()) {
        parameters.mRTCP.mCName = string(zsLib::createUUID());
        parameters.mRTCP.mCName.replaceAll("-","");
        parameters.mRTCP.mReducedSize = true;
        parameters.mRTCP.mMux = true;
      }

      if (parameters.mEncodings.size() < 1) {
        IRTPTypes::EncodingParameters encoding;
        encoding.mActive = true;
        
        SSRCType nextSSRC {};
        if (options) {
          auto kind = UseRTPTypesHelper::getCodecsKind(parameters);
          if (kind.hasValue()) {
            nextSSRC = getNextSSRC(kind.value(), *(options->mAudioSSRCQueue), *(options->mVideoSSRCQueue));
          }
        }

        if (0 == nextSSRC) {
          nextSSRC = getRandomSSRC();
        }
        encoding.mSSRC = nextSSRC;

        for (auto iter = parameters.mCodecs.begin(); iter != parameters.mCodecs.end(); ++iter) {
          auto supportedCodec = IRTPTypes::toSupportedCodec((*iter).mName);
          auto supportedKind = IRTPTypes::getCodecKind(supportedCodec);

          switch (supportedKind) {
            case IRTPTypes::CodecKind_Unknown:

            case IRTPTypes::CodecKind_Audio:
            case IRTPTypes::CodecKind_Video:
            case IRTPTypes::CodecKind_AV:
            case IRTPTypes::CodecKind_AudioSupplemental:
            case IRTPTypes::CodecKind_Data:                 break;

            case IRTPTypes::CodecKind_RTX: {
              encoding.mRTX = IRTPTypes::RTXParameters();
              encoding.mRTX.value().mSSRC = getRandomSSRC();
              break;
            }
            case IRTPTypes::CodecKind_FEC: {
              if (IRTPTypes::SupportedCodec_ULPFEC == supportedCodec) {
                for (auto iterFEC = capabilities.mFECMechanisms.begin(); iterFEC != capabilities.mFECMechanisms.end(); ++iterFEC) {
                  auto &mechanism = (*iterFEC);
                  if (IRTPTypes::KnownFECMechanism_RED_ULPFEC != IRTPTypes::toKnownFECMechanism(mechanism)) continue;
                  encoding.mFEC = IRTPTypes::FECParameters();
                  encoding.mFEC.value().mMechanism = mechanism;
                  encoding.mFEC.value().mSSRC = getRandomSSRC();
                  break;
                }
              }
            }
          }
        }
        parameters.mEncodings.push_back(encoding);
      }
    }

  } // namespace adapter
} // namespace ortc
