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

//#include <openpeer/services/IHelper.h>
//
//#include <zsLib/Log.h>
//#include <zsLib/Numeric.h>
//#include <zsLib/Stringize.h>
//#include <zsLib/XML.h>


namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortclib_adapter) } }

namespace ortc
{
  namespace adapter
  {
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

  } // namespace adapter
} // namespace ortc
