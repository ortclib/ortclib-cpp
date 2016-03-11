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
#include <ortc/internal/ortc_Helper.h>

#include <ortc/IRTPTypes.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

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

      UseServicesHelper::debugAppend(resultEl, "payload type", mPayloadType);
      UseServicesHelper::debugAppend(resultEl, "kind", mKind);
      UseServicesHelper::debugAppend(resultEl, "allow neutral kind", mAllowNeutralKind);
      UseServicesHelper::debugAppend(resultEl, "codec kind", mCodecKind.hasValue() ? IRTPTypes::toString(mCodecKind.value()) : (const char *)NULL);
      UseServicesHelper::debugAppend(resultEl, "supported codec", mSupportedCodec.hasValue() ? IRTPTypes::toString(mSupportedCodec.value()) : (const char *)NULL);
      UseServicesHelper::debugAppend(resultEl, "clock rate", mClockRate);
      UseServicesHelper::debugAppend(resultEl, "match clock rate not set", mMatchClockRateNotSet);
      UseServicesHelper::debugAppend(resultEl, "disallowed matches", mDisallowedPayloadtypeMatches.size());

      if (mDisallowedPayloadtypeMatches.size() > 0) {
        ElementPtr disallowEl = Element::create("disallowed");
        for (auto iter = mDisallowedPayloadtypeMatches.begin(); iter != mDisallowedPayloadtypeMatches.end(); ++iter) {
          UseServicesHelper::debugAppend(resultEl, "disallowed payload", *iter);
        }
        UseServicesHelper::debugAppend(resultEl, disallowEl);
      }
      UseServicesHelper::debugAppend(resultEl, "disallow multiple matches", mDisallowMultipleMatches);

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

      bool checkMaxPTime = false;
      bool checkNumChannels = true;

      // do codec specific compatibility test(s)
      switch (supportedCodec) {
        case IRTPTypes::SupportedCodec_Unknown:
        {
          break;
        }
        case IRTPTypes::SupportedCodec_Opus:              break;
        case IRTPTypes::SupportedCodec_Isac:              break;
        case IRTPTypes::SupportedCodec_G722:              break;
        case IRTPTypes::SupportedCodec_ILBC:              break;
        case IRTPTypes::SupportedCodec_PCMU:              break;
        case IRTPTypes::SupportedCodec_PCMA:              break;

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

      if (checkMaxPTime) {
        if (oldCodec.mMaxPTime != newCodec.mMaxPTime) return false;       // not compatible
      } else {
        ioRank += (oldCodec.mMaxPTime == newCodec.mMaxPTime ? 0.01f : -0.01f);
      }
      if (checkNumChannels) {
        if (oldCodec.mNumChannels != newCodec.mNumChannels) return false; // not compatible

        ioRank += (oldCodec.mNumChannels == newCodec.mNumChannels ? 0.01f : -0.01f);
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

      for (auto iter = params.mCodecs.begin(); iter != params.mCodecs.end(); ++iter) {
        auto &codec = (*iter);

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

        if ((options.mDisallowMultipleMatches.hasValue()) &&
            (options.mDisallowMultipleMatches.value())) {
          if (foundCodec) {
            ZS_LOG_WARNING(Trace, slog("codec choice is ambiguous") + ZS_PARAM("found", codec.toDebug()) + ZS_PARAM("previously found", foundCodec->toDebug()) + options.toDebug())
            return NULL;
          }
          foundCodec = &codec;
        } else {
          return &codec;
        }
      }

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
    const RTPTypesHelper::CodecParameters *RTPTypesHelper::pickRTXCodec(
                                                                        Optional<IMediaStreamTrackTypes::Kinds> kind,
                                                                        const Parameters &params,
                                                                        Optional<PayloadType> packetRTXPayloadType,
                                                                        const EncodingParameters *encoding,
                                                                        const EncodingParameters *baseEncoding
                                                                        )
    {
      Optional<PayloadType> rtxPayloadType;

      bool foundEncoding = false;
      bool usesRTX = false;

      if ((!rtxPayloadType.hasValue()) &&
          (encoding)) {
        foundEncoding = true;
        if (encoding->mRTX.hasValue()) {
          usesRTX = true;
          rtxPayloadType = encoding->mRTX.value().mPayloadType;
        }
      }

      if ((!rtxPayloadType.hasValue()) &&
          (baseEncoding)) {
        foundEncoding = true;
        if (baseEncoding->mRTX.hasValue()) {
          usesRTX = true;
          rtxPayloadType = baseEncoding->mRTX.value().mPayloadType;
        }
      }

      if (!rtxPayloadType.hasValue()) {
        if (params.mEncodings.size() > 0) {
          auto &frontEncoding = params.mEncodings.front();
          foundEncoding = true;
          if (frontEncoding.mRTX.hasValue()) {
            usesRTX = true;
            rtxPayloadType = frontEncoding.mRTX.value().mPayloadType;
          }
        }
      }

      if (packetRTXPayloadType.hasValue()) {
        if (rtxPayloadType.hasValue()) {
          if (rtxPayloadType.value() != packetRTXPayloadType.value()) {
            ZS_LOG_INSANE(slog("cannot match RTX codec as RTX payload type specified do not match encoding") + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()))
            return NULL;
          }
        } else {
          rtxPayloadType = packetRTXPayloadType;
        }
      }

      if ((!usesRTX) &&
          (foundEncoding)) {
        ZS_LOG_INSANE(slog("encoding is not using RTX") + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()))
        return NULL;
      }

      auto mainCodec = pickCodec(kind, params, Optional<PayloadType>(), encoding, baseEncoding);
      if (!mainCodec) return NULL;

      FindCodecOptions options;

      options.mPayloadType = rtxPayloadType;
      options.mClockRate = mainCodec->mClockRate;
      options.mMatchClockRateNotSet = true;
      options.mCodecKind = IRTPTypes::CodecKind_RTX;
      options.mDisallowMultipleMatches = true;

      auto foundRTXCodec = findCodec(params, options);

      if (!foundRTXCodec) {
        ZS_LOG_WARNING(Debug, slog("did not find an appropriate RTX codec") + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()) + options.toDebug())
        return NULL;
      }

      ZS_LOG_TRACE(slog("found RTX codec") + foundRTXCodec->toDebug() + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()) + options.toDebug())
      return foundRTXCodec;
    }

    //-------------------------------------------------------------------------
    const RTPTypesHelper::CodecParameters *RTPTypesHelper::pickFECCodec(
                                                                        Optional<IMediaStreamTrackTypes::Kinds> kind,
                                                                        const Parameters &params,
                                                                        Optional<PayloadType> packetFECPayloadType,
                                                                        const EncodingParameters *encoding,
                                                                        const EncodingParameters *baseEncoding
                                                                        )
    {
      IRTPTypes::KnownFECMechanisms mechanism = IRTPTypes::KnownFECMechanism_Unknown;

      bool foundEncoding = false;
      bool usesFEC = false;

      if (NULL != encoding) {
        foundEncoding = true;
        if (encoding->mFEC.hasValue()) {
          usesFEC = true;
          mechanism = IRTPTypes::toKnownFECMechanism(encoding->mFEC.value().mMechanism);
        }
      }

      if ((IRTPTypes::KnownFECMechanism_Unknown == mechanism) &&
          (baseEncoding)) {
        foundEncoding = true;
        if (baseEncoding->mFEC.hasValue()) {
          usesFEC = true;
          mechanism = IRTPTypes::toKnownFECMechanism(baseEncoding->mFEC.value().mMechanism);
        }
      }

      if (IRTPTypes::KnownFECMechanism_Unknown == mechanism) {
        if (params.mEncodings.size() > 0) {
          auto &frontEncoding = params.mEncodings.front();
          foundEncoding = true;
          if (frontEncoding.mFEC.hasValue()) {
            usesFEC = true;
            mechanism = IRTPTypes::toKnownFECMechanism(frontEncoding.mFEC.value().mMechanism);
          }
        }
      }

      if ((!usesFEC) &&
          (foundEncoding)) {
        ZS_LOG_INSANE(slog("encoding is not using FEC") + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()))
        return NULL;
      }

      auto mainCodec = pickCodec(kind, params, Optional<PayloadType>(), encoding, baseEncoding);
      if (!mainCodec) return NULL;

      FindCodecOptions options;
      options.mCodecKind = IRTPTypes::CodecKind_FEC;
      options.mClockRate = mainCodec->mClockRate;
      options.mMatchClockRateNotSet = true;

      switch (mechanism) {
        case IRTPTypes::KnownFECMechanism_Unknown:      {
          ZS_LOG_WARNING(Trace, slog("FEC mechanism is not known") + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()))
          return NULL;
        }
        case IRTPTypes::KnownFECMechanism_RED:          {
          options.mSupportedCodec = IRTPTypes::SupportedCodec_RED;
          options.mDisallowMultipleMatches = true;
          break;
        }
        case IRTPTypes::KnownFECMechanism_RED_ULPFEC:   {
          options.mSupportedCodec = IRTPTypes::SupportedCodec_RED;
          break;
        }
        case IRTPTypes::KnownFECMechanism_FlexFEC:      {
          options.mSupportedCodec = IRTPTypes::SupportedCodec_FlexFEC;
          options.mDisallowMultipleMatches = true;
          break;
        }
      }

      const CodecParameters *foundFECCodec = NULL;
      const CodecParameters *oldFECMatch = NULL;

      while (true) {
        foundFECCodec = findCodec(params, options);
        if (!foundFECCodec) break;

        if (IRTPTypes::KnownFECMechanism_RED_ULPFEC != mechanism) break;

        if (!foundFECCodec->mParameters) {
          options.mDisallowedPayloadtypeMatches.insert(foundFECCodec->mPayloadType);
          continue;                               // RED+ULPFEC requires finding ULPFEC codec
        }

        auto redCodecParams = IRTPTypes::REDCodecParameters::convert(foundFECCodec->mParameters);
        if (!redCodecParams) {
          options.mDisallowedPayloadtypeMatches.insert(foundFECCodec->mPayloadType);
          continue;                                          // must specify codec embedding for RED
        }

        bool foundULP = false;

        for (auto iterRed = redCodecParams->mPayloadTypes.begin(); iterRed != redCodecParams->mPayloadTypes.end(); ++iterRed) {
          auto redPayloadType = (*iterRed);

          FindCodecOptions ulpOptions;

          ulpOptions.mSupportedCodec = IRTPTypes::SupportedCodec_ULPFEC;
          ulpOptions.mPayloadType = redPayloadType;
          ulpOptions.mClockRate = mainCodec->mClockRate;
          ulpOptions.mMatchClockRateNotSet = true;

          auto foundULPCodec = findCodec(params, ulpOptions);
          if (!foundULPCodec) continue;

          foundULP = true;
          break;
        }

        if (!foundULP) {
          options.mDisallowedPayloadtypeMatches.insert(foundFECCodec->mPayloadType);
          continue;
        }

        if (oldFECMatch) {
          ZS_LOG_WARNING(Detail, slog("FEC payload to use is ambiguous") + ZS_PARAM("found", foundFECCodec->toDebug()) + ZS_PARAM("previously found", oldFECMatch->toDebug()))
          return NULL;
        }

        oldFECMatch = foundFECCodec;
        options.mDisallowedPayloadtypeMatches.insert(foundFECCodec->mPayloadType);
      }

      if (oldFECMatch) foundFECCodec = oldFECMatch;

      if (!foundFECCodec) {
        ZS_LOG_WARNING(Debug, slog("did not find an appropriate FEC codec") + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()))
        return NULL;
      }

      ZS_LOG_TRACE(slog("found FEC codec") + foundFECCodec->toDebug() + params.toDebug() + ZS_PARAM("encoding", encoding ? encoding->toDebug() : ElementPtr()) + ZS_PARAM("base encoding", baseEncoding ? baseEncoding->toDebug() : ElementPtr()))
      return foundFECCodec;
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

        if (encoding->mDependencyEncodingIDs.size() < 1) {
          return encoding;
        }

        for (auto iter = encoding->mDependencyEncodingIDs.begin(); iter != encoding->mDependencyEncodingIDs.end(); ++iter) {
          auto &dependencyID = (*iter);
          auto foundPrevious = previouslyChecked.find(dependencyID);
          if (foundPrevious != previouslyChecked.end()) continue;

          auto found = encodings.find(dependencyID);
          if (found == encodings.end()) continue;

          pending[found->first] = found->second;
        }
      }

      return inEncoding;
    }

    //-------------------------------------------------------------------------
    RTPTypesHelper::EncodingParameters *RTPTypesHelper::pickEncodingToFill(
                                                                           Optional<IMediaStreamTrackTypes::Kinds> kind,
                                                                           PayloadType packetPayloadType,
                                                                           Parameters &filledParams,
                                                                           const CodecParameters * &outCodecParameters,
                                                                           IRTPTypes::SupportedCodecs &outSupportedCodec,
                                                                           IRTPTypes::CodecKinds &outCodecKind,
                                                                           EncodingParameters * &outBaseEncoding
                                                                           )
    {
      outSupportedCodec = IRTPTypes::SupportedCodec_Unknown;
      outCodecKind = IRTPTypes::CodecKind_Unknown;
      outBaseEncoding = NULL;

      outCodecParameters = RTPTypesHelper::pickCodec(kind, filledParams, packetPayloadType);
      if (NULL == outCodecParameters) {
        ZS_LOG_INSANE(slog("cannot match because codec is not found in parameters") + filledParams.toDebug() + ZS_PARAM("packet payload type", packetPayloadType) + ZS_PARAM("kind", kind.hasValue() ? IMediaStreamTrackTypes::toString(kind.value()) : (const char *)NULL))
        return NULL;
      }

      outSupportedCodec = IRTPTypes::toSupportedCodec(outCodecParameters->mName);
      outCodecKind = IRTPTypes::getCodecKind(outSupportedCodec);

      if (filledParams.mEncodings.size() < 1) {
        // "latch all" allows this codec to match all incoming packets
        return NULL;
      }

      (*(filledParams.mEncodings.begin()));

      for (auto encodingIter = filledParams.mEncodings.begin(); encodingIter != filledParams.mEncodings.end(); ++encodingIter) {

        auto &encoding = (*encodingIter);

        EncodingParameters &baseEncoding = *(RTPTypesHelper::findEncodingBase(filledParams, &encoding));

        const CodecParameters *baseCodec = NULL;
        if (baseEncoding.mCodecPayloadType.hasValue()) {
          baseCodec = RTPTypesHelper::pickCodec(kind, filledParams);
        }

        switch (outCodecKind) {
          case IRTPTypes::CodecKind_Unknown:  ASSERT(false) break;
          case IRTPTypes::CodecKind_Audio:
          case IRTPTypes::CodecKind_Video:
          case IRTPTypes::CodecKind_AV:
          case IRTPTypes::CodecKind_Data:
          {
            if (baseCodec) {
              if (baseCodec->mPayloadType != packetPayloadType) goto not_possible_match;
            }

            if (baseEncoding.mSSRC.hasValue()) {
              if (!IRTPTypes::isMRSTCodec(outSupportedCodec)) goto not_possible_match;
            }

            if (encoding.mSSRC.hasValue()) goto not_possible_layer_match;

            outBaseEncoding = &baseEncoding;
            return &encoding;
          }
          case IRTPTypes::CodecKind_AudioSupplemental:
          {
            if (!baseCodec) goto not_possible_match;

            if (baseCodec->mClockRate.hasValue()) {
              if (outCodecParameters->mClockRate.hasValue()) {  // NOTE: will allow match if supplemental codec does not have a clock rate specified at all
                if (baseCodec->mClockRate.value() != outCodecParameters->mClockRate.value()) goto not_possible_match;
              }
            }

            outBaseEncoding = &baseEncoding;
            return &encoding;
          }
          case IRTPTypes::CodecKind_RTX:    {
            if (!baseCodec) goto not_possible_match;

            if (!baseEncoding.mSSRC.hasValue()) goto not_possible_match;

            if ((baseEncoding.mRTX.hasValue()) &&
                (baseEncoding.mRTX.value().mSSRC.hasValue())) {
              if (!IRTPTypes::isMRSTCodec(outSupportedCodec)) goto not_possible_match;
            }

            if ((encoding.mRTX.hasValue()) &&
                (encoding.mRTX.value().mSSRC.hasValue())) goto not_possible_layer_match;

            auto foundRTXCodec = RTPTypesHelper::pickRTXCodec(kind, filledParams, packetPayloadType, &encoding, &baseEncoding);
            if (!foundRTXCodec) goto not_possible_layer_match;

            outBaseEncoding = &baseEncoding;
            return &encoding;
          }
          case IRTPTypes::CodecKind_FEC:    {
            if (!baseCodec) goto not_possible_match;

            if (!baseEncoding.mSSRC.hasValue()) goto not_possible_match;

            if ((baseEncoding.mFEC.hasValue()) &&
                (baseEncoding.mFEC.value().mSSRC.hasValue())) {
              if (!IRTPTypes::isMRSTCodec(outSupportedCodec)) goto not_possible_match;
            }

            if ((encoding.mFEC.hasValue()) &&
                (encoding.mFEC.value().mSSRC.hasValue())) goto not_possible_layer_match;

            auto foundFECCodec = RTPTypesHelper::pickFECCodec(kind, filledParams, packetPayloadType, &encoding, &baseEncoding);
            if (!foundFECCodec) goto not_possible_layer_match;

            outBaseEncoding = &baseEncoding;
            return &encoding;
          }
        }

      not_possible_layer_match: {}
      }

    not_possible_match: {}

      outCodecParameters = NULL;
      outSupportedCodec = IRTPTypes::SupportedCodec_Unknown;
      outCodecKind = IRTPTypes::CodecKind_Unknown;
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
          FECMechanism fecMechanism(UseServicesHelper::getElementTextAndDecode(fecMechanismEl));
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
        fecMechanismsEl->adoptAsLastChild(UseServicesHelper::createElementWithTextAndJSONEncode("fecMechanism", fecMechanism));
      }
      elem->adoptAsLastChild(fecMechanismsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::Capabilities::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::Capabilities");

    ElementPtr codecsEl = Element::create("codecs");
    ElementPtr headersEl = Element::create("headers");
    ElementPtr fecMechanismsEl = Element::create("fec mechanisms");

    for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter)
    {
      auto value = (*iter);
      UseServicesHelper::debugAppend(codecsEl, value.toDebug());
    }
    if (codecsEl->hasChildren()) {
      UseServicesHelper::debugAppend(resultEl, codecsEl);
    }

    for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter)
    {
      auto value = (*iter);
      UseServicesHelper::debugAppend(headersEl, value.toDebug());
    }
    if (headersEl->hasChildren()) {
      UseServicesHelper::debugAppend(resultEl, headersEl);
    }

    for (auto iter = mFECMechanisms.begin(); iter != mFECMechanisms.end(); ++iter)
    {
      auto value = (*iter);
      UseServicesHelper::debugAppend(fecMechanismsEl, "fec mechanism", value);
    }
    if (fecMechanismsEl->hasChildren()) {
      UseServicesHelper::debugAppend(resultEl,  fecMechanismsEl);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::Capabilities::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::Capabilities:");

    hasher.update("codecs:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

    for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value.hash());
    }

    hasher.update("headers:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

    for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value.hash());
    }

    hasher.update("fec:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

    for (auto iter = mFECMechanisms.begin(); iter != mFECMechanisms.end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value);
    }

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "name", mName);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "kind", mKind);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "clockRate", mClockRate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "preferredPayloadType", mPreferredPayloadType);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "maxptime", mMaxPTime);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "numChannels", mNumChannels);

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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "maxTemporalLayers", mMaxTemporalLayers);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "maxSpatialLayers", mMaxSpatialLayers);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecCapability", "svcMultiStreamSupport", mSVCMultiStreamSupport);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::CodecCapability::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "name", mName, false);
    UseHelper::adoptElementValue(elem, "kind", mKind, false);

    UseHelper::adoptElementValue(elem, "clockRate", mClockRate);
    UseHelper::adoptElementValue(elem, "preferredPayloadType", mPreferredPayloadType);
    UseHelper::adoptElementValue(elem, "maxptime", mMaxPTime);
    UseHelper::adoptElementValue(elem, "numChannels", mNumChannels);

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

    UseHelper::adoptElementValue(elem, "maxTemporalLayers", mMaxTemporalLayers);
    UseHelper::adoptElementValue(elem, "maxSpatialLayers", mMaxSpatialLayers);
    UseHelper::adoptElementValue(elem, "svcMultiStreamSupport", mSVCMultiStreamSupport);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::CodecCapability::CodecCapability(const CodecCapability &source) :
    mName(source.mName),
    mKind(source.mKind),
    mClockRate(source.mClockRate),
    mPreferredPayloadType(source.mPreferredPayloadType),
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
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::CodecCapability");

    UseServicesHelper::debugAppend(resultEl, "name", mName);
    UseServicesHelper::debugAppend(resultEl, "kind", mKind);
    UseServicesHelper::debugAppend(resultEl, "clock rate", mClockRate);
    UseServicesHelper::debugAppend(resultEl, "preferred payload type", mPreferredPayloadType);
    UseServicesHelper::debugAppend(resultEl, "max ptime", mMaxPTime);
    UseServicesHelper::debugAppend(resultEl, "number of channels", mNumChannels);

    ElementPtr feedbacksEl = Element::create("rtcpFeedbacks");

    for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter) {
      auto value = (*iter);
      UseServicesHelper::debugAppend(feedbacksEl, value.toDebug());
    }
    if (feedbacksEl->hasChildren()) {
      UseServicesHelper::debugAppend(resultEl, feedbacksEl);
    }

    SupportedCodecs supported = toSupportedCodec(mName);

    // scope: output params
    {
      bool found = false;

      if (mParameters) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityParameters::convert(mParameters);
            if (codec) {
              UseServicesHelper::debugAppend(resultEl, codec->toDebug());
              found = true;
            }
            break;
          }
          case SupportedCodec_VP8:    {
            auto codec = VP8CodecCapabilityParameters::convert(mParameters);
            if (codec) {
              UseServicesHelper::debugAppend(resultEl, codec->toDebug());
              found = true;
            }
            break;
          }
          case SupportedCodec_H264:   {
            auto codec = H264CodecCapabilityParameters::convert(mParameters);
            if (codec) {
              UseServicesHelper::debugAppend(resultEl, codec->toDebug());
              found = true;
            }
            break;
          }
          default: break;
        }
      }

      if (!found) {
        UseServicesHelper::debugAppend(resultEl, "parameters", (bool)mParameters);
      }
    }

    // scope: output options
    {
      bool found = false;

      if (mOptions) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityOptions::convert(mOptions);
            if (codec) {
              UseServicesHelper::debugAppend(resultEl, codec->toDebug());
              found = true;
            }
            break;
          }
          default: break;
        }
      }

      if (!found) {
        UseServicesHelper::debugAppend(resultEl, "options", (bool)mOptions);
      }
    }

    UseServicesHelper::debugAppend(resultEl, "max temporal layers", mMaxTemporalLayers);
    UseServicesHelper::debugAppend(resultEl, "max spatial layers", mMaxSpatialLayers);

    UseServicesHelper::debugAppend(resultEl, "svc multistream support", mSVCMultiStreamSupport);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::CodecCapability::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::CodecCapability:");

    hasher.update(mName);
    hasher.update(":");
    hasher.update(mKind);
    hasher.update(":");
    hasher.update(mClockRate);
    hasher.update(":");
    hasher.update(mPreferredPayloadType);
    hasher.update(":");
    hasher.update(mMaxPTime);
    hasher.update(":");
    hasher.update(mNumChannels);

    hasher.update("feedback:0e69ea312f56834897bc0c29eb74bf991bee8d86");

    for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value.hash());
    }

    hasher.update(":feedback:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

    SupportedCodecs supported = toSupportedCodec(mName);

    // scope: output params
    {
      hasher.update(":");

      if (mParameters) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityParameters::convert(mParameters);
            if (codec) hasher.update(codec->hash());
            break;
          }
          case SupportedCodec_VP8:    {
            auto codec = VP8CodecCapabilityParameters::convert(mParameters);
            if (codec) hasher.update(codec->hash());
            break;
          }
          case SupportedCodec_H264:   {
            auto codec = H264CodecCapabilityParameters::convert(mParameters);
            if (codec) hasher.update(codec->hash());
            break;
          }
          default: break;
        }
      }
    }

    // scope: output options
    {
      hasher.update(":");

      if (mOptions) {
        switch (supported) {
          case SupportedCodec_Opus: {
            auto codec = OpusCodecCapabilityOptions::convert(mOptions);
            if (codec) hasher.update(codec->hash());
            break;
          }
          default: break;
        }
      }
    }

    hasher.update((bool)mParameters);
    hasher.update(":");
    hasher.update((bool)mOptions);
    hasher.update(":");
    hasher.update(mMaxTemporalLayers);
    hasher.update(":");
    hasher.update(mMaxSpatialLayers);
    hasher.update(":");
    hasher.update(mSVCMultiStreamSupport);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::OpusCodecCapabilityOptions
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecCapabilityOptions::OpusCodecCapabilityOptions(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "complexity", mComplexity);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "signal", mSignal);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "application", mApplication);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "packetLossPerc", mPacketLossPerc);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityOptions", "PredictionDisabled", mPredictionDisabled);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecCapabilityOptions::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "complexity", mComplexity);
    UseHelper::adoptElementValue(elem, "signal", mSignal);
    UseHelper::adoptElementValue(elem, "application", mApplication);
    UseHelper::adoptElementValue(elem, "packetLossPerc", mPacketLossPerc);
    UseHelper::adoptElementValue(elem, "predictionDisabled", mPredictionDisabled);

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
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::OpusCodecCapabilityOptions");

    UseServicesHelper::debugAppend(resultEl, "complexity", mComplexity);
    UseServicesHelper::debugAppend(resultEl, "signal", mSignal);
    UseServicesHelper::debugAppend(resultEl, "application", mApplication);
    UseServicesHelper::debugAppend(resultEl, "packet loss percentage", mPacketLossPerc);
    UseServicesHelper::debugAppend(resultEl, "prediction disabled", mPredictionDisabled);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::OpusCodecCapabilityOptions::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::OpusCodecCapabilityOptions:");

    hasher.update(mComplexity);
    hasher.update(":");
    hasher.update(mSignal);
    hasher.update(":");
    hasher.update(mApplication);
    hasher.update(":");
    hasher.update(mPacketLossPerc);
    hasher.update(":");
    hasher.update(mPredictionDisabled);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "maxPlaybackRate", mMaxPlaybackRate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "ptime", mPTime);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "maxAverageBitrate", mMaxAverageBitrate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "stereo", mStereo);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "cbr", mCBR);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "useInbandFec", mUseInbandFEC);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "useDtx", mUseDTX);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "spropMaxCaptureRate", mSPropMaxCaptureRate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecCapabilityParameters", "spropStereo", mSPropStereo);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecCapabilityParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "maxPlaybackRate", mMaxPlaybackRate);
    UseHelper::adoptElementValue(elem, "ptime", mPTime);
    UseHelper::adoptElementValue(elem, "maxAverageBitrate", mMaxAverageBitrate);
    UseHelper::adoptElementValue(elem, "stereo", mStereo);
    UseHelper::adoptElementValue(elem, "cbr", mCBR);
    UseHelper::adoptElementValue(elem, "useInbandFec", mUseInbandFEC);
    UseHelper::adoptElementValue(elem, "useDtx", mUseDTX);
    UseHelper::adoptElementValue(elem, "spropMaxCaptureRate", mSPropMaxCaptureRate);
    UseHelper::adoptElementValue(elem, "spropStereo", mSPropStereo);

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
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::OpusCodecCapabilityParameters");

    UseServicesHelper::debugAppend(resultEl, "max playback rate", mMaxPlaybackRate);
    UseServicesHelper::debugAppend(resultEl, "ptime", mPTime);
    UseServicesHelper::debugAppend(resultEl, "max average bitrate", mMaxAverageBitrate);
    UseServicesHelper::debugAppend(resultEl, "stereo", mStereo);
    UseServicesHelper::debugAppend(resultEl, "cbr", mCBR);
    UseServicesHelper::debugAppend(resultEl, "use inband fec", mUseInbandFEC);
    UseServicesHelper::debugAppend(resultEl, "use dtx", mUseDTX);

    UseServicesHelper::debugAppend(resultEl, "sprop max capture rate", mSPropMaxCaptureRate);
    UseServicesHelper::debugAppend(resultEl, "sprop stereo", mSPropStereo);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::OpusCodecCapabilityParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::OpusCodecCapabilityParameters:");

    hasher.update(mMaxPlaybackRate);
    hasher.update(":");
    hasher.update(mPTime);
    hasher.update(":");
    hasher.update(mMaxAverageBitrate);
    hasher.update(":");
    hasher.update(mStereo);
    hasher.update(":");
    hasher.update(mCBR);
    hasher.update(":");
    hasher.update(mUseInbandFEC);
    hasher.update(":");
    hasher.update(mUseDTX);

    hasher.update(":");
    hasher.update(mSPropMaxCaptureRate);
    hasher.update(":");
    hasher.update(mSPropStereo);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::VP8CodecCapabilityParameters", "maxFr", mMaxFT);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::VP8CodecCapabilityParameters", "maxFs", mMaxFS);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::VP8CodecCapabilityParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "maxFr", mMaxFT);
    UseHelper::adoptElementValue(elem, "maxFs", mMaxFS);

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
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::VP8CodecCapabilityParameters");

    UseServicesHelper::debugAppend(resultEl, "max ft", mMaxFT);
    UseServicesHelper::debugAppend(resultEl, "max fs", mMaxFS);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::VP8CodecCapabilityParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::VP8CodecCapabilityParameters:");

    hasher.update(mMaxFT);
    hasher.update(":");
    hasher.update(mMaxFS);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "profileLevelId", mProfileLevelID);

    {
      ElementPtr packetizationsEl = elem->findFirstChildElement("packetizationModes");
      if (packetizationsEl) {
        ElementPtr packetizationEl = packetizationsEl->findFirstChildElement("packetizationMode");
        while (packetizationEl) {
          decltype(mPacketizationModes)::value_type value {};
          String str = UseServicesHelper::getElementText(packetizationEl);
          try {
            value = Numeric<decltype(mPacketizationModes)::value_type>(str);
          } catch(Numeric<decltype(mPacketizationModes)::value_type>::ValueOutOfRange &) {
            ZS_LOG_WARNING(Debug, slog("packetization mode value out of range") + ZS_PARAM("value", str))
          }
          mPacketizationModes.push_back(value);
          packetizationEl = packetizationEl->findNextSiblingElement("packetizationMode");
        }
      }
    }

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxMbps", mMaxMBPS);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxSmbps", mMaxSMBPS);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxSmbps", mMaxSMBPS);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "mMaxFs", mMaxFS);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxCpb", mMaxCPB);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxDpb", mMaxDPB);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::H264CodecCapabilityParameters", "maxBr", mMaxBR);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::H264CodecCapabilityParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "profileLevelId", mProfileLevelID);

    if (mPacketizationModes.size() > 0) {
      ElementPtr packetizationsEl = Element::create("packetizationModes");
      for (auto iter = mPacketizationModes.begin(); iter != mPacketizationModes.end(); ++iter) {
        auto value = (*iter);
        UseHelper::adoptElementValue(packetizationsEl, "packetizationMode", value);
      }
      elem->adoptAsLastChild(packetizationsEl);
    }

    UseHelper::adoptElementValue(elem, "maxMbps", mMaxMBPS);
    UseHelper::adoptElementValue(elem, "maxSmbps", mMaxSMBPS);
    UseHelper::adoptElementValue(elem, "maxSmbps", mMaxSMBPS);
    UseHelper::adoptElementValue(elem, "mMaxFs", mMaxFS);
    UseHelper::adoptElementValue(elem, "maxCpb", mMaxCPB);
    UseHelper::adoptElementValue(elem, "maxDpb", mMaxDPB);
    UseHelper::adoptElementValue(elem, "maxBr", mMaxBR);

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
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::H264CodecCapabilityParameters");

    UseServicesHelper::debugAppend(resultEl, "profile level id", mProfileLevelID);

    if (mPacketizationModes.size() > 0) {
      ElementPtr packetizationModesEl = Element::create("packetization modes");
      for (auto iter = mPacketizationModes.begin(); iter != mPacketizationModes.end(); ++iter) {
        auto &mode = (*iter);
        UseServicesHelper::debugAppend(packetizationModesEl, "packetization mode", mode);
      }
      UseServicesHelper::debugAppend(resultEl, packetizationModesEl);
    }

    UseServicesHelper::debugAppend(resultEl, "max mbps", mMaxMBPS);
    UseServicesHelper::debugAppend(resultEl, "max smbps", mMaxSMBPS);
    UseServicesHelper::debugAppend(resultEl, "max fs", mMaxFS);
    UseServicesHelper::debugAppend(resultEl, "max cpb", mMaxCPB);
    UseServicesHelper::debugAppend(resultEl, "max dpb", mMaxDPB);
    UseServicesHelper::debugAppend(resultEl, "max br", mMaxBR);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::H264CodecCapabilityParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::H264CodecCapabilityParameters:");

    hasher.update(mProfileLevelID);
    hasher.update(":packetizationmodes");

    for (auto iter = mPacketizationModes.begin(); iter != mPacketizationModes.end(); ++iter) {
      auto &mode = (*iter);
      hasher.update(":");
      hasher.update(mode);
    }

    hasher.update(":packetizationmodes:");

    hasher.update(mMaxMBPS);
    hasher.update(":");
    hasher.update(mMaxSMBPS);
    hasher.update(":");
    hasher.update(mMaxFS);
    hasher.update(":");
    hasher.update(mMaxCPB);
    hasher.update(":");
    hasher.update(mMaxDPB);
    hasher.update(":");
    hasher.update(mMaxBR);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtension", "kind", mKind);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtension", "uri", mURI);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtension", "preferredId", mPreferredID);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtension", "preferredEncrypt", mPreferredEncrypt);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtension::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "kind", mKind, false);
    UseHelper::adoptElementValue(elem, "uri", mURI, false);
    UseHelper::adoptElementValue(elem, "preferredId", mPreferredID);
    UseHelper::adoptElementValue(elem, "preferredEncrypt", mPreferredEncrypt);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtension::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::HeaderExtensions");

    UseServicesHelper::debugAppend(resultEl, "kind", mKind);
    UseServicesHelper::debugAppend(resultEl, "uri", mURI);
    UseServicesHelper::debugAppend(resultEl, "preferred id", mPreferredID);
    UseServicesHelper::debugAppend(resultEl, "prefer encrypt", mPreferredEncrypt);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::HeaderExtension::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::HeaderExtensions:");

    hasher.update(mKind);
    hasher.update(":");
    hasher.update(mURI);
    hasher.update(":");
    hasher.update(mPreferredID);
    hasher.update(":");
    hasher.update(mPreferredEncrypt);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPFeedback", "type", mType);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPFeedback", "parameter", mParameter);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTCPFeedback::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "type", mType, false);
    UseHelper::adoptElementValue(elem, "parameter", mParameter, false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTCPFeedback::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::RTCPFeedback");

    UseServicesHelper::debugAppend(resultEl, "type", mType);
    UseServicesHelper::debugAppend(resultEl, "parameter", mParameter);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTCPFeedback::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::RTCPFeedback:");

    hasher.update(mType);
    hasher.update(":");
    hasher.update(mParameter);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPParameters", "ssrc", mSSRC);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPParameters", "cname", mCName);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPParameters", "reducedSize", mReducedSize);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTCPParameters", "mux", mMux);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTCPParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "ssrc", mSSRC);
    UseHelper::adoptElementValue(elem, "cname", mCName, false);
    UseHelper::adoptElementValue(elem, "reducedSize", mReducedSize);
    UseHelper::adoptElementValue(elem, "mux", mMux);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTCPParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::RTCPParameters");

    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
    UseServicesHelper::debugAppend(resultEl, "cname", mCName);
    UseServicesHelper::debugAppend(resultEl, "reduced size", mReducedSize);
    UseServicesHelper::debugAppend(resultEl, "mux", mMux);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTCPParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::RTCPParameters:");

    hasher.update(mSSRC);
    hasher.update(":");
    hasher.update(mCName);
    hasher.update(":");
    hasher.update(mReducedSize);
    hasher.update(":");
    hasher.update(mMux);

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::Parameters::Parameters(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::Parameters", "muxId", mMuxID);

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
        String str = UseServicesHelper::getElementTextAndDecode(subEl);
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

    UseHelper::adoptElementValue(elem, "muxId", mMuxID, false);

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

    UseHelper::adoptElementValue(elem, IRTPTypes::toString(mDegredationPreference), false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::Parameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::Parameters");

    UseServicesHelper::debugAppend(resultEl, "ssrc", mMuxID);

    if (mCodecs.size() > 0) {
      ElementPtr codecsEl = Element::create("codecs");
      for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter) {
        auto value = (*iter);
        UseServicesHelper::debugAppend(codecsEl, value.toDebug());
      }
      UseServicesHelper::debugAppend(resultEl, codecsEl);
    }

    if (mHeaderExtensions.size() > 0) {
      ElementPtr headersEl = Element::create("headers");
      for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter) {
        auto value = (*iter);
        UseServicesHelper::debugAppend(headersEl, value.toDebug());
      }
      UseServicesHelper::debugAppend(resultEl, headersEl);
    }

    if (mEncodings.size() > 0) {
      ElementPtr encodingsEl = Element::create("encodings");
      
      for (auto iter = mEncodings.begin(); iter != mEncodings.end(); ++iter) {
        auto value = (*iter);
        UseServicesHelper::debugAppend(encodingsEl, value.toDebug());
      }
      UseServicesHelper::debugAppend(resultEl, encodingsEl);
    }

    UseServicesHelper::debugAppend(resultEl, "rtcp params", mRTCP.toDebug());
    UseServicesHelper::debugAppend(resultEl, "delegration preference", toString(mDegredationPreference));

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::Parameters::hash(const HashOptions &options) const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::Parameters:");

    if (options.mMuxID) {
      hasher.update(mMuxID);
    }

    if (options.mCodecs) {
      hasher.update("codecs:0e69ea312f56834897bc0c29eb74bf991bee8d86");

      for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter) {
        auto value = (*iter);
        hasher.update(":");
        hasher.update(value.hash());
      }
    }

    if (options.mHeaderExtensions) {
      hasher.update("headers:0e69ea312f56834897bc0c29eb74bf991bee8d86");

      for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter) {
        auto value = (*iter);
        hasher.update(":");
        hasher.update(value.hash());
      }
    }

    if (options.mEncodingParameters) {
      hasher.update("encodings:0e69ea312f56834897bc0c29eb74bf991bee8d86");

      for (auto iter = mEncodings.begin(); iter != mEncodings.end(); ++iter) {
        auto value = (*iter);
        hasher.update(":");
        hasher.update(value.hash());
      }
    }

    if (options.mRTCP) {
      hasher.update("rtcp:72b2b94700e10e41adba3cdf656abed590bb65f4:");
      hasher.update(mRTCP.hash());
    }

    if (options.mDegredationPreference) {
      hasher.update("degredation:14bac0ecdadf8b017403d37459be8490:");
      hasher.update(mDegredationPreference);
    }

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "name", mName);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "payloadType", mPayloadType);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "clockRate", mClockRate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "maxptime", mMaxPTime);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::CodecParameters", "numChannels", mNumChannels);

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

    UseHelper::adoptElementValue(elem, "name", mName, false);
    UseHelper::adoptElementValue(elem, "payloadType", mPayloadType);
    UseHelper::adoptElementValue(elem, "clockRate", mClockRate);
    UseHelper::adoptElementValue(elem, "maxptime", mMaxPTime);
    UseHelper::adoptElementValue(elem, "numChannels", mNumChannels);

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
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::CodecParameters");

    UseServicesHelper::debugAppend(resultEl, "name", mName);
    UseServicesHelper::debugAppend(resultEl, "payload type", mPayloadType);
    UseServicesHelper::debugAppend(resultEl, "clock rate", mClockRate);
    UseServicesHelper::debugAppend(resultEl, "max ptime", mMaxPTime);
    UseServicesHelper::debugAppend(resultEl, "number of channels", mNumChannels);

    if (mRTCPFeedback.size() > 0) {
      ElementPtr feedbacksEl = Element::create("feedbacks");
      for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter) {
        auto value = (*iter);
        UseServicesHelper::debugAppend(feedbacksEl, value.toDebug());
      }
      UseServicesHelper::debugAppend(resultEl, feedbacksEl);
    }

    SupportedCodecs supported = toSupportedCodec(mName);

    bool found = false;

    if (mParameters) {
      switch (supported) {
        case SupportedCodec_Opus: {
          auto codec = OpusCodecParameters::convert(mParameters);
          if (codec) {
            found = true;
            UseServicesHelper::debugAppend(resultEl, codec->toDebug());
          }
          break;
        }
        case SupportedCodec_VP8: {
          auto codec = VP8CodecParameters::convert(mParameters);
          if (codec) {
            found = true;
            UseServicesHelper::debugAppend(resultEl, codec->toDebug());
          }
          break;
        }
        case SupportedCodec_H264: {
          auto codec = H264CodecParameters::convert(mParameters);
          if (codec) {
            found = true;
            UseServicesHelper::debugAppend(resultEl, codec->toDebug());
          }
          break;
        }
        case SupportedCodec_RTX: {
          auto codec = RTXCodecParameters::convert(mParameters);
          if (codec) {
            found = true;
            UseServicesHelper::debugAppend(resultEl, codec->toDebug());
          }
          break;
        }
        case SupportedCodec_RED: {
          auto codec = REDCodecParameters::convert(mParameters);
          if (codec) {
            found = true;
            UseServicesHelper::debugAppend(resultEl, codec->toDebug());
          }
          break;
        }
        case SupportedCodec_FlexFEC: {
          auto codec = FlexFECCodecParameters::convert(mParameters);
          if (codec) {
            found = true;
            UseServicesHelper::debugAppend(resultEl, codec->toDebug());
          }
          break;
        }
        default: break;
      }
    }

    if (!found) {
      UseServicesHelper::debugAppend(resultEl, "parameters", (bool)mParameters);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::CodecParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::Parameters:");

    hasher.update(mName);
    hasher.update(":");
    hasher.update(mPayloadType);
    hasher.update(":");
    hasher.update(mClockRate);
    hasher.update(":");
    hasher.update(mMaxPTime);
    hasher.update(":");
    hasher.update(mNumChannels);

    hasher.update("feedback:0e69ea312f56834897bc0c29eb74bf991bee8d86");

    for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter) {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value.hash());
    }

    auto supported = toSupportedCodec(mName);

    hasher.update(":");
    if (mParameters) {
      switch (supported) {
        case SupportedCodec_Opus: {
          auto codec = OpusCodecParameters::convert(mParameters);
          if (codec) hasher.update(codec->hash());
          break;
        }
        case SupportedCodec_VP8: {
          auto codec = VP8CodecParameters::convert(mParameters);
          if (codec) hasher.update(codec->hash());
          break;
        }
        case SupportedCodec_H264: {
          auto codec = H264CodecParameters::convert(mParameters);
          if (codec) hasher.update(codec->hash());
          break;
        }
        case SupportedCodec_RTX: {
          auto codec = RTXCodecParameters::convert(mParameters);
          if (codec) hasher.update(codec->hash());
          break;
        }
        case SupportedCodec_RED: {
          auto codec = REDCodecParameters::convert(mParameters);
          if (codec) hasher.update(codec->hash());
          break;
        }
        case SupportedCodec_FlexFEC: {
          auto codec = FlexFECCodecParameters::convert(mParameters);
          if (codec) hasher.update(codec->hash());
          break;
        }
        default: break;
      }
    }

    hasher.update(":");
    hasher.update((bool)mParameters);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::OpusCodecParameters
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IRTPTypes::OpusCodecParameters::toString(Signals signal)
  {
    switch (signal) {
      case Signal_Auto:         return "auto";
      case Signal_Music:        return "music";
      case Signal_Voice:        return "voice";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecParameters::Signals IRTPTypes::OpusCodecParameters::toSignal(const char *signal)
  {
    String signalStr(signal);

    for (Signals index = Signal_First; index <= Signal_Last; index = static_cast<Signals>(static_cast<std::underlying_type<Signals>::type>(index) + 1)) {
      if (signalStr == IRTPTypes::OpusCodecParameters::toString(index)) return index;
    }

    return Signal_Auto;
  }

  //---------------------------------------------------------------------------
  const char *IRTPTypes::OpusCodecParameters::toString(Applications application)
  {
    switch (application) {
      case Application_VoIP:      return "voip";
      case Application_Audio:     return "audio";
      case Application_LowDelay:  return "lowdelay";
    }

    return "unknown";
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecParameters::Applications IRTPTypes::OpusCodecParameters::toApplication(const char *application)
  {
    String applicationStr(application);

    for (Applications index = Application_First; index <= Application_Last; index = static_cast<Applications>(static_cast<std::underlying_type<Applications>::type>(index) + 1)) {
      if (applicationStr == IRTPTypes::OpusCodecParameters::toString(index)) return index;
    }

    return Application_VoIP;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::OpusCodecParameters::OpusCodecParameters(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "maxPlaybackRate", mMaxPlaybackRate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "ptime", mPTime);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "maxAverageBitrate", mMaxAverageBitrate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "cbr", mCBR);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "useInbandFec", mUseInbandFEC);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "useDtx", mUseDTX);

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "complexity", mComplexity);

    {
      ElementPtr subEl = elem->findFirstChildElement("signal");
      if (subEl) {
        String str = UseServicesHelper::getElementTextAndDecode(subEl);
        try {
          mSignal = toSignal(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("signal value is not value") + ZS_PARAM("value", str))
        }
      }
    }
    {
      ElementPtr subEl = elem->findFirstChildElement("application");
      if (subEl) {
        String str = UseServicesHelper::getElementTextAndDecode(subEl);
        try {
          mApplication = toApplication(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("signal value is not value") + ZS_PARAM("value", str))
        }
      }
    }

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "packetLossPerc", mPacketLossPerc);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "predictionDisabled", mPredictionDisabled);

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "spropMaxCaptureRate", mSPropMaxCaptureRate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::OpusCodecParameters", "spropStereo", mSPropStereo);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::OpusCodecParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "maxPlaybackRate", mMaxPlaybackRate);
    UseHelper::adoptElementValue(elem, "ptime", mPTime);
    UseHelper::adoptElementValue(elem, "maxAverageBitrate", mMaxAverageBitrate);
    UseHelper::adoptElementValue(elem, "stereo", mStereo);
    UseHelper::adoptElementValue(elem, "cbr", mCBR);
    UseHelper::adoptElementValue(elem, "useInbandFec", mUseInbandFEC);
    UseHelper::adoptElementValue(elem, "UseDtx", mUseDTX);

    UseHelper::adoptElementValue(elem, "complexity", mComplexity);
    UseHelper::adoptElementValue(elem, "signal", toString(mSignal), false);
    UseHelper::adoptElementValue(elem, "application", toString(mApplication), false);
    UseHelper::adoptElementValue(elem, "packetLossPerc", mPacketLossPerc);
    UseHelper::adoptElementValue(elem, "predictionDisabled", mPredictionDisabled);

    UseHelper::adoptElementValue(elem, "spropMaxCaptureRate", mSPropMaxCaptureRate);
    UseHelper::adoptElementValue(elem, "spropStereo", mSPropStereo);

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
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::OpusCodecParameters");

    UseServicesHelper::debugAppend(resultEl, "max playback rate", mMaxPlaybackRate);
    UseServicesHelper::debugAppend(resultEl, "ptime", mPTime);
    UseServicesHelper::debugAppend(resultEl, "max average bitrate", mMaxAverageBitrate);
    UseServicesHelper::debugAppend(resultEl, "stereo", mStereo);
    UseServicesHelper::debugAppend(resultEl, "cbr", mCBR);
    UseServicesHelper::debugAppend(resultEl, "use inband fec", mUseInbandFEC);
    UseServicesHelper::debugAppend(resultEl, "use dtx", mUseDTX);

    UseServicesHelper::debugAppend(resultEl, "complexity", mComplexity);
    UseServicesHelper::debugAppend(resultEl, "signal", mSignal.hasValue() ? toString(mSignal.value()) : (const char *)NULL);
    UseServicesHelper::debugAppend(resultEl, "application", mApplication.hasValue() ? toString(mApplication.value()) : (const char *)NULL);
    UseServicesHelper::debugAppend(resultEl, "packet loss percentage", mPacketLossPerc);
    UseServicesHelper::debugAppend(resultEl, "prediction disabled", mPredictionDisabled);

    UseServicesHelper::debugAppend(resultEl, "sprop max capture rate", mSPropMaxCaptureRate);
    UseServicesHelper::debugAppend(resultEl, "sprop stereo", mSPropStereo);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::OpusCodecParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::OpusCodecParameters:");

    hasher.update(mComplexity);
    hasher.update(":");
    hasher.update(mSignal);
    hasher.update(":");
    hasher.update(mApplication);
    hasher.update(":");
    hasher.update(mPacketLossPerc);
    hasher.update(":");
    hasher.update(mPredictionDisabled);

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::RTXCodecParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::RTXCodecParameters::RTXCodecParameters(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTXCodecParameters", "rtxTime", mRTXTime);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTXCodecParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "rtxTime", mRTXTime);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::RTXCodecParametersPtr IRTPTypes::RTXCodecParameters::create(const RTXCodecParameters &capability)
  {
    return make_shared<RTXCodecParameters>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::RTXCodecParametersPtr IRTPTypes::RTXCodecParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(RTXCodecParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTXCodecParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::RTXCodecParameters");

    UseServicesHelper::debugAppend(resultEl, "rtx time", mRTXTime);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTXCodecParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::RTXCodecParameters:");

    hasher.update(mRTXTime);

    return hasher.final();
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
          String str = UseServicesHelper::getElementText(payloadTypeEl);
          try {
            value = Numeric<decltype(mPayloadTypes)::value_type>(str);
          } catch(Numeric<decltype(mPayloadTypes)::value_type>::ValueOutOfRange &) {
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
        UseHelper::adoptElementValue(payloadTypesEl, "payloadType", value);
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
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::REDCodecParameters");

    if (mPayloadTypes.size()) {
      ElementPtr payloadTypesEl = Element::create("payload types");
      for (auto iter = mPayloadTypes.begin(); iter != mPayloadTypes.end(); ++iter) {
        auto payloadType = (*iter);
        UseServicesHelper::debugAppend(payloadTypesEl, "payload type", payloadType);
      }
      UseServicesHelper::debugAppend(resultEl, payloadTypesEl);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::REDCodecParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::REDCodecParameters:");

    hasher.update("payloadTypes");
    for (auto iter = mPayloadTypes.begin(); iter != mPayloadTypes.end(); ++iter) {
      auto payloadType = (*iter);
      hasher.update(":");
      hasher.update(payloadType);
    }
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::FlexFECCodecParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPTypes::FlexFECCodecParameters::FlexFECCodecParameters(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::FlexFECCodecParameters", "rtxTime", mRepairWindow);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::FlexFECCodecParameters", "l", mL);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::FlexFECCodecParameters", "d", mD);

    {
      ElementPtr subEl = elem->findFirstChildElement("toP");
      if (subEl) {
        String str = UseServicesHelper::getElementTextAndDecode(subEl);
        try {
          mToP = IRTPTypes::FlexFECCodecParameters::toToP(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("degredation preference is not valid") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::FlexFECCodecParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "repairWindow", mRepairWindow);
    UseHelper::adoptElementValue(elem, "l", mL);
    UseHelper::adoptElementValue(elem, "d", mD);
    UseHelper::adoptElementValue(elem, "toP", string(mToP), false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  const char *IRTPTypes::FlexFECCodecParameters::toString(ToPs top)
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
  IRTPTypes::FlexFECCodecParameters::ToPs IRTPTypes::FlexFECCodecParameters::toToP(const char *top)
  {
    String topStr(top);

    for (ToPs index = ToP_First; index <= ToP_Last; index = static_cast<ToPs>(static_cast<std::underlying_type<ToPs>::type>(index) + 1)) {
      if (topStr == IRTPTypes::FlexFECCodecParameters::toString(index)) return index;
    }

    return ToP_Reserved;
  }

  //---------------------------------------------------------------------------
  IRTPTypes::FlexFECCodecParametersPtr IRTPTypes::FlexFECCodecParameters::create(const FlexFECCodecParameters &capability)
  {
    return make_shared<FlexFECCodecParameters>(capability);
  }

  //---------------------------------------------------------------------------
  IRTPTypes::FlexFECCodecParametersPtr IRTPTypes::FlexFECCodecParameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(FlexFECCodecParameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::FlexFECCodecParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::FlexFECCodecParameters");

    UseServicesHelper::debugAppend(resultEl, "repair window", mRepairWindow);
    UseServicesHelper::debugAppend(resultEl, "L", mL);
    UseServicesHelper::debugAppend(resultEl, "D", mD);
    UseServicesHelper::debugAppend(resultEl, "ToP", mToP.hasValue() ? toString(mToP.value()) : (const char *)NULL);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::FlexFECCodecParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::FlexFECCodecParameters:");

    hasher.update(mRepairWindow);
    hasher.update(":");
    hasher.update(mL);
    hasher.update(":");
    hasher.update(mD);
    hasher.update(":");
    hasher.update(mToP);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtensionParameters", "uri", mURI);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtensionParameters", "id", mID);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::HeaderExtensionParameters", "encrypt", mEncrypt);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtensionParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "uri", mURI, false);
    UseHelper::adoptElementValue(elem, "id", mID);
    UseHelper::adoptElementValue(elem, "encrypt", mEncrypt);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtensionParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::HeaderExtensionParameters");

    UseServicesHelper::debugAppend(resultEl, "uri", mURI);
    UseServicesHelper::debugAppend(resultEl, "id", mID);
    UseServicesHelper::debugAppend(resultEl, "encrypt", mEncrypt);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::HeaderExtensionParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::HeaderExtensionParameters:");

    hasher.update(mURI);
    hasher.update(":");
    hasher.update(mID);
    hasher.update(":");
    hasher.update(mEncrypt);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::FECParameters", "ssrc", mSSRC);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::FECParameters", "mechanism", mMechanism);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::FECParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "ssrc", mSSRC);
    UseHelper::adoptElementValue(elem, "mechanism", mMechanism, false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::FECParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::FECParameters");

    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
    UseServicesHelper::debugAppend(resultEl, "mechanism", mMechanism);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::FECParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::FECParameters:");

    hasher.update(mSSRC);
    hasher.update(":");
    hasher.update(mMechanism);

    return hasher.final();
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTXParameters", "ssrc", mSSRC);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::RTXParameters", "payloadType", mPayloadType);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTXParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "ssrc", mSSRC);
    UseHelper::adoptElementValue(elem, "payloadType", mPayloadType);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RTXParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::RTXParameters");

    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
    UseServicesHelper::debugAppend(resultEl, "payload type", mPayloadType);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTXParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::RTXParameters:");

    hasher.update(mSSRC);
    hasher.update(":");
    hasher.update(mPayloadType);
    hasher.update(":");

    return hasher.final();
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
      case PriorityType_Unknown:        return "";
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

    return PriorityType_Unknown;
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

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "ssrc", mSSRC);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "codecPayloadType", mCodecPayloadType);

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
        String str = UseServicesHelper::getElementTextAndDecode(subEl);
        try {
          mPriority = IRTPTypes::toPriorityType(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("priority type is not valid") + ZS_PARAM("value", str))
        }
      }
    }

    UseHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "maxBitrate", mMaxBitrate);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "minQuality", mMinQuality);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "resolutionScale", mResolutionScale);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "framerateScale", mFramerateScale);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "active", mActive);
    UseHelper::getElementValue(elem, "ortc::IRTPTypes::EncodingParameters", "encodingId", mEncodingID);

    {
      ElementPtr dependencyEncodingIDsEl = elem->findFirstChildElement("dependencyEncodingIds");
      if (dependencyEncodingIDsEl) {
        ElementPtr dependencyEncodingIDEl = dependencyEncodingIDsEl->findFirstChildElement("dependencyEncodingId");
        while (dependencyEncodingIDEl) {
          mDependencyEncodingIDs.push_back(UseServicesHelper::getElementTextAndDecode(dependencyEncodingIDEl));
          dependencyEncodingIDEl = dependencyEncodingIDEl->findNextSiblingElement("dependencyEncodingId");
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::EncodingParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "ssrc", mSSRC);
    UseHelper::adoptElementValue(elem, "codecPayloadType", mCodecPayloadType);

    if (mFEC.hasValue()) {
      elem->adoptAsLastChild(mFEC.value().createElement("fec"));
    }
    if (mRTX.hasValue()) {
      elem->adoptAsLastChild(mRTX.value().createElement("rtx"));
    }

    UseHelper::adoptElementValue(elem, "priority", toString(mPriority), false);
    UseHelper::adoptElementValue(elem, "maxBitrate", mMaxBitrate);
    UseHelper::adoptElementValue(elem, "minQuality", mMinQuality);
    UseHelper::adoptElementValue(elem, "resolutionScale", mResolutionScale);
    UseHelper::adoptElementValue(elem, "resolutionScale", mFramerateScale);
    UseHelper::adoptElementValue(elem, "active", mActive);
    UseHelper::adoptElementValue(elem, "encodingId", mEncodingID, false);

    if (mDependencyEncodingIDs.size() > 0) {
      ElementPtr dependencyEncodingIDsEl = Element::create("dependencyEncodingIds");

      for (auto iter = mDependencyEncodingIDs.begin(); iter != mDependencyEncodingIDs.end(); ++iter) {
        auto value = (*iter);
        UseHelper::adoptElementValue(dependencyEncodingIDsEl, "dependencyEncodingId", value, false);
      }

      elem->adoptAsLastChild(dependencyEncodingIDsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::EncodingParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::EncodingParameters");

    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
    UseServicesHelper::debugAppend(resultEl, "codec payload type", mCodecPayloadType);
    UseServicesHelper::debugAppend(resultEl, "fec", mFEC.hasValue() ? mFEC.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "rtx", mRTX.hasValue() ? mRTX.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "priority", toString(mPriority));
    UseServicesHelper::debugAppend(resultEl, "max bitrate", mMaxBitrate);
    UseServicesHelper::debugAppend(resultEl, "min quality", mMinQuality);
    UseServicesHelper::debugAppend(resultEl, "resolution scale", mResolutionScale);
    UseServicesHelper::debugAppend(resultEl, "framerate scale", mFramerateScale);
    UseServicesHelper::debugAppend(resultEl, "active", mActive);
    UseServicesHelper::debugAppend(resultEl, "encoding id", mEncodingID);

    if (mDependencyEncodingIDs.size() > 0) {
      ElementPtr depedenciesEl = Element::create("dependency encoding ids");

      for (auto iter = mDependencyEncodingIDs.begin(); iter != mDependencyEncodingIDs.end(); ++iter) {
        auto value = (*iter);
        UseServicesHelper::debugAppend(depedenciesEl, "dependency encoding id", value);
      }
      UseServicesHelper::debugAppend(resultEl, depedenciesEl);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::EncodingParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::EncodingParameters:");

    hasher.update(mSSRC);
    hasher.update(":");
    hasher.update(mCodecPayloadType);
    hasher.update(":");
    hasher.update(mFEC.hasValue() ? mFEC.value().hash() : String());
    hasher.update(":");
    hasher.update(mRTX.hasValue() ? mRTX.value().hash() : String());
    hasher.update(":");
    hasher.update(mPriority);
    hasher.update(":");
    hasher.update(mMaxBitrate);
    hasher.update(":");
    hasher.update(mMinQuality);
    hasher.update(":");
    hasher.update(mResolutionScale);
    hasher.update(":");
    hasher.update(mFramerateScale);
    hasher.update(":");
    hasher.update(mActive);
    hasher.update(":");
    hasher.update(mEncodingID);

    for (auto iter = mDependencyEncodingIDs.begin(); iter != mDependencyEncodingIDs.end(); ++iter) {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value);
    }

    return hasher.final();
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
      case HeaderExtensionURI_TransportSequenceNumber:            return "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions";
    }

    return "unknown";
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

    return "unknown";  }

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
