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

#include <ortc/internal/types.h>

#include <ortc/IMediaStreamTrack.h>
#include <ortc/IRTPTypes.h>

#define ORTC_INTERNAL_RTPTYPESHELPER_MAX_CODEC_DEPTH (4)

namespace ortc
{
  namespace internal
  {
    class RTPTypesHelper
    {
    public:
      typedef IRTPTypes::PayloadType PayloadType;

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::CodecParameters, CodecParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::EncodingParameters, EncodingParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(std::list<ParametersPtr>, ParametersPtrList);

      ZS_DECLARE_TYPEDEF_PTR(Parameters, OldParameters);
      ZS_DECLARE_TYPEDEF_PTR(Parameters, NewParameters);

      typedef std::pair<OldParametersPtr, NewParametersPtr> OldNewParametersPair;

      ZS_DECLARE_TYPEDEF_PTR(std::list<OldNewParametersPair>, ParametersPtrPairList);

      struct FindCodecOptions
      {
        typedef std::set<PayloadType> PayloadTypeSet;

        Optional<PayloadType> mPayloadType;
        Optional<IMediaStreamTrackTypes::Kinds> mKind;
        Optional<bool> mAllowNeutralKind;
        Optional<IRTPTypes::CodecKinds> mCodecKind;
        Optional<IRTPTypes::SupportedCodecs> mSupportedCodec;
        Optional<ULONG> mClockRate;
        Optional<bool> mMatchClockRateNotSet;
        PayloadTypeSet mDisallowedPayloadtypeMatches;
        Optional<PayloadType> mRTXAptPayloadType;
        Optional<PayloadTypeSet> mREDCodecPayloadTypes;
        Optional<bool> mAllowREDMatchEmptyList;

        Optional<bool> mDisallowMultipleMatches;

        ElementPtr toDebug() const noexcept;
      };

      struct DecodedCodecInfo
      {
        struct DepthInfo
        {
          const CodecParameters *mCodecParameters {};
          IRTPTypes::SupportedCodecs mSupportedCodec {IRTPTypes::SupportedCodec_Unknown};
          IRTPTypes::CodecKinds mCodecKind {IRTPTypes::CodecKind_Unknown};

          DepthInfo() noexcept {}
          DepthInfo(const DepthInfo &info) noexcept {(*this) = info;}
          ElementPtr toDebug() const noexcept;
        };

        size_t mFilledDepth {};
        DepthInfo mDepth[ORTC_INTERNAL_RTPTYPESHELPER_MAX_CODEC_DEPTH];

        DecodedCodecInfo() noexcept {}
        DecodedCodecInfo(const DecodedCodecInfo &info) noexcept {(*this) = info;}
        ElementPtr toDebug() const noexcept;
      };
      

      static void validateCodecParameters(
                                          const Parameters &params,
                                          Optional<IMediaStreamTrackTypes::Kinds> &ioKind
                                          ) noexcept(false); // throws InvalidParameters

      static void splitParamsIntoChannels(
                                          const Parameters &params,
                                          ParametersPtrList &outParamsGroupedIntoChannels
                                          ) noexcept(false);

      static void calculateDeltaChangesInChannels(
                                                  Optional<IMediaStreamTrackTypes::Kinds> kind,
                                                  const ParametersPtrList &inExistingParamsGroupedIntoChannels,
                                                  const ParametersPtrList &inNewParamsGroupedIntoChannels,
                                                  ParametersPtrPairList &outUnchangedChannels,
                                                  ParametersPtrList &outNewChannels,
                                                  ParametersPtrPairList &outUpdatedChannels,
                                                  ParametersPtrList &outRemovedChannels
                                                  ) noexcept;

      static bool isGeneralizedSSRCCompatibleChange(
                                                    const Parameters &oldParams,
                                                    const Parameters &newParams
                                                    ) noexcept;

      static bool isCompatibleCodec(
                                    const CodecParameters &oldCodec,
                                    const CodecParameters &newCodec,
                                    float &ioRank
                                    ) noexcept;

      static const CodecParameters *findCodec(
                                              const Parameters &params,
                                              const FindCodecOptions &options
                                              ) noexcept;

      static const CodecParameters *pickCodec(
                                              Optional<IMediaStreamTrackTypes::Kinds> kind,
                                              const Parameters &params,
                                              Optional<PayloadType> packetPayloadType = Optional<PayloadType>(),
                                              const EncodingParameters *encoding = NULL,
                                              const EncodingParameters *baseEncoding = NULL
                                              ) noexcept;

      static bool isRankableMatch(
                                  Optional<IMediaStreamTrackTypes::Kinds> kind,
                                  const Parameters &oldChannelParams,
                                  const Parameters &newChannelParams,
                                  float &outRank
                                  ) noexcept;

      static EncodingParameters *findEncodingBase(
                                                  Parameters &inParams,
                                                  EncodingParameters *inEncoding
                                                  ) noexcept;

      static EncodingParameters *pickEncodingToFill(
                                                    Optional<IMediaStreamTrackTypes::Kinds> kind,
                                                    const RTPPacket &packet,
                                                    Parameters &filledParams,
                                                    const DecodedCodecInfo &decodedCodec,
                                                    EncodingParameters * &outBaseEncoding
                                                    ) noexcept;

      static Optional<IMediaStreamTrackTypes::Kinds> getCodecsKind(const Parameters &params) noexcept;


      static bool decodePacketCodecs(
                                     Optional<IMediaStreamTrackTypes::Kinds> &ioKind,
                                     const RTPPacket &packet,
                                     const Parameters &params,
                                     DecodedCodecInfo &decodedCodecInfo
                                     ) noexcept;

      static void getRTXCodecPayload(
                                     const BYTE *packetPayload,
                                     size_t packetPayloadSizeInBytes,
                                     const BYTE * &outInnterPayload,
                                     size_t &outInnerPayloadSizeBytes
                                     ) noexcept;

      static Optional<PayloadType> getRedCodecPayload(
                                                      const BYTE *packetPayload,
                                                      size_t packetPayloadSizeInBytes,
                                                      const BYTE * &outInnterPayload,
                                                      size_t &outInnerPayloadSizeBytes
                                                      ) noexcept;

      static Optional<PayloadType> getFecRecoveryPayloadType(
                                                             const BYTE *packetPayload,
                                                             size_t packetPayloadSizeInBytes
                                                             ) noexcept;

      static Log::Params slog(const char *message) noexcept;
    };
  }
}
