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


#include <ortc/adapter/internal/ortc_adapter_SDPParser.h>
#include <ortc/adapter/internal/ortc_adapter_SessionDescription.h>
#include <ortc/adapter/internal/ortc_adapter_Helper.h>

#include <ortc/adapter/IHelper.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/Log.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/XML.h>
#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>

#ifdef _MSC_VER
#pragma warning(3 : 4062)
#endif //_MSC_VER

namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortclib_adapter) } }

namespace ortc
{
  namespace adapter
  {
    using zsLib::Stringize;
    using zsLib::Numeric;

    ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Helpers
      #pragma mark

      //-----------------------------------------------------------------------
      static Log::Params slog(const char *message)
      {
        return Log::Params(message, "ortc::adapter::SDPParser");
      }

      //-----------------------------------------------------------------------
      static String createTransportIDFromIndex(size_t index)
      {
        auto hasher = IHasher::sha1();
        hasher->update("transport_index:");
        hasher->update(index);
        return hasher->finalizeAsString();
      }

      //-----------------------------------------------------------------------
      static String createMediaLineIDFromIndex(size_t index)
      {
        auto hasher = IHasher::sha1();
        hasher->update("media_line_index:");
        hasher->update(index);
        return hasher->finalizeAsString();
      }

      //-----------------------------------------------------------------------
      static String createSenderIDFromIndex(size_t index)
      {
        auto hasher = IHasher::sha1();
        hasher->update("sender_index:");
        hasher->update(index);
        return hasher->finalizeAsString();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SDPParser
      #pragma mark

      //-----------------------------------------------------------------------
      void SDPParser::createDescriptionDetails(
                                               const SDP &sdp,
                                               Description &ioDescription
                                               )
      {
        bool needsDetails = ((bool)sdp.mOLine) ||
          ((bool)sdp.mSLine) ||
          ((bool)sdp.mTLine) ||
          ((bool)sdp.mCLine) ||
          ((bool)sdp.mCLine);

        if (!needsDetails) return;

        auto details = make_shared<Description::Details>();
        ioDescription.mDetails = details;

        details->mUsername = ((bool)sdp.mOLine) ? sdp.mOLine->mUsername : String();
        details->mSessionID = ((bool)sdp.mOLine) ? sdp.mOLine->mSessionID : 0;
        details->mSessionVersion = ((bool)sdp.mOLine) ? sdp.mOLine->mSessionVersion : 0;
        details->mSessionName = ((bool)sdp.mSLine) ? sdp.mSLine->mSessionName : String();
        details->mStartTime = ((bool)sdp.mTLine) ? sdp.mTLine->mStartTime : 0;
        details->mEndTime = ((bool)sdp.mTLine) ? sdp.mTLine->mEndTime : 0;

        bool needUnicastAddress = ((bool)sdp.mOLine);
        if (needUnicastAddress) {
          auto unicast = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();

          unicast->mNetType = ((bool)sdp.mOLine) ? sdp.mOLine->mNetType : String();
          unicast->mAddrType = ((bool)sdp.mOLine) ? sdp.mOLine->mAddrType : String();
          unicast->mConnectionAddress = ((bool)sdp.mOLine) ? sdp.mOLine->mUnicastAddress : String();

          details->mUnicaseAddress = unicast;
        }

        bool needConnectionData = ((bool)sdp.mCLine);
        if (needConnectionData) {
          auto connectionData = make_shared<ISessionDescriptionTypes::ConnectionData>();
          connectionData->mRTP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();

          connectionData->mRTP->mNetType = ((bool)sdp.mCLine) ? sdp.mCLine->mNetType : String();
          connectionData->mRTP->mAddrType = ((bool)sdp.mCLine) ? sdp.mCLine->mAddrType : String();
          connectionData->mRTP->mConnectionAddress = ((bool)sdp.mCLine) ? sdp.mCLine->mConnectionAddress : String();

          details->mConnectionData = connectionData;
        }
      }

      //-----------------------------------------------------------------------
      static IDTLSTransportTypes::Roles toDtlsRole(const char *setup)
      {
        String str(setup);

        static const char *checkSetup[] =
        {
          "active",
          "passive",
          "actpass",
          "holdconn"
        };
        static IDTLSTransportTypes::Roles resultRole[] =
        {
          IDTLSTransportTypes::Role_Client,
          IDTLSTransportTypes::Role_Server,
          IDTLSTransportTypes::Role_Auto,
          IDTLSTransportTypes::Role_Auto
        };

        for (size_t index = 0; index <= (sizeof(resultRole) / sizeof(resultRole[0])); ++index) {
          if (0 == str.compareNoCase(checkSetup[index])) return resultRole[index];
        }
        return IDTLSTransportTypes::Role_Auto;
      }

      //-----------------------------------------------------------------------
      static void convertCrypto(
                                const ISDPTypes::ACryptoLineList &inCryptoLines,
                                ISRTPSDESTransportTypes::Parameters &outCrypto
                                )
      {
        for (auto iter = inCryptoLines.begin(); iter != inCryptoLines.end(); ++iter) {
          auto &acrypto = *(*iter);

          ISRTPSDESTransportTypes::CryptoParameters cryptoParams;
          cryptoParams.mTag = SafeInt<decltype(cryptoParams.mTag)>(acrypto.mTag);
          cryptoParams.mCryptoSuite = acrypto.mCryptoSuite;

          for (auto iterKeyParms = acrypto.mKeyParams.begin(); iterKeyParms != acrypto.mKeyParams.end(); ++iterKeyParms) {
            auto &akeyParam = (*iterKeyParms);
            ISRTPSDESTransportTypes::KeyParameters keyParams;
            keyParams.mKeyMethod = akeyParam.first;

            IHelper::SplitMap keyInfoSplit;
            IHelper::split(akeyParam.second, keyInfoSplit, "|");
            IHelper::splitTrim(keyInfoSplit);
            IHelper::splitPruneEmpty(keyInfoSplit);
            ORTC_THROW_INVALID_PARAMETERS_IF(keyInfoSplit.size() < 1);

            keyParams.mKeySalt = keyInfoSplit[0];
            if (keyInfoSplit.size() > 1) {
              keyParams.mLifetime = keyInfoSplit[1];
            }
            if (keyInfoSplit.size() > 2) {
              IHelper::SplitMap mkiSplit;
              IHelper::split(keyInfoSplit[2], mkiSplit, "|");
              IHelper::splitTrim(mkiSplit);
              IHelper::splitPruneEmpty(mkiSplit);
              ORTC_THROW_INVALID_PARAMETERS_IF(mkiSplit.size() < 2);

              keyParams.mMKIValue = mkiSplit[0];
              try {
                keyParams.mMKILength = Numeric<decltype(keyParams.mMKILength)>(mkiSplit[1]);
              } catch (const Numeric<decltype(keyParams.mMKILength)>::ValueOutOfRange &) {
                ORTC_THROW_INVALID_PARAMETERS("mki length value is out of range: " + mkiSplit[1]);
              }
            }

            cryptoParams.mKeyParams.push_back(keyParams);
          }

          for (auto iterSessionParms = acrypto.mSessionParams.begin(); iterSessionParms != acrypto.mSessionParams.end(); ++iterSessionParms) {
            auto &asessionParam = (*iterSessionParms);
            cryptoParams.mSessionParams.push_back(asessionParam);
          }

          outCrypto.mCryptoParams.push_back(cryptoParams);
        }
      }

      //-----------------------------------------------------------------------
      static void convertDTLSFingerprints(
                                          const ISDPTypes::AFingerprintLineList &inFingerprintLines,
                                          IDTLSTransportTypes::Parameters &outParameters
                                          )
      {

        for (auto iter = inFingerprintLines.begin(); iter != inFingerprintLines.end(); ++iter) {
          auto &afingerprint = *(*iter);
          ICertificateTypes::Fingerprint certFingerprint;
          certFingerprint.mAlgorithm = afingerprint.mHashFunc;
          certFingerprint.mValue = afingerprint.mFingerprint;
          outParameters.mFingerprints.push_back(certFingerprint);
        }
      }

      //-----------------------------------------------------------------------
      static IICETypes::CandidatePtr convertCandidate(const ISDPTypes::ACandidateLine &acandidate)
      {
        auto candidate = make_shared<IICETypes::Candidate>();

        candidate->mFoundation = acandidate.mFoundation;
        candidate->mComponent = static_cast<decltype(candidate->mComponent)>(static_cast<std::underlying_type<decltype(candidate->mComponent)>::type>(SafeInt<std::underlying_type<decltype(candidate->mComponent)>::type>(acandidate.mComponentID)));
        if (!((candidate->mComponent >= IICETypes::Component_First) &&
              (candidate->mComponent <= IICETypes::Component_Last))) {
          ORTC_THROW_INVALID_PARAMETERS("component is out of range: " + string(acandidate.mComponentID));
        }
        candidate->mPriority = acandidate.mPriority;
        candidate->mProtocol = IICETypes::toProtocol(acandidate.mTransport);
        candidate->mIP = acandidate.mConnectionAddress;
        candidate->mPort = acandidate.mPort;
        candidate->mCandidateType = IICETypes::toCandidateType(acandidate.mCandidateType);
        candidate->mRelatedAddress = acandidate.mRelAddr;
        candidate->mRelatedPort = acandidate.mRelPort;

        for (auto iterExt = acandidate.mExtensionPairs.begin(); iterExt != acandidate.mExtensionPairs.end(); ++iterExt) {
          auto &extName = (*iterExt).first;
          auto &extValue = (*iterExt).second;

          if (0 == extName.compareNoCase("tcptype")) {
            candidate->mTCPType = IICETypes::toTCPCandidateType(extValue);
            continue;
          }
          if (0 == extName.compareNoCase("unfreezepriority")) {
            try {
              candidate->mUnfreezePriority = Numeric<decltype(candidate->mUnfreezePriority)>(extValue);
            }
            catch (const Numeric<decltype(candidate->mUnfreezePriority)>::ValueOutOfRange &) {
              ORTC_THROW_INVALID_PARAMETERS("unfreeze priority is out of range: " + extValue);
            }
            continue;
          }
          if (0 == extName.compareNoCase("interfacetype")) {
            candidate->mInterfaceType = extValue;
            continue;
          }
        }

        return candidate;
      }

      //-----------------------------------------------------------------------
      static void convertCandidates(
                                    const ISDPTypes::ACandidateLineList &inCandidateLines,
                                    ISessionDescriptionTypes::ICECandidateList &outRTPCandidates,
                                    ISessionDescriptionTypes::Transport::ParametersPtr &ioRTCPTransport
                                    )
      {
        for (auto iter = inCandidateLines.begin(); iter != inCandidateLines.end(); ++iter)
        {
          auto &acandidate = *(*iter);
          auto candidate = convertCandidate(acandidate);

          if (0 == acandidate.mComponentID) {
            outRTPCandidates.push_back(candidate);
          } else {
            if (!ioRTCPTransport) {
              ioRTCPTransport = make_shared<ISessionDescriptionTypes::Transport::Parameters>();
            }
            ioRTCPTransport->mICECandidates.push_back(candidate);
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::createTransports(
                                       const SDP &sdp,
                                       Description &ioDescription
                                       )
      {
        size_t index = 0;

        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter, ++index)
        {
          auto &mline = *(*iter);

          if (!mline.mAICEUFragLine) continue;  // no ice transport = no transport

          auto transport = make_shared<ISessionDescriptionTypes::Transport>();
          transport->mRTP = make_shared<ISessionDescriptionTypes::Transport::Parameters>();

          bool requireRTCP = (mline.mRTCPMux.hasValue() ? (!mline.mRTCPMux.value()) : true);
          if (requireRTCP) {
            transport->mRTCP = make_shared<ISessionDescriptionTypes::Transport::Parameters>();
          }

          transport->mRTP->mICEParameters = make_shared<IICETypes::Parameters>();
          transport->mRTP->mICEParameters->mUsernameFragment = ((bool)mline.mAICEUFragLine) ? mline.mAICEUFragLine->mICEUFrag : String();
          transport->mRTP->mICEParameters->mPassword = ((bool)mline.mAICEPwdLine) ? mline.mAICEPwdLine->mICEPwd : String();
          transport->mRTP->mICEParameters->mICELite = sdp.mICELite.hasValue() ? sdp.mICELite.value() : false;

          if (mline.mAFingerprintLines.size() > 0) {
            transport->mRTP->mDTLSParameters = make_shared<IDTLSTransportTypes::Parameters>();
            transport->mRTP->mDTLSParameters->mRole = ((bool)mline.mASetupLine) ? toDtlsRole(mline.mASetupLine->mSetup) : IDTLSTransportTypes::Role_Auto;
            convertDTLSFingerprints(mline.mAFingerprintLines, *(transport->mRTP->mDTLSParameters));
          }

          if (mline.mACryptoLines.size() > 0) {
            transport->mRTP->mSRTPSDESParameters = make_shared<ISRTPSDESTransportTypes::Parameters>();
            convertCrypto(mline.mACryptoLines, *transport->mRTP->mSRTPSDESParameters);
          }

          convertCandidates(mline.mACandidateLines, transport->mRTP->mICECandidates, transport->mRTCP);

          if ((mline.mEndOfCandidates.hasValue()) &&
              (mline.mEndOfCandidates.value())) {
            transport->mRTP->mEndOfCandidates = true;
            if (transport->mRTCP) {
              transport->mRTCP->mEndOfCandidates = true;
            }
          }

          if (mline.mAMIDLine) {
            transport->mID = mline.mAMIDLine->mMID;
          } else {
            transport->mID = createTransportIDFromIndex(index);
          }
          ioDescription.mTransports.push_back(transport);
        }
      }

      //-----------------------------------------------------------------------
      static ISessionDescriptionTypes::MediaDirections toDirection(ISDPTypes::Directions direction)
      {
        switch (direction)
        {
          case ISDPTypes::Direction_None: return ISessionDescriptionTypes::MediaDirection_Inactive;
          case ISDPTypes::Direction_Send: return ISessionDescriptionTypes::MediaDirection_SendOnly;
          case ISDPTypes::Direction_Receive: return ISessionDescriptionTypes::MediaDirection_ReceiveOnly;
          case ISDPTypes::Direction_SendReceive: return ISessionDescriptionTypes::MediaDirection_SendReceive;
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("sdp direction was not handled");
      }

      //-----------------------------------------------------------------------
      static void fillMediaLine(
                                size_t index,
                                const ISDPTypes::SDP &sdp,
                                const ISDPTypes::MLine &mline,
                                ISessionDescriptionTypes::Description &description,
                                ISessionDescriptionTypes::MediaLine &mediaLine
                                )
      {
        mediaLine.mDetails = make_shared<ISessionDescriptionTypes::MediaLine::Details>();
        mediaLine.mDetails->mInternalIndex = index;
        mediaLine.mDetails->mProtocol = mline.mProtoStr;

        mediaLine.mDetails->mConnectionData = make_shared<ISessionDescriptionTypes::ConnectionData>();
        mediaLine.mDetails->mConnectionData->mRTP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();
        mediaLine.mDetails->mConnectionData->mRTP->mPort = SafeInt<decltype(mediaLine.mDetails->mConnectionData->mRTP->mPort.mType)>(mline.mPort);

        if (mline.mCLine) {
          mediaLine.mDetails->mConnectionData->mRTP->mNetType = mline.mCLine->mNetType;
          mediaLine.mDetails->mConnectionData->mRTP->mAddrType = mline.mCLine->mAddrType;
          mediaLine.mDetails->mConnectionData->mRTP->mConnectionAddress = mline.mCLine->mConnectionAddress;
        }
        if (mline.mARTCPLine) {
          mediaLine.mDetails->mConnectionData->mRTCP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();
          mediaLine.mDetails->mConnectionData->mRTCP->mNetType = mline.mARTCPLine->mNetType;
          mediaLine.mDetails->mConnectionData->mRTCP->mAddrType = mline.mARTCPLine->mAddrType;
          mediaLine.mDetails->mConnectionData->mRTCP->mConnectionAddress = mline.mARTCPLine->mConnectionAddress;
          mediaLine.mDetails->mConnectionData->mRTCP->mPort = mline.mARTCPLine->mPort;
        }

        mediaLine.mMediaType = mline.mMedia;
        if (mline.mMediaDirection.hasValue()) {
          mediaLine.mDetails->mMediaDirection = toDirection(mline.mMediaDirection.value());
        } else if (sdp.mMediaDirection.hasValue()) {
          mediaLine.mDetails->mMediaDirection = toDirection(sdp.mMediaDirection.value());
        }

        String foundBundleID;
        String searchForTransportID;

        if (mline.mAMIDLine) {
          mediaLine.mID = mline.mAMIDLine->mMID;
          searchForTransportID = mediaLine.mID;

          // figure out if this transport is part of a bundle
          for (auto iter = sdp.mAGroupLines.begin(); iter != sdp.mAGroupLines.end(); ++iter)
          {
            auto &group = *(*iter);

            String firstBundleID;

            if (0 != group.mSemantic.compareNoCase("BUNDLE")) continue;
            for (auto iterID = group.mIdentificationTags.begin(); iterID != group.mIdentificationTags.end(); ++iterID)
            {
              auto &mid = (*iterID);
              if (firstBundleID.isEmpty()) firstBundleID = mid;
              if (mid != mediaLine.mID) continue;
              foundBundleID = firstBundleID;
              break;
            }

            if (foundBundleID.hasData()) break;
          }
        } else {
          mediaLine.mID = createMediaLineIDFromIndex(index);
          searchForTransportID = createTransportIDFromIndex(index);
        }

        for (auto iter = description.mTransports.begin(); iter != description.mTransports.end(); ++iter) {
          auto &transport = *(*iter);
          if (transport.mID != searchForTransportID) continue;
          mediaLine.mDetails->mPrivateTransportID = searchForTransportID;
        }

        if (foundBundleID.hasData()) {
          bool found = false;
          for (auto iter = description.mTransports.begin(); iter != description.mTransports.end(); ++iter) {
            auto &transport = *(*iter);
            if (transport.mID != foundBundleID) continue;
            mediaLine.mTransportID = foundBundleID;
            found = true;
          }
          if (!found) foundBundleID.clear();
        }

        if (foundBundleID.isEmpty()) {
          mediaLine.mTransportID = mediaLine.mDetails->mPrivateTransportID;
        }

        if (mediaLine.mTransportID == mediaLine.mDetails->mPrivateTransportID) {
          mediaLine.mDetails->mPrivateTransportID.clear();
        }
      }

      //-----------------------------------------------------------------------
      static void fixIntoCodecSpecificList(
                                           const ISDPTypes::StringList &formatSpecificList,
                                           ISDPTypes::KeyValueList &outKeyValues
                                           )
      {
        String params = IHelper::combine(formatSpecificList, ";");

        IHelper::SplitMap formatSplit;
        IHelper::split(params, formatSplit, ";");
        IHelper::splitTrim(formatSplit);
        IHelper::splitPruneEmpty(formatSplit);

        for (auto iter = formatSplit.begin(); iter != formatSplit.end(); ++iter) {
          auto &keyValue = (*iter).second;

          IHelper::SplitMap keyValueSplit;
          IHelper::split(keyValue, keyValueSplit, "=");
          IHelper::splitTrim(keyValueSplit);
          IHelper::splitPruneEmpty(keyValueSplit);
          ORTC_THROW_INVALID_PARAMETERS_IF(keyValueSplit.size() < 1);

          String key = keyValueSplit[0];
          String value = (keyValueSplit.size() > 1 ? keyValueSplit[1] : String());

          outKeyValues.push_back(ISDPTypes::KeyValuePair(key, value));
        }
      }

      //-----------------------------------------------------------------------
      void fillCodecFormatSpecific(
                                   IRTPTypes::SupportedCodecs supportedCodec,
                                   IRTPTypes::CodecCapability &codecCapability,
                                   const ISDPTypes::AFMTPLine &format
                                   )
      {
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_Unknown:  return;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:            {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            auto opusParams = make_shared<IRTPTypes::OpusCodecCapabilityParameters>();

            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("maxplaybackrate")) {
                try {
                  opusParams->mMaxPlaybackRate = Numeric<decltype(opusParams->mMaxPlaybackRate.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mMaxPlaybackRate.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus max playback rate is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("sprop-maxcapturerate")) {
                try {
                  opusParams->mSPropMaxCaptureRate = Numeric<decltype(opusParams->mSPropMaxCaptureRate.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mSPropMaxCaptureRate.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus sprop max capture rate is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("maxptime")) {
                try {
                  codecCapability.mMaxPTime = Milliseconds(Numeric<Milliseconds::rep>(value));
                } catch (const Numeric<Milliseconds::rep>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max ptime is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("ptime")) {
                try {
                  codecCapability.mPTime = Milliseconds(Numeric<Milliseconds::rep>(value));
                } catch (const Numeric<Milliseconds::rep>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("ptime is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("maxaveragebitrate")) {
                try {
                  opusParams->mMaxAverageBitrate = Numeric<decltype(opusParams->mMaxAverageBitrate.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mMaxAverageBitrate.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus max average bitrate is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("stereo")) {
                try {
                  opusParams->mStereo = Numeric<decltype(opusParams->mStereo.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mStereo.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus stero value is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("sprop-stereo")) {
                try {
                  opusParams->mSPropStereo = Numeric<decltype(opusParams->mSPropStereo.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mSPropStereo.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus sprop-stereo value is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("cbr")) {
                try {
                  opusParams->mCBR = Numeric<decltype(opusParams->mCBR.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mCBR.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus cbr value is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("useinbandfec")) {
                try {
                  opusParams->mUseInbandFEC = Numeric<decltype(opusParams->mUseInbandFEC.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mUseInbandFEC.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus use inband fec value is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("usedtx")) {
                try {
                  opusParams->mUseDTX = Numeric<decltype(opusParams->mUseDTX.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mUseDTX.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus use inband fec value is not valid: " + value);
                }
              }
            }
            codecCapability.mParameters = opusParams;
            break;
          }
          case IRTPTypes::SupportedCodec_Isac:            return;
          case IRTPTypes::SupportedCodec_G722:            return;
          case IRTPTypes::SupportedCodec_ILBC:            return;
          case IRTPTypes::SupportedCodec_PCMU:            return;
          case IRTPTypes::SupportedCodec_PCMA:            return;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:             {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            auto vp8Params = make_shared<IRTPTypes::VP8CodecCapabilityParameters>();
            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("max-fr")) {
                try {
                  vp8Params->mMaxFR = Numeric<decltype(vp8Params->mMaxFR.mType)>(value);
                } catch (const Numeric<decltype(vp8Params->mMaxFR.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("vp8 max fr is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-fs")) {
                try {
                  vp8Params->mMaxFS = Numeric<decltype(vp8Params->mMaxFS.mType)>(value);
                } catch (const Numeric<decltype(vp8Params->mMaxFS.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("vp8 max fs is not valid: " + value);
                }
              }
            }

            codecCapability.mParameters = vp8Params;
            break;
          }
          case IRTPTypes::SupportedCodec_VP9:             return;
          case IRTPTypes::SupportedCodec_H264:            {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            auto h264Params = make_shared<IRTPTypes::H264CodecCapabilityParameters>();
            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("packetization-mode")) {
                try {
                  h264Params->mPacketizationModes.push_back(Numeric<IRTPTypes::H264CodecCapabilityParameters::PacketizationModeList::value_type>(value));
                } catch (const Numeric<IRTPTypes::H264CodecCapabilityParameters::PacketizationModeList::value_type>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("h264 packetization mode is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("profile-level-id")) {
                try {
                  h264Params->mProfileLevelID = Numeric<decltype(h264Params->mProfileLevelID.mType)>(value, true, 16);
                } catch (const Numeric<decltype(h264Params->mProfileLevelID.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("profile level id is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-mbps")) {
                try {
                  h264Params->mMaxMBPS = Numeric<decltype(h264Params->mMaxMBPS.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxMBPS.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max mbps is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-smbps")) {
                try {
                  h264Params->mMaxSMBPS = Numeric<decltype(h264Params->mMaxSMBPS.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxSMBPS.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max smbps is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-fs")) {
                try {
                  h264Params->mMaxFS = Numeric<decltype(h264Params->mMaxFS.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxFS.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max fs is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-cpb")) {
                try {
                  h264Params->mMaxCPB = Numeric<decltype(h264Params->mMaxCPB.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxCPB.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max cpb is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-dpb")) {
                try {
                  h264Params->mMaxDPB = Numeric<decltype(h264Params->mMaxDPB.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxDPB.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max dpb is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-br")) {
                try {
                  h264Params->mMaxBR = Numeric<decltype(h264Params->mMaxBR.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxBR.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max br is not valid: " + value);
                }
              }
            }

            codecCapability.mParameters = h264Params;
            break;
          }

          // RTX
          case IRTPTypes::SupportedCodec_RTX:             {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            bool foundApt = false;

            auto rtxParams = make_shared<IRTPTypes::RTXCodecCapabilityParameters>();
            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("apt")) {
                try {
                  rtxParams->mApt = Numeric<decltype(rtxParams->mApt)>(value);
                  foundApt = true;
                } catch (const Numeric<decltype(rtxParams->mApt)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("rtx apt is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("rtx-time")) {
                try {
                  rtxParams->mRTXTime = Milliseconds(Numeric<Milliseconds::rep>(value));
                } catch (const Numeric<Milliseconds::rep>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("rtx time is not valid: " + value);
                }
              }
            }

            ORTC_THROW_INVALID_PARAMETERS_IF(!foundApt);

            codecCapability.mParameters = rtxParams;
            break;
          }

          // FEC
          case IRTPTypes::SupportedCodec_RED:             return;
          case IRTPTypes::SupportedCodec_ULPFEC:          return;
          case IRTPTypes::SupportedCodec_FlexFEC:         {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            bool foundRepairWindow = false;

            auto flexFECParams = make_shared<IRTPTypes::FlexFECCodecCapabilityParameters>();
            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("ToP")) {
                try {
                  ULONG top = Numeric<ULONG>(value);
                  flexFECParams->mToP = IRTPTypes::FlexFECCodecCapabilityParameters::ToPs(top);
                } catch (const Numeric<ULONG>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("flexfec ToP fr is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("L")) {
                try {
                  flexFECParams->mL = Numeric<decltype(flexFECParams->mL.mType)>(value);
                } catch (const Numeric<decltype(flexFECParams->mL.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("flexfec L is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("D")) {
                try {
                  flexFECParams->mD = Numeric<decltype(flexFECParams->mD.mType)>(value);
                } catch (const Numeric<decltype(flexFECParams->mD.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("flexfec D is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("repair-window")) {
                try {
                  flexFECParams->mRepairWindow = Microseconds(Numeric<Microseconds::rep>(value));
                  foundRepairWindow = true;
                } catch (const Numeric<Microseconds::rep>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("flexfec D is not valid: " + value);
                }
              }
            }

            ORTC_THROW_INVALID_PARAMETERS_IF(!foundRepairWindow);

            codecCapability.mParameters = flexFECParams;
            break;
          }

          case IRTPTypes::SupportedCodec_CN:              return;

          case IRTPTypes::SupportedCodec_TelephoneEvent:  return;
        }
      }

      //-----------------------------------------------------------------------
      bool fillCapabilities(
                            ISDPTypes::Locations location,
                            const ISDPTypes::SDP &sdp,
                            const ISDPTypes::MLine &mline,
                            ISessionDescriptionTypes::Description &description,
                            ISessionDescriptionTypes::MediaLine &mediaLine,
                            IRTPTypes::Capabilities &senderCapabilities,
                            IRTPTypes::Capabilities &receiverCapabilities
                            )
      {
        auto matchCodecKind = IRTPTypes::toCodecKind(mline.mMedia);
        if ((IRTPTypes::CodecKind_Audio != matchCodecKind) &&
            (IRTPTypes::CodecKind_Video != matchCodecKind)) {
          ZS_LOG_WARNING(Debug, internal::slog("unable to understand media kind") + ZS_PARAM("media", mline.mMedia));
          return false;
        }

        Milliseconds ptime {};
        Milliseconds maxPTime{};
        if (mline.mAPTimeLine) {
          ptime = mline.mAPTimeLine->mPTime;
        }
        if (mline.mAMaxPTimeLine) {
          maxPTime = mline.mAMaxPTimeLine->mMaxPTime;
        }

        bool foundULPFEC = false;
        bool foundRED = false;
        bool foundFlexFEC = false;

        for (auto iter = mline.mFmts.begin(); iter != mline.mFmts.end(); ++iter)
        {
          auto &payloadTypeStr = (*iter);

          ISDPTypes::PayloadType pt {};

          try {
            pt = Numeric<decltype(pt)>(payloadTypeStr);
          } catch (const Numeric<decltype(pt)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("media line payload type is not understood");
          }

          ISDPTypes::ARTPMapLinePtr rtpMap;
          ISDPTypes::AFMTPLinePtr format;

          for (auto iterRTPMaps = mline.mARTPMapLines.begin(); iterRTPMaps != mline.mARTPMapLines.end(); ++iterRTPMaps) {
            auto checkRTPMap = (*iterRTPMaps);
            if (checkRTPMap->mPayloadType != pt) continue;

            rtpMap = checkRTPMap;
            break;
          }

          for (auto iterFMTP = mline.mAFMTPLines.begin(); iterFMTP != mline.mAFMTPLines.end(); ++iterFMTP) {
            auto checkFMTP = (*iterFMTP);
            if (checkFMTP->mFormat != pt) continue;

            format = checkFMTP;
            break;
          }

          if (!rtpMap) {
            auto reservedType = IRTPTypes::toReservedCodec(pt);
            auto supportedType = IRTPTypes::toSupportedCodec(reservedType);
            auto codecKind = IRTPTypes::getCodecKind(supportedType);

            if (IRTPTypes::SupportedCodec_Unknown == supportedType) {
              ZS_LOG_WARNING(Debug, internal::slog("codec payload type is not understood") + ZS_PARAM("payload type", pt));
              continue;
            }

            rtpMap = make_shared<ISDPTypes::ARTPMapLine>(Noop{});
            rtpMap->mPayloadType = pt;
            rtpMap->mEncodingName = IRTPTypes::toString(supportedType);
            rtpMap->mClockRate = IRTPTypes::getDefaultClockRate(reservedType);
            rtpMap->mEncodingParameters = IRTPTypes::getDefaultNumberOfChannels(reservedType);
          }

          auto supportedType = IRTPTypes::toSupportedCodec(rtpMap->mEncodingName);
          if (IRTPTypes::SupportedCodec_Unknown == supportedType) {
            ZS_LOG_WARNING(Debug, internal::slog("codec payload type is not supported") + ZS_PARAM("codec name", supportedType));
            continue;
          }

          bool fillKind = false;
          auto codecKind = IRTPTypes::getCodecKind(supportedType);
          if (IRTPTypes::CodecKind_Audio == codecKind) {
            ORTC_THROW_INVALID_PARAMETERS_IF(IRTPTypes::CodecKind_Video == matchCodecKind);
            fillKind = true;
          } else if (IRTPTypes::CodecKind_Video == codecKind) {
            ORTC_THROW_INVALID_PARAMETERS_IF(IRTPTypes::CodecKind_Audio == matchCodecKind);
            fillKind = true;
          }

          auto codecCapability = make_shared<IRTPTypes::CodecCapability>();
          codecCapability->mName = rtpMap->mEncodingName;
          codecCapability->mKind = (fillKind ? String(IRTPTypes::toString(codecKind)) : String());
          codecCapability->mClockRate = rtpMap->mClockRate;
          codecCapability->mPreferredPayloadType = pt;
          codecCapability->mPTime = ptime;
          codecCapability->mNumChannels = rtpMap->mEncodingParameters;
          if (format) {
            fillCodecFormatSpecific(supportedType, *codecCapability, *format);
          }
          if (IRTPTypes::requiresCapabilityParameters(supportedType)) {
            ORTC_THROW_INVALID_PARAMETERS_IF(!codecCapability->mParameters);
          }

          switch (supportedType) {
            case IRTPTypes::SupportedCodec_RED:     foundRED = true; break;
            case IRTPTypes::SupportedCodec_ULPFEC:  foundULPFEC = true; break;
            case IRTPTypes::SupportedCodec_FlexFEC: foundFlexFEC = true; break;
            default: break;
          }

          for (auto iterFB = mline.mARTCPFBLines.begin(); iterFB != mline.mARTCPFBLines.end(); ++iterFB) {
            auto &fb = *(*iterFB);

            if (fb.mPayloadType.hasValue()) {
              if (fb.mPayloadType.value() != codecCapability->mPreferredPayloadType) continue;
            }
            IRTPTypes::RTCPFeedback fbInfo;
            fbInfo.mType = fb.mID;
            fbInfo.mParameter = fb.mParam1;
            codecCapability->mRTCPFeedback.push_back(fbInfo);
          }

          senderCapabilities.mCodecs.push_back(*codecCapability);
          receiverCapabilities.mCodecs.push_back(*codecCapability);
        }

        if (foundRED) {
          if (foundULPFEC) {
            senderCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC));
            receiverCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC));
          } else {
            senderCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED));
            receiverCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED));
          }
        }
        if (foundFlexFEC) {
          senderCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC));
          receiverCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC));
        }

        for (auto iter = mline.mAExtmapLines.begin(); iter != mline.mAExtmapLines.end(); ++iter) {
          auto &extmap = *(*iter);

          IRTPTypes::HeaderExtension ext;
          ext.mKind = IRTPTypes::toString(matchCodecKind);
          ext.mPreferredEncrypt = false;
          ext.mPreferredID = SafeInt<decltype(ext.mPreferredID)>(extmap.mID);
          ext.mURI = extmap.mURI;

          // remote sender goes to sender capabilities
          if (ISDPTypes::isApplicable(ISDPTypes::ActorRole_Sender, ISDPTypes::Location_Local, extmap.mDirection)) {
            senderCapabilities.mHeaderExtensions.push_back(ext);
          }
          // remote receiver goes to receiver capabilities
          if (ISDPTypes::isApplicable(ISDPTypes::ActorRole_Receiver, ISDPTypes::Location_Local, extmap.mDirection)) {
            receiverCapabilities.mHeaderExtensions.push_back(ext);
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      void SDPParser::createRTPMediaLines(
                                          Locations location,
                                          const SDP &sdp,
                                          Description &ioDescription
                                          )
      {
        size_t index = 0;

        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter, ++index)
        {
          auto &mline = *(*iter);

          if (ProtocolType_RTP != mline.mProto) continue;

          auto mediaLine = make_shared<ISessionDescriptionTypes::RTPMediaLine>();
          fillMediaLine(index, sdp, mline, ioDescription, *mediaLine);

          if (mediaLine->mTransportID.isEmpty()) {
            ZS_LOG_WARNING(Debug, internal::slog("could not match RTP media line to a transport (thus ignoring mline)") + mediaLine->toDebug());
            continue;
          }

          mediaLine->mSenderCapabilities = make_shared<IRTPTypes::Capabilities>();
          mediaLine->mReceiverCapabilities = make_shared<IRTPTypes::Capabilities>();

          fillCapabilities(location, sdp, mline, ioDescription, *mediaLine, *mediaLine->mSenderCapabilities, *mediaLine->mReceiverCapabilities);

          ioDescription.mRTPMediaLines.push_back(mediaLine);
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::createSCTPMediaLines(
                                           Locations location,
                                           const SDP &sdp,
                                           Description &ioDescription
                                           )
      {
        size_t index = 0;

        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter, ++index)
        {
          auto &mline = *(*iter);

          if (ProtocolType_SCTP != mline.mProto) continue;

          auto mediaLine = make_shared<ISessionDescriptionTypes::SCTPMediaLine>();
          fillMediaLine(index, sdp, mline, ioDescription, *mediaLine);

          if (mediaLine->mTransportID.isEmpty()) {
            ZS_LOG_WARNING(Debug, internal::slog("could not match SCTP media line to a transport (thus ignoring mline)") + mediaLine->toDebug());
            continue;
          }

          mediaLine->mCapabilities = make_shared<ISCTPTransportTypes::Capabilities>();

          if (mline.mASCTPPortLine) {
            mediaLine->mPort = SafeInt<decltype(mediaLine->mPort.mType)>(mline.mASCTPPortLine->mPort);
          }

          mediaLine->mCapabilities->mMaxMessageSize = SafeInt<decltype(mediaLine->mCapabilities->mMaxMessageSize)>(mline.mAMaxMessageSize ? mline.mAMaxMessageSize->mMaxMessageSize : 0xFFFF);

          ioDescription.mSCTPMediaLines.push_back(mediaLine);
        }
      }

      //-----------------------------------------------------------------------
      static void fillREDFormatParameters(
                                          const ISDPTypes::MLine &mline,
                                          IRTPTypes::Parameters &parameters
                                          )
      {
        String redStr(IRTPTypes::toString(IRTPTypes::SupportedCodec_RED));
        for (auto iterCodec = parameters.mCodecs.begin(); iterCodec != parameters.mCodecs.end(); ++iterCodec) {
          auto &codec = (*iterCodec);
          if (0 != codec.mName.compareNoCase(redStr)) continue;

          IRTPTypes::REDCodecParametersPtr redParameters;

          auto redPayloadType = codec.mPayloadType;
          for (auto iterFormats = mline.mAFMTPLines.begin(); iterFormats != mline.mAFMTPLines.end(); ++iterFormats) {
            auto &format = *(*iterFormats);
            if (format.mFormat != redPayloadType) continue;
            String combinedREDFormat = IHelper::combine(format.mFormatSpecific, "/");
            IHelper::SplitMap redSplit;
            IHelper::split(combinedREDFormat, redSplit, "/");
            IHelper::splitTrim(redSplit);
            IHelper::splitPruneEmpty(redSplit);
            for (auto iterRED = redSplit.begin(); iterRED != redSplit.end(); ++iterRED) {
              auto &redFormatPayloadTypeStr = (*iterRED).second;
              try {
                IRTPTypes::PayloadType redFormatPayloadType = Numeric<IRTPTypes::PayloadType>(redFormatPayloadTypeStr);
                if (nullptr == redParameters) redParameters = make_shared<IRTPTypes::REDCodecParameters>();
                redParameters->mPayloadTypes.push_back(redFormatPayloadType);
              } catch (const Numeric<ISDPTypes::PayloadType>::ValueOutOfRange &) {
                ORTC_THROW_INVALID_PARAMETERS("RED payload specific format is not valid: " + combinedREDFormat);
              }
            }
            break;
          }

          codec.mParameters = redParameters;
        }
      }

      //-----------------------------------------------------------------------
      static void fillStreamIDs(
                                const ISDPTypes::SDP &sdp,
                                const ISDPTypes::MLine &mline,
                                ISessionDescriptionTypes::RTPSender &sender
                                )
      {
        // scope: first check for MSID lines
        {
          for (auto iter = mline.mAMSIDLines.begin(); iter != mline.mAMSIDLines.end(); ++iter) {
            auto msidLine = *(*iter);
            if (sender.mMediaStreamTrackID.isEmpty()) {
              sender.mMediaStreamTrackID = msidLine.mAppData;
            }
            if (msidLine.mID.hasData()) {
              sender.mMediaStreamIDs.insert(msidLine.mID);
            }
          }

          if (sender.mMediaStreamIDs.size() > 0) return;
        }

        // scope: check the a=ssrc lines
        {
          for (auto iter = mline.mASSRCLines.begin(); iter != mline.mASSRCLines.end(); ++iter) {
            auto &ssrcLine = *(*iter);
            if (ssrcLine.mAttributeValues.size() < 1) continue;
            if (0 != ssrcLine.mAttributeValues.front().first.compareNoCase("msid")) continue;

            auto &msid = ssrcLine.mAttributeValues.front().second;
            String trackID;
            if (ssrcLine.mAttributeValues.size() > 1) {
              auto iterValues = ssrcLine.mAttributeValues.begin();
              ++iterValues;
              trackID = (*iterValues).first;
            }

            if (sender.mMediaStreamTrackID.isEmpty()) {
              sender.mMediaStreamTrackID = trackID;
            }
            if (msid.hasData()) {
              sender.mMediaStreamIDs.insert(msid);
            }
          }
        }

      }

      //-----------------------------------------------------------------------
      void SDPParser::createRTPSenderLines(
                                           Locations location,
                                           const SDP &sdp,
                                           Description &ioDescription
                                           )
      {
        size_t index = 0;

        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter, ++index)
        {
          auto &mline = *(*iter);

          if (ProtocolType_RTP != mline.mProto) continue;

          if (mline.mMediaDirection.hasValue()) {
            if (!isApplicable(ActorRole_Sender, Location_Local, mline.mMediaDirection.value())) continue;
          }

          auto sender = make_shared<ISessionDescriptionTypes::RTPSender>();
          sender->mDetails = make_shared<ISessionDescription::RTPSender::Details>();
          sender->mDetails->mInternalRTPMediaLineIndex = index;

          String mid = (mline.mAMIDLine ? mline.mAMIDLine->mMID : String());

          sender->mID = mid.isEmpty() ? createSenderIDFromIndex(index) : mid;
          sender->mRTPMediaLineID = mid.isEmpty() ? createMediaLineIDFromIndex(index) : mid;

          ISessionDescriptionTypes::RTPMediaLinePtr foundMediaLine;

          for (auto iterMedia = ioDescription.mRTPMediaLines.begin(); iterMedia != ioDescription.mRTPMediaLines.end(); ++iterMedia) {
            auto &mediaLine = (*iterMedia);

            if (mediaLine->mID != sender->mRTPMediaLineID) continue;
            foundMediaLine = mediaLine;
            break;
          }

          if (!foundMediaLine) {
            ZS_LOG_WARNING(Debug, internal::slog("did not find associated media line") + ZS_PARAM("media line id", sender->mRTPMediaLineID));
            continue;
          }

          sender->mParameters = IHelper::capabilitiesToParameters(*foundMediaLine->mSenderCapabilities);

          sender->mParameters->mMuxID = mid;
          sender->mParameters->mRTCP.mMux = (mline.mRTCPMux.hasValue() ? mline.mRTCPMux.value() : false);
          sender->mParameters->mRTCP.mReducedSize = (mline.mRTCPRSize.hasValue() ? mline.mRTCPRSize.value() : false);
          fillREDFormatParameters(mline, *sender->mParameters);
          fillStreamIDs(sdp, mline, *sender);

#define TODO_FIX_FOR_SIMULCAST_SDP_WHEN_SPECIFICATION_IS_MORE_SETTLED 1
#define TODO_FIX_FOR_SIMULCAST_SDP_WHEN_SPECIFICATION_IS_MORE_SETTLED 2

          IRTPTypes::EncodingParameters encoding;

          for (auto iterSSRC = mline.mASSRCLines.begin(); iterSSRC != mline.mASSRCLines.end(); ++iterSSRC) {
            auto &ssrc = *(*iterSSRC);
            if (ssrc.mAttributeValues.size() < 1) continue;
            if (0 != ssrc.mAttributeValues.front().first.compareNoCase("cname")) continue;
            sender->mParameters->mRTCP.mCName = ssrc.mAttributeValues.front().second;
            encoding.mSSRC = ssrc.mSSRC;
            break;
          }

          for (auto iterSSRC = mline.mASSRCGroupLines.begin(); iterSSRC != mline.mASSRCGroupLines.end(); ++iterSSRC) {
            auto &ssrc = *(*iterSSRC);
            if (0 != ssrc.mSemantics.compareNoCase("FID")) continue;
            ORTC_THROW_INVALID_PARAMETERS_IF(ssrc.mSSRCs.size() < 2);
            if (!encoding.mSSRC.hasValue()) encoding.mSSRC = ssrc.mSSRCs.front();
            encoding.mRTX = IRTPTypes::RTXParameters();
            encoding.mRTX.value().mSSRC = (*(++(ssrc.mSSRCs.begin())));
            break;
          }

          for (auto iterSSRC = mline.mASSRCGroupLines.begin(); iterSSRC != mline.mASSRCGroupLines.end(); ++iterSSRC) {
            auto &ssrc = *(*iterSSRC);
            if (0 != ssrc.mSemantics.compareNoCase("FEC-FR")) continue;
            ORTC_THROW_INVALID_PARAMETERS_IF(ssrc.mSSRCs.size() < 2);
            if (!encoding.mSSRC.hasValue()) encoding.mSSRC = ssrc.mSSRCs.front();
            encoding.mFEC = IRTPTypes::FECParameters();
            encoding.mFEC.value().mSSRC = (*(++(ssrc.mSSRCs.begin())));

            auto &mechanisms = foundMediaLine->mSenderCapabilities->mFECMechanisms;
            if (mechanisms.size() > 0) {
              encoding.mFEC.value().mMechanism = mechanisms.front();
            }
            break;
          }

          sender->mParameters->mEncodings.push_back(encoding);

          ioDescription.mRTPSenders.push_back(sender);
        }
      }

      //-----------------------------------------------------------------------
      ISDPTypes::DescriptionPtr SDPParser::createDescription(
                                                             Locations location,
                                                             const SDP &sdp
                                                             )
      {
        DescriptionPtr result(make_shared<Description>());

        try {
          createDescriptionDetails(sdp, *result);
          createTransports(sdp, *result);
          createRTPMediaLines(location, sdp, *result);
          createSCTPMediaLines(location, sdp, *result);
          createRTPSenderLines(location, sdp, *result);
        } catch (const SafeIntException &e) {
          ORTC_THROW_INVALID_PARAMETERS("value found out of legal value range" + string(e.m_code));
        }

        return result;
      }

      //-----------------------------------------------------------------------
      IICETypes::GatherCandidatePtr SDPParser::getCandidateFromSDP(const char *candidate)
      {
        String str(candidate);

        {
          String prefix = str.substr(0, strlen("a="));
          if (0 == prefix.compareNoCase("a=")) {
            str = str.substr(strlen("a="));
            str.trim();
          }
        }

        if (0 == str.compareNoCase("end-of-candidates")) {
          auto result = make_shared<IICETypes::CandidateComplete>();
          result->mComplete = true;
          return result;
        }

        IICETypes::CandidatePtr result;

        {
          String prefix = str.substr(0, strlen("candidate:"));
          if (0 == prefix.compareNoCase("candidate:")) {
            str = str.substr(strlen("candidate:"));
            str.trim();
          }
        }

        try {
          auto aCandidate = make_shared<ACandidateLine>(nullptr, str);
          result = internal::convertCandidate(*aCandidate);
        } catch (const SafeIntException &e) {
          ZS_LOG_WARNING(Debug, internal::slog("unable to convert to candidate") + ZS_PARAM("candidate", str) + ZS_PARAM("e", e.m_code));
          return result;
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, internal::slog("unable to convert to candidate") + ZS_PARAM("candidate", str));
          return result;
        }

        return result;
      }

    }  // namespace internal
  } // namespace adapter
} // namespace ortc
