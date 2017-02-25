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

#include <ortc/internal/types.h>
#include <ortc/internal/platform.h>
#include <ortc/internal/ortc_RTPTypes.h>
#include <ortc/internal/ortc_RTPUtils.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_Helper.h>

#include <ortc/IRTPTypes.h>
#include <ortc/IHelper.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>
#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>
#include <ortc/types.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

  using zsLib::Numeric;
  using zsLib::Log;

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    const float kExactMatchRankAmount = 1000000.0;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPTypesHelper::FindCodecOptions
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr RTPTypesHelper::FindCodecOptions::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPTypesHelper::FindCodecOptions");

      IHelper::debugAppend(resultEl, "payload type", mPayloadType);
      IHelper::debugAppend(resultEl, "kind", mKind);
      IHelper::debugAppend(resultEl, "allow neutral kind", mAllowNeutralKind);
      IHelper::debugAppend(resultEl, "codec kind", mCodecKind.hasValue() ? IRTPTypes::toString(mCodecKind.value()) : (const char *)NULL);
      IHelper::debugAppend(resultEl, "supported codec", mSupportedCodec.hasValue() ? IRTPTypes::toString(mSupportedCodec.value()) : (const char *)NULL);
      IHelper::debugAppend(resultEl, "clock rate", mClockRate);
      IHelper::debugAppend(resultEl, "match clock rate not set", mMatchClockRateNotSet);
      IHelper::debugAppend(resultEl, "disallowed matches", mDisallowedPayloadtypeMatches.size());

      if (mDisallowedPayloadtypeMatches.size() > 0) {
        ElementPtr disallowEl = Element::create("disallowed");
        for (auto iter = mDisallowedPayloadtypeMatches.begin(); iter != mDisallowedPayloadtypeMatches.end(); ++iter) {
          IHelper::debugAppend(resultEl, "disallowed payload", *iter);
        }
        IHelper::debugAppend(resultEl, disallowEl);
      }
      IHelper::debugAppend(resultEl, "rtx apt payload", mRTXAptPayloadType);
      IHelper::debugAppend(resultEl, "disallow multiple matches", mDisallowMultipleMatches);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPTypesHelper::DecodedCodecInfo
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr RTPTypesHelper::DecodedCodecInfo::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPTypesHelper::DecodedCodecInfo");

      ElementPtr depthInfosEl = Element::create("depthInfos");

      for (size_t index = 0; index < ORTC_INTERNAL_RTPTYPESHELPER_MAX_CODEC_DEPTH; ++index)
      {
        auto &depthInfo = mDepth[index];

        IHelper::debugAppend(depthInfosEl, depthInfo.toDebug());
      }

      if (depthInfosEl->hasChildren()) {
        IHelper::debugAppend(depthInfosEl, depthInfosEl);
      }

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPTypesHelper::DecodedCodecInfo::DepthInfo
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr RTPTypesHelper::DecodedCodecInfo::DepthInfo::toDebug() const
    {
      if (NULL == mCodecParameters) return ElementPtr();

      ElementPtr resultEl = Element::create("ortc::RTPTypesHelper::DecodedCodecInfo:DepthInfo");
      IHelper::debugAppend(resultEl, "codec", mCodecParameters->toDebug());
      IHelper::debugAppend(resultEl, "supported codec", IRTPTypes::toString(mSupportedCodec));
      IHelper::debugAppend(resultEl, "codec kind", IRTPTypes::toString(mCodecKind));
      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPTypesHelper
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPTypesHelper::validateCodecParameters(
                                                 const Parameters &params,
                                                 Optional<IMediaStreamTrackTypes::Kinds> &ioKind
                                                 ) throw (InvalidParameters)
    {
      for (auto iter = params.mCodecs.begin(); iter != params.mCodecs.end(); ++iter)
      {
        auto &codecInfo = (*iter);

        IRTPTypes::SupportedCodecs supported = IRTPTypes::toSupportedCodec(codecInfo.mName);
        IRTPTypes::CodecKinds codecKind = IRTPTypes::getCodecKind(supported);

        ORTC_THROW_INVALID_PARAMETERS_IF((IRTPTypes::SupportedCodec_Unknown == supported) ||
                                         (IRTPTypes::CodecKind_Unknown == codecKind))

        switch (codecKind) {
          case IRTPTypes::CodecKind_Unknown: break;
          case IRTPTypes::CodecKind_Audio:
          case IRTPTypes::CodecKind_AudioSupplemental:
          {
            if (ioKind.hasValue()) {
              ORTC_THROW_INVALID_PARAMETERS_IF(IMediaStreamTrackTypes::Kind_Audio != ioKind.value())
            }
            ioKind = IMediaStreamTrackTypes::Kind_Audio;
            break;
          }
          case IRTPTypes::CodecKind_Video:
          {
            if (ioKind.hasValue()) {
              ORTC_THROW_INVALID_PARAMETERS_IF(IMediaStreamTrackTypes::Kind_Video != ioKind.value())
            }
            ioKind = IMediaStreamTrackTypes::Kind_Video;
            break;
          }
          case IRTPTypes::CodecKind_AV:
          {
            break;
          }
          case IRTPTypes::CodecKind_RTX:
          {
            IRTPTypes::RTXCodecParametersPtr rtxParams = IRTPTypes::RTXCodecParameters::convert(codecInfo.mParameters);
            ORTC_THROW_INVALID_PARAMETERS_IF(!rtxParams)

            bool foundAptCodec = false;

            for (auto iterApt = params.mCodecs.begin(); iterApt != params.mCodecs.end(); ++iterApt)
            {
              auto &aptCodecInfo = (*iterApt);
              if (aptCodecInfo.mPayloadType != rtxParams->mApt) continue;

              foundAptCodec = true;

              IRTPTypes::SupportedCodecs aptSupported = IRTPTypes::toSupportedCodec(aptCodecInfo.mName);
              IRTPTypes::CodecKinds aptCodecKind = IRTPTypes::getCodecKind(aptSupported);

              ORTC_THROW_INVALID_PARAMETERS_IF((IRTPTypes::SupportedCodec_Unknown == aptSupported) ||
                                               (IRTPTypes::CodecKind_Unknown == aptCodecKind))

              ORTC_THROW_INVALID_PARAMETERS_IF(IRTPTypes::CodecKind_RTX == aptCodecKind)

              if (codecInfo.mClockRate.hasValue()) {
                if (aptCodecInfo.mClockRate.hasValue()) {
                  ORTC_THROW_INVALID_PARAMETERS_IF(codecInfo.mClockRate.value() != aptCodecInfo.mClockRate.value())
                }
              }
              break;
            }
            ORTC_THROW_INVALID_PARAMETERS_IF(!foundAptCodec)
            break;
          }
          case IRTPTypes::CodecKind_Data: break;
          case IRTPTypes::CodecKind_FEC:
          {
            switch (supported) {
              case IRTPTypes::SupportedCodec_RED:
              {
                IRTPTypes::REDCodecParametersPtr redParams = IRTPTypes::REDCodecParameters::convert(codecInfo.mParameters);

                if (redParams) {
                  for (auto iterRedPayloads = redParams->mPayloadTypes.begin(); iterRedPayloads != redParams->mPayloadTypes.end(); ++iterRedPayloads)
                  {
                    auto &payloadType = (*iterRedPayloads);

                    bool foundRedPayload = false;
                    for (auto redIter = params.mCodecs.begin(); redIter != params.mCodecs.end(); ++redIter)
                    {
                      auto &codecRed = (*redIter);
                      if (payloadType != codecRed.mPayloadType) continue;

                      IRTPTypes::SupportedCodecs redSupported = IRTPTypes::toSupportedCodec(codecRed.mName);
                      ORTC_THROW_INVALID_PARAMETERS_IF((IRTPTypes::SupportedCodec_Unknown == redSupported) ||
                                                       (IRTPTypes::SupportedCodec_RED == redSupported) ||
                                                       (IRTPTypes::SupportedCodec_RTX == redSupported))

                      if (codecInfo.mClockRate.hasValue()) {
                        if (codecRed.mClockRate.hasValue()) {
                          ORTC_THROW_INVALID_PARAMETERS_IF(codecInfo.mClockRate.value() != codecRed.mClockRate.value())
                        }
                      }
                      foundRedPayload = true;
                    }

                    ORTC_THROW_INVALID_PARAMETERS_IF(!foundRedPayload)
                  }
                }
                break;
              }
              case IRTPTypes::SupportedCodec_ULPFEC:
              {
                break;
              }
              case IRTPTypes::SupportedCodec_FlexFEC:
              {
                IRTPTypes::FlexFECCodecParametersPtr flexFECParams = IRTPTypes::FlexFECCodecParameters::convert(codecInfo.mParameters);
                break;
              }
              default:
              {
                ORTC_THROW_INVALID_PARAMETERS(String("FEC codec type is not understood: ") + IRTPTypes::toString(supported))
                break;
              }
            }
          }
        }
      }

      for (auto iter = params.mEncodings.begin(); iter != params.mEncodings.end(); ++iter)
      {
        auto &encoding = (*iter);

        bool supportMRST = false;

        Optional<PayloadType> payload = encoding.mCodecPayloadType;
        Optional<ULONG> clockRate;

        if (encoding.mCodecPayloadType.hasValue())
        {
          FindCodecOptions options;
          options.mPayloadType = encoding.mCodecPayloadType.value();
          auto foundCodec = findCodec(params, options);
          ORTC_THROW_INVALID_PARAMETERS_IF(NULL == foundCodec)

          clockRate = foundCodec->mClockRate;

          auto supportedCodec = IRTPTypes::toSupportedCodec(foundCodec->mName);
          auto codecKind = IRTPTypes::getCodecKind(supportedCodec);

          supportMRST = IRTPTypes::isMRSTCodec(supportedCodec);

          switch (codecKind) {
            case IRTPTypes::CodecKind_Unknown:            ORTC_THROW_INVALID_PARAMETERS("illegal codec")
            case IRTPTypes::CodecKind_Audio:
            case IRTPTypes::CodecKind_AudioSupplemental:
            case IRTPTypes::CodecKind_Video:
            case IRTPTypes::CodecKind_AV:                 break;
            case IRTPTypes::CodecKind_Data:
            case IRTPTypes::CodecKind_RTX:                ORTC_THROW_INVALID_PARAMETERS("RTX codec cannot be the main encoding payload type, enabled RTX parameters instead")
            case IRTPTypes::CodecKind_FEC:                ORTC_THROW_INVALID_PARAMETERS("FEC codec cannot be the main encoding payload type, enable FEC parameters instead")
          }
        }

        auto baseEncoding = findEncodingBase(const_cast<Parameters &>(params), &(const_cast<EncodingParameters &>(encoding)));
        ORTC_THROW_INVALID_PARAMETERS_IF(NULL == baseEncoding)
        if (baseEncoding != &(encoding))
        {
          payload = baseEncoding->mCodecPayloadType;

          if (baseEncoding->mCodecPayloadType.hasValue()) {
            if (encoding.mCodecPayloadType.hasValue()) {
              // cannot change codecs from base to dependency
              ORTC_THROW_INVALID_PARAMETERS_IF(baseEncoding->mCodecPayloadType.value() != encoding.mCodecPayloadType.value())
            }
            FindCodecOptions options;
            options.mPayloadType = baseEncoding->mCodecPayloadType.value();

            auto foundBaseCodec = findCodec(params, options);
            ORTC_THROW_INVALID_PARAMETERS_IF(NULL == foundBaseCodec)

            auto supportedCodec = IRTPTypes::toSupportedCodec(foundBaseCodec->mName);
            supportMRST = IRTPTypes::isMRSTCodec(supportedCodec);

            clockRate = foundBaseCodec->mClockRate;
          } else {
            // cannot have a codec on dependency but then have none on the base
            ORTC_THROW_INVALID_PARAMETERS_IF(encoding.mCodecPayloadType.hasValue())
          }

          if (encoding.mSSRC.hasValue())
          {
            // cannot have an SSRC on dependency if base doesn't have an SSRC
            ORTC_THROW_INVALID_PARAMETERS_IF(!baseEncoding->mSSRC.hasValue())

            if (encoding.mSSRC.value() != baseEncoding->mSSRC.value())
            {
              // only can change SSRC if an MRST codec
              ORTC_THROW_INVALID_PARAMETERS_IF(!supportMRST)
            }
          }
        }

        if (encoding.mRTX.hasValue())
        {
          FindCodecOptions options;
          options.mCodecKind = IRTPTypes::CodecKind_RTX;
          options.mRTXAptPayloadType = payload;
          options.mClockRate = clockRate;
          if (options.mClockRate.hasValue()) {
            options.mMatchClockRateNotSet = true;
          }

          auto foundCodec = RTPTypesHelper::findCodec(params, options);
          ORTC_THROW_INVALID_PARAMETERS_IF(NULL == foundCodec)
        }

        if (encoding.mFEC.hasValue())
        {
          auto mechanism = IRTPTypes::toKnownFECMechanism(encoding.mFEC.value().mMechanism);
          switch (mechanism) {
            case IRTPTypes::KnownFECMechanism_Unknown:    ORTC_THROW_INVALID_PARAMETERS("unknown FEC mechanism")
            case IRTPTypes::KnownFECMechanism_RED:        ORTC_THROW_INVALID_PARAMETERS("RED is not a supported FEC mechanism")
            case IRTPTypes::KnownFECMechanism_RED_ULPFEC:
            {
              {
                FindCodecOptions options;
                options.mSupportedCodec = IRTPTypes::SupportedCodec_RED;
                options.mClockRate = clockRate;
                if (options.mClockRate.hasValue()) {
                  options.mMatchClockRateNotSet = true;
                }
                options.mAllowREDMatchEmptyList = true;

                auto foundCodec = findCodec(params, options);
                ORTC_THROW_INVALID_PARAMETERS_IF(!foundCodec)
              }
              {
                FindCodecOptions options;
                options.mSupportedCodec = IRTPTypes::SupportedCodec_ULPFEC;
                options.mClockRate = clockRate;
                if (options.mClockRate.hasValue()) {
                  options.mMatchClockRateNotSet = true;
                }

                auto foundCodec = findCodec(params, options);
                ORTC_THROW_INVALID_PARAMETERS_IF(!foundCodec)
              }
              break;
            }
            case IRTPTypes::KnownFECMechanism_FlexFEC:
            {
              FindCodecOptions options;
              options.mSupportedCodec = IRTPTypes::SupportedCodec_FlexFEC;
              options.mClockRate = clockRate;
              options.mMatchClockRateNotSet = true;

              auto foundCodec = findCodec(params, options);
              ORTC_THROW_INVALID_PARAMETERS_IF(!foundCodec)
            }
          }
        }
      }

    }

    //-------------------------------------------------------------------------
    void RTPTypesHelper::splitParamsIntoChannels(
                                                const Parameters &params,
                                                ParametersPtrList &outParamsGroupedIntoChannels
                                                )
    {
      typedef IRTPTypes::EncodingID EncodingID;
      typedef std::map<EncodingID, ParametersPtr> StreamMap;

      StreamMap streamMap;

      for (auto iter = params.mEncodings.begin(); iter != params.mEncodings.end(); ++iter)
      {
        auto &encoding = (*iter);
        if (encoding.mDependencyEncodingIDs.size() > 0) continue; // skip all that are dependent on other layers

        ParametersPtr tmpParam(make_shared<Parameters>());
        tmpParam->mCodecs = params.mCodecs;
        tmpParam->mHeaderExtensions = params.mHeaderExtensions;
        tmpParam->mMuxID = params.mMuxID;
        tmpParam->mRTCP = params.mRTCP;
        tmpParam->mEncodings.push_back(encoding);

        outParamsGroupedIntoChannels.push_back(tmpParam);

        if (encoding.mEncodingID.hasData()) {
          streamMap[encoding.mEncodingID] = tmpParam;
        }
      }

      bool missingEntry = false;
      bool foundEntry = false;

      do {
        missingEntry = false;
        foundEntry = false;

        for (auto iter = params.mEncodings.begin(); iter != params.mEncodings.end(); ++iter) {
          auto &encoding = (*iter);

          if (encoding.mDependencyEncodingIDs.size() < 1) continue;         // skip all that do not have any dependencies

          ORTC_THROW_INVALID_PARAMETERS_IF(encoding.mEncodingID.isEmpty())  // if dependencies exist, this layer must have an encoding id

          auto foundExisting = streamMap.find(encoding.mEncodingID);
          if (foundExisting != streamMap.end()) continue;                   // already processed this entry

          for (auto iterDependency = encoding.mDependencyEncodingIDs.begin(); iterDependency != encoding.mDependencyEncodingIDs.end(); ++iterDependency) {
            auto &dependencyID = (*iterDependency);
            auto foundDependency = streamMap.find(dependencyID);
            if (foundDependency == streamMap.end()) continue;

            ParametersPtr existingParam = (*foundDependency).second;
            existingParam->mEncodings.push_back(encoding);

            streamMap[encoding.mEncodingID] = existingParam;
            foundEntry = true;
            goto next;
          }

          missingEntry = true;
          goto next;

        next: {}
        }

      } while ((missingEntry) &&
               (foundEntry));

      ORTC_THROW_INVALID_PARAMETERS_IF((missingEntry) &&
                                       (!foundEntry))

      if (outParamsGroupedIntoChannels.size() < 1) {
        // ensure at least one channel exists
        outParamsGroupedIntoChannels.push_back(make_shared<Parameters>(params));
      }
    }

    //-------------------------------------------------------------------------
    void RTPTypesHelper::calculateDeltaChangesInChannels(
                                                         Optional<IMediaStreamTrackTypes::Kinds> kind,
                                                         const ParametersPtrList &inExistingParamsGroupedIntoChannels,
                                                         const ParametersPtrList &inNewParamsGroupedIntoChannels,
                                                         ParametersPtrPairList &outUnchangedChannels,
                                                         ParametersPtrList &outNewChannels,
                                                         ParametersPtrPairList &outUpdatedChannels,
                                                         ParametersPtrList &outRemovedChannels
                                                         )
    {
      typedef String Hash;
      typedef std::pair<Hash, ParametersPtr> HashParameterPair;
      typedef std::list<HashParameterPair> HashParameterPairList;

      ZS_LOG_DEBUG(slog("calculating delate changes in channels") + ZS_PARAM("existing", inExistingParamsGroupedIntoChannels.size()) + ZS_PARAM("new", inNewParamsGroupedIntoChannels.size()))

      // scope: pre-screen special cases
      {
        if (inExistingParamsGroupedIntoChannels.size() < 1) {
          ZS_LOG_TRACE(slog("all channels are new"))
          outNewChannels = inNewParamsGroupedIntoChannels;
          return;
        }

        if (inNewParamsGroupedIntoChannels.size() < 1) {
          ZS_LOG_TRACE(slog("all channels are gone"))
          // special case where all are now "gone"
          outRemovedChannels = inExistingParamsGroupedIntoChannels;
          return;
        }
      }

      ParametersPtrList oldList(inExistingParamsGroupedIntoChannels);
      ParametersPtrList newList(inNewParamsGroupedIntoChannels);

      // scope: SSRC is not a factor thus check to see what is left is an "update" compatible change (or not)
      {
        if (newList.size() < 1) goto non_compatible_change;
        if (oldList.size() < 1) goto non_compatible_change;

        if (!isGeneralizedSSRCCompatibleChange(*(oldList.front()), *(newList.front()))) goto non_compatible_change;

        goto do_more_matching;

      non_compatible_change:
        {
          outNewChannels = newList;
          outRemovedChannels = oldList;

          ZS_LOG_TRACE(slog("no more compatible changes found") + ZS_PARAM("remove size", outRemovedChannels.size()) + ZS_PARAM("add size", outNewChannels.size()))
          return;
        }

      do_more_matching: {}
      }

      HashParameterPairList newHashedList;

      Parameters::HashOptions hashOptions;

      hashOptions.mHeaderExtensions = false;
      hashOptions.mRTCP = false;
      
      // scope: calculate hashes for new list
      {
        for (auto iter_doNotUse = newList.begin(); iter_doNotUse != newList.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto params = (*current);
          auto hash = params->hash(hashOptions);

          newHashedList.push_back(HashParameterPair(hash, params));
        }
      }

      // scope: find matches by encoding ID
      {
        for (auto iterOld_doNotUse = oldList.begin(); iterOld_doNotUse != oldList.end(); ) {
          auto currentOld = iterOld_doNotUse;
          ++iterOld_doNotUse;

          auto oldParams = (*currentOld);

          if (oldParams->mEncodings.size() < 1) continue;

          auto &oldEncodingBase = (*(oldParams->mEncodings.begin()));
          if (oldEncodingBase.mEncodingID.isEmpty()) continue;

          auto iterNew_doNotUse = newList.begin();
          auto iterNewHash_doNotUse = newHashedList.begin();

          for (; iterNew_doNotUse != newList.end();) {
            auto currentNew = iterNew_doNotUse;
            ++iterNew_doNotUse;

            auto currentNewHash = iterNewHash_doNotUse;
            ++iterNewHash_doNotUse;

            auto &newHash = (*currentNewHash).first;

            auto newParams = (*currentNew);

            if (newParams->mEncodings.size() < 1) continue;

            auto &newEncodingBase = (*(newParams->mEncodings.begin()));

            if (newEncodingBase.mEncodingID.isEmpty()) continue;

            if (oldEncodingBase.mEncodingID != newEncodingBase.mEncodingID) continue;

            auto oldHash = oldParams->hash(hashOptions);

            float rank {};
            if (oldParams->hash() == newParams->hash()) {
              // an exact match
              ZS_LOG_TRACE(slog("parameters are unchanged") + oldParams->toDebug())
              outUnchangedChannels.push_back(OldNewParametersPair(oldParams, newParams));
            } else {
              if (oldHash == newHash) {
                ZS_LOG_TRACE(slog("parameters are almost an exact match (but some options have changed)") + ZS_PARAM("old", oldParams->toDebug()) + ZS_PARAM("new", newParams->toDebug()))
                outUpdatedChannels.push_back(OldNewParametersPair(oldParams, newParams));
              } else if (isRankableMatch(kind, *oldParams, *newParams, rank)) {
                ZS_LOG_TRACE(slog("parameter changes are compatible (thus performing an update)") + ZS_PARAM("old", oldParams->toDebug()) + ZS_PARAM("new", newParams->toDebug()))
                outUpdatedChannels.push_back(OldNewParametersPair(oldParams, newParams));
              } else {
                ZS_LOG_TRACE(slog("parameters changes do not appear to be compatible (thus removing old channel and creating new channel)") + ZS_PARAM("old", oldParams->toDebug()) + ZS_PARAM("new", newParams->toDebug()))
                outRemovedChannels.push_back(oldParams);
                outNewChannels.push_back(newParams);
              }
            }

            oldList.erase(currentOld);
            newList.erase(currentNew);
            newHashedList.erase(currentNewHash);
            break;
          }
        }
      }

      // scope: old params with non-matching encoding ID entries must be removed
      {
        for (auto iterOld_doNotUse = oldList.begin(); iterOld_doNotUse != oldList.end(); ) {
          auto currentOld = iterOld_doNotUse;
          ++iterOld_doNotUse;

          auto oldParams = (*currentOld);

          if (oldParams->mEncodings.size() < 1) continue;

          auto &firstOld = oldParams->mEncodings.front();
          if (firstOld.mEncodingID.isEmpty()) continue;

          ZS_LOG_TRACE(slog("old parameters did not have an encoding ID match (thus must remove)") + oldParams->toDebug())
          outRemovedChannels.push_back(oldParams);

          oldList.erase(currentOld);
        }
      }

      // scope: new params with non-matching encoding ID entries must be added
      {
        auto iterNew_doNotUse = newList.begin();
        auto iterNewHash_doNotUse = newHashedList.begin();

        for (; iterNew_doNotUse != newList.end();) {
          auto currentNew = iterNew_doNotUse;
          ++iterNew_doNotUse;

          auto currentNewHash = iterNewHash_doNotUse;
          ++iterNewHash_doNotUse;

          auto newParams = (*currentNew);

          if (newParams->mEncodings.size() < 1) continue;

          auto &firstNew = newParams->mEncodings.front();
          if (firstNew.mEncodingID.isEmpty()) continue;

          ZS_LOG_TRACE(slog("new parameters did not have an encoding ID match (thus must add)") + newParams->toDebug())
          outNewChannels.push_back(newParams);
          
          newList.erase(currentNew);
          newHashedList.erase(currentNewHash);
        }
      }

      // scope: find exact matches
      {
        for (auto iterOld_doNotUse = oldList.begin(); iterOld_doNotUse != oldList.end(); ) {
          auto currentOld = iterOld_doNotUse;
          ++iterOld_doNotUse;

          auto oldParams = (*currentOld);
          auto oldHash = oldParams->hash(hashOptions);

          auto iterNew_doNotUse = newList.begin();
          auto iterNewHash_doNotUse = newHashedList.begin();

          for (; iterNew_doNotUse != newList.end();) {
            auto currentNew = iterNew_doNotUse;
            ++iterNew_doNotUse;

            auto currentNewHash = iterNewHash_doNotUse;
            ++iterNewHash_doNotUse;

            auto &newHash = (*currentNewHash).first;

            if (oldHash != newHash) continue;   // not an exact match

            auto newParams = (*currentNew);

            if (oldParams->hash() == newParams->hash()) {
              // an exact match
              ZS_LOG_TRACE(slog("parameters are unchanged") + oldParams->toDebug())
              outUnchangedChannels.push_back(OldNewParametersPair(oldParams, newParams));
            } else {
              ZS_LOG_TRACE(slog("parameters are almost an exact match (but some options have changed)") + ZS_PARAM("old", oldParams->toDebug()) + ZS_PARAM("new", newParams->toDebug()))
              outUpdatedChannels.push_back(OldNewParametersPair(oldParams, newParams));
            }

            // remove this entry since it's now been processed
            oldList.erase(currentOld);
            newList.erase(currentNew);
            newHashedList.erase(currentNewHash);
            break;
          }
        }
      }

      newHashedList.clear();  // do not need this list anymore

      // scope: find exact ssrc matching base layer SSRCs
      {
        for (auto iterOld_doNotUse = oldList.begin(); iterOld_doNotUse != oldList.end(); ) {
          auto currentOld = iterOld_doNotUse;
          ++iterOld_doNotUse;

          auto oldParams = (*currentOld);

          for (auto iterNew_doNotUse = newList.begin(); iterNew_doNotUse != newList.end();) {
            auto currentNew = iterNew_doNotUse;
            ++iterNew_doNotUse;

            auto newParams = (*currentNew);

            if (oldParams->mEncodings.size() < 1) continue;
            if (newParams->mEncodings.size() < 1) continue;

            auto &firstOld = oldParams->mEncodings.front();
            auto &firstNew = newParams->mEncodings.front();

            if (!firstOld.mSSRC.hasValue()) continue;
            if (!firstNew.mSSRC.hasValue()) continue;

            if (firstOld.mSSRC.value() != firstNew.mSSRC.value()) continue;

            float rank {};
            if (isRankableMatch(kind, *oldParams, *newParams, rank)) {
              ZS_LOG_TRACE(slog("parameters has an SSRC match and changes are compatible (thus must update channel)") + ZS_PARAM("old", oldParams->toDebug()) + ZS_PARAM("new", newParams->toDebug()))
              outUpdatedChannels.push_back(OldNewParametersPair(oldParams, newParams));
            } else {
              ZS_LOG_TRACE(slog("parameters has an SSRC match but changes are not compatible (thus must remove old and add channel)") + ZS_PARAM("old", oldParams->toDebug()) + ZS_PARAM("new", newParams->toDebug()))
              outRemovedChannels.push_back(oldParams);
              outNewChannels.push_back(newParams);
            }

            oldList.erase(currentOld);
            newList.erase(currentNew);
            break;
          }
        }
      }

      // scope: old params with non-matching SSRC entries must be removed
      {
        for (auto iterOld_doNotUse = oldList.begin(); iterOld_doNotUse != oldList.end(); ) {
          auto currentOld = iterOld_doNotUse;
          ++iterOld_doNotUse;

          auto oldParams = (*currentOld);

          if (oldParams->mEncodings.size() < 1) continue;

          auto &firstOld = oldParams->mEncodings.front();
          if (!firstOld.mSSRC.hasValue()) continue;

          ZS_LOG_TRACE(slog("old parameters did not have an SSRC match (thus must remove)") + oldParams->toDebug())
          outRemovedChannels.push_back(oldParams);

          oldList.erase(currentOld);
        }
      }

      // scope: new params with non-matching SSRC entries must be added
      {
        for (auto iterNew_doNotUse = newList.begin(); iterNew_doNotUse != newList.end();) {
          auto currentNew = iterNew_doNotUse;
          ++iterNew_doNotUse;

          auto newParams = (*currentNew);

          if (newParams->mEncodings.size() < 1) continue;

          auto &firstNew = newParams->mEncodings.front();
          if (!firstNew.mSSRC.hasValue()) continue;

          ZS_LOG_TRACE(slog("new parameters did not have an SSRC match (thus must add)") + newParams->toDebug())
          outNewChannels.push_back(newParams);

          newList.erase(currentNew);
        }
      }

      // scope: find closest matches
      {
        for (auto iterOld_doNotUse = oldList.begin(); iterOld_doNotUse != oldList.end(); ) {
          auto currentOld = iterOld_doNotUse;
          ++iterOld_doNotUse;

          auto oldParams = (*currentOld);

          float closestRank = 0.0;
          ParametersPtr closestMatchNew;
          auto closestMatchNewIter = newList.end();

          for (auto iterNew_doNotUse = newList.begin(); iterNew_doNotUse != newList.end();) {
            auto currentNew = iterNew_doNotUse;
            ++iterNew_doNotUse;

            auto newParams = (*currentNew);

            float rank {};
            if (!isRankableMatch(kind, *oldParams, *newParams, rank)) continue;

            if (!closestMatchNew) {
              // first time match
              closestRank = rank;
              closestMatchNew = newParams;
              closestMatchNewIter = currentNew;
              continue;
            }

            if (rank < closestRank) continue; // this not not a better match

            closestRank = rank;
            closestMatchNew = newParams;
            closestMatchNewIter = currentNew;
          }

          if (closestMatchNew) {
            ZS_LOG_INSANE(slog("close channel params match was found (thus going to update)") + oldParams->toDebug())
            outUpdatedChannels.push_back(OldNewParametersPair(oldParams, closestMatchNew));

            oldList.erase(currentOld);
            newList.erase(closestMatchNewIter);
          } else {
            ZS_LOG_INSANE(slog("old channel params was not found (thus going to remove)") + oldParams->toDebug())

            oldList.erase(currentOld);
            outRemovedChannels.push_back(oldParams);
          }
        }
      }

      // scope: all unprocessed "new" list channels must be added
      {
        for (auto iterOld = oldList.begin(); iterOld != oldList.end(); ++iterOld) {
          auto &oldParams = (*iterOld);
          ZS_LOG_TRACE(slog("old parameters did not have any match (thus must remove)") + oldParams->toDebug())
          outRemovedChannels.push_back(oldParams);
        }
        for (auto iterNew = newList.begin(); iterNew != newList.end(); ++iterNew) {
          auto &newParams = (*iterNew);
          ZS_LOG_TRACE(slog("new parameters did not have any match (thus must add)") + newParams->toDebug())
          outNewChannels.push_back(newParams);
        }
      }

      ZS_LOG_TRACE(slog("delta calculated for channel params") +
                   ZS_PARAM("kind", (kind.hasValue() ? IMediaStreamTrackTypes::toString(kind.value()) : "")) +
                   ZS_PARAM("unchanged channels", outUnchangedChannels.size()) +
                   ZS_PARAM("new channels", outNewChannels.size()) +
                   ZS_PARAM("udpated channels", outUpdatedChannels.size()) +
                   ZS_PARAM("removed channels", outRemovedChannels.size()))
    }

    //-------------------------------------------------------------------------
    bool RTPTypesHelper::isGeneralizedSSRCCompatibleChange(
                                                          const Parameters &oldParams,
                                                          const Parameters &newParams
                                                          )
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      // these changes must cause an SSRC change to occur
      if (oldParams.mMuxID != newParams.mMuxID) return false;

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPTypesHelper::isCompatibleCodec(
                                          const CodecParameters &oldCodec,
                                          const CodecParameters &newCodec,
                                          float &ioRank
                                          )
    {
      auto supportedCodec = IRTPTypes::toSupportedCodec(oldCodec.mName);

      bool checkPTime = false;
      bool checkNumChannels = true;

      // do codec specific compatibility test(s)
      switch (supportedCodec) {
        case IRTPTypes::SupportedCodec_Unknown:
        {
          break;
        }
        case IRTPTypes::SupportedCodec_Opus:              
        case IRTPTypes::SupportedCodec_Isac:              
        case IRTPTypes::SupportedCodec_G722:              
        case IRTPTypes::SupportedCodec_ILBC:              
        case IRTPTypes::SupportedCodec_PCMU:              
        case IRTPTypes::SupportedCodec_PCMA:              
        {
          checkPTime = true;
          checkNumChannels = true;
          break;
        }

          // video codecs
        case IRTPTypes::SupportedCodec_VP8:               break;
        case IRTPTypes::SupportedCodec_VP9:               break;
        case IRTPTypes::SupportedCodec_H264:              break;

          // RTX
        case IRTPTypes::SupportedCodec_RTX:               break;

          // FEC
        case IRTPTypes::SupportedCodec_RED:               break;
        case IRTPTypes::SupportedCodec_ULPFEC:            break;
        case IRTPTypes::SupportedCodec_FlexFEC:           break;

        case IRTPTypes::SupportedCodec_CN:                break;
          
        case IRTPTypes::SupportedCodec_TelephoneEvent:    break;
      }

      if (checkPTime) {
        if (oldCodec.mPTime != newCodec.mPTime) return false;             // not compatible
        if (oldCodec.mMaxPTime != newCodec.mMaxPTime) return false;       // not compatible
      } else {
        if ((Milliseconds() != oldCodec.mPTime) ||
            (Milliseconds() != newCodec.mPTime)) {
          ioRank += (oldCodec.mPTime == newCodec.mPTime ? 0.01f : -0.01f);
        }
        if ((Milliseconds() != oldCodec.mMaxPTime) ||
            (Milliseconds() != newCodec.mMaxPTime)) {
          ioRank += (oldCodec.mMaxPTime == newCodec.mMaxPTime ? 0.01f : -0.01f);
        }
      }
      if (checkNumChannels) {
        if (oldCodec.mNumChannels.hasValue()) {
          if (!newCodec.mNumChannels.hasValue()) return false;            // not compatible
          if (oldCodec.mNumChannels.value() != newCodec.mNumChannels.value()) return false; // not compatible
          ioRank += (oldCodec.mNumChannels == newCodec.mNumChannels ? 0.02f : -0.02f);
        } else {
          if (newCodec.mNumChannels.hasValue()) return false;             // not compatible
          ioRank += (oldCodec.mNumChannels == newCodec.mNumChannels ? 0.01f : -0.01f);
        }
      }

      ioRank += 0.1f;
      return true;
    }

    //-------------------------------------------------------------------------
    const RTPTypesHelper::CodecParameters *RTPTypesHelper::findCodec(
                                                                     const Parameters &params,
                                                                     const FindCodecOptions &options
                                                                     )
    {
      const CodecParameters *foundCodec = NULL;
      const CodecParameters *preferredCodec = NULL;

      size_t preferredLevel = 0;

      for (auto iter = params.mCodecs.begin(); iter != params.mCodecs.end(); ++iter) {
        auto &codec = (*iter);

        size_t matchLevel = 0;

        IRTPTypes::SupportedCodecs supported = IRTPTypes::SupportedCodec_Unknown;
        IRTPTypes::CodecKinds codecKind = IRTPTypes::CodecKind_Unknown;

        if (options.mDisallowedPayloadtypeMatches.size() > 0) {
          auto found = options.mDisallowedPayloadtypeMatches.find(codec.mPayloadType);
          if (found != options.mDisallowedPayloadtypeMatches.end()) continue;
        }

        if (options.mPayloadType.hasValue()) {
          if (options.mPayloadType.value() != codec.mPayloadType) continue;
        }

        if ((options.mKind.hasValue()) ||
            (options.mSupportedCodec.hasValue()) ||
            (options.mCodecKind.hasValue()))
        {
          supported = IRTPTypes::toSupportedCodec(codec.mName);
        }

        if ((options.mKind.hasValue()) ||
            (options.mCodecKind.hasValue()))
        {
          codecKind = IRTPTypes::getCodecKind(supported);
        }

        if (options.mSupportedCodec.hasValue()) {
          if (options.mSupportedCodec.value() != IRTPTypes::SupportedCodec_Unknown) {
            if (options.mSupportedCodec.value() != supported) continue;
          }
        }

        if (options.mCodecKind.hasValue()) {
          if (options.mCodecKind.value() != IRTPTypes::CodecKind_Unknown) {
            if (options.mCodecKind.value() != codecKind) continue;
          }
        }

        if (options.mClockRate.hasValue()) {
          if (codec.mClockRate.hasValue()) {
            if (codec.mClockRate.value() != options.mClockRate.value()) continue;
            ++matchLevel;
          } else {
            if (options.mMatchClockRateNotSet.hasValue()) {
              if (!options.mMatchClockRateNotSet.value()) continue;
            } else {
              continue; // must match exact clock rate
            }
          }
        } else {
          if (options.mMatchClockRateNotSet.hasValue()) {
            if (options.mMatchClockRateNotSet.value()) {
              if (codec.mClockRate.hasValue()) continue;  // cannot match as clock rate was set
            } else {
              if (!codec.mClockRate.hasValue()) continue; // cannot match as clock rate was not set
            }
          }
        }

        if (options.mKind.hasValue()) {
          switch (codecKind) {
            case IRTPTypes::CodecKind_Unknown:  continue;
            case IRTPTypes::CodecKind_Audio:
            case IRTPTypes::CodecKind_AudioSupplemental:    {
              if (IMediaStreamTrackTypes::Kind_Audio != options.mKind.value()) continue;
              break;
            }
            case IRTPTypes::CodecKind_Video:                {
              if (IMediaStreamTrackTypes::Kind_Video != options.mKind.value()) continue;
              break;
            }
            case IRTPTypes::CodecKind_AV:       break;
            case IRTPTypes::CodecKind_Data:
            case IRTPTypes::CodecKind_RTX:
            case IRTPTypes::CodecKind_FEC:
            {
              if (!options.mAllowNeutralKind.hasValue()) continue;
              if (!options.mAllowNeutralKind.value()) continue;
              break;
            }
          }
        }

        if (options.mREDCodecPayloadTypes.hasValue()) {
          IRTPTypes::REDCodecParametersPtr redCodecParams = IRTPTypes::REDCodecParameters::convert(codec.mParameters);
          if (redCodecParams) {
            if (redCodecParams->mPayloadTypes.size() > 0) {
              auto &redPayloadTypes = options.mREDCodecPayloadTypes.value();

              auto iter1 = redPayloadTypes.begin();
              auto iter2 = redCodecParams->mPayloadTypes.begin();

              bool allMatched = true;

              for (; iter1 != redPayloadTypes.end() && iter2 != redCodecParams->mPayloadTypes.end(); ++iter1, ++iter2)
              {
                auto pt1 = (*iter1);
                auto pt2 = (*iter2);
                if (pt1 == pt2) continue;

                allMatched = false;
                break;
              }

              if (!allMatched) continue;

              if ((iter1 != redPayloadTypes.end()) ||
                  (iter2 != redCodecParams->mPayloadTypes.end())) continue;

              matchLevel += 2;
            } else {
              if (!options.mAllowREDMatchEmptyList.hasValue()) continue;
              if (!options.mAllowREDMatchEmptyList.value()) continue;
            }
          } else {
            if (!options.mAllowREDMatchEmptyList.hasValue()) continue;
            if (!options.mAllowREDMatchEmptyList.value()) continue;
          }
        }

        if (options.mRTXAptPayloadType.hasValue()) {
          IRTPTypes::RTXCodecParametersPtr rtxCodecParams = IRTPTypes::RTXCodecParameters::convert(codec.mParameters);
          if (!rtxCodecParams) continue;

          if (rtxCodecParams->mApt != options.mRTXAptPayloadType.value()) continue;
          matchLevel += 2;
        }

        if ((options.mDisallowMultipleMatches.hasValue()) &&
            (options.mDisallowMultipleMatches.value())) {
          if (foundCodec) {
            ZS_LOG_WARNING(Trace, slog("codec choice is ambiguous") + ZS_PARAM("found", codec.toDebug()) + ZS_PARAM("previously found", foundCodec->toDebug()) + options.toDebug())
            return NULL;
          }
          foundCodec = &codec;
          if (matchLevel > preferredLevel) {
            preferredCodec = foundCodec;
          }
        } else {
          return &codec;
        }
      }

      if (preferredCodec) return preferredCodec;
      return foundCodec;
    }

    //-------------------------------------------------------------------------
    const RTPTypesHelper::CodecParameters *RTPTypesHelper::pickCodec(
                                                                     Optional<IMediaStreamTrackTypes::Kinds> kind,
                                                                     const Parameters &params,
                                                                     Optional<PayloadType> packetPayloadType,
                                                                     const EncodingParameters *encoding,
                                                                     const EncodingParameters *baseEncoding
                                                                     )
    {
      typedef IRTPTypes::CodecKinds CodecKinds;

      Optional<PayloadType> payloadType;

      if (NULL != encoding) {
        if (encoding->mCodecPayloadType.hasValue()) {
          payloadType = encoding->mCodecPayloadType;
        }
      }

      if ((!payloadType.hasValue()) &&
          (baseEncoding)) {
        if (baseEncoding->mCodecPayloadType.hasValue()) {
          payloadType = baseEncoding->mCodecPayloadType;
        }
      }

      if (!payloadType.hasValue()) {
        if (params.mEncodings.size() > 0) {
          auto &frontEncoding = params.mEncodings.front();
          if (frontEncoding.mCodecPayloadType.hasValue()) {
            payloadType = frontEncoding.mCodecPayloadType;
          }
        }
      }

      FindCodecOptions findOptions;

      if (packetPayloadType.hasValue()) {
        payloadType = packetPayloadType;
        findOptions.mAllowNeutralKind = true;
      }

      findOptions.mPayloadType = payloadType;
      findOptions.mKind = kind;

      auto foundCodec = findCodec(params, findOptions);

      if (!foundCodec) {
        ZS_LOG_INSANE(slog("did not codec to use") + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()) + findOptions.toDebug())
        return NULL;
      }

      ZS_LOG_INSANE(slog("found codec to use") + foundCodec->toDebug() + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()) + findOptions.toDebug())
      return foundCodec;
    }

    //-------------------------------------------------------------------------
    bool RTPTypesHelper::isRankableMatch(
                                        Optional<IMediaStreamTrackTypes::Kinds> kind,
                                        const Parameters &oldParams,
                                        const Parameters &newParams,
                                        float &outRank
                                        )
    {
      outRank = 0;

      if (oldParams.mEncodings.size() < 1) {
        if (newParams.mEncodings.size() > 0) return false;
      }
      if (newParams.mEncodings.size() < 1) {
        if (oldParams.mEncodings.size() > 0) return false;
      }

      if (oldParams.mEncodings.size() < 1) {
        // all codecs must match compatibly
        for (auto iterOldCodec = oldParams.mCodecs.begin(); iterOldCodec != oldParams.mCodecs.end(); ++iterOldCodec) {
          auto &oldCodec = (*iterOldCodec);

          FindCodecOptions options;
          options.mClockRate = oldCodec.mClockRate;
          if (!oldCodec.mClockRate.hasValue()) {
            options.mMatchClockRateNotSet = true;
          }
          options.mPayloadType = oldCodec.mPayloadType;
          options.mSupportedCodec = IRTPTypes::toSupportedCodec(oldCodec.mName);
          options.mDisallowMultipleMatches = true;

          auto newCodec = findCodec(newParams, options);
          if (!newCodec) {
            ZS_LOG_TRACE(slog("old supplied codec no longer matches with new codecs (thus old params are not compatible)") + ZS_PARAM("old", oldParams.toDebug()) + ZS_PARAM("new", newParams.toDebug()) + oldCodec.toDebug())
            return false;
          }

          if (!isCompatibleCodec(*newCodec, oldCodec, outRank)) {
            ZS_LOG_TRACE(slog("old supplied codec no longer compatible with new codec") + ZS_PARAM("old", oldParams.toDebug()) + ZS_PARAM("new", newParams.toDebug()) + ZS_PARAM("old codec", oldCodec.toDebug()) + ZS_PARAM("new codec", newCodec->toDebug()))
            return false;
          }
        }

        goto check_other_properties;
      }

      // scope: check codec used in encoding params
      {
        auto oldCodec = pickCodec(kind, oldParams);
        auto newCodec = pickCodec(kind, newParams);

        if (!oldCodec) return false;
        if (!newCodec) return false;

        //  payload type cannot change for picked codec
        if (oldCodec->mPayloadType != newCodec->mPayloadType) return false;

        // make sure meaning of codec is the same
        if (oldCodec->mName != newCodec->mName) return false;
        if (oldCodec->mClockRate != newCodec->mClockRate) return false;

        if (!isCompatibleCodec(*oldCodec, *newCodec, outRank)) return false;
      }

    check_other_properties:
      {
        if (oldParams.mEncodings.size() > 0) {
          ASSERT(newParams.mEncodings.size() > 0)

          auto &oldEncoding = oldParams.mEncodings.front();
          auto &newEncoding = newParams.mEncodings.front();

          // make sure the rid (if specified) matches
          if (oldEncoding.mEncodingID != newEncoding.mEncodingID) return false; // a non-match on the RID is not the same stream

          if (oldEncoding.mEncodingID.hasData()) {
            // this must be an exact match (thus weight so heavily that it will be chosen)
            outRank += kExactMatchRankAmount;
          }
        }

        outRank += (oldParams.mEncodings.size() == newParams.mEncodings.size() ? 1.0f : -0.2f);

        for (auto iterOldEncoding = oldParams.mEncodings.begin(); iterOldEncoding != oldParams.mEncodings.end(); ++iterOldEncoding)
        {
          auto &oldEncoding = (*iterOldEncoding);

          bool foundLayer = false;
          for (auto iterNewEncoding = newParams.mEncodings.begin(); iterNewEncoding != newParams.mEncodings.end(); ++iterNewEncoding)
          {
            auto &newEncoding = (*iterNewEncoding);

            if (oldEncoding.mEncodingID != newEncoding.mEncodingID) continue;

            foundLayer = true;
            outRank += (oldEncoding.hash() == newEncoding.hash() ? 0.3f : -0.1f);
            break;
          }
          if (!foundLayer) outRank -= 0.2f;
        }

        for (auto iterNewEncoding = newParams.mEncodings.begin(); iterNewEncoding != newParams.mEncodings.end(); ++iterNewEncoding)
        {
          auto &newEncoding = (*iterNewEncoding);

          bool foundLayer = false;

          for (auto iterOldEncoding = oldParams.mEncodings.begin(); iterOldEncoding != oldParams.mEncodings.end(); ++iterOldEncoding)
          {
            auto &oldEncoding = (*iterOldEncoding);
            if (oldEncoding.mEncodingID != newEncoding.mEncodingID) continue;

            foundLayer = true;
            break;
          }
          if (!foundLayer) outRank -= 0.2f;
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    RTPTypesHelper::EncodingParameters *RTPTypesHelper::findEncodingBase(
                                                                         Parameters &inParams,
                                                                         EncodingParameters *inEncoding
                                                                         )
    {
      typedef std::map<String, EncodingParameters *> DependencyMap;

      if (!inEncoding) {
        if (inParams.mEncodings.size() < 1) return NULL;
        inEncoding = &(*(inParams.mEncodings.begin()));
      }

      if (inEncoding->mDependencyEncodingIDs.size() < 1) {
        // if there are no dependencies then this is the base
        return inEncoding;
      }

      DependencyMap encodings;

      for (auto iter = inParams.mEncodings.begin(); iter != inParams.mEncodings.end(); ++iter)
      {
        auto &encoding = (*iter);

        encodings[encoding.mEncodingID] = &encoding;
      }

      DependencyMap previouslyChecked;
      DependencyMap pending;

      pending[inEncoding->mEncodingID] = inEncoding;

      while (pending.size() > 0) {
        auto check = pending.begin();
        String encodingID = check->first;
        EncodingParameters *encoding = check->second;
        pending.erase(check);

        if (encoding->mDependencyEncodingIDs.size() < 1) return encoding;

        previouslyChecked[check->first] = check->second;

        for (auto iter = encoding->mDependencyEncodingIDs.begin(); iter != encoding->mDependencyEncodingIDs.end(); ++iter) {
          auto &dependencyID = (*iter);
          auto foundPrevious = previouslyChecked.find(dependencyID);
          if (foundPrevious != previouslyChecked.end()) continue;

          auto found = encodings.find(dependencyID);
          if (found == encodings.end()) continue;

          pending[found->first] = found->second;
        }
      }

      return NULL;
    }

    //-------------------------------------------------------------------------
    static bool verifyFECKnownMechanism(
                                        const IRTPTypes::EncodingParameters &encoding,
                                        IRTPTypes::SupportedCodecs supportedCodec
                                        )
    {
      if (!encoding.mFEC.hasValue()) return false;

      if (encoding.mFEC.value().mMechanism.isEmpty()) {
        // This is not leegally normally but the latch all capability needs to
        // be able to specify an unknown mechansim. As such, an empty string
        // is used to signify the capability of using any FEC mechanism (which
        // cannot be specified by a user of the API externally).
        return true;
      }

      IRTPTypes::KnownFECMechanisms knownMechanism = IRTPTypes::toKnownFECMechanism(encoding.mFEC.value().mMechanism);
      switch (knownMechanism)
      {
        case IRTPTypes::KnownFECMechanism_Unknown:    break;
        case IRTPTypes::KnownFECMechanism_RED:        break;
        case IRTPTypes::KnownFECMechanism_RED_ULPFEC:
        {
          if (IRTPTypes::SupportedCodec_RED != supportedCodec) break;
          return true;
        }
        case IRTPTypes::KnownFECMechanism_FlexFEC:
        {
          if (IRTPTypes::SupportedCodec_FlexFEC != supportedCodec) break;
          return true;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    RTPTypesHelper::EncodingParameters *RTPTypesHelper::pickEncodingToFill(
                                                                           Optional<IMediaStreamTrackTypes::Kinds> kind,
                                                                           const RTPPacket &packet,
                                                                           Parameters &filledParams,
                                                                           const DecodedCodecInfo &decodedCodec,
                                                                           EncodingParameters * &outBaseEncoding
                                                                           )
    {
      outBaseEncoding = NULL;

      if (filledParams.mEncodings.size() < 1) {
        // "latch all" allows this codec to match all incoming packets
        return NULL;
      }

      (*(filledParams.mEncodings.begin()));

      for (auto encodingIter = filledParams.mEncodings.begin(); encodingIter != filledParams.mEncodings.end(); ++encodingIter) {

        auto &encoding = (*encodingIter);

        EncodingParameters &baseEncoding = *(RTPTypesHelper::findEncodingBase(filledParams, &encoding));

        const CodecParameters *baseCodec = NULL;
        IRTPTypes::SupportedCodecs baseSupportedCodec = IRTPTypes::SupportedCodec_Unknown;
        IRTPTypes::CodecKinds baseCodecKind = IRTPTypes::CodecKind_Unknown;
        if (baseEncoding.mCodecPayloadType.hasValue()) {
          baseCodec = RTPTypesHelper::pickCodec(kind, filledParams);
          if (baseCodec) {
            baseSupportedCodec = IRTPTypes::toSupportedCodec(baseCodec->mName);
            baseCodecKind = IRTPTypes::getCodecKind(baseSupportedCodec);
          }
        }

        switch (decodedCodec.mDepth[0].mCodecKind) {
          case IRTPTypes::CodecKind_Unknown:  ASSERT(false) break;
          case IRTPTypes::CodecKind_Audio:
          case IRTPTypes::CodecKind_Video:
          case IRTPTypes::CodecKind_AV:
          case IRTPTypes::CodecKind_Data:
          {
            if (baseCodec) {
              if (baseCodec->mPayloadType != packet.pt()) goto not_possible_match;
            }

            if (encoding.mCodecPayloadType.hasValue()) {
              if (encoding.mCodecPayloadType.value() != packet.pt()) goto not_possible_match;
            }

            if (encoding.mSSRC.hasValue()) {
              if (encoding.mSSRC.value() != packet.ssrc()) goto not_possible_match;
            }
            
            if (&baseEncoding != &encoding) {
              if (!baseEncoding.mSSRC.hasValue()) goto not_possible_match;

              if (packet.ssrc() != baseEncoding.mSSRC.value()) {
                if (!IRTPTypes::isMRSTCodec(decodedCodec.mDepth[0].mSupportedCodec)) goto not_possible_match;
              }
            }

            outBaseEncoding = &baseEncoding;
            return &encoding;
          }
          case IRTPTypes::CodecKind_AudioSupplemental:
          {
            if (!baseCodec) goto not_possible_match;
            if (&baseEncoding != &encoding) goto not_possible_match;

            if (!encoding.mSSRC.hasValue()) goto not_possible_match;

            if (encoding.mSSRC.value() != packet.ssrc()) goto not_possible_match;

            if (baseCodec->mClockRate.hasValue()) {
              if (decodedCodec.mDepth[0].mCodecParameters->mClockRate.hasValue()) {  // NOTE: will allow match if supplemental codec does not have a clock rate specified at all
                if (baseCodec->mClockRate.value() != decodedCodec.mDepth[0].mCodecParameters->mClockRate.value()) goto not_possible_match;
              }
            }

            outBaseEncoding = &baseEncoding;
            return &encoding;
          }
          case IRTPTypes::CodecKind_RTX:    {
            if (!baseCodec) goto not_possible_match;
            if (!baseEncoding.mSSRC.hasValue()) goto not_possible_match;
            if (!baseEncoding.mCodecPayloadType.hasValue()) goto not_possible_match;

            if (!encoding.mRTX.hasValue()) goto not_possible_match;

            if (encoding.mRTX.value().mSSRC.hasValue()) {
              if (encoding.mRTX.value().mSSRC.value() != packet.ssrc()) goto not_possible_match;
            }

            // not possible to switch codec and still match
            switch (decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecKind)
            {
              case IRTPTypes::CodecKind_Audio:
              case IRTPTypes::CodecKind_Video:
              case IRTPTypes::CodecKind_AV:
              case IRTPTypes::CodecKind_Data:
              {
                if (baseEncoding.mCodecPayloadType.value() != decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecParameters->mPayloadType) goto not_possible_match;
                break;
              }
              case IRTPTypes::CodecKind_AudioSupplemental:
              {
                if ((baseCodecKind != IRTPTypes::CodecKind_Audio) &&
                    (baseCodecKind != IRTPTypes::CodecKind_AV)) goto not_possible_match;
                break;
              }
              case IRTPTypes::CodecKind_Unknown:
              case IRTPTypes::CodecKind_RTX:
              case IRTPTypes::CodecKind_FEC:
              {
                goto not_possible_match;
              }
            }

            for (size_t depth = 0; depth < ORTC_INTERNAL_RTPTYPESHELPER_MAX_CODEC_DEPTH; ++depth)
            {
              if (IRTPTypes::CodecKind_FEC != decodedCodec.mDepth[depth].mCodecKind) continue;
              if (!encoding.mFEC.hasValue()) goto not_possible_match;
              if (!verifyFECKnownMechanism(encoding, decodedCodec.mDepth[depth].mSupportedCodec)) goto not_possible_match;
            }

            if (&baseEncoding != &encoding) {
              if (IRTPTypes::CodecKind_AudioSupplemental == decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecKind) goto not_possible_match;

              if (baseEncoding.mRTX.value().mSSRC.value() != packet.ssrc()) {
                if (!IRTPTypes::isMRSTCodec(baseSupportedCodec)) goto not_possible_match;
              }
            }

            outBaseEncoding = &baseEncoding;
            return &encoding;
          }
          case IRTPTypes::CodecKind_FEC:    {
            if (!encoding.mFEC.hasValue()) goto not_possible_match;

            if (encoding.mFEC.value().mSSRC.hasValue()) {
              if (encoding.mFEC.value().mSSRC.value() != packet.mSSRC) goto not_possible_match;
            }

            if (!verifyFECKnownMechanism(encoding, decodedCodec.mDepth[0].mSupportedCodec)) goto not_possible_match;

            if (baseEncoding.mCodecPayloadType.hasValue())
            {
              switch (decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecKind)
              {
                case IRTPTypes::CodecKind_Audio:
                case IRTPTypes::CodecKind_Video:
                case IRTPTypes::CodecKind_AV:
                case IRTPTypes::CodecKind_Data:
                {
                  if (baseEncoding.mCodecPayloadType.value() != decodedCodec.mDepth[decodedCodec.mFilledDepth].mCodecParameters->mPayloadType) goto not_possible_match;
                  break;
                }
                case IRTPTypes::CodecKind_AudioSupplemental:
                {
                  if ((baseCodecKind != IRTPTypes::CodecKind_Audio) &&
                      (baseCodecKind != IRTPTypes::CodecKind_AV)) goto not_possible_match;

                  if (&baseEncoding != &encoding) goto not_possible_match;
                  break;
                }
                case IRTPTypes::CodecKind_Unknown:
                case IRTPTypes::CodecKind_RTX:
                case IRTPTypes::CodecKind_FEC:
                {
                  goto not_possible_match;
                }
              }
            }

            if (baseEncoding.mFEC.value().mSSRC.hasValue())
            {
              if (baseEncoding.mFEC.value().mSSRC.value() != packet.ssrc()) {
                if (!IRTPTypes::isMRSTCodec(decodedCodec.mDepth[decodedCodec.mFilledDepth].mSupportedCodec)) goto not_possible_match;
              }
            }

            if (!baseCodec) goto not_possible_match;
            if (!baseEncoding.mSSRC.hasValue()) goto not_possible_match;

            outBaseEncoding = &baseEncoding;
            return &encoding;
          }
        }

      not_possible_match: {}
      }

      outBaseEncoding = NULL;
      return NULL;
    }

    //-------------------------------------------------------------------------
    Optional<IMediaStreamTrackTypes::Kinds> RTPTypesHelper::getCodecsKind(const Parameters &params)
    {
      Optional<IMediaStreamTrack::Kinds> foundKind;

      for (auto iter = params.mCodecs.begin(); iter != params.mCodecs.end(); ++iter) {
        auto &codec = (*iter);

        auto knownCodec = IRTPTypes::toSupportedCodec(codec.mName);

        auto codecKind = IRTPTypes::getCodecKind(knownCodec);

        switch (codecKind) {
          case IRTPTypes::CodecKind_Audio:
          case IRTPTypes::CodecKind_AudioSupplemental:
          {
            if (foundKind.hasValue()) {
              if (foundKind.value() != IMediaStreamTrack::Kind_Audio) return Optional<IMediaStreamTrack::Kinds>();
            }
            foundKind = IMediaStreamTrack::Kind_Audio;
            break;
          }
          case IRTPTypes::CodecKind_Video:
          {
            if (foundKind.hasValue()) {
              if (foundKind.value() != IMediaStreamTrack::Kind_Video) return Optional<IMediaStreamTrack::Kinds>();
            }
            foundKind = IMediaStreamTrack::Kind_Video;
            break;
          }
          case IRTPTypes::CodecKind_Unknown:
          case IRTPTypes::CodecKind_AV:
          case IRTPTypes::CodecKind_RTX:
          case IRTPTypes::CodecKind_FEC:
          case IRTPTypes::CodecKind_Data:
          {
            // codec kind is not a media kind
            break;
          }
        }
      }

      return foundKind;
    }

    //-------------------------------------------------------------------------
    static bool checkDecodedKind(
                                 Optional<IMediaStreamTrackTypes::Kinds> &ioKind,
                                 IMediaStreamTrackTypes::Kinds foundKind
                                 )
    {
      if (ioKind.hasValue()) {
        if (ioKind.value() != foundKind) {
          ZS_LOG_WARNING(Trace, RTPTypesHelper::slog("packet type mismatch") + ZS_PARAM("expecting", IMediaStreamTrackTypes::toString(ioKind.value())) + ZS_PARAM("found", IMediaStreamTrackTypes::toString(foundKind)))
          return false;
        }
        return true;
      }
      ioKind = foundKind;
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPTypesHelper::decodePacketCodecs(
                                            Optional<IMediaStreamTrackTypes::Kinds> &ioKind,
                                            const RTPPacket &packet,
                                            const Parameters &params,
                                            DecodedCodecInfo &decodedCodecInfo
                                            )
    {
      decodedCodecInfo.mFilledDepth = 0;

      size_t &nextDepth = decodedCodecInfo.mFilledDepth;

      decodedCodecInfo.mDepth[nextDepth].mCodecParameters = RTPTypesHelper::pickCodec(ioKind, params, packet.pt());
      if (NULL == decodedCodecInfo.mDepth[nextDepth].mCodecParameters) {
        ZS_LOG_WARNING(Trace, slog("cannot decode because codec is not found in parameters") + params.toDebug() + ZS_PARAM("packet payload type", packet.pt()) + ZS_PARAM("kind", ioKind.hasValue() ? IMediaStreamTrackTypes::toString(ioKind.value()) : (const char *)NULL))
        return false;
      }

      decodedCodecInfo.mDepth[nextDepth].mSupportedCodec = IRTPTypes::toSupportedCodec(decodedCodecInfo.mDepth[nextDepth].mCodecParameters->mName);
      decodedCodecInfo.mDepth[nextDepth].mCodecKind = IRTPTypes::getCodecKind(decodedCodecInfo.mDepth[nextDepth].mSupportedCodec);

      switch (decodedCodecInfo.mDepth[nextDepth].mCodecKind) {
        case IRTPTypes::CodecKind_Unknown:
        {
          ASSERT(false);
          return false;
        }
        case IRTPTypes::CodecKind_Audio:              return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Audio);
        case IRTPTypes::CodecKind_AudioSupplemental:  return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Audio);
        case IRTPTypes::CodecKind_Video:              return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Video);
        case IRTPTypes::CodecKind_AV:                 return true;
        case IRTPTypes::CodecKind_Data:               return true;
        case IRTPTypes::CodecKind_RTX:                break;
        case IRTPTypes::CodecKind_FEC:                break;
      }


      const BYTE *innerPayload {};
      size_t innerPayloadSize {};

      if (IRTPTypes::CodecKind_RTX == decodedCodecInfo.mDepth[nextDepth].mCodecKind) {
        IRTPTypes::RTXCodecParametersPtr rtxParams = IRTPTypes::RTXCodecParameters::convert(decodedCodecInfo.mDepth[nextDepth].mCodecParameters->mParameters);
        if (!rtxParams) {
          ZS_LOG_WARNING(Trace, slog("cannot decode because apt is not set on RTX parameters") + params.toDebug() + ZS_PARAM("packet payload type", packet.pt()) + ZS_PARAM("kind", ioKind.hasValue() ? IMediaStreamTrackTypes::toString(ioKind.value()) : (const char *)NULL))
          return false;
        }

        ++nextDepth;

        FindCodecOptions options;
        options.mPayloadType = rtxParams->mApt;
        decodedCodecInfo.mDepth[nextDepth].mCodecParameters = findCodec(params, options);
        if (NULL == decodedCodecInfo.mDepth[nextDepth].mCodecParameters) {
          ZS_LOG_WARNING(Debug, slog("cannot find decoded RTX") + params.toDebug() + rtxParams->toDebug())
          return false;
        }

        decodedCodecInfo.mDepth[nextDepth].mSupportedCodec = IRTPTypes::toSupportedCodec(decodedCodecInfo.mDepth[nextDepth].mCodecParameters->mName);
        decodedCodecInfo.mDepth[nextDepth].mCodecKind = IRTPTypes::getCodecKind(decodedCodecInfo.mDepth[nextDepth].mSupportedCodec);

        getRTXCodecPayload(packet.payload(), packet.payloadSize(), innerPayload, innerPayloadSize);

        switch (decodedCodecInfo.mDepth[nextDepth].mCodecKind) {
          case IRTPTypes::CodecKind_Unknown:            ASSERT(false); return false;
          case IRTPTypes::CodecKind_Audio:              return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Audio);
          case IRTPTypes::CodecKind_AudioSupplemental:  return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Audio);
          case IRTPTypes::CodecKind_Video:              return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Video);
          case IRTPTypes::CodecKind_AV:                 return true;
          case IRTPTypes::CodecKind_Data:               return true;
          case IRTPTypes::CodecKind_RTX:                ASSERT(false); return false;
          case IRTPTypes::CodecKind_FEC:                break;
        }
      } else {
        innerPayload = packet.payload();
        innerPayloadSize = packet.payloadSize();
      }

      switch (decodedCodecInfo.mDepth[nextDepth].mSupportedCodec)
      {
        case IRTPTypes::SupportedCodec_RED:
        {
          auto redInnerCodecPayloadType = getRedCodecPayload(innerPayload, innerPayloadSize, innerPayload, innerPayloadSize);
          if (!redInnerCodecPayloadType.hasValue()) {
            ZS_LOG_WARNING(Trace, slog("cannot extract RED codec payload information") + decodedCodecInfo.toDebug())
            return false;
          }

          ++nextDepth;

          FindCodecOptions options;
          options.mPayloadType = redInnerCodecPayloadType;

          decodedCodecInfo.mDepth[nextDepth].mCodecParameters = findCodec(params, options);

          if (NULL == decodedCodecInfo.mDepth[nextDepth].mCodecParameters) {
            ZS_LOG_WARNING(Trace, slog("RED codec type was not understood") + decodedCodecInfo.toDebug())
            return false;
          }

          decodedCodecInfo.mDepth[nextDepth].mSupportedCodec = IRTPTypes::toSupportedCodec(decodedCodecInfo.mDepth[nextDepth].mCodecParameters->mName);
          decodedCodecInfo.mDepth[nextDepth].mCodecKind = IRTPTypes::getCodecKind(decodedCodecInfo.mDepth[nextDepth].mSupportedCodec);

          switch (decodedCodecInfo.mDepth[nextDepth].mCodecKind) {
            case IRTPTypes::CodecKind_Unknown:            {
              ZS_LOG_WARNING(Trace, slog("RED codec type was not understood") + decodedCodecInfo.toDebug())
              return false;
            }
            case IRTPTypes::CodecKind_Audio:              return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Audio);
            case IRTPTypes::CodecKind_AudioSupplemental:  return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Audio);
            case IRTPTypes::CodecKind_Video:              return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Video);
            case IRTPTypes::CodecKind_AV:                 return true;
            case IRTPTypes::CodecKind_Data:               return true;
            case IRTPTypes::CodecKind_RTX:                {
              ZS_LOG_WARNING(Trace, slog("RED codec type containing RTX codec is not supported") + decodedCodecInfo.toDebug())
              return false;
            }
            case IRTPTypes::CodecKind_FEC:                break;
          }
          if (IRTPTypes::SupportedCodec_ULPFEC != decodedCodecInfo.mDepth[nextDepth].mSupportedCodec) {
            ZS_LOG_WARNING(Trace, slog("RED codec type containing FEC codec that is not ULPFEC is not supported") + decodedCodecInfo.toDebug())
            return false;
          }
          ++nextDepth;
          break;
        }
        case IRTPTypes::SupportedCodec_ULPFEC:
        {
          ZS_LOG_WARNING(Trace, slog("only RED+ULPFEC is supported") + decodedCodecInfo.mDepth[nextDepth].toDebug())
          return false;
        }
        case IRTPTypes::SupportedCodec_FlexFEC:
        {
          ++nextDepth;
          goto decode_recovery_packet;
        }
        default:
        {
          ASSERT(false);
          return false;
        }
      }

    decode_recovery_packet:
      {
        auto fecRecoveryPayloadType = getFecRecoveryPayloadType(innerPayload, innerPayloadSize);
        if (!fecRecoveryPayloadType.hasValue()) {
          ZS_LOG_WARNING(Trace, slog("FEC recovery codec type was not understood") + decodedCodecInfo.toDebug())
          return false;
        }

        FindCodecOptions options;
        options.mPayloadType = fecRecoveryPayloadType;

        decodedCodecInfo.mDepth[nextDepth].mCodecParameters = findCodec(params, options);

        if (NULL == decodedCodecInfo.mDepth[nextDepth].mCodecParameters) {
          ZS_LOG_WARNING(Trace, slog("Recovery codec type was not understood") + decodedCodecInfo.toDebug())
          return false;
        }

        decodedCodecInfo.mDepth[nextDepth].mSupportedCodec = IRTPTypes::toSupportedCodec(decodedCodecInfo.mDepth[nextDepth].mCodecParameters->mName);
        decodedCodecInfo.mDepth[nextDepth].mCodecKind = IRTPTypes::getCodecKind(decodedCodecInfo.mDepth[nextDepth].mSupportedCodec);

        switch (decodedCodecInfo.mDepth[nextDepth].mCodecKind) {
          case IRTPTypes::CodecKind_Unknown:            {
            ZS_LOG_WARNING(Trace, slog("Recovery codec type was not understood") + decodedCodecInfo.toDebug())
            return false;
          }
          case IRTPTypes::CodecKind_Audio:              return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Audio);
          case IRTPTypes::CodecKind_AudioSupplemental:  return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Audio);
          case IRTPTypes::CodecKind_Video:              return checkDecodedKind(ioKind, IMediaStreamTrackTypes::Kind_Video);
          case IRTPTypes::CodecKind_AV:                 return true;
          case IRTPTypes::CodecKind_Data:               return true;
          case IRTPTypes::CodecKind_RTX:                {
            ZS_LOG_WARNING(Trace, slog("Recovery codec type containing RTX codec is not supported") + decodedCodecInfo.toDebug())
            return false;
          }
          case IRTPTypes::CodecKind_FEC:                {
            ZS_LOG_WARNING(Trace, slog("Recovery codec type containing another recovery codec is not supported") + decodedCodecInfo.toDebug())
            return false;
          }
        }
      }
      
      return true;
    }
    
    //-------------------------------------------------------------------------
    void RTPTypesHelper::getRTXCodecPayload(
                                            const BYTE *packetPayload,
                                            size_t packetPayloadSizeInBytes,
                                            const BYTE * &outInnterPayload,
                                            size_t &outInnerPayloadSizeBytes
                                            )
    {
      if (packetPayloadSizeInBytes < sizeof(WORD)) {
        outInnterPayload = packetPayload + packetPayloadSizeInBytes;
        outInnerPayloadSizeBytes = 0;
        return;
      }

      outInnterPayload = &(packetPayload[sizeof(WORD)]);
      outInnerPayloadSizeBytes = packetPayloadSizeInBytes - sizeof(WORD);
    }

    //-------------------------------------------------------------------------
    Optional<IRTPTypes::PayloadType> RTPTypesHelper::getRedCodecPayload(
                                                                        const BYTE *packetPayload,
                                                                        size_t packetPayloadSizeInBytes,
                                                                        const BYTE * &outInnterPayload,
                                                                        size_t &outInnerPayloadSizeBytes
                                                                        )
    {
      Optional<IRTPTypes::PayloadType> result;
      if (NULL == packetPayload) return result;
      if (sizeof(DWORD) > packetPayloadSizeInBytes) return result;

      result = static_cast<PayloadType>(0x7F & packetPayload[0]);

      outInnerPayloadSizeBytes = static_cast<size_t>(RTPUtils::getBE16(&(packetPayload[2])));

      if (outInnerPayloadSizeBytes + sizeof(DWORD) > packetPayloadSizeInBytes) {
        outInnerPayloadSizeBytes = packetPayloadSizeInBytes - sizeof(DWORD);
      }

      outInnterPayload = &(packetPayload[sizeof(DWORD)]);
      return result;
    }

    //-------------------------------------------------------------------------
    Optional<IRTPTypes::PayloadType> RTPTypesHelper::getFecRecoveryPayloadType(
                                                                               const BYTE *packetPayload,
                                                                               size_t packetPayloadSizeInBytes
                                                                               )
    {
      Optional<IRTPTypes::PayloadType> result;
      if (NULL == packetPayload) return result;
      if (sizeof(WORD) > packetPayloadSizeInBytes) return result;

      result = static_cast<PayloadType>(0x7F & packetPayload[1]);
      return result;
    }
    
    //-------------------------------------------------------------------------
    Log::Params RTPTypesHelper::slog(const char *message)
    {
      return Log::Params(message, "ortc::RTPTypesHelper");
    }


  } // namespace internal

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark (helpers)
  #pragma mark

  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message)
  {
    return Log::Params(message, "ortc::IRTPTypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::DegradationPreferences
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(DegradationPreferences preference)
  {
    switch (preference) {
      case DegradationPreference_MaintainFramerate:   return "maintain-framerate";
      case DegradationPreference_MaintainResolution:  return "maintain-resolution";
      case DegradationPreference_Balanced:            return "balanced";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::DegradationPreferences IRTPTypes::toDegredationPreference(const char *preference) throw (InvalidParameters)
  {
    String str(preference);
    for (IRTPTypes::DegradationPreferences index = IRTPTypes::DegradationPreference_First; index <= IRTPTypes::DegradationPreference_Last; index = static_cast<IRTPTypes::DegradationPreferences>(static_cast<std::underlying_type<IRTPTypes::DegradationPreferences>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IRTPTypes::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return DegradationPreference_First;
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::Capabilities
  #pragma mark


  //---------------------------------------------------------------------------
  IRTPTypes::Capabilities::Capabilities(ElementPtr elem)
  {
    if (!elem) return;

    {
      ElementPtr codecsEl = elem->findFirstChildElement("codecs");
      if (codecsEl) {
        ElementPtr codecEl = codecsEl->findFirstChildElement("codec");
        while (codecEl) {
          CodecCapability codec(codecEl);
          mCodecs.push_back(codec);
          codecEl = codecEl->findNextSiblingElement("codec");
        }
      }
    }
    {
      ElementPtr headerExtensionsEl = elem->findFirstChildElement("headerExtensions");
      if (headerExtensionsEl) {
        ElementPtr headerExtensionEl = headerExtensionsEl->findFirstChildElement("headerExtension");
        while (headerExtensionEl) {
          HeaderExtension headerExtension(headerExtensionEl);
          mHeaderExtensions.push_back(headerExtension);
          headerExtensionEl = headerExtensionEl->findNextSiblingElement("headerExtension");
        }
      }
    }
    {
      ElementPtr fecMechanismsEl = elem->findFirstChildElement("fecMechanisms");
      if (fecMechanismsEl) {
        ElementPtr fecMechanismEl = fecMechanismsEl->findFirstChildElement("fecMechanism");
        while (fecMechanismEl) {
          FECMechanism fecMechanism(IHelper::getElementTextAndDecode(fecMechanismEl));
          mFECMechanisms.push_back(fecMechanism);
          fecMechanismEl = fecMechanismEl->findNextSiblingElement("fecMechanism");
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::Capabilities::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mCodecs.size() > 0) {
      ElementPtr codecsEl = Element::create("codecs");

      for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter) {
        auto &codec = (*iter);
        codecsEl->adoptAsLastChild(codec.createElement("codec"));
      }
      elem->adoptAsLastChild(codecsEl);
    }

    if (mHeaderExtensions.size() > 0) {
      ElementPtr headerExtensionsEl = Element::create("headerExtensions");

      for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter) {
        auto &headerExtension = (*iter);
        headerExtensionsEl->adoptAsLastChild(headerExtension.createElement("headerExtension"));
      }
      elem->adoptAsLastChild(headerExtensionsEl);
    }

    if (mFECMechanisms.size() > 0) {
      ElementPtr fecMechanismsEl = Element::create("fecMechanisms");

      for (auto iter = mFECMechanisms.begin(); iter != mFECMechanisms.end(); ++iter) {
        auto &fecMechanism = (*iter);
        fecMechanismsEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("fecMechanism", fecMechanism));
      }
      elem->adoptAsLastChild(fecMechanismsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::Capabilities::toDebug() const
  {
    return createElement("ortc::IRTPTypes::Capabilities");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::Capabilities::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::Capabilities:");

    hasher->update("codecs:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

    for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter)
    {
      auto value = (*iter);
      hasher->update(":");
      hasher->update(value.hash());
    }

    hasher->update("headers:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

    for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter)
    {
      auto value = (*iter);
      hasher->update(":");
      hasher->update(value.hash());
    }

    hasher->update("fec:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

    for (auto iter = mFECMechanisms.begin(); iter != mFECMechanisms.end(); ++iter)
    {
      auto value = (*iter);
      hasher->update(":");
      hasher->update(value);
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::CodecCapability
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::CodecCapability::CodecCapability(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "name", mName);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "kind", mKind);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "clockRate", mClockRate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "preferredPayloadType", mPreferredPayloadType);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "ptime", mPTime);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "maxptime", mMaxPTime);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "numChannels", mNumChannels);

    {
      ElementPtr feedbacksEl = elem->findFirstChildElement("rtcpFeedbacks");
      if (feedbacksEl) {
        ElementPtr feedbackEl = feedbacksEl->findFirstChildElement("rtcpFeedback");
        while (feedbackEl) {
          RTCPFeedback feedback(feedbackEl);
          mRTCPFeedback.push_back(feedback);
          feedbackEl = feedbackEl->findNextSiblingElement("rtcpFeedback");
        }
      }
    }

    {
      ElementPtr parametersEl = elem->findFirstChildElement("parameters");
      if (parametersEl) {
        SupportedCodecs supported = toSupportedCodec(mName);
        switch (supported) {
          case SupportedCodec_Opus: {
            mParameters = make_shared<OpusCodecCapabilityParameters>(parametersEl);
            break;
          }
          case SupportedCodec_VP8:    {
            mParameters = make_shared<VP8CodecCapabilityParameters>(parametersEl);
            break;
          }
          case SupportedCodec_H264:   {
            mParameters = make_shared<H264CodecCapabilityParameters>(parametersEl);
            break;
          }
          case SupportedCodec_RTX:   {
            mParameters = make_shared<RTXCodecCapabilityParameters>(parametersEl);
            break;
          }
          case SupportedCodec_FlexFEC:   {
            mParameters = make_shared<FlexFECCodecCapabilityParameters>(parametersEl);
            break;
          }
          default: break;
        }
      }
    }

    {
      ElementPtr optionsEl = elem->findFirstChildElement("options");
      if (optionsEl) {
        SupportedCodecs supported = toSupportedCodec(mName);
        switch (supported) {
          case SupportedCodec_Opus: {
            mOptions = make_shared<OpusCodecCapabilityOptions>(optionsEl);
            break;
          }
          default: break;
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "maxTemporalLayers", mMaxTemporalLayers);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "maxSpatialLayers", mMaxSpatialLayers);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "svcMultiStreamSupport", mSVCMultiStreamSupport);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::CodecCapability::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    Optional<IMediaStreamTrackTypes::Kinds> kind;
    if (mKind.hasData()) {
      kind = IMediaStreamTrackTypes::toKind(mKind);
    }

    IHelper::adoptElementValue(elem, "name", mName, false);
    IHelper::adoptElementValue(elem, "kind", mKind, false);

    IHelper::adoptElementValue(elem, "clockRate", mClockRate);
    IHelper::adoptElementValue(elem, "preferredPayloadType", mPreferredPayloadType);
    if (Milliseconds() != mPTime) {
      IHelper::adoptElementValue(elem, "ptime", mPTime.count());
    }
    if (Milliseconds() != mMaxPTime) {
      IHelper::adoptElementValue(elem, "maxptime", mMaxPTime.count());
    }
    IHelper::adoptElementValue(elem, "numChannels", mNumChannels);

    if (mRTCPFeedback.size() > 0) {
      ElementPtr rtcpFeedbacksEl = Element::create("rtcpFeedbacks");

      for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter) {
        auto &value = (*iter);
        rtcpFeedbacksEl->adoptAsLastChild(value.createElement("rtcpFeedback"));
      }

      elem->adoptAsLastChild(rtcpFeedbacksEl);
    }

    if ((mParameters) ||
        (mOptions)) {
      SupportedCodecs supported = toSupportedCodec(mName);
      if (mParameters) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityParameters::convert(mParameters);
            if (codec) {
              elem->adoptAsLastChild(codec->createElement("parameters"));
            }
            break;
          }
          case SupportedCodec_VP8:    {
            auto codec = VP8CodecCapabilityParameters::convert(mParameters);
            if (codec) {
              elem->adoptAsLastChild(codec->createElement("parameters"));
            }
            break;
          }
          case SupportedCodec_H264:   {
            auto codec = H264CodecCapabilityParameters::convert(mParameters);
            if (codec) {
              elem->adoptAsLastChild(codec->createElement("parameters"));
            }
            break;
          }
          case SupportedCodec_RTX:   {
            auto codec = RTXCodecCapabilityParameters::convert(mParameters);
            if (codec) {
              elem->adoptAsLastChild(codec->createElement("parameters"));
            }
            break;
          }
          case SupportedCodec_FlexFEC:   {
            auto codec = FlexFECCodecCapabilityParameters::convert(mParameters);
            if (codec) {
              elem->adoptAsLastChild(codec->createElement("parameters"));
            }
            break;
          }
          default: break;
        }
      }
      if (mOptions) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityOptions::convert(mOptions);
            if (codec) {
              elem->adoptAsLastChild(codec->createElement("options"));
            }
            break;
          }
          default: break;
        }
      }
    }

    if (kind.hasValue()) {
      if (IMediaStreamTrackTypes::Kind_Video == kind.value()) {
        IHelper::adoptElementValue(elem, "maxTemporalLayers", mMaxTemporalLayers);
        IHelper::adoptElementValue(elem, "maxSpatialLayers", mMaxSpatialLayers);
        if (mSVCMultiStreamSupport) {
          IHelper::adoptElementValue(elem, "svcMultiStreamSupport", mSVCMultiStreamSupport);
        }
      }
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::CodecCapability::CodecCapability(const CodecCapability &source) :
    mName(source.mName),
    mKind(source.mKind),
    mClockRate(source.mClockRate),
    mPreferredPayloadType(source.mPreferredPayloadType),
    mPTime(source.mPTime),
    mMaxPTime(source.mMaxPTime),
    mNumChannels(source.mNumChannels),
    mRTCPFeedback(source.mRTCPFeedback),
    mMaxTemporalLayers(source.mMaxTemporalLayers),
    mMaxSpatialLayers(source.mMaxSpatialLayers),
    mSVCMultiStreamSupport(source.mSVCMultiStreamSupport)
  {
    if ((source.mParameters) ||
        (source.mOptions)) {
      SupportedCodecs supported = toSupportedCodec(source.mName);
      if (source.mParameters) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityParameters::convert(source.mParameters);
            if (codec) {
              mParameters = OpusCodecCapabilityParameters::create(*codec);
            }
            break;
          }
          case SupportedCodec_VP8:    {
            auto codec = VP8CodecCapabilityParameters::convert(source.mParameters);
            if (codec) {
              mParameters = VP8CodecCapabilityParameters::create(*codec);
            }
            break;
          }
          case SupportedCodec_H264:   {
            auto codec = H264CodecCapabilityParameters::convert(source.mParameters);
            if (codec) {
              mParameters = H264CodecCapabilityParameters::create(*codec);
            }
            break;
          }
          case SupportedCodec_RTX:   {
            auto codec = RTXCodecCapabilityParameters::convert(source.mParameters);
            if (codec) {
              mParameters = RTXCodecCapabilityParameters::create(*codec);
            }
            break;
          }
          case SupportedCodec_FlexFEC:   {
            auto codec = FlexFECCodecCapabilityParameters::convert(source.mParameters);
            if (codec) {
              mParameters = FlexFECCodecCapabilityParameters::create(*codec);
            }
            break;
          }
          default: break;
        }
      }
      if (source.mOptions) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityOptions::convert(source.mOptions);
            if (codec) {
              mOptions = OpusCodecCapabilityOptions::create(*codec);
            }
            break;
          }
          default: break;
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::CodecCapability::toDebug() const
  {
    return createElement("ortc::IRTPTypes::CodecCapability");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::CodecCapability::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::CodecCapability:");

    hasher->update(mName);
    hasher->update(":");
    hasher->update(mKind);
    hasher->update(":");
    hasher->update(mClockRate);
    hasher->update(":");
    hasher->update(mPreferredPayloadType);
    hasher->update(":");
    hasher->update(mPTime);
    hasher->update(":");
    hasher->update(mMaxPTime);
    hasher->update(":");
    hasher->update(mNumChannels);

    hasher->update("feedback:0e69ea312f56834897bc0c29eb74bf991bee8d86");

    for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter)
    {
      auto value = (*iter);
      hasher->update(":");
      hasher->update(value.hash());
    }

    hasher->update(":feedback:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

    SupportedCodecs supported = toSupportedCodec(mName);

    // scope: output params
    {
      hasher->update(":");

      if (mParameters) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityParameters::convert(mParameters);
            if (codec) hasher->update(codec->hash());
            break;
          }
          case SupportedCodec_VP8:    {
            auto codec = VP8CodecCapabilityParameters::convert(mParameters);
            if (codec) hasher->update(codec->hash());
            break;
          }
          case SupportedCodec_H264:   {
            auto codec = H264CodecCapabilityParameters::convert(mParameters);
            if (codec) hasher->update(codec->hash());
            break;
          }
          case SupportedCodec_RTX:   {
            auto codec = RTXCodecCapabilityParameters::convert(mParameters);
            if (codec) hasher->update(codec->hash());
            break;
          }
          case SupportedCodec_FlexFEC:   {
            auto codec = FlexFECCodecCapabilityParameters::convert(mParameters);
            if (codec) hasher->update(codec->hash());
            break;
          }
          default: break;
        }
      }
    }

    // scope: output options
    {
      hasher->update(":");

      if (mOptions) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityOptions::convert(mOptions);
            if (codec) hasher->update(codec->hash());
            break;
          }
          default: break;
        }
      }
    }

    hasher->update((bool)mParameters);
    hasher->update(":");
    hasher->update((bool)mOptions);
    hasher->update(":");
    hasher->update(mMaxTemporalLayers);
    hasher->update(":");
    hasher->update(mMaxSpatialLayers);
    hasher->update(":");
    hasher->update(mSVCMultiStreamSupport);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::OpusCodecCapabilityOptions
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::OpusCodecCapabilityOptions::toString(Signals signal)
  {
    switch (signal) {
      case Signal_Auto:         return "auto";
      case Signal_Music:        return "music";
      case Signal_Voice:        return "voice";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecParameters::Signals IRTPTypes::OpusCodecCapabilityOptions::toSignal(const char *signal)
  {
    String signalStr(signal);
    if (signalStr.isEmpty()) return Signal_Auto;

    for (Signals index = Signal_First; index <= Signal_Last; index = static_cast<Signals>(static_cast<std::underlying_type<Signals>::type>(index) + 1)) {
      if (0 == signalStr.compareNoCase(IRTPTypes::OpusCodecCapabilityOptions::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS((String("unknown signal: ") + signal).c_str())
  }

  //---------------------------------------------------------------------------
  const char *IRTPTypes::OpusCodecCapabilityOptions::toString(Applications application)
  {
    switch (application) {
      case Application_VoIP:      return "voip";
      case Application_Audio:     return "audio";
      case Application_LowDelay:  return "lowdelay";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecParameters::Applications IRTPTypes::OpusCodecCapabilityOptions::toApplication(const char *application)
  {
    String applicationStr(application);
    if (applicationStr.isEmpty()) return Application_VoIP;

    for (Applications index = Application_First; index <= Application_Last; index = static_cast<Applications>(static_cast<std::underlying_type<Applications>::type>(index) + 1)) {
      if (0 == applicationStr.compareNoCase(IRTPTypes::OpusCodecCapabilityOptions::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS((String("unknown application: ") + applicationStr).c_str())
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecCapabilityOptions::OpusCodecCapabilityOptions(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "complexity", mComplexity);
    {
      String value;
      IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "signal", value);
      if (value.hasData()) {
        try {
          mSignal = toSignal(value);
        } catch (const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("unknown opus signal") + ZS_PARAM("value", value))
        }
      }
    }
    {
      String value;
      IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "application", value);
      if (value.hasData()) {
        try {
          mApplication = toApplication(value);
        } catch (const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("unknown opus appliation") + ZS_PARAM("value", value))
        }
      }
    }
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "packetLossPerc", mPacketLossPerc);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "PredictionDisabled", mPredictionDisabled);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecCapabilityOptions::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "complexity", mComplexity);
    if (mSignal.hasValue()) {
      IHelper::adoptElementValue(elem, "signal", OpusCodecCapabilityOptions::toString(mSignal.value()), false);
    }
    if (mApplication.hasValue()) {
      IHelper::adoptElementValue(elem, "application", OpusCodecCapabilityOptions::toString(mApplication.value()), false);
    }
    IHelper::adoptElementValue(elem, "signal", mSignal);
    IHelper::adoptElementValue(elem, "application", mApplication);
    IHelper::adoptElementValue(elem, "packetLossPerc", mPacketLossPerc);
    IHelper::adoptElementValue(elem, "predictionDisabled", mPredictionDisabled);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecCapabilityOptionsPtr IRTPTypes::OpusCodecCapabilityOptions::create(const OpusCodecCapabilityOptions &capability)
  {
    return make_shared<OpusCodecCapabilityOptions>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecCapabilityOptionsPtr IRTPTypes::OpusCodecCapabilityOptions::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(OpusCodecCapabilityOptions, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecCapabilityOptions::toDebug() const
  {
    return createElement("ortc::IRTPTypes::OpusCodecCapabilityOptions");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::OpusCodecCapabilityOptions::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::OpusCodecCapabilityOptions:");

    hasher->update(mComplexity);
    hasher->update(":");
    hasher->update(mSignal);
    hasher->update(":");
    hasher->update(mApplication);
    hasher->update(":");
    hasher->update(mPacketLossPerc);
    hasher->update(":");
    hasher->update(mPredictionDisabled);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::OpusCodecCapabilityParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecCapabilityParameters::OpusCodecCapabilityParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "maxPlaybackRate", mMaxPlaybackRate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "maxAverageBitrate", mMaxAverageBitrate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "stereo", mStereo);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "cbr", mCBR);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "useInbandFec", mUseInbandFEC);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "useDtx", mUseDTX);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "spropMaxCaptureRate", mSPropMaxCaptureRate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "spropStereo", mSPropStereo);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecCapabilityParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "maxPlaybackRate", mMaxPlaybackRate);
    IHelper::adoptElementValue(elem, "maxAverageBitrate", mMaxAverageBitrate);
    IHelper::adoptElementValue(elem, "stereo", mStereo);
    IHelper::adoptElementValue(elem, "cbr", mCBR);
    IHelper::adoptElementValue(elem, "useInbandFec", mUseInbandFEC);
    IHelper::adoptElementValue(elem, "useDtx", mUseDTX);
    IHelper::adoptElementValue(elem, "spropMaxCaptureRate", mSPropMaxCaptureRate);
    IHelper::adoptElementValue(elem, "spropStereo", mSPropStereo);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }
  
  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecCapabilityParametersPtr IRTPTypes::OpusCodecCapabilityParameters::create(const OpusCodecCapabilityParameters &capability)
  {
    return make_shared<OpusCodecCapabilityParameters>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecCapabilityParametersPtr IRTPTypes::OpusCodecCapabilityParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(OpusCodecCapabilityParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecCapabilityParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::OpusCodecCapabilityParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::OpusCodecCapabilityParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::OpusCodecCapabilityParameters:");

    hasher->update(mMaxPlaybackRate);
    hasher->update(":");
    hasher->update(mMaxAverageBitrate);
    hasher->update(":");
    hasher->update(mStereo);
    hasher->update(":");
    hasher->update(mCBR);
    hasher->update(":");
    hasher->update(mUseInbandFEC);
    hasher->update(":");
    hasher->update(mUseDTX);

    hasher->update(":");
    hasher->update(mSPropMaxCaptureRate);
    hasher->update(":");
    hasher->update(mSPropStereo);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::VP8CodecCapability
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::VP8CodecCapabilityParameters::VP8CodecCapabilityParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::VP8CodecCapabilityParameters", "maxFr", mMaxFR);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::VP8CodecCapabilityParameters", "maxFs", mMaxFS);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::VP8CodecCapabilityParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "maxFr", mMaxFR);
    IHelper::adoptElementValue(elem, "maxFs", mMaxFS);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::VP8CodecCapabilityParametersPtr IRTPTypes::VP8CodecCapabilityParameters::create(const VP8CodecCapabilityParameters &capability)
  {
    return make_shared<VP8CodecCapabilityParameters>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::VP8CodecCapabilityParametersPtr IRTPTypes::VP8CodecCapabilityParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(VP8CodecCapabilityParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::VP8CodecCapabilityParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::VP8CodecCapabilityParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::VP8CodecCapabilityParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::VP8CodecCapabilityParameters:");


    hasher->update(mMaxFR);
    hasher->update(":");
    hasher->update(mMaxFS);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::H264CodecCapabilityParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::H264CodecCapabilityParameters::H264CodecCapabilityParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "profileLevelId", mProfileLevelID);

    {
      ElementPtr packetizationsEl = elem->findFirstChildElement("packetizationModes");
      if (packetizationsEl) {
        ElementPtr packetizationEl = packetizationsEl->findFirstChildElement("packetizationMode");
        while (packetizationEl) {
          decltype(mPacketizationModes)::value_type value {};
          String str = IHelper::getElementText(packetizationEl);
          try {
            value = Numeric<decltype(mPacketizationModes)::value_type>(str);
          } catch(const Numeric<decltype(mPacketizationModes)::value_type>::ValueOutOfRange &) {
            ZS_LOG_WARNING(Debug, slog("packetization mode value out of range") + ZS_PARAM("value", str))
          }
          mPacketizationModes.push_back(value);
          packetizationEl = packetizationEl->findNextSiblingElement("packetizationMode");
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxMbps", mMaxMBPS);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxSmbps", mMaxSMBPS);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxSmbps", mMaxSMBPS);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "mMaxFs", mMaxFS);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxCpb", mMaxCPB);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxDpb", mMaxDPB);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxBr", mMaxBR);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::H264CodecCapabilityParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "profileLevelId", mProfileLevelID);

    if (mPacketizationModes.size() > 0) {
      ElementPtr packetizationsEl = Element::create("packetizationModes");
      for (auto iter = mPacketizationModes.begin(); iter != mPacketizationModes.end(); ++iter) {
        auto value = (*iter);
        IHelper::adoptElementValue(packetizationsEl, "packetizationMode", value);
      }
      elem->adoptAsLastChild(packetizationsEl);
    }

    IHelper::adoptElementValue(elem, "maxMbps", mMaxMBPS);
    IHelper::adoptElementValue(elem, "maxSmbps", mMaxSMBPS);
    IHelper::adoptElementValue(elem, "maxSmbps", mMaxSMBPS);
    IHelper::adoptElementValue(elem, "mMaxFs", mMaxFS);
    IHelper::adoptElementValue(elem, "maxCpb", mMaxCPB);
    IHelper::adoptElementValue(elem, "maxDpb", mMaxDPB);
    IHelper::adoptElementValue(elem, "maxBr", mMaxBR);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::H264CodecCapabilityParametersPtr IRTPTypes::H264CodecCapabilityParameters::create(const H264CodecCapabilityParameters &capability)
  {
    return make_shared<H264CodecCapabilityParameters>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::H264CodecCapabilityParametersPtr IRTPTypes::H264CodecCapabilityParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(H264CodecCapabilityParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::H264CodecCapabilityParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::H264CodecCapabilityParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::H264CodecCapabilityParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::H264CodecCapabilityParameters:");

    hasher->update(mProfileLevelID);
    hasher->update(":packetizationmodes");

    for (auto iter = mPacketizationModes.begin(); iter != mPacketizationModes.end(); ++iter) {
      auto &mode = (*iter);
      hasher->update(":");
      hasher->update(mode);
    }

    hasher->update(":packetizationmodes:");

    hasher->update(mMaxMBPS);
    hasher->update(":");
    hasher->update(mMaxSMBPS);
    hasher->update(":");
    hasher->update(mMaxFS);
    hasher->update(":");
    hasher->update(mMaxCPB);
    hasher->update(":");
    hasher->update(mMaxDPB);
    hasher->update(":");
    hasher->update(mMaxBR);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::RTXCodecCapabilityParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::RTXCodecCapabilityParameters::RTXCodecCapabilityParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTXCodecCapabilityParameters", "apt", mApt);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTXCodecCapabilityParameters", "rtxTime", mRTXTime);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTXCodecCapabilityParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "apt", mApt);
    IHelper::adoptElementValue(elem, "rtxTime", mRTXTime);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::RTXCodecParametersPtr IRTPTypes::RTXCodecCapabilityParameters::create(const RTXCodecParameters &params)
  {
    return make_shared<RTXCodecParameters>(params);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::RTXCodecParametersPtr IRTPTypes::RTXCodecCapabilityParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(RTXCodecParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTXCodecCapabilityParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::RTXCodecCapabilityParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTXCodecCapabilityParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::RTXCodecCapabilityParameters:");

    hasher->update(mApt);
    hasher->update(mRTXTime);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::FlexFECCodecCapabilityParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::FlexFECCodecCapabilityParameters::FlexFECCodecCapabilityParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::FlexFECCodecCapabilityParameters", "rtxTime", mRepairWindow);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::FlexFECCodecCapabilityParameters", "l", mL);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::FlexFECCodecCapabilityParameters", "d", mD);

    {
      ElementPtr subEl = elem->findFirstChildElement("toP");
      if (subEl) {
        String str = IHelper::getElementTextAndDecode(subEl);
        try {
          mToP = IRTPTypes::FlexFECCodecParameters::toToP(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("degredation preference is not valid") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::FlexFECCodecCapabilityParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "repairWindow", mRepairWindow);
    IHelper::adoptElementValue(elem, "l", mL);
    IHelper::adoptElementValue(elem, "d", mD);
    IHelper::adoptElementValue(elem, "toP", string(mToP), false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  const char *IRTPTypes::FlexFECCodecCapabilityParameters::toString(ToPs top)
  {
    switch (top) {
      case ToP_1DInterleavedFEC:      return "1d-interleaved-fec";
      case ToP_1DNonInterleavedFEC:   return "1d-non-interleaved-fec";
      case ToP_2DParityFEEC:          return "2d-parity-fec";
      case ToP_Reserved:              return "reserved";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::FlexFECCodecCapabilityParameters::ToPs IRTPTypes::FlexFECCodecCapabilityParameters::toToP(const char *top)
  {
    String topStr(top);

    for (ToPs index = ToP_First; index <= ToP_Last; index = static_cast<ToPs>(static_cast<std::underlying_type<ToPs>::type>(index) + 1)) {
      if (topStr == IRTPTypes::FlexFECCodecCapabilityParameters::toString(index)) return index;
    }

    return ToP_Reserved;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::FlexFECCodecCapabilityParameters::ToPs IRTPTypes::FlexFECCodecCapabilityParameters::toToP(ULONG value)
  {
    ToPs result = static_cast<ToPs>((std::underlying_type<ToPs>::type)(SafeInt<std::underlying_type<ToPs>::type>(value)));

    switch (value)
    {
      case ToP_1DInterleavedFEC:      return result;
      case ToP_1DNonInterleavedFEC:   return result;
      case ToP_2DParityFEEC:          return result;
      case ToP_Reserved:              return result;
    }

    ORTC_THROW_INVALID_PARAMETERS("ToP is not supported: " + string(value));
  }

  //---------------------------------------------------------------------------
  IRTPTypes::FlexFECCodecCapabilityParametersPtr IRTPTypes::FlexFECCodecCapabilityParameters::create(const FlexFECCodecParameters &capability)
  {
    return make_shared<FlexFECCodecParameters>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::FlexFECCodecCapabilityParametersPtr IRTPTypes::FlexFECCodecCapabilityParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(FlexFECCodecCapabilityParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::FlexFECCodecCapabilityParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::FlexFECCodecCapabilityParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::FlexFECCodecCapabilityParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::FlexFECCodecCapabilityParameters:");

    hasher->update(mRepairWindow);
    hasher->update(":");
    hasher->update(mL);
    hasher->update(":");
    hasher->update(mD);
    hasher->update(":");
    hasher->update(mToP);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::HeaderExtensions
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::HeaderExtension::HeaderExtension(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtension", "kind", mKind);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtension", "uri", mURI);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtension", "preferredId", mPreferredID);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtension", "preferredEncrypt", mPreferredEncrypt);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtension::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "kind", mKind, false);
    IHelper::adoptElementValue(elem, "uri", mURI, false);
    IHelper::adoptElementValue(elem, "preferredId", mPreferredID);
    IHelper::adoptElementValue(elem, "preferredEncrypt", mPreferredEncrypt);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtension::toDebug() const
  {
   return createElement("ortc::IRTPTypes::HeaderExtensions");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::HeaderExtension::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::HeaderExtensions:");

    hasher->update(mKind);
    hasher->update(":");
    hasher->update(mURI);
    hasher->update(":");
    hasher->update(mPreferredID);
    hasher->update(":");
    hasher->update(mPreferredEncrypt);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::RTCPFeedback
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::RTCPFeedback::RTCPFeedback(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPFeedback", "type", mType);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPFeedback", "parameter", mParameter);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTCPFeedback::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "type", mType, false);
    IHelper::adoptElementValue(elem, "parameter", mParameter, false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTCPFeedback::toDebug() const
  {
    return createElement("ortc::IRTPTypes::RTCPFeedback");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTCPFeedback::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::RTCPFeedback:");

    hasher->update(mType);
    hasher->update(":");
    hasher->update(mParameter);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::RTCPParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::RTCPParameters::RTCPParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPParameters", "ssrc", mSSRC);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPParameters", "cname", mCName);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPParameters", "reducedSize", mReducedSize);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPParameters", "mux", mMux);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTCPParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "ssrc", mSSRC);
    IHelper::adoptElementValue(elem, "cname", mCName, false);
    IHelper::adoptElementValue(elem, "reducedSize", mReducedSize);
    IHelper::adoptElementValue(elem, "mux", mMux);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTCPParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::RTCPParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTCPParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::RTCPParameters:");

    hasher->update(mSSRC);
    hasher->update(":");
    hasher->update(mCName);
    hasher->update(":");
    hasher->update(mReducedSize);
    hasher->update(":");
    hasher->update(mMux);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::Parameters::Parameters(const Parameters &op2) :
    mMuxID(op2.mMuxID),
    mCodecs(op2.mCodecs),
    mHeaderExtensions(op2.mHeaderExtensions),
    mEncodings(op2.mEncodings),
    mRTCP(op2.mRTCP),
    mDegredationPreference(op2.mDegredationPreference)
  {
  }

  //---------------------------------------------------------------------------
  IRTPTypes::Parameters &IRTPTypes::Parameters::operator=(const Parameters &op2)
  {
    if (this == (&op2)) return *this;

    mMuxID = op2.mMuxID;
    mCodecs = op2.mCodecs;
    mHeaderExtensions = op2.mHeaderExtensions;
    mEncodings = op2.mEncodings;
    mRTCP = op2.mRTCP;
    mDegredationPreference = op2.mDegredationPreference;

    return *this;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::Parameters::Parameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::Parameters", "muxId", mMuxID);

    {
      ElementPtr codecsEl = elem->findFirstChildElement("codecs");
      if (codecsEl) {
        ElementPtr codecEl = codecsEl->findFirstChildElement("codec");
        while (codecEl) {
          CodecParameters codec(codecEl);
          mCodecs.push_back(codec);
          codecEl = codecEl->findNextSiblingElement("codec");
        }
      }
    }

    {
      ElementPtr headerExtensionsEl = elem->findFirstChildElement("headerExtensions");
      if (headerExtensionsEl) {
        ElementPtr headerExtensionEl = headerExtensionsEl->findFirstChildElement("headerExtension");
        while (headerExtensionEl) {
          HeaderExtensionParameters ext(headerExtensionEl);
          mHeaderExtensions.push_back(ext);
          headerExtensionEl = headerExtensionEl->findNextSiblingElement("headerExtension");
        }
      }
    }

    {
      ElementPtr encodingsEl = elem->findFirstChildElement("encodings");
      if (encodingsEl) {
        ElementPtr encodingEl = encodingsEl->findFirstChildElement("encoding");
        while (encodingEl) {
          EncodingParameters encoding(encodingEl);
          mEncodings.push_back(encoding);
          encodingEl = encodingEl->findNextSiblingElement("encoding");
        }
      }
    }

    {
      ElementPtr rtcpParamsEl = elem->findFirstChildElement("rtcp");
      if (rtcpParamsEl) {
        mRTCP = RTCPParameters(rtcpParamsEl);
      }
    }

    {
      ElementPtr subEl = elem->findFirstChildElement("degredationPreference");
      if (subEl) {
        String str = IHelper::getElementTextAndDecode(subEl);
        try {
          mDegredationPreference = IRTPTypes::toDegredationPreference(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("degredation preference is not valid") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::Parameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "muxId", mMuxID, false);

    if (mCodecs.size() > 0) {
      ElementPtr codecsEl = Element::create("codecs");
      for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter) {
        auto &value = (*iter);
        codecsEl->adoptAsLastChild(value.createElement("codec"));
      }
      elem->adoptAsLastChild(codecsEl);
    }
    if (mHeaderExtensions.size() > 0) {
      ElementPtr headerExtensionsEl = Element::create("headerExtensions");
      for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter) {
        auto &value = (*iter);
        headerExtensionsEl->adoptAsLastChild(value.createElement("headerExtension"));
      }
      elem->adoptAsLastChild(headerExtensionsEl);
    }
    if (mEncodings.size() > 0) {
      ElementPtr encodingsEl = Element::create("encodings");
      for (auto iter = mEncodings.begin(); iter != mEncodings.end(); ++iter) {
        auto &value = (*iter);
        encodingsEl->adoptAsLastChild(value.createElement("encoding"));
      }
      elem->adoptAsLastChild(encodingsEl);
    }

    elem->adoptAsLastChild(mRTCP.createElement("rtcp"));

    IHelper::adoptElementValue(elem, "degredationPreference", IRTPTypes::toString(mDegredationPreference), false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::Parameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::Parameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::Parameters::hash(const HashOptions &options) const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::Parameters:");

    if (options.mMuxID) {
      hasher->update(mMuxID);
    }

    if (options.mCodecs) {
      hasher->update("codecs:0e69ea312f56834897bc0c29eb74bf991bee8d86");

      for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter) {
        auto value = (*iter);
        hasher->update(":");
        hasher->update(value.hash());
      }
    }

    if (options.mHeaderExtensions) {
      hasher->update("headers:0e69ea312f56834897bc0c29eb74bf991bee8d86");

      for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter) {
        auto value = (*iter);
        hasher->update(":");
        hasher->update(value.hash());
      }
    }

    if (options.mEncodingParameters) {
      hasher->update("encodings:0e69ea312f56834897bc0c29eb74bf991bee8d86");

      for (auto iter = mEncodings.begin(); iter != mEncodings.end(); ++iter) {
        auto value = (*iter);
        hasher->update(":");
        hasher->update(value.hash());
      }
    }

    if (options.mRTCP) {
      hasher->update("rtcp:72b2b94700e10e41adba3cdf656abed590bb65f4:");
      hasher->update(mRTCP.hash());
    }

    if (options.mDegredationPreference) {
      hasher->update("degredation:14bac0ecdadf8b017403d37459be8490:");
      hasher->update(mDegredationPreference);
    }

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::CodecParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::CodecParameters::CodecParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "name", mName);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "payloadType", mPayloadType);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "clockRate", mClockRate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "ptime", mPTime);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "maxptime", mMaxPTime);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "numChannels", mNumChannels);

    {
      ElementPtr rtcpFeedbacksEl = elem->findFirstChildElement("rtcpFeedbacks");
      if (rtcpFeedbacksEl) {
        ElementPtr rtcpFeedbackEl = rtcpFeedbacksEl->findFirstChildElement("rtcpFeedback");
        while (rtcpFeedbackEl) {
          RTCPFeedback feedback(rtcpFeedbackEl);
          mRTCPFeedback.push_back(feedback);
          rtcpFeedbackEl = rtcpFeedbackEl->findNextSiblingElement("rtcpFeedback");
        }
      }
    }

    {
      ElementPtr paramsEl = elem->findFirstChildElement("parameters");
      if (paramsEl) {
        SupportedCodecs supported = toSupportedCodec(mName);
        switch (supported) {
          case SupportedCodec_Opus: {
            mParameters = make_shared<OpusCodecParameters>(paramsEl);
            break;
          }
          case SupportedCodec_VP8: {
            mParameters = make_shared<VP8CodecParameters>(paramsEl);
            break;
          }
          case SupportedCodec_H264: {
            mParameters = make_shared<H264CodecParameters>(paramsEl);
            break;
          }
          case SupportedCodec_RTX: {
            mParameters = make_shared<RTXCodecParameters>(paramsEl);
            break;
          }
          case SupportedCodec_RED: {
            mParameters = make_shared<REDCodecParameters>(paramsEl);
            break;
          }
          case SupportedCodec_FlexFEC: {
            mParameters = make_shared<FlexFECCodecParameters>(paramsEl);
            break;
          }
          default: break;
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::CodecParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "name", mName, false);
    IHelper::adoptElementValue(elem, "payloadType", mPayloadType);
    IHelper::adoptElementValue(elem, "clockRate", mClockRate);
    if (Milliseconds() != mPTime) {
      IHelper::adoptElementValue(elem, "ptime", mPTime);
    }
    if (Milliseconds() != mMaxPTime) {
      IHelper::adoptElementValue(elem, "maxptime", mMaxPTime.count());
    }
    IHelper::adoptElementValue(elem, "numChannels", mNumChannels);

    if (mRTCPFeedback.size() > 0) {
      ElementPtr codecsEl = Element::create("rtcpFeedbacks");
      for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter) {
        auto &value = (*iter);
        codecsEl->adoptAsLastChild(value.createElement("rtcpFeedback"));
      }
      elem->adoptAsLastChild(codecsEl);
    }

    if (mParameters) {
      SupportedCodecs supported = toSupportedCodec(mName);
      switch (supported) {
        case SupportedCodec_Opus: {
          auto params = OpusCodecParameters::convert(mParameters);
          if (params) {
            elem->adoptAsLastChild(params->createElement("parameters"));
          }
          break;
        }
        case SupportedCodec_VP8: {
          auto params = VP8CodecParameters::convert(mParameters);
          if (params) {
            elem->adoptAsLastChild(params->createElement("parameters"));
          }
          break;
        }
        case SupportedCodec_H264: {
          auto params = H264CodecParameters::convert(mParameters);
          if (params) {
            elem->adoptAsLastChild(params->createElement("parameters"));
          }
          break;
        }
        case SupportedCodec_RTX: {
          auto params = RTXCodecParameters::convert(mParameters);
          if (params) {
            elem->adoptAsLastChild(params->createElement("parameters"));
          }
          break;
        }
        case SupportedCodec_RED: {
          auto params = REDCodecParameters::convert(mParameters);
          if (params) {
            elem->adoptAsLastChild(params->createElement("parameters"));
          }
          break;
        }
        case SupportedCodec_FlexFEC: {
          auto params = FlexFECCodecParameters::convert(mParameters);
          if (params) {
            elem->adoptAsLastChild(params->createElement("parameters"));
          }
          break;
        }
        default: break;
      }
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::CodecParameters::CodecParameters(const CodecParameters &source) :
    mName(source.mName),
    mPayloadType(source.mPayloadType),
    mClockRate(source.mClockRate),
    mPTime(source.mPTime),
    mMaxPTime(source.mMaxPTime),
    mNumChannels(source.mNumChannels),
    mRTCPFeedback(source.mRTCPFeedback)
  {
    if (source.mParameters) {
      SupportedCodecs supported = toSupportedCodec(source.mName);
      switch (supported) {
        case SupportedCodec_Opus: {
          auto codec = OpusCodecParameters::convert(source.mParameters);
          if (codec) {
            mParameters = OpusCodecParameters::create(*codec);
          }
          break;
        }
        case SupportedCodec_VP8: {
          auto codec = VP8CodecParameters::convert(source.mParameters);
          if (codec) {
            mParameters = VP8CodecParameters::create(*codec);
          }
          break;
        }
        case SupportedCodec_H264: {
          auto codec = H264CodecParameters::convert(source.mParameters);
          if (codec) {
            mParameters = H264CodecParameters::create(*codec);
          }
          break;
        }
        case SupportedCodec_RTX: {
          auto codec = RTXCodecParameters::convert(source.mParameters);
          if (codec) {
            mParameters = RTXCodecParameters::create(*codec);
          }
          break;
        }
        case SupportedCodec_RED: {
          auto codec = REDCodecParameters::convert(source.mParameters);
          if (codec) {
            mParameters = REDCodecParameters::create(*codec);
          }
          break;
        }
        case SupportedCodec_FlexFEC: {
          auto codec = FlexFECCodecParameters::convert(source.mParameters);
          if (codec) {
            mParameters = FlexFECCodecParameters::create(*codec);
          }
          break;
        }
        default: break;
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::CodecParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::CodecParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::CodecParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::Parameters:");

    hasher->update(mName);
    hasher->update(":");
    hasher->update(mPayloadType);
    hasher->update(":");
    hasher->update(mClockRate);
    hasher->update(":");
    hasher->update(mPTime);
    hasher->update(":");
    hasher->update(mMaxPTime);
    hasher->update(":");
    hasher->update(mNumChannels);

    hasher->update("feedback:0e69ea312f56834897bc0c29eb74bf991bee8d86");

    for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter) {
      auto value = (*iter);
      hasher->update(":");
      hasher->update(value.hash());
    }

    auto supported = toSupportedCodec(mName);

    hasher->update(":");
    if (mParameters) {
      switch (supported) {
        case SupportedCodec_Opus: {
          auto codec = OpusCodecParameters::convert(mParameters);
          if (codec) hasher->update(codec->hash());
          break;
        }
        case SupportedCodec_VP8: {
          auto codec = VP8CodecParameters::convert(mParameters);
          if (codec) hasher->update(codec->hash());
          break;
        }
        case SupportedCodec_H264: {
          auto codec = H264CodecParameters::convert(mParameters);
          if (codec) hasher->update(codec->hash());
          break;
        }
        case SupportedCodec_RTX: {
          auto codec = RTXCodecParameters::convert(mParameters);
          if (codec) hasher->update(codec->hash());
          break;
        }
        case SupportedCodec_RED: {
          auto codec = REDCodecParameters::convert(mParameters);
          if (codec) hasher->update(codec->hash());
          break;
        }
        case SupportedCodec_FlexFEC: {
          auto codec = FlexFECCodecParameters::convert(mParameters);
          if (codec) hasher->update(codec->hash());
          break;
        }
        default: break;
      }
    }

    hasher->update(":");
    hasher->update((bool)mParameters);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::OpusCodecParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecParameters::OpusCodecParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "maxPlaybackRate", mMaxPlaybackRate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "maxAverageBitrate", mMaxAverageBitrate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "cbr", mCBR);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "useInbandFec", mUseInbandFEC);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "useDtx", mUseDTX);

    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "complexity", mComplexity);

    {
      ElementPtr subEl = elem->findFirstChildElement("signal");
      if (subEl) {
        String str = IHelper::getElementTextAndDecode(subEl);
        try {
          mSignal = OpusCodecCapabilityOptions::toSignal(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("signal value is not value") + ZS_PARAM("value", str))
        }
      }
    }
    {
      ElementPtr subEl = elem->findFirstChildElement("application");
      if (subEl) {
        String str = IHelper::getElementTextAndDecode(subEl);
        try {
          mApplication = OpusCodecCapabilityOptions::toApplication(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("signal value is not value") + ZS_PARAM("value", str))
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "packetLossPerc", mPacketLossPerc);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "predictionDisabled", mPredictionDisabled);

    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "spropMaxCaptureRate", mSPropMaxCaptureRate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "spropStereo", mSPropStereo);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "maxPlaybackRate", mMaxPlaybackRate);
    IHelper::adoptElementValue(elem, "maxAverageBitrate", mMaxAverageBitrate);
    IHelper::adoptElementValue(elem, "stereo", mStereo);
    IHelper::adoptElementValue(elem, "cbr", mCBR);
    IHelper::adoptElementValue(elem, "useInbandFec", mUseInbandFEC);
    IHelper::adoptElementValue(elem, "UseDtx", mUseDTX);

    IHelper::adoptElementValue(elem, "complexity", mComplexity);
    IHelper::adoptElementValue(elem, "signal", OpusCodecCapabilityOptions::toString(mSignal), false);
    IHelper::adoptElementValue(elem, "application", OpusCodecCapabilityOptions::toString(mApplication), false);
    IHelper::adoptElementValue(elem, "packetLossPerc", mPacketLossPerc);
    IHelper::adoptElementValue(elem, "predictionDisabled", mPredictionDisabled);

    IHelper::adoptElementValue(elem, "spropMaxCaptureRate", mSPropMaxCaptureRate);
    IHelper::adoptElementValue(elem, "spropStereo", mSPropStereo);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecParametersPtr IRTPTypes::OpusCodecParameters::create(const OpusCodecParameters &capability)
  {
    return make_shared<OpusCodecParameters>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecParametersPtr IRTPTypes::OpusCodecParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(OpusCodecParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::OpusCodecParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::OpusCodecParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::OpusCodecParameters:");

    hasher->update(mComplexity);
    hasher->update(":");
    hasher->update(mSignal);
    hasher->update(":");
    hasher->update(mApplication);
    hasher->update(":");
    hasher->update(mPacketLossPerc);
    hasher->update(":");
    hasher->update(mPredictionDisabled);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::REDCodecParameters
  #pragma mark


  //---------------------------------------------------------------------------
  IRTPTypes::REDCodecParameters::REDCodecParameters(ElementPtr elem)
  {
    if (!elem) return;

    {
      ElementPtr payloadTypesEl = elem->findFirstChildElement("payloadTypes");
      if (payloadTypesEl) {
        ElementPtr payloadTypeEl = payloadTypesEl->findFirstChildElement("payloadType");
        while (payloadTypeEl) {
          decltype(mPayloadTypes)::value_type value {};
          String str = IHelper::getElementText(payloadTypeEl);
          try {
            value = Numeric<decltype(mPayloadTypes)::value_type>(str);
          } catch(const Numeric<decltype(mPayloadTypes)::value_type>::ValueOutOfRange &) {
            ZS_LOG_WARNING(Debug, slog("payload type value out of range") + ZS_PARAM("value", str))
          }
          mPayloadTypes.push_back(value);
          payloadTypeEl = payloadTypeEl->findNextSiblingElement("payloadType");
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::REDCodecParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mPayloadTypes.size() > 0) {
      ElementPtr payloadTypesEl = Element::create("payloadTypes");

      for (auto iter = mPayloadTypes.begin(); iter != mPayloadTypes.end(); ++iter) {
        auto &value = (*iter);
        IHelper::adoptElementValue(payloadTypesEl, "payloadType", value);
      }

      elem->adoptAsLastChild(payloadTypesEl);
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::REDCodecParametersPtr IRTPTypes::REDCodecParameters::create(const REDCodecParameters &capability)
  {
    return make_shared<REDCodecParameters>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::REDCodecParametersPtr IRTPTypes::REDCodecParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(REDCodecParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::REDCodecParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::REDCodecParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::REDCodecParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::REDCodecParameters:");

    hasher->update("payloadTypes");
    for (auto iter = mPayloadTypes.begin(); iter != mPayloadTypes.end(); ++iter) {
      auto payloadType = (*iter);
      hasher->update(":");
      hasher->update(payloadType);
    }
    hasher->update(":");

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::HeaderExtensionParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::HeaderExtensionParameters::HeaderExtensionParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtensionParameters", "uri", mURI);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtensionParameters", "id", mID);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtensionParameters", "encrypt", mEncrypt);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtensionParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "uri", mURI, false);
    IHelper::adoptElementValue(elem, "id", mID);
    IHelper::adoptElementValue(elem, "encrypt", mEncrypt);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtensionParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::HeaderExtensionParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::HeaderExtensionParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::HeaderExtensionParameters:");

    hasher->update(mURI);
    hasher->update(":");
    hasher->update(mID);
    hasher->update(":");
    hasher->update(mEncrypt);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::FECParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::FECParameters::FECParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::FECParameters", "ssrc", mSSRC);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::FECParameters", "mechanism", mMechanism);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::FECParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "ssrc", mSSRC);
    IHelper::adoptElementValue(elem, "mechanism", mMechanism, false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::FECParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::FECParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::FECParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::FECParameters:");

    hasher->update(mSSRC);
    hasher->update(":");
    hasher->update(mMechanism);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::RTXParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::RTXParameters::RTXParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::RTXParameters", "ssrc", mSSRC);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTXParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "ssrc", mSSRC);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTXParameters::toDebug() const
  {
    return createElement("ortc::IRTPTypes::RTXParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTXParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::RTXParameters:");

    hasher->update(mSSRC);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::PriorityTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(PriorityTypes type)
  {
    switch (type) {
      case PriorityType_VeryLow:        return "very-low";
      case PriorityType_Low:            return "low";
      case PriorityType_Medium:         return "medium";
      case PriorityType_High:           return "high";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::PriorityTypes IRTPTypes::toPriorityType(const char *type)
  {
    String typeStr(type);

    for (PriorityTypes index = PriorityType_First; index <= PriorityType_Last; index = static_cast<PriorityTypes>(static_cast<std::underlying_type<PriorityTypes>::type>(index) + 1)) {
      if (typeStr == IRTPTypes::toString(index)) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("unknown priority type");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::EncodingParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::EncodingParameters::EncodingParameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "ssrc", mSSRC);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "codecPayloadType", mCodecPayloadType);

    {
      ElementPtr subEl = elem->findFirstChildElement("fec");
      if (subEl) {
        mFEC = FECParameters(subEl);
      }
    }
    {
      ElementPtr subEl = elem->findFirstChildElement("rtx");
      if (subEl) {
        mRTX = RTXParameters(subEl);
      }
    }

    if (mFEC.hasValue()) {
      elem->adoptAsLastChild(mFEC.value().createElement("fec"));
    }
    if (mRTX.hasValue()) {
      elem->adoptAsLastChild(mRTX.value().createElement("rtx"));
    }

    {
      ElementPtr subEl = elem->findFirstChildElement("priority");
      if (subEl) {
        String str = IHelper::getElementTextAndDecode(subEl);
        try {
          mPriority = IRTPTypes::toPriorityType(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("priority type is not valid") + ZS_PARAM("value", str))
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "maxBitrate", mMaxBitrate);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "minQuality", mMinQuality);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "resolutionScale", mResolutionScale);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "framerateScale", mFramerateScale);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "active", mActive);
    IHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "encodingId", mEncodingID);

    {
      ElementPtr dependencyEncodingIDsEl = elem->findFirstChildElement("dependencyEncodingIds");
      if (dependencyEncodingIDsEl) {
        ElementPtr dependencyEncodingIDEl = dependencyEncodingIDsEl->findFirstChildElement("dependencyEncodingId");
        while (dependencyEncodingIDEl) {
          mDependencyEncodingIDs.push_back(IHelper::getElementTextAndDecode(dependencyEncodingIDEl));
          dependencyEncodingIDEl = dependencyEncodingIDEl->findNextSiblingElement("dependencyEncodingId");
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::EncodingParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "ssrc", mSSRC);
    IHelper::adoptElementValue(elem, "codecPayloadType", mCodecPayloadType);

    if (mFEC.hasValue()) {
      elem->adoptAsLastChild(mFEC.value().createElement("fec"));
    }
    if (mRTX.hasValue()) {
      elem->adoptAsLastChild(mRTX.value().createElement("rtx"));
    }

    IHelper::adoptElementValue(elem, "priority", toString(mPriority), false);
    IHelper::adoptElementValue(elem, "maxBitrate", mMaxBitrate);
    IHelper::adoptElementValue(elem, "minQuality", mMinQuality);
    IHelper::adoptElementValue(elem, "resolutionScale", mResolutionScale);
    IHelper::adoptElementValue(elem, "resolutionScale", mFramerateScale);
    IHelper::adoptElementValue(elem, "active", mActive);
    IHelper::adoptElementValue(elem, "encodingId", mEncodingID, false);

    if (mDependencyEncodingIDs.size() > 0) {
      ElementPtr dependencyEncodingIDsEl = Element::create("dependencyEncodingIds");

      for (auto iter = mDependencyEncodingIDs.begin(); iter != mDependencyEncodingIDs.end(); ++iter) {
        auto value = (*iter);
        IHelper::adoptElementValue(dependencyEncodingIDsEl, "dependencyEncodingId", value, false);
      }

      elem->adoptAsLastChild(dependencyEncodingIDsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::EncodingParameters::toDebug() const
  {
   return createElement("ortc::IRTPTypes::EncodingParameters");
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::EncodingParameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IRTPTypes::EncodingParameters:");

    hasher->update(mSSRC);
    hasher->update(":");
    hasher->update(mCodecPayloadType);
    hasher->update(":");
    hasher->update(mFEC.hasValue() ? mFEC.value().hash() : String());
    hasher->update(":");
    hasher->update(mRTX.hasValue() ? mRTX.value().hash() : String());
    hasher->update(":");
    hasher->update(mPriority);
    hasher->update(":");
    hasher->update(mMaxBitrate);
    hasher->update(":");
    hasher->update(mMinQuality);
    hasher->update(":");
    hasher->update(mResolutionScale);
    hasher->update(":");
    hasher->update(mFramerateScale);
    hasher->update(":");
    hasher->update(mActive);
    hasher->update(":");
    hasher->update(mEncodingID);

    for (auto iter = mDependencyEncodingIDs.begin(); iter != mDependencyEncodingIDs.end(); ++iter) {
      auto value = (*iter);
      hasher->update(":");
      hasher->update(value);
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::CodecKinds
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(CodecKinds kind)
  {
    switch (kind) {
      case CodecKind_Unknown:           return "";
      case CodecKind_Audio:             return "audio";
      case CodecKind_Video:             return "video";
      case CodecKind_AV:                return "av";
      case CodecKind_RTX:               return "rtx";
      case CodecKind_FEC:               return "fec";
      case CodecKind_AudioSupplemental: return "audio-supplemental";
      case CodecKind_Data:              return "data";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::CodecKinds IRTPTypes::toCodecKind(const char *kind)
  {
    String kindStr(kind);

    for (CodecKinds index = CodecKind_First; index <= CodecKind_Last; index = static_cast<CodecKinds>(static_cast<std::underlying_type<CodecKinds>::type>(index) + 1)) {
      if (kindStr == IRTPTypes::toString(index)) return index;
    }

    return CodecKind_Unknown;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::SupportedCodecs
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return "";

      case SupportedCodec_Opus:               return "opus";
      case SupportedCodec_Isac:               return "isac";
      case SupportedCodec_G722:               return "g722";
      case SupportedCodec_ILBC:               return "ilbc";
      case SupportedCodec_PCMU:               return "pcmu";
      case SupportedCodec_PCMA:               return "pcma";

      case SupportedCodec_VP8:                return "VP8";
      case SupportedCodec_VP9:                return "VP9";
      case SupportedCodec_H264:               return "H264";

      case SupportedCodec_RTX:                return "rtx";

      case SupportedCodec_RED:                return "red";
      case SupportedCodec_ULPFEC:             return "ulpfec";
      case SupportedCodec_FlexFEC:            return "flexfec";

      case SupportedCodec_CN:                 return "cn";
        
      case SupportedCodec_TelephoneEvent:     return "telephone-event";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::SupportedCodecs IRTPTypes::toSupportedCodec(const char *codec)
  {
    String codecStr(codec);

    for (SupportedCodecs index = SupportedCodec_First; index <= SupportedCodec_Last; index = static_cast<SupportedCodecs>(static_cast<std::underlying_type<SupportedCodecs>::type>(index) + 1)) {
      if (0 == codecStr.compareNoCase(IRTPTypes::toString(index))) return index;
    }

    return SupportedCodec_Unknown;
  }

  //---------------------------------------------------------------------------
  bool IRTPTypes::hasCapabilityOptions(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return false;

      case SupportedCodec_Opus:               return true;
      case SupportedCodec_Isac:               return false;
      case SupportedCodec_G722:               return false;
      case SupportedCodec_ILBC:               return false;
      case SupportedCodec_PCMU:               return false;
      case SupportedCodec_PCMA:               return false;

      case SupportedCodec_VP8:                return false;
      case SupportedCodec_VP9:                return false;
      case SupportedCodec_H264:               return false;

      case SupportedCodec_RTX:                return false;

      case SupportedCodec_RED:                return false;
      case SupportedCodec_ULPFEC:             return false;
      case SupportedCodec_FlexFEC:            return false;

      case SupportedCodec_CN:                 return false;
        
      case SupportedCodec_TelephoneEvent:     return false;
    }

    ORTC_THROW_NOT_SUPPORTED_ERRROR("missing definition for supported codec");
  }

  //---------------------------------------------------------------------------
  bool IRTPTypes::hasCapabilityParameters(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return false;

      case SupportedCodec_Opus:               return true;
      case SupportedCodec_Isac:               return false;
      case SupportedCodec_G722:               return false;
      case SupportedCodec_ILBC:               return false;
      case SupportedCodec_PCMU:               return false;
      case SupportedCodec_PCMA:               return false;

      case SupportedCodec_VP8:                return true;
      case SupportedCodec_VP9:                return false;
      case SupportedCodec_H264:               return true;

      case SupportedCodec_RTX:                return true;

      case SupportedCodec_RED:                return false;
      case SupportedCodec_ULPFEC:             return false;
      case SupportedCodec_FlexFEC:            return true;

      case SupportedCodec_CN:                 return false;
        
      case SupportedCodec_TelephoneEvent:     return false;
    }

    ORTC_THROW_NOT_SUPPORTED_ERRROR("missing definition for supported codec");
  }

  //---------------------------------------------------------------------------
  bool IRTPTypes::hasParameters(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return false;

      case SupportedCodec_Opus:               return true;
      case SupportedCodec_Isac:               return false;
      case SupportedCodec_G722:               return false;
      case SupportedCodec_ILBC:               return false;
      case SupportedCodec_PCMU:               return false;
      case SupportedCodec_PCMA:               return false;

      case SupportedCodec_VP8:                return true;
      case SupportedCodec_VP9:                return false;
      case SupportedCodec_H264:               return true;

      case SupportedCodec_RTX:                return true;

      case SupportedCodec_RED:                return true;
      case SupportedCodec_ULPFEC:             return false;
      case SupportedCodec_FlexFEC:            return true;

      case SupportedCodec_CN:                 return false;
        
      case SupportedCodec_TelephoneEvent:     return false;
    }

    ORTC_THROW_NOT_SUPPORTED_ERRROR("missing definition for supported codec");
  }

  //---------------------------------------------------------------------------
  bool IRTPTypes::requiresCapabilityParameters(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return false;

      case SupportedCodec_Opus:               return false;
      case SupportedCodec_Isac:               return false;
      case SupportedCodec_G722:               return false;
      case SupportedCodec_ILBC:               return false;
      case SupportedCodec_PCMU:               return false;
      case SupportedCodec_PCMA:               return false;

      case SupportedCodec_VP8:                return false;
      case SupportedCodec_VP9:                return false;
      case SupportedCodec_H264:               return false;

      case SupportedCodec_RTX:                return true;

      case SupportedCodec_RED:                return false;
      case SupportedCodec_ULPFEC:             return false;
      case SupportedCodec_FlexFEC:            return true;

      case SupportedCodec_CN:                 return false;

      case SupportedCodec_TelephoneEvent:     return false;
    }

    ORTC_THROW_NOT_SUPPORTED_ERRROR("missing definition for supported codec");
  }

  //---------------------------------------------------------------------------
  bool IRTPTypes::requiresParameters(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return false;

      case SupportedCodec_Opus:               return false;
      case SupportedCodec_Isac:               return false;
      case SupportedCodec_G722:               return false;
      case SupportedCodec_ILBC:               return false;
      case SupportedCodec_PCMU:               return false;
      case SupportedCodec_PCMA:               return false;

      case SupportedCodec_VP8:                return false;
      case SupportedCodec_VP9:                return false;
      case SupportedCodec_H264:               return false;

      case SupportedCodec_RTX:                return true;

      case SupportedCodec_RED:                return false;
      case SupportedCodec_ULPFEC:             return false;
      case SupportedCodec_FlexFEC:            return true;

      case SupportedCodec_CN:                 return false;

      case SupportedCodec_TelephoneEvent:     return false;
    }

    ORTC_THROW_NOT_SUPPORTED_ERRROR("missing definition for supported codec");
  }

  //---------------------------------------------------------------------------
  IRTPTypes::CodecKinds IRTPTypes::getCodecKind(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return CodecKind_Unknown;

      case SupportedCodec_Opus:               return CodecKind_Audio;
      case SupportedCodec_Isac:               return CodecKind_Audio;
      case SupportedCodec_G722:               return CodecKind_Audio;
      case SupportedCodec_ILBC:               return CodecKind_Audio;
      case SupportedCodec_PCMU:               return CodecKind_Audio;
      case SupportedCodec_PCMA:               return CodecKind_Audio;

      case SupportedCodec_VP8:                return CodecKind_Video;
      case SupportedCodec_VP9:                return CodecKind_Video;
      case SupportedCodec_H264:               return CodecKind_Video;

      case SupportedCodec_RTX:                return CodecKind_RTX;

      case SupportedCodec_RED:                return CodecKind_FEC;
      case SupportedCodec_ULPFEC:             return CodecKind_FEC;
      case SupportedCodec_FlexFEC:            return CodecKind_FEC;

      case SupportedCodec_CN:                 return CodecKind_AudioSupplemental;

      case SupportedCodec_TelephoneEvent:     return CodecKind_AudioSupplemental;
    }
    
    return CodecKind_Unknown;
  }

  //---------------------------------------------------------------------------
  bool IRTPTypes::isSRSTCodec(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return false;

      case SupportedCodec_Opus:               return false;
      case SupportedCodec_Isac:               return false;
      case SupportedCodec_G722:               return false;
      case SupportedCodec_ILBC:               return false;
      case SupportedCodec_PCMU:               return false;
      case SupportedCodec_PCMA:               return false;

      case SupportedCodec_VP8:                return true;
      case SupportedCodec_VP9:                return true;
      case SupportedCodec_H264:               return true;

      case SupportedCodec_RTX:                return false;

      case SupportedCodec_RED:                return false;
      case SupportedCodec_ULPFEC:             return false;
      case SupportedCodec_FlexFEC:            return false;

      case SupportedCodec_CN:                 return false;

      case SupportedCodec_TelephoneEvent:     return false;
    }

    return false;
  }
  

  //---------------------------------------------------------------------------
  bool IRTPTypes::isMRSTCodec(SupportedCodecs codec)
  {
    switch (codec) {
      case SupportedCodec_Unknown:            return false;

      case SupportedCodec_Opus:               return false;
      case SupportedCodec_Isac:               return false;
      case SupportedCodec_G722:               return false;
      case SupportedCodec_ILBC:               return false;
      case SupportedCodec_PCMU:               return false;
      case SupportedCodec_PCMA:               return false;

      case SupportedCodec_VP8:                return false;
      case SupportedCodec_VP9:                return false;
      case SupportedCodec_H264:               return false;

      case SupportedCodec_RTX:                return false;

      case SupportedCodec_RED:                return false;
      case SupportedCodec_ULPFEC:             return false;
      case SupportedCodec_FlexFEC:            return false;

      case SupportedCodec_CN:                 return false;

      case SupportedCodec_TelephoneEvent:     return false;
    }
    
    return false;
  }
  

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::ReservedCodecPayloadTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(ReservedCodecPayloadTypes reservedCodec)
  {
    switch (reservedCodec) {
      case ReservedCodecPayloadType_Unknown:      return "";
      case ReservedCodecPayloadType_PCMU_8000:    return "pcmu";

      case ReservedCodecPayloadType_GSM_8000:     return "gsm";
      case ReservedCodecPayloadType_G723_8000:    return "g723";
      case ReservedCodecPayloadType_DVI4_8000:    return "dvi4";
      case ReservedCodecPayloadType_DVI4_16000:   return "dvi4";
      case ReservedCodecPayloadType_LPC_8000:     return "lpc";
      case ReservedCodecPayloadType_PCMA_8000:    return "pcma";
      case ReservedCodecPayloadType_G722_8000:    return "g722";
      case ReservedCodecPayloadType_L16_44100_2:  return "l16";
      case ReservedCodecPayloadType_L16_44100_1:  return "l16";
      case ReservedCodecPayloadType_QCELP_8000:   return "qcelp";
      case ReservedCodecPayloadType_CN_8000:      return "cn";
      case ReservedCodecPayloadType_MPA_90000:    return "mpa";
      case ReservedCodecPayloadType_G728_8000:    return "g728";
      case ReservedCodecPayloadType_DVI4_11025:   return "dvi4";
      case ReservedCodecPayloadType_DVI4_22050:   return "dvi4";
      case ReservedCodecPayloadType_G729_8000:    return "g729";

      case ReservedCodecPayloadType_CelB_90000:   return "CelB";
      case ReservedCodecPayloadType_JPEG_90000:   return "jpeg";

      case ReservedCodecPayloadType_nv_90000:     return "nv";

      case ReservedCodecPayloadType_H261_90000:   return "H261";
      case ReservedCodecPayloadType_MPV_90000:    return "MPV";
      case ReservedCodecPayloadType_MP2T_90000:   return "MP2T";
      case ReservedCodecPayloadType_H263_90000:   return "H263";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::ReservedCodecPayloadTypes IRTPTypes::toReservedCodec(PayloadType pt)
  {
    auto result = static_cast<ReservedCodecPayloadTypes>(pt);

    switch (result) {
      case ReservedCodecPayloadType_Unknown:      break;
      case ReservedCodecPayloadType_PCMU_8000:    return result;

      case ReservedCodecPayloadType_GSM_8000:     return result;
      case ReservedCodecPayloadType_G723_8000:    return result;
      case ReservedCodecPayloadType_DVI4_8000:    return result;
      case ReservedCodecPayloadType_DVI4_16000:   return result;
      case ReservedCodecPayloadType_LPC_8000:     return result;
      case ReservedCodecPayloadType_PCMA_8000:    return result;
      case ReservedCodecPayloadType_G722_8000:    return result;
      case ReservedCodecPayloadType_L16_44100_2:  return result;
      case ReservedCodecPayloadType_L16_44100_1:  return result;
      case ReservedCodecPayloadType_QCELP_8000:   return result;
      case ReservedCodecPayloadType_CN_8000:      return result;
      case ReservedCodecPayloadType_MPA_90000:    return result;
      case ReservedCodecPayloadType_G728_8000:    return result;
      case ReservedCodecPayloadType_DVI4_11025:   return result;
      case ReservedCodecPayloadType_DVI4_22050:   return result;
      case ReservedCodecPayloadType_G729_8000:    return result;

      case ReservedCodecPayloadType_CelB_90000:   return result;
      case ReservedCodecPayloadType_JPEG_90000:   return result;

      case ReservedCodecPayloadType_nv_90000:     return result;

      case ReservedCodecPayloadType_H261_90000:   return result;
      case ReservedCodecPayloadType_MPV_90000:    return result;
      case ReservedCodecPayloadType_MP2T_90000:   return result;
      case ReservedCodecPayloadType_H263_90000:   return result;
    }

    return ReservedCodecPayloadType_Unknown;
  }


  //---------------------------------------------------------------------------
  IRTPTypes::ReservedCodecPayloadTypes IRTPTypes::toReservedCodec(const char *encodingName)
  {
    static ReservedCodecPayloadTypes types[] = {
      ReservedCodecPayloadType_PCMU_8000,

      ReservedCodecPayloadType_GSM_8000,
      ReservedCodecPayloadType_G723_8000,
      ReservedCodecPayloadType_DVI4_8000,
      ReservedCodecPayloadType_DVI4_16000,
      ReservedCodecPayloadType_LPC_8000,
      ReservedCodecPayloadType_PCMA_8000,
      ReservedCodecPayloadType_G722_8000,
      ReservedCodecPayloadType_L16_44100_2,
      ReservedCodecPayloadType_L16_44100_1,
      ReservedCodecPayloadType_QCELP_8000,
      ReservedCodecPayloadType_CN_8000,
      ReservedCodecPayloadType_MPA_90000,
      ReservedCodecPayloadType_G728_8000,
      ReservedCodecPayloadType_DVI4_11025,
      ReservedCodecPayloadType_DVI4_22050,
      ReservedCodecPayloadType_G729_8000,

      ReservedCodecPayloadType_CelB_90000,
      ReservedCodecPayloadType_JPEG_90000,

      ReservedCodecPayloadType_nv_90000,

      ReservedCodecPayloadType_H261_90000,
      ReservedCodecPayloadType_MPV_90000,
      ReservedCodecPayloadType_MP2T_90000,
      ReservedCodecPayloadType_H263_90000,

      ReservedCodecPayloadType_Unknown
    };

    String encodingNameStr(encodingName);

    for (size_t index = 0; ReservedCodecPayloadType_Unknown != types[index]; ++index) {
      if (0 == encodingNameStr.compareNoCase(toString(types[index]))) return types[index];
    }
    return ReservedCodecPayloadType_Unknown;
  }

  //---------------------------------------------------------------------------
  ULONG IRTPTypes::getDefaultClockRate(ReservedCodecPayloadTypes reservedCodec)
  {
    switch (reservedCodec) {
      case ReservedCodecPayloadType_Unknown:      return 0;
      case ReservedCodecPayloadType_PCMU_8000:    return 8000;

      case ReservedCodecPayloadType_GSM_8000:     return 8000;
      case ReservedCodecPayloadType_G723_8000:    return 8000;
      case ReservedCodecPayloadType_DVI4_8000:    return 8000;
      case ReservedCodecPayloadType_DVI4_16000:   return 16000;
      case ReservedCodecPayloadType_LPC_8000:     return 8000;
      case ReservedCodecPayloadType_PCMA_8000:    return 8000;
      case ReservedCodecPayloadType_G722_8000:    return 8000;
      case ReservedCodecPayloadType_L16_44100_2:  return 44100;
      case ReservedCodecPayloadType_L16_44100_1:  return 44100;
      case ReservedCodecPayloadType_QCELP_8000:   return 8000;
      case ReservedCodecPayloadType_CN_8000:      return 8000;
      case ReservedCodecPayloadType_MPA_90000:    return 90000;
      case ReservedCodecPayloadType_G728_8000:    return 8000;
      case ReservedCodecPayloadType_DVI4_11025:   return 11025;
      case ReservedCodecPayloadType_DVI4_22050:   return 22050;
      case ReservedCodecPayloadType_G729_8000:    return 8000;

      case ReservedCodecPayloadType_CelB_90000:   return 90000;
      case ReservedCodecPayloadType_JPEG_90000:   return 90000;

      case ReservedCodecPayloadType_nv_90000:     return 90000;

      case ReservedCodecPayloadType_H261_90000:   return 90000;
      case ReservedCodecPayloadType_MPV_90000:    return 90000;
      case ReservedCodecPayloadType_MP2T_90000:   return 90000;
      case ReservedCodecPayloadType_H263_90000:   return 90000;
    }
    
    return 0;
  }

  //---------------------------------------------------------------------------
  Optional<ULONG> IRTPTypes::getDefaultNumberOfChannels(ReservedCodecPayloadTypes reservedCodec)
  {
    Optional<ULONG> result;

    switch (reservedCodec) {
      case ReservedCodecPayloadType_Unknown:      break;
      case ReservedCodecPayloadType_PCMU_8000:    result = 1; break;

      case ReservedCodecPayloadType_GSM_8000:     result = 1; break;
      case ReservedCodecPayloadType_G723_8000:    result = 1; break;
      case ReservedCodecPayloadType_DVI4_8000:    result = 1; break;
      case ReservedCodecPayloadType_DVI4_16000:   result = 1; break;
      case ReservedCodecPayloadType_LPC_8000:     result = 1; break;
      case ReservedCodecPayloadType_PCMA_8000:    result = 1; break;
      case ReservedCodecPayloadType_G722_8000:    result = 1; break;
      case ReservedCodecPayloadType_L16_44100_2:  result = 2; break;
      case ReservedCodecPayloadType_L16_44100_1:  result = 1; break;
      case ReservedCodecPayloadType_QCELP_8000:   result = 1; break;
      case ReservedCodecPayloadType_CN_8000:      result = 1; break;
      case ReservedCodecPayloadType_MPA_90000:    break;
      case ReservedCodecPayloadType_G728_8000:    result = 1; break;
      case ReservedCodecPayloadType_DVI4_11025:   result = 1; break;
      case ReservedCodecPayloadType_DVI4_22050:   result = 1; break;
      case ReservedCodecPayloadType_G729_8000:    result = 1; break;

      case ReservedCodecPayloadType_CelB_90000:   break;
      case ReservedCodecPayloadType_JPEG_90000:   break;

      case ReservedCodecPayloadType_nv_90000:     break;

      case ReservedCodecPayloadType_H261_90000:   break;
      case ReservedCodecPayloadType_MPV_90000:    break;
      case ReservedCodecPayloadType_MP2T_90000:   break;
      case ReservedCodecPayloadType_H263_90000:   break;
    }

    return result;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::CodecKinds IRTPTypes::getCodecKind(ReservedCodecPayloadTypes reservedCodec)
  {
    switch (reservedCodec) {
      case ReservedCodecPayloadType_Unknown:      return CodecKind_Unknown;
      case ReservedCodecPayloadType_PCMU_8000:    return CodecKind_Audio;

      case ReservedCodecPayloadType_GSM_8000:     return CodecKind_Audio;
      case ReservedCodecPayloadType_G723_8000:    return CodecKind_Audio;
      case ReservedCodecPayloadType_DVI4_8000:    return CodecKind_Audio;
      case ReservedCodecPayloadType_DVI4_16000:   return CodecKind_Audio;
      case ReservedCodecPayloadType_LPC_8000:     return CodecKind_Audio;
      case ReservedCodecPayloadType_PCMA_8000:    return CodecKind_Audio;
      case ReservedCodecPayloadType_G722_8000:    return CodecKind_Audio;
      case ReservedCodecPayloadType_L16_44100_2:  return CodecKind_Audio;
      case ReservedCodecPayloadType_L16_44100_1:  return CodecKind_Audio;
      case ReservedCodecPayloadType_QCELP_8000:   return CodecKind_Audio;
      case ReservedCodecPayloadType_CN_8000:      return CodecKind_Audio;
      case ReservedCodecPayloadType_MPA_90000:    return CodecKind_Video;
      case ReservedCodecPayloadType_G728_8000:    return CodecKind_Audio;
      case ReservedCodecPayloadType_DVI4_11025:   return CodecKind_Audio;
      case ReservedCodecPayloadType_DVI4_22050:   return CodecKind_Audio;
      case ReservedCodecPayloadType_G729_8000:    return CodecKind_Audio;

      case ReservedCodecPayloadType_CelB_90000:   return CodecKind_Audio;
      case ReservedCodecPayloadType_JPEG_90000:   return CodecKind_Video;

      case ReservedCodecPayloadType_nv_90000:     return CodecKind_Video;

      case ReservedCodecPayloadType_H261_90000:   return CodecKind_Video;
      case ReservedCodecPayloadType_MPV_90000:    return CodecKind_Video;
      case ReservedCodecPayloadType_MP2T_90000:   return CodecKind_AV;
      case ReservedCodecPayloadType_H263_90000:   return CodecKind_Video;
    }
    
    return CodecKind_Unknown;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::SupportedCodecs IRTPTypes::toSupportedCodec(ReservedCodecPayloadTypes reservedCodec)
  {
    switch (reservedCodec) {
      case ReservedCodecPayloadType_Unknown:      return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_PCMU_8000:    return SupportedCodec_PCMU;

      case ReservedCodecPayloadType_GSM_8000:     return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_G723_8000:    return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_DVI4_8000:    return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_DVI4_16000:   return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_LPC_8000:     return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_PCMA_8000:    return SupportedCodec_PCMA;
      case ReservedCodecPayloadType_G722_8000:    return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_L16_44100_2:  return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_L16_44100_1:  return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_QCELP_8000:   return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_CN_8000:      return SupportedCodec_CN;
      case ReservedCodecPayloadType_MPA_90000:    return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_G728_8000:    return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_DVI4_11025:   return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_DVI4_22050:   return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_G729_8000:    return SupportedCodec_Unknown;

      case ReservedCodecPayloadType_CelB_90000:   return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_JPEG_90000:   return SupportedCodec_Unknown;

      case ReservedCodecPayloadType_nv_90000:     return SupportedCodec_Unknown;

      case ReservedCodecPayloadType_H261_90000:   return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_MPV_90000:    return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_MP2T_90000:   return SupportedCodec_Unknown;
      case ReservedCodecPayloadType_H263_90000:   return SupportedCodec_Unknown;
    }

    return SupportedCodec_Unknown;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::HeaderExtensionURIs
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(HeaderExtensionURIs extension)
  {
    switch (extension) {
      case HeaderExtensionURI_Unknown:                            return "";
      case HeaderExtensionURI_MuxID:                              return "urn:ietf:params:rtp-hdrext:sdes:mid";
      case HeaderExtensionURI_ClienttoMixerAudioLevelIndication:  return "urn:ietf:params:rtp-hdrext:ssrc-audio-level";
      case HeaderExtensionURI_MixertoClientAudioLevelIndication:  return "urn:ietf:params:rtp-hdrext:csrc-audio-level";
      case HeaderExtensionURI_FrameMarking:                       return "urn:ietf:params:rtp-hdrext:framemarkinginfo";
      case HeaderExtensionURI_RID:                                return "urn:ietf:params:rtp-hdrext:rid";
      case HeaderExtensionURI_3gpp_VideoOrientation:              return "urn:3gpp:video-orientation";
      case HeaderExtensionURI_3gpp_VideoOrientation6:             return "urn:3gpp:video-orientation:6";
      case HeaderExtensionURI_TransmissionTimeOffsets:            return "urn:ietf:params:rtp-hdrext:toffset";
      case HeaderExtensionURI_AbsoluteSendTime:                   return "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";
      case HeaderExtensionURI_TransportSequenceNumber:            return "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01";
    }

    ORTC_THROW_NOT_SUPPORTED_ERRROR("header extension uri is not known");
  }

  //---------------------------------------------------------------------------
  IRTPTypes::HeaderExtensionURIs IRTPTypes::toHeaderExtensionURI(const char *uri)
  {
    String uriStr(uri);

    for (HeaderExtensionURIs index = HeaderExtensionURI_First; index <= HeaderExtensionURI_Last; index = static_cast<HeaderExtensionURIs>(static_cast<std::underlying_type<HeaderExtensionURIs>::type>(index) + 1)) {
      if (uriStr == IRTPTypes::toString(index)) return index;
    }

    return HeaderExtensionURI_Unknown;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::toKind(HeaderExtensionURIs extension)
  {
    switch (extension) {
      case HeaderExtensionURI_Unknown:                            return "";
      case HeaderExtensionURI_MuxID:                              return "";
      case HeaderExtensionURI_ClienttoMixerAudioLevelIndication:  return toString(CodecKind_Audio);
      case HeaderExtensionURI_MixertoClientAudioLevelIndication:  return toString(CodecKind_Audio);
      case HeaderExtensionURI_FrameMarking:                       return toString(CodecKind_Video);
      case HeaderExtensionURI_RID:                                return "";
      case HeaderExtensionURI_3gpp_VideoOrientation:              return toString(CodecKind_Video);
      case HeaderExtensionURI_3gpp_VideoOrientation6:             return toString(CodecKind_Video);
      case HeaderExtensionURI_TransmissionTimeOffsets:            return "";
      case HeaderExtensionURI_AbsoluteSendTime:                   return "";
      case HeaderExtensionURI_TransportSequenceNumber:            return "";
    }

    ORTC_THROW_NOT_SUPPORTED_ERRROR("header extension uri is not known");
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::KnownFeedbackTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(KnownFECMechanisms mechanism)
  {
    switch (mechanism) {
      case KnownFECMechanism_Unknown:       return "";
      case KnownFECMechanism_RED:           return "red";
      case KnownFECMechanism_RED_ULPFEC:    return "red+ulpfec";
      case KnownFECMechanism_FlexFEC:       return "flexfec";
    }

    return "unknown";  
  }

  //---------------------------------------------------------------------------
  IRTPTypes::KnownFECMechanisms IRTPTypes::toKnownFECMechanism(const char *mechanism)
  {
    String mechanismStr(mechanism);

    for (KnownFECMechanisms index = KnownFECMechanism_First; index <= KnownFECMechanism_Last; index = static_cast<KnownFECMechanisms>(static_cast<std::underlying_type<KnownFECMechanisms>::type>(index) + 1)) {
      if (mechanismStr == IRTPTypes::toString(index)) return index;
    }

    return KnownFECMechanism_Unknown;
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::KnownFeedbackTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(KnownFeedbackTypes type)
  {
    switch (type) {
      case KnownFeedbackType_Unknown:               return "";

      case KnownFeedbackType_ACK:                   return "ack";
      case KnownFeedbackType_APP:                   return "app";
      case KnownFeedbackType_CCM:                   return "ccm";
      case KnownFeedbackType_NACK:                  return "nack";
      case KnownFeedbackType_TRR_INT:               return "trr-int";
      case KnownFeedbackType_3gpp_roi_arbitrary:    return "3gpp-roi-arbitrary";
      case KnownFeedbackType_3gpp_roi_predefined:   return "3gpp-roi-predefined";
      case KnownFeedbackType_REMB:                  return "goog-remb";
      case KnownFeedbackType_transport_cc:          return "transport-cc";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::KnownFeedbackTypes IRTPTypes::toKnownFeedbackType(const char *type)
  {
    String typeStr(type);

    for (KnownFeedbackTypes index = KnownFeedbackType_First; index <= KnownFeedbackType_Last; index = static_cast<KnownFeedbackTypes>(static_cast<std::underlying_type<KnownFeedbackTypes>::type>(index) + 1)) {
      if (0 == typeStr.compareNoCase(IRTPTypes::toString(index))) return index;
    }

    return KnownFeedbackType_Unknown;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::KnownFeedbackParameters
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(KnownFeedbackParameters mechanism)
  {
    switch (mechanism) {
      case KnownFeedbackParameter_Unknown:    return "";

      case KnownFeedbackParameter_SLI:        return "sli";
      case KnownFeedbackParameter_PLI:        return "pli";
      case KnownFeedbackParameter_RPSI:       return "rpsi";
      case KnownFeedbackParameter_APP:        return "app";
      case KnownFeedbackParameter_RAI:        return "rai";
      case KnownFeedbackParameter_TLLEI:      return "tllei";
      case KnownFeedbackParameter_PSLEI:      return "pslei";
      case KnownFeedbackParameter_FIR:        return "fir";
      case KnownFeedbackParameter_TMMBR:      return "tmmbr";
      case KnownFeedbackParameter_TSTR:       return "tstr";
      case KnownFeedbackParameter_VBCM:       return "vbcm";
      case KnownFeedbackParameter_PAUSE:      return "pause";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::KnownFeedbackParameters IRTPTypes::toKnownFeedbackParameter(const char *mechanism)
  {
    String mechanismStr(mechanism);

    for (KnownFeedbackParameters index = KnownFeedbackParameter_First; index <= KnownFeedbackParameter_Last; index = static_cast<KnownFeedbackParameters>(static_cast<std::underlying_type<KnownFeedbackParameters>::type>(index) + 1)) {
      if (0 == mechanismStr.compareNoCase(IRTPTypes::toString(index))) return index;
    }

    return KnownFeedbackParameter_Unknown;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::KnownFeedbackTypesSet IRTPTypes::getUseableWithFeedbackTypes(KnownFeedbackParameters mechanism)
  {
    KnownFeedbackTypesSet result;

    switch (mechanism) {
      case KnownFeedbackParameter_Unknown:    break;

      case KnownFeedbackParameter_SLI:
      case KnownFeedbackParameter_PLI:
      case KnownFeedbackParameter_RAI:
      case KnownFeedbackParameter_TLLEI:
      case KnownFeedbackParameter_PSLEI:      {
        result.insert(KnownFeedbackType_NACK);
        break;
      }
      case KnownFeedbackParameter_RPSI:
      case KnownFeedbackParameter_APP:        {
        result.insert(KnownFeedbackType_ACK);
        result.insert(KnownFeedbackType_NACK);
        break;
      }
      case KnownFeedbackParameter_FIR:
      case KnownFeedbackParameter_TMMBR:
      case KnownFeedbackParameter_TSTR:
      case KnownFeedbackParameter_VBCM:
      case KnownFeedbackParameter_PAUSE:      {
        result.insert(KnownFeedbackType_CCM);
        break;
      }
    }
    return result;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::SupportedRTCPMechanisms
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::toString(SupportedRTCPMechanisms mechanism)
  {
    switch (mechanism) {
      case SupportedRTCPMechanism_Unknown:   return "";

      case SupportedRTCPMechanism_SR:        return "sr";
      case SupportedRTCPMechanism_RR:        return "rr";
      case SupportedRTCPMechanism_SDES:      return "sdes";
      case SupportedRTCPMechanism_BYE:       return "bye";
      case SupportedRTCPMechanism_RTPFB:     return "rtpfb";
      case SupportedRTCPMechanism_PSFB:      return "psfb";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::SupportedRTCPMechanisms IRTPTypes::toSupportedRTCPMechanism(const char *mechanism)
  {
    String mechanismStr(mechanism);

    for (SupportedRTCPMechanisms index = SupportedRTCPMechanism_First; index <= SupportedRTCPMechanism_Last; index = static_cast<SupportedRTCPMechanisms>(static_cast<std::underlying_type<SupportedRTCPMechanisms>::type>(index) + 1)) {
      if (0 == mechanismStr.compareNoCase(IRTPTypes::toString(index))) return index;
    }

    return SupportedRTCPMechanism_Unknown;
  }
}
