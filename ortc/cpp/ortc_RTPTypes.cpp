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

#include <ortc/IRTPTypes.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPTypes::RTPypesHelper
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPypesHelper::splitParamsIntoChannels(
                                                const Parameters &params,
                                                ParametersPtrList &outGroupedIntoChannels
                                                )
    {
      typedef IRTPTypes::EncodingID EncodingID;
      typedef std::map<EncodingID, ParametersPtr> StreamMap;

      StreamMap streamMap;

      for (auto iter = params.mEncodingParameters.begin(); iter != params.mEncodingParameters.end(); ++iter)
      {
        auto &encoding = (*iter);
        if (encoding.mDependencyEncodingIDs.size() > 0) continue; // skip all that are dependent on other layers

        ParametersPtr tmpParam(make_shared<Parameters>());
        tmpParam->mCodecs = params.mCodecs;
        tmpParam->mHeaderExtensions = params.mHeaderExtensions;
        tmpParam->mMuxID = params.mMuxID;
        tmpParam->mRTCP = params.mRTCP;
        tmpParam->mEncodingParameters.push_back(encoding);

        outGroupedIntoChannels.push_back(tmpParam);

        if (encoding.mEncodingID.hasData()) {
          streamMap[encoding.mEncodingID] = tmpParam;
        }
      }

      bool missingEntry = false;
      bool foundEntry = false;

      do {
        missingEntry = false;
        foundEntry = false;

        for (auto iter = params.mEncodingParameters.begin(); iter != params.mEncodingParameters.end(); ++iter) {
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
            existingParam->mEncodingParameters.push_back(encoding);

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
    }

    //-------------------------------------------------------------------------
    Log::Params RTPypesHelper::slog(const char *message)
    {
      return Log::Params(message, "ortc::RTPypesHelper");
    }


  } // namespace internal

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::Capabilities
  #pragma mark

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
  ElementPtr IRTPTypes::CodecCapability::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::CodecCapability");

    UseServicesHelper::debugAppend(resultEl, "name", mName);
    UseServicesHelper::debugAppend(resultEl, "kind", mKind);
    UseServicesHelper::debugAppend(resultEl, "clock rate", mClockRate);
    UseServicesHelper::debugAppend(resultEl, "preferred payload type", mPreferredPayloadType);
    UseServicesHelper::debugAppend(resultEl, "max ptime", mMaxPTime);
    UseServicesHelper::debugAppend(resultEl, "number of channels", mNumChannels);

    ElementPtr feedbacksEl = Element::create("feedbacks");

    for (auto iter = mFeedback.begin(); iter != mFeedback.end(); ++iter) {
      auto value = (*iter);
      UseServicesHelper::debugAppend(feedbacksEl, value.toDebug());
    }
    if (feedbacksEl->hasChildren()) {
      UseServicesHelper::debugAppend(resultEl, feedbacksEl);
    }

    UseServicesHelper::debugAppend(resultEl, "parameters", (bool)mParameters);
    UseServicesHelper::debugAppend(resultEl, "options", (bool)mOptions);

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

    for (auto iter = mFeedback.begin(); iter != mFeedback.end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value.hash());
    }

    hasher.update(":feedback:0e69ea312f56834897bc0c29eb74bf991bee8d86:");

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
  #pragma mark IRTPTypes::HeaderExtensions
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::HeaderExtensions::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::HeaderExtensions");

    UseServicesHelper::debugAppend(resultEl, "kind", mKind);
    UseServicesHelper::debugAppend(resultEl, "uri", mURI);
    UseServicesHelper::debugAppend(resultEl, "preferred id", mPreferredID);
    UseServicesHelper::debugAppend(resultEl, "prefer encrypt", mPreferredEncrypt);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::HeaderExtensions::hash() const
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
  #pragma mark IRTPTypes::RtcpFeedback
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::RtcpFeedback::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::RtcpFeedback");

    UseServicesHelper::debugAppend(resultEl, "type", mType);
    UseServicesHelper::debugAppend(resultEl, "parameter", mParameter);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RtcpFeedback::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::RtcpFeedback:");

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
  ElementPtr IRTPTypes::Parameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::Parameters");

    UseServicesHelper::debugAppend(resultEl, "ssrc", mMuxID);

    ElementPtr codecsEl = Element::create("codecs");
    ElementPtr headersEl = Element::create("headers");
    ElementPtr encodingsEl = Element::create("encodings");

    for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter) {
      auto value = (*iter);
      UseServicesHelper::debugAppend(codecsEl, value.toDebug());
    }
    if (codecsEl) UseServicesHelper::debugAppend(resultEl, codecsEl);

    for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter) {
      auto value = (*iter);
      UseServicesHelper::debugAppend(headersEl, value.toDebug());
    }
    if (headersEl) UseServicesHelper::debugAppend(resultEl, headersEl);

    for (auto iter = mEncodingParameters.begin(); iter != mEncodingParameters.end(); ++iter) {
      auto value = (*iter);
      UseServicesHelper::debugAppend(encodingsEl, value.toDebug());
    }
    if (encodingsEl) UseServicesHelper::debugAppend(resultEl, encodingsEl);

    UseServicesHelper::debugAppend(resultEl, "rtcp params", mRTCP.toDebug());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::Parameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::Parameters:");

    hasher.update(mMuxID);

    hasher.update("codecs:0e69ea312f56834897bc0c29eb74bf991bee8d86");

    for (auto iter = mCodecs.begin(); iter != mCodecs.end(); ++iter) {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value.hash());
    }

    hasher.update("headers:0e69ea312f56834897bc0c29eb74bf991bee8d86");

    for (auto iter = mHeaderExtensions.begin(); iter != mHeaderExtensions.end(); ++iter) {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value.hash());
    }


    hasher.update("encodings:0e69ea312f56834897bc0c29eb74bf991bee8d86");

    for (auto iter = mEncodingParameters.begin(); iter != mEncodingParameters.end(); ++iter) {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value.hash());
    }

    hasher.update(":");
    hasher.update(mRTCP.hash());

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
  ElementPtr IRTPTypes::CodecParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::CodecParameters");

    UseServicesHelper::debugAppend(resultEl, "name", mName);
    UseServicesHelper::debugAppend(resultEl, "payload type", mPayloadType);
    UseServicesHelper::debugAppend(resultEl, "clock rate", mClockRate);
    UseServicesHelper::debugAppend(resultEl, "max ptime", mMaxPTime);
    UseServicesHelper::debugAppend(resultEl, "number of channels", mNumChannels);

    ElementPtr feedbacksEl = Element::create("feedbacks");

    for (auto iter = mRTCPFeedback.begin(); iter != mRTCPFeedback.end(); ++iter) {
      auto value = (*iter);
      UseServicesHelper::debugAppend(feedbacksEl, value.toDebug());
    }
    if (feedbacksEl) UseServicesHelper::debugAppend(resultEl, feedbacksEl);

    UseServicesHelper::debugAppend(resultEl, "parameters", (bool)mParameters);

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

    hasher.update(":");
    hasher.update((bool)mParameters);

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
  ElementPtr IRTPTypes::RTXParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::RTXParameters");

    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IRTPTypes::RTXParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IRTPTypes::RTXParameters:");

    hasher.update(mSSRC);

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPTypes::EncodingParameters
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IRTPTypes::EncodingParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IRTPTypes::EncodingParameters");

    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
    UseServicesHelper::debugAppend(resultEl, "codec payload type", mCodecPayloadType);
    UseServicesHelper::debugAppend(resultEl, "fec", mFEC.hasValue() ? mFEC.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "rtx", mRTX.hasValue() ? mRTX.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "priority", mPriority);
    UseServicesHelper::debugAppend(resultEl, "max bitrate", mMaxBitrate);
    UseServicesHelper::debugAppend(resultEl, "min quality", mMinQuality);
    UseServicesHelper::debugAppend(resultEl, "framerate bias", mFramerateBias);
    UseServicesHelper::debugAppend(resultEl, "active", mActive);
    UseServicesHelper::debugAppend(resultEl, "encoding id", mEncodingID);

    ElementPtr depedenciesEl = Element::create("dependency encoding ids");

    for (auto iter = mDependencyEncodingIDs.begin(); iter != mDependencyEncodingIDs.end(); ++iter) {
      auto value = (*iter);
      UseServicesHelper::debugAppend(depedenciesEl, "dependency encoding id", value);
    }
    if (depedenciesEl->hasChildren()) UseServicesHelper::debugAppend(resultEl, depedenciesEl);

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
    hasher.update(mFramerateBias);
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
  const char *IRTPTypes::toString(CodecKinds kind)
  {
    switch (kind) {
      case CodecKind_Unknown:  return "";
      case CodecKind_Audio:    return "audio";
      case CodecKind_Video:    return "video";
      case CodecKind_AV:       return "av";
      case CodecKind_RTX:      return "rtx";
      case CodecKind_FEC:      return "fec";
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

      case SupportedCodec_CN:                 return CodecKind_Audio;

      case SupportedCodec_TelephoneEvent:     return CodecKind_Audio;
    }
    
    return CodecKind_Unknown;
  }

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
