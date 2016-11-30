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

#pragma once

#include <ortc/adapter/types.h>

#include <ortc/IHelper.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IMediaStreamTrack.h>

#include <queue>

namespace ortc
{
  namespace adapter
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IHelper
    #pragma mark

    interaction IHelper : public ortc::IHelper
    {
      typedef IRTPTypes::SSRCType SSRCType;
      typedef std::queue<SSRCType> SSRCQueue;

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Capabilities, RTPCapabilities);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, RTPParameters);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::CodecCapability, RTPCodecCapability);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::CodecCapabilitiesList, RTPCodecCapabilitiesList);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::CodecParameters, RTPCodecParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::CodecParametersList, RTPCodecParametersList);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::OpusCodecCapabilityParameters, RTPOpusCodecCapabilityParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::VP8CodecCapabilityParameters, RTPVP8CodecCapabilityParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::H264CodecCapabilityParameters, RTPH264CodecCapabilityParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::RTXCodecCapabilityParameters, RTPRTXCodecCapabilityParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::FlexFECCodecCapabilityParameters, RTPFlexFECCodecCapabilityParameters);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::OpusCodecParameters, RTPOpusCodecParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::VP8CodecParameters, RTPVP8CodecParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::H264CodecParameters, RTPH264CodecParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::RTXCodecParameters, RTPRTXCodecParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::REDCodecParameters, RTPREDCodecParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::FlexFECCodecParameters, RTPFlexFECCodecParameters);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtensionsList, RTPHeaderExtensionsList);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtensionParametersList, RTPHeaderExtensionParametersList);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtension, RTPHeaderExtension);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtensionParameters, RTPHeaderExtensionParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtensionsList, RTPHeaderExtensionsList);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtensionParametersList, RTPHeaderExtensionParametersList);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::FECMechanism, RTPFECMechanism);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::FECMechanismList, RTPFECMechanismList);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::RTCPFeedback, RTPRTCPFeedback);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::RTCPFeedbackList, RTPRTCPFeedbackList);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::CodecParametersList, RTPCodecParametersList);

      typedef IRTPTypes::CodecKinds RTPCodecKinds;

      enum IDPreferences
      {
        IDPreference_First,

        IDPreference_Local = IDPreference_First,
        IDPreference_Remote,

        IDPreference_Last = IDPreference_Remote,
      };

      static const char *toString(IDPreferences pref);
      static bool useLocal(IDPreferences pref) { return pref == IDPreference_Local; }
      static bool useRemote(IDPreferences pref) { return pref == IDPreference_Remote; }
      static IRTPTypes::SSRCType getRandomSSRC();

      static SSRCType peekNextSSRC(
                                   IMediaStreamTrackTypes::Kinds kind,
                                   SSRCQueue &audioSSRCQueue,
                                   SSRCQueue &videoSSRCQueue
                                   );
      static SSRCType getNextSSRC(
                                  IMediaStreamTrackTypes::Kinds kind,
                                  SSRCQueue &audioSSRCQueue,
                                  SSRCQueue &videoSSRCQueue
                                  );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Negotiation (conversion)
      #pragma mark

      static RTPParametersPtr capabilitiesToParameters(const RTPCapabilities &capabilities);
      static RTPCapabilitiesPtr parametersToCapabilities(const RTPParameters &parameters);

      static RTPCodecParametersListPtr capabilitiesToParameters(const RTPCodecCapabilitiesList &capabiliites);
      static RTPCodecCapabilitiesListPtr parametersToCapabilities(const RTPCodecParametersList &parameters);

      static RTPCodecParametersPtr capabilitiesToParameters(const RTPCodecCapability &capabilities);
      static RTPCodecCapabilityPtr parametersToCapabilities(const RTPCodecParameters &parameters);

      static RTPOpusCodecParametersPtr opusCodecCapabilityParametersToParameters(const RTPOpusCodecCapabilityParameters &capabilityParameters);
      static RTPOpusCodecCapabilityParametersPtr opusCodecParametersToCapabilityParameters(const RTPOpusCodecParameters &parameters);

      static RTPVP8CodecParametersPtr vp8CodecCapabilityParametersToParameters(const RTPVP8CodecCapabilityParameters &capabilityParameters);
      static RTPVP8CodecCapabilityParametersPtr vp8CodecParametersToCapabilityParameters(const RTPVP8CodecParameters &parameters);

      static RTPH264CodecParametersPtr h264CodecCapabilityParametersToParameters(const RTPH264CodecCapabilityParameters &capabilityParameters);
      static RTPH264CodecCapabilityParametersPtr h264CodecParametersToCapabilityParameters(const RTPH264CodecParameters &parameters);

      static RTPRTXCodecParametersPtr rtxCodecCapabilityParametersToParameters(const RTPRTXCodecCapabilityParameters &capabilityParameters);
      static RTPRTXCodecCapabilityParametersPtr rtxCodecParametersToCapabilityParameters(const RTPRTXCodecParameters &parameters);

      static RTPFlexFECCodecParametersPtr flexFECCodecCapabilityParametersToParameters(const RTPFlexFECCodecCapabilityParameters &capabilityParameters);
      static RTPFlexFECCodecCapabilityParametersPtr flexFECCodecParametersToCapabilityParameters(const RTPFlexFECCodecParameters &parameters);

      static RTPHeaderExtensionParametersListPtr capabilitiesToParameters(const RTPHeaderExtensionsList &capabilities);
      static RTPHeaderExtensionsListPtr parametersToCapabilities(const RTPHeaderExtensionParametersList &parameters);

      static RTPHeaderExtensionParametersPtr capabilitiesToParameters(const RTPHeaderExtension &capabilities);
      static RTPHeaderExtensionPtr parametersToCapabilities(const RTPHeaderExtensionParameters &parameters);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Negotiation (unions)
      #pragma mark

      static RTPCapabilitiesPtr createUnion(
                                            const RTPCapabilities &localCapabilities,
                                            const RTPCapabilities &remoteCapabilities,
                                            IDPreferences preference
                                            );

      static RTPCodecCapabilitiesListPtr createUnion(
                                                     const RTPCodecCapabilitiesList &local,
                                                     const RTPCodecCapabilitiesList &remote,
                                                     IDPreferences preference
                                                     );

      static RTPHeaderExtensionsListPtr createUnion(
                                                    const RTPHeaderExtensionsList &local,
                                                    const RTPHeaderExtensionsList &remote,
                                                    IDPreferences preference
                                                    );

      static RTPFECMechanismListPtr createUnion(
                                                const RTPFECMechanismList &local,
                                                const RTPFECMechanismList &remote,
                                                IDPreferences preference
                                                );

      static RTPRTCPFeedbackListPtr createUnion(
                                                const RTPRTCPFeedbackList &local,
                                                const RTPRTCPFeedbackList &remote,
                                                IDPreferences preference
                                                );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Negotiation (filter)
      #pragma mark

      static RTPParametersPtr filterParameters(
                                               const RTPParameters &parameters,
                                               const RTPCapabilities &capabilities
                                               );

      static RTPCodecParametersListPtr filterParameters(
                                                        const RTPCodecParametersList &codecParameters,
                                                        const RTPCodecCapabilitiesList &codecCapabilities
                                                        );

      static RTPHeaderExtensionParametersListPtr filterParameters(
                                                                  const RTPHeaderExtensionParametersList &headerParameters,
                                                                  const RTPHeaderExtensionsList &headerCapabilities
                                                                  );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Negotiation (validation)
      #pragma mark

      static bool isCompatible(
                               const RTPCapabilities &capabilities,
                               const RTPParameters &parameters
                               );

      static bool isCompatible(
                               const RTPCapabilities &existingCapabilities,
                               const RTPCapabilities &proposedCapabilities
                               );

      static bool hasSupportedMediaCodec(const RTPCapabilities &capabilities);
      static bool hasSupportedMediaCodec(const RTPParameters &parameters);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Negotiation (fill)
      #pragma mark

      struct FillParametersOptions
      {
        FillParametersOptions(
                              SSRCQueue &audioQueue,
                              SSRCQueue &videoQueue
                              ) : mAudioSSRCQueue(&audioQueue), mVideoSSRCQueue(&videoQueue) {}

        SSRCQueue *mAudioSSRCQueue {};
        SSRCQueue *mVideoSSRCQueue {};
      };

      static void fillParameters(
                                 RTPParameters &ioParameters,
                                 const RTPCapabilities &capabilities,
                                 FillParametersOptions *options = NULL
                                 );
    };
  }
}
