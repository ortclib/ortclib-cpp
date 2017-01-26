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

#include <ortc/internal/ortc_Helper.h>

#include <ortc/adapter/IHelper.h>

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
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SDPParser
      #pragma mark

      //-----------------------------------------------------------------------
      void appendToBundle(
                          ISDPTypes::BundleMap &ioBundle,
                          const ISessionDescription::MediaLine &mediaLine
                          )
      {
        if (mediaLine.mID.isEmpty()) return;
        if (mediaLine.mTransportID.isEmpty()) return;

        auto found = ioBundle.find(mediaLine.mTransportID);
        if (found == ioBundle.end()) {
          ISDPTypes::BundledSet bundles;
          bundles.insert(mediaLine.mID);
          ioBundle[mediaLine.mID] = bundles;
          return;
        }

        auto &bundleSet = (*found).second;
        bundleSet.insert(mediaLine.mID);
      }

      //-----------------------------------------------------------------------
      void SDPParser::createSDPSessionLevel(
                                            const GeneratorOptions &options,
                                            const Description &description,
                                            SDP &ioSDP
                                            )
      {
        auto &result = ioSDP;
        result.mOLine = make_shared<OLine>(Noop{});
        result.mSLine = make_shared<SLine>(Noop{});
        result.mTLine = make_shared<TLine>(Noop{});

        if (description.mDetails) {
          result.mOLine->mUsername = description.mDetails->mUsername;
          result.mOLine->mSessionID = description.mDetails->mSessionID;
          result.mOLine->mSessionVersion = description.mDetails->mSessionVersion;
          if (description.mDetails->mConnectionData) {
            if (description.mDetails->mConnectionData->mRTP) {
              result.mOLine->mNetType = description.mDetails->mConnectionData->mRTP->mNetType;
              result.mOLine->mAddrType = description.mDetails->mConnectionData->mRTP->mAddrType;
              result.mOLine->mUnicastAddress = description.mDetails->mConnectionData->mRTP->mConnectionAddress;
            }
          }
          result.mSLine->mSessionName = description.mDetails->mSessionName;
        }

        ISDPTypes::BundleMap bundles;

        for (auto iter = description.mRTPMediaLines.begin(); iter != description.mRTPMediaLines.end(); ++iter) {
          auto &mediaLine = *(*iter);
          appendToBundle(bundles, mediaLine);
        }
        for (auto iter = description.mSCTPMediaLines.begin(); iter != description.mSCTPMediaLines.end(); ++iter) {
          auto &mediaLine = *(*iter);
          appendToBundle(bundles, mediaLine);
        }

        for (auto iter = bundles.begin(); iter != bundles.end(); ++iter) {
          auto &bundleID = (*iter).first;
          auto &bundleSet = (*iter).second;

          if (bundleSet.size() < 2) continue;

          auto groupLine = make_shared<AGroupLine>(Noop{});
          groupLine->mSemantic = "BUNDLE";

          for (auto iterSet = bundleSet.begin(); iterSet != bundleSet.end(); ++iterSet) {
            auto &mid = (*iterSet);
            groupLine->mIdentificationTags.push_back(mid);
          }

          result.mAGroupLines.push_back(groupLine);
        }
      }

      //-----------------------------------------------------------------------
      static void createSDPMediaLineBase(
                                         const ISessionDescriptionTypes::Description &description,
                                         const ISessionDescriptionTypes::MediaLine &mediaLine,
                                         ISDPTypes::SDP &ioSDP,
                                         ISDPTypes::MLine &ioMLine
                                         )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);

        mline.mMedia = mediaLine.mMediaType;
        mline.mPort = 9;
        mline.mProtoStr = mediaLine.mDetails->mProtocol;
        mline.mProto = ISDPTypes::toProtocolType(mline.mProtoStr);

        // figure out the port based upon the associated transport's ICE candidates
        for (auto iter = description.mTransports.begin(); iter != description.mTransports.end(); ++iter) {
          auto &transport = *(*iter);
          if (0 != transport.mID.compareNoCase(mediaLine.mTransportID)) continue;

          if (!transport.mRTP) break;
          if (transport.mRTP->mICECandidates.size() < 1) break;

          for (auto iterCandidate = transport.mRTP->mICECandidates.begin(); iterCandidate != transport.mRTP->mICECandidates.end(); ++iterCandidate) {
            auto &candidate = *(*iterCandidate);
            if (IICETypes::CandidateType_Host != candidate.mCandidateType) continue;
            mline.mPort = candidate.mPort;
          }

          if (9 != mline.mPort) break;
        }
      }

      //-----------------------------------------------------------------------
      static void createSDPRTPMediaLine(
                                        const ISessionDescriptionTypes::Description &description,
                                        const ISessionDescriptionTypes::RTPMediaLine &mediaLine,
                                        ISDPTypes::SDP &ioSDP,
                                        ISDPTypes::MLine &ioMLine
                                        )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        createSDPMediaLineBase(description, mediaLine, ioSDP, ioMLine);

        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mReceiverCapabilities);
        ORTC_THROW_INVALID_PARAMETERS_IF(mediaLine.mReceiverCapabilities->mCodecs.size() < 1);

        auto &codecs = mediaLine.mReceiverCapabilities->mCodecs;
        for (auto iter = codecs.begin(); iter != codecs.end(); ++iter) {
          auto &codec = (*iter);
          mline.mFmts.push_back(string(codec.mPreferredPayloadType));
        }
      }

      //-----------------------------------------------------------------------
      static void createSDPSCTPMediaLine(
                                         const ISessionDescriptionTypes::Description &description,
                                         const ISessionDescriptionTypes::SCTPMediaLine &mediaLine,
                                         ISDPTypes::SDP &ioSDP,
                                         ISDPTypes::MLine &ioMLine
                                         )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        createSDPMediaLineBase(description, mediaLine, ioSDP, ioMLine);

        mline.mFmts.push_back("webrtc-datachannel");
      }

      //-----------------------------------------------------------------------
      static void figureOutBundle(
                                  const ISessionDescriptionTypes::Description &description,
                                  const ISessionDescriptionTypes::MediaLine &mediaLine,
                                  ISDPTypes::SDP &ioSDP,
                                  ISessionDescriptionTypes::TransportPtr &outAnnounceTransport
                                  )
      {
        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);

        String searchTransport = mediaLine.mDetails->mPrivateTransportID;
        if (searchTransport.isEmpty()) {
          searchTransport = mediaLine.mTransportID;
        }

        for (auto iter = description.mTransports.begin(); iter != description.mTransports.end(); ++iter) {
          auto transport = (*iter);
          if (transport->mID != searchTransport) continue;
          outAnnounceTransport = transport;
        }

        if (searchTransport.hasData()) return;

        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails->mInternalIndex.hasValue());

        auto compareValue = mediaLine.mDetails->mInternalIndex.value();

        for (auto iter = description.mRTPMediaLines.begin(); iter != description.mRTPMediaLines.end(); ++iter) {
          auto &checkMediaLine = *(*iter);
          if (!checkMediaLine.mDetails) continue;
          if (!checkMediaLine.mDetails->mInternalIndex.hasValue()) continue;
          if (checkMediaLine.mDetails->mInternalIndex.value() < compareValue) {
            // no need to announce this transport
            outAnnounceTransport.reset();
            return;
          }
        }

        for (auto iter = description.mSCTPMediaLines.begin(); iter != description.mSCTPMediaLines.end(); ++iter) {
          auto &checkMediaLine = *(*iter);
          if (!checkMediaLine.mDetails) continue;
          if (!checkMediaLine.mDetails->mInternalIndex.hasValue()) continue;
          if (checkMediaLine.mDetails->mInternalIndex.value() < compareValue) {
            // no need to announce this transport
            outAnnounceTransport.reset();
            return;
          }
        }
      }

      //-----------------------------------------------------------------------
      static ISDPTypes::ACandidateLinePtr fillCandidate(const IICETypes::Candidate &candidate)
      {
        auto aCandidate = make_shared<ISDPTypes::ACandidateLine>(Noop{});
        aCandidate->mFoundation = candidate.mFoundation;
        aCandidate->mComponentID = SafeInt<decltype(aCandidate->mComponentID)>(static_cast<std::underlying_type<decltype(candidate.mComponent)>::type>(candidate.mComponent));
        aCandidate->mTransport = IICETypes::toString(candidate.mProtocol);
        aCandidate->mPriority = candidate.mPriority;
        aCandidate->mConnectionAddress = candidate.mIP;
        aCandidate->mPort = candidate.mPort;
        aCandidate->mCandidateType = IICETypes::toString(candidate.mCandidateType);
        aCandidate->mRelAddr = candidate.mRelatedAddress;
        aCandidate->mRelPort = candidate.mRelatedPort;

        if (IICETypes::Protocol_TCP == candidate.mProtocol) {
          aCandidate->mExtensionPairs.push_back(ISDPTypes::ACandidateLine::ExtensionPair("tcptype", IICETypes::toString(candidate.mTCPType)));
        }

        if (0 != candidate.mUnfreezePriority) {
          aCandidate->mExtensionPairs.push_back(ISDPTypes::ACandidateLine::ExtensionPair("unfreezepriority", string(candidate.mUnfreezePriority)));
        }
        if (candidate.mInterfaceType.hasData()) {
          aCandidate->mExtensionPairs.push_back(ISDPTypes::ACandidateLine::ExtensionPair("interfacetype", candidate.mInterfaceType));
        }
        return aCandidate;
      }

      //-----------------------------------------------------------------------
      static void fillCandidates(
                                 const ISessionDescriptionTypes::MediaLine &mediaLine,
                                 const ISessionDescriptionTypes::Transport &transport,
                                 ISDPTypes::MLine &ioMLine,
                                 const ISessionDescriptionTypes::ICECandidateList &candidates
                                 )
      {
        auto &mline = ioMLine;

        for (auto iter = candidates.begin(); iter != candidates.end(); ++iter) {
          auto &candidate = *(*iter);
          mline.mACandidateLines.push_back(fillCandidate(candidate));
        }
      }

      //-----------------------------------------------------------------------
      static void fillCandidates(
                                 const ISessionDescriptionTypes::MediaLine &mediaLine,
                                 const ISessionDescriptionTypes::Transport &transport,
                                 ISDPTypes::MLine &ioMLine
                                 )
      {
        auto &mline = ioMLine;

        // figure out if the end of candidates flag needs to be set
        if (transport.mRTP) {
          if (transport.mRTCP) {
            if ((transport.mRTP->mEndOfCandidates) &&
                (transport.mRTCP->mEndOfCandidates)) {
              mline.mEndOfCandidates = true;
            }
          } else {
            if (transport.mRTP->mEndOfCandidates) {
              mline.mEndOfCandidates = true;
            }
          }
        }

        // fill in candidates for RTP/RTCP
        if (transport.mRTP) fillCandidates(mediaLine, transport, mline, transport.mRTP->mICECandidates);
        if (transport.mRTCP) fillCandidates(mediaLine, transport, mline, transport.mRTCP->mICECandidates);
      }

      //-----------------------------------------------------------------------
      void fillMediaLine(
                         const SDPParser::GeneratorOptions &options,
                         const ISessionDescriptionTypes::Description &description,
                         const ISessionDescriptionTypes::MediaLine &mediaLine,
                         ISDPTypes::SDP &ioSDP,
                         ISDPTypes::MLine &ioMLine
                         )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);

        ISessionDescriptionTypes::TransportPtr transport;
        figureOutBundle(description, mediaLine, result, transport);

        if (transport) {
          fillCandidates(mediaLine, *transport, mline);
        } else {
          if (options.mBundleOnly) {
            mline.mBundleOnly = true;
          }
        }

        mline.mMediaDirection = ISDPTypes::toDirection(ISessionDescription::toString(mediaLine.mDetails->mMediaDirection));
        mline.mRTCPMux = true;
        mline.mRTCPRSize = true;
        mline.mAMIDLine = make_shared<ISDPTypes::AMIDLine>(Noop{});
        mline.mAMIDLine->mMID = mediaLine.mID;
        mline.mCLine = make_shared<ISDPTypes::CLine>(Noop{});
        mline.mCLine->mNetType = "IN";
        mline.mCLine->mAddrType = "IP4";
        mline.mCLine->mConnectionAddress = "0.0.0.0";
        mline.mARTCPLine = make_shared<ISDPTypes::ARTCPLine>(Noop{});
        mline.mARTCPLine->mPort = 0;
        mline.mARTCPLine->mNetType = "IN";
        mline.mARTCPLine->mAddrType = "IP4";
        mline.mARTCPLine->mConnectionAddress = "0.0.0.0";

        // select port for RTCP based on RTCP candidate (if applicable)
        if (transport) {
          if (transport->mRTP) {
            mline.mAICEUFragLine = make_shared<ISDPTypes::AICEUFragLine>(Noop{});
            mline.mAICEPwdLine = make_shared<ISDPTypes::AICEPwdLine>(Noop{});
            ORTC_THROW_INVALID_PARAMETERS_IF(!transport->mRTP->mICEParameters);
            mline.mAICEUFragLine->mICEUFrag = transport->mRTP->mICEParameters->mUsernameFragment;
            mline.mAICEPwdLine->mICEPwd = transport->mRTP->mICEParameters->mPassword;
            if (transport->mRTP->mICEParameters->mICELite) result.mICELite = true;

            if (transport->mRTP->mDTLSParameters) {
              for (auto iter = transport->mRTP->mDTLSParameters->mFingerprints.begin(); iter != transport->mRTP->mDTLSParameters->mFingerprints.end(); ++iter) {
                auto &fingerprint = (*iter);
                auto fingerprintLine = make_shared<ISDPTypes::AFingerprintLine>(Noop{});
                fingerprintLine->mHashFunc = fingerprint.mAlgorithm;
                fingerprintLine->mFingerprint = fingerprint.mValue;
                mline.mAFingerprintLines.push_back(fingerprintLine);
              }
              mline.mASetupLine = make_shared<ISDPTypes::ASetupLine>(Noop{});
              auto role = transport->mRTP->mDTLSParameters->mRole;
              switch (role) {
                case IDTLSTransportTypes::Role_Auto: mline.mASetupLine->mSetup = "actpass"; break;
                case IDTLSTransportTypes::Role_Client: mline.mASetupLine->mSetup = "active"; break;
                case IDTLSTransportTypes::Role_Server: mline.mASetupLine->mSetup = "passive"; break;
              }
            }
            if (transport->mRTP->mSRTPSDESParameters) {
              for (auto iter = transport->mRTP->mSRTPSDESParameters->mCryptoParams.begin(); iter != transport->mRTP->mSRTPSDESParameters->mCryptoParams.end(); ++iter) {
                auto &crypto = (*iter);
                auto cryptoLine = make_shared<ISDPTypes::ACryptoLine>(Noop{});
                cryptoLine->mTag = crypto.mTag;
                cryptoLine->mCryptoSuite = crypto.mCryptoSuite;
                for (auto iterKeyParam = crypto.mKeyParams.begin(); iterKeyParam != crypto.mKeyParams.end(); ++iterKeyParam) {
                  auto &keyParam = (*iterKeyParam);

                  IHelper::SplitMap splitsMKI;
                  splitsMKI[0] = keyParam.mMKIValue;
                  splitsMKI[1] = (0 != keyParam.mMKILength ? string(keyParam.mMKILength) : String());
                  IHelper::splitPruneEmpty(splitsMKI);

                  IHelper::SplitMap splits;
                  splits[0] = keyParam.mKeySalt;
                  splits[1] = keyParam.mLifetime;
                  splits[2] = IHelper::combine(splitsMKI, ":");
                  IHelper::splitPruneEmpty(splits);

                  cryptoLine->mKeyParams.push_back(ISDPTypes::KeyValuePair(keyParam.mKeyMethod, IHelper::combine(splits,"|")));
                }
                cryptoLine->mSessionParams = crypto.mSessionParams;
                mline.mACryptoLines.push_back(cryptoLine);
              }
            }
          }

          if (transport->mRTCP) {
            for (auto iter = transport->mRTCP->mICECandidates.begin(); iter != transport->mRTCP->mICECandidates.end(); ++iter) {
              auto &candidate = *(*iter);
              if (IICETypes::CandidateType_Host != candidate.mCandidateType) continue;
              mline.mARTCPLine->mPort = candidate.mPort;
              break;
            }
          }
        }
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             const IRTPTypes::OpusCodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (source.mMaxPlaybackRate.hasValue()) {
          result.mFormatSpecific.push_back(String("maxplaybackrate=") + string(source.mMaxPlaybackRate.value()) + ";");
        }
        if (source.mMaxAverageBitrate.hasValue()) {
          result.mFormatSpecific.push_back(String("maxaveragebitrate=") + string(source.mMaxAverageBitrate.value()) + ";");
        }
        if (source.mStereo.hasValue()) {
          result.mFormatSpecific.push_back(String("stereo=") + (source.mStereo.value() ? "true" : "false") + ";");
        }
        if (source.mCBR.hasValue()) {
          result.mFormatSpecific.push_back(String("cbr=") + (source.mCBR.value() ? "true" : "false") + ";");
        }
        if (source.mUseInbandFEC.hasValue()) {
          result.mFormatSpecific.push_back(String("useinbandfec=") + (source.mUseInbandFEC.value() ? "true" : "false") + ";");
        }
        if (source.mUseDTX.hasValue()) {
          result.mFormatSpecific.push_back(String("dtx=") + (source.mUseDTX.value() ? "true" : "false") + ";");
        }
        if (source.mSPropMaxCaptureRate.hasValue()) {
          result.mFormatSpecific.push_back(String("sprop-maxcapturerate=") + string(source.mSPropMaxCaptureRate.value()) + ";");
        }
        if (source.mSPropStereo.hasValue()) {
          result.mFormatSpecific.push_back(String("sprop-stereo=") + (source.mSPropStereo.value() ? "true" : "false") + ";");
        }
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             const IRTPTypes::VP8CodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (source.mMaxFR.hasValue()) {
          result.mFormatSpecific.push_back(String("max-fr=") + string(source.mMaxFR.value()) + ";");
        }
        if (source.mMaxFS.hasValue()) {
          result.mFormatSpecific.push_back(String("max-fs=") + string(source.mMaxFS.value()) + ";");
        }
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             const IRTPTypes::H264CodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (source.mProfileLevelID.hasValue()) {
          result.mFormatSpecific.push_back(String("profile-level-id=") + string(source.mProfileLevelID.value()) + ";");
        }
        for (auto iter = source.mPacketizationModes.begin(); iter != source.mPacketizationModes.end(); ++iter) {
          auto mode = (*iter);
          result.mFormatSpecific.push_back(String("packetization-mode=") + string(mode) + ";");
        }
        if (source.mMaxMBPS.hasValue()) {
          result.mFormatSpecific.push_back(String("max-mbps=") + string(source.mMaxMBPS.value()) + ";");
        }
        if (source.mMaxSMBPS.hasValue()) {
          result.mFormatSpecific.push_back(String("max-smbps=") + string(source.mMaxSMBPS.value()) + ";");
        }
        if (source.mMaxFS.hasValue()) {
          result.mFormatSpecific.push_back(String("max-fs=") + string(source.mMaxFS.value()) + ";");
        }
        if (source.mMaxCPB.hasValue()) {
          result.mFormatSpecific.push_back(String("max-cpb=") + string(source.mMaxCPB.value()) + ";");
        }
        if (source.mMaxDPB.hasValue()) {
          result.mFormatSpecific.push_back(String("max-dpb=") + string(source.mMaxDPB.value()) + ";");
        }
        if (source.mMaxBR.hasValue()) {
          result.mFormatSpecific.push_back(String("max-br=") + string(source.mMaxBR.value()) + ";");
        }
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             const IRTPTypes::RTXCodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (Milliseconds() != source.mRTXTime) {
          result.mFormatSpecific.push_back(String("apt=") + string(source.mApt) + ";rtx-time=" + string(source.mRTXTime.count()) + ";");
          return;
        }
        result.mFormatSpecific.push_back(String("apt=") + string(source.mApt) + ";");
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             const IRTPTypes::FlexFECCodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (source.mL.hasValue()) {
          result.mFormatSpecific.push_back(String("L:") + string(source.mL.value()) + ";");
        }
        if (source.mD.hasValue()) {
          result.mFormatSpecific.push_back(String("D:") + string(source.mD.value()) + ";");
        }
        if (source.mToP.hasValue()) {
          result.mFormatSpecific.push_back(String("ToP:") + string(static_cast<std::underlying_type<IRTPTypes::FlexFECCodecCapabilityParameters::ToPs>::type>(source.mToP.value())) + ";");
        }
        if (Microseconds() != source.mRepairWindow) {
          result.mFormatSpecific.push_back(String("repair-window:") + string(source.mRepairWindow.count()));
        }
      }

      //-----------------------------------------------------------------------
      static ISDPTypes::AFMTPLinePtr fillFormat(
                                                const IRTPTypes::CodecCapability &codec,
                                                ISDPTypes::PayloadType pt
                                                )
      {
        if (!codec.mParameters) return nullptr;

        auto format = make_shared<ISDPTypes::AFMTPLine>(Noop{});
        format->mFormat = pt;

        auto supportedCodec = IRTPTypes::toSupportedCodec(codec.mName);
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_Unknown: break;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:     {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::OpusCodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }
          case IRTPTypes::SupportedCodec_Isac:     break;
          case IRTPTypes::SupportedCodec_G722:     break;
          case IRTPTypes::SupportedCodec_ILBC:     break;
          case IRTPTypes::SupportedCodec_PCMU:     break;
          case IRTPTypes::SupportedCodec_PCMA:     break;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:      {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::VP8CodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }
          case IRTPTypes::SupportedCodec_VP9:      break;
          case IRTPTypes::SupportedCodec_H264:     {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::H264CodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }

          // RTX
          case IRTPTypes::SupportedCodec_RTX: {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::RTXCodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }

          // FEC
          case IRTPTypes::SupportedCodec_RED:      break;
          case IRTPTypes::SupportedCodec_ULPFEC:   break;
          case IRTPTypes::SupportedCodec_FlexFEC: {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::FlexFECCodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }

          case IRTPTypes::SupportedCodec_CN:       break;

          case IRTPTypes::SupportedCodec_TelephoneEvent:   break;
        }

        if (format->mFormatSpecific.size() < 1) return nullptr;

        return format;
      }

      //-----------------------------------------------------------------------
      static void fillRTPMediaLine(
                                   const SDPParser::GeneratorOptions &options,
                                   const ISessionDescriptionTypes::Description &description,
                                   const ISessionDescriptionTypes::RTPMediaLine &mediaLine,
                                   ISDPTypes::SDP &ioSDP,
                                   ISDPTypes::MLine &ioMLine
                                   )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        if (mediaLine.mReceiverCapabilities) {
          Milliseconds ptime {};
          Milliseconds maxPTime {};
          bool ignorePTime = false;

          for (auto iter = mediaLine.mReceiverCapabilities->mCodecs.begin(); iter != mediaLine.mReceiverCapabilities->mCodecs.end(); ++iter) {
            auto &codec = (*iter);
            auto rtpmap = make_shared<ISDPTypes::ARTPMapLine>(Noop{});
            rtpmap->mPayloadType = codec.mPreferredPayloadType;
            rtpmap->mEncodingName = codec.mName;
            rtpmap->mClockRate = codec.mClockRate.hasValue() ? codec.mClockRate.value() : 0;
            if (!ignorePTime) {
              if (Milliseconds() != codec.mPTime) {
                if (Milliseconds() == ptime) {
                  ptime = codec.mPTime;
                } else {
                  ignorePTime = true;
                }
              }
            }
            if (Milliseconds() != codec.mMaxPTime) {
              if (Milliseconds() == maxPTime) {
                maxPTime = codec.mMaxPTime;
              } else if (codec.mMaxPTime.count() < maxPTime.count()) {
                maxPTime = codec.mMaxPTime;
              }
            }
            if (codec.mNumChannels.hasValue()) {
              rtpmap->mEncodingParameters = codec.mNumChannels.value();
            }
            mline.mARTPMapLines.push_back(rtpmap);
            auto format = fillFormat(codec, codec.mPreferredPayloadType);
            if (format) {
              mline.mAFMTPLines.push_back(format);
            }
          }
          if ((!ignorePTime) &&
              (Milliseconds() != ptime)) {
            mline.mAPTimeLine = make_shared<ISDPTypes::APTimeLine>(Noop{});
            mline.mAPTimeLine->mPTime = ptime;
          }
          if (Milliseconds() != maxPTime) {
            mline.mAMaxPTimeLine = make_shared<ISDPTypes::AMaxPTimeLine>(Noop{});
            mline.mAMaxPTimeLine->mMaxPTime = maxPTime;
          }
        }
      }
      
      //-----------------------------------------------------------------------
      static void addCNAME(
                           ISDPTypes::MLine &mline,
                           ISDPTypes::SSRCType ssrc,
                           const String &cname
                           )
      {
        if (!cname.hasData()) return;

        auto ssrcRTCP = make_shared<ISDPTypes::ASSRCLine>(Noop{});
        ssrcRTCP->mSSRC = ssrc;
        ssrcRTCP->mAttributeValues.push_back(ISDPTypes::KeyValuePair("cname", cname));
        mline.mASSRCLines.push_back(ssrcRTCP);
      }

      //-----------------------------------------------------------------------
      static void addSSRCGroup(
                              ISDPTypes::MLine &mline,
                              const char *groupSemantic,
                              ISDPTypes::SSRCType mainSSRC,
                              ISDPTypes::SSRCType relatedSSRC,
                              const String &cname
                              )
      {
        auto ssrcFID = make_shared<ISDPTypes::ASSRCGroupLine>(Noop{});
        ssrcFID->mSemantics = String(groupSemantic);
        ssrcFID->mSSRCs.push_back(mainSSRC);
        ssrcFID->mSSRCs.push_back(relatedSSRC);
        mline.mASSRCGroupLines.push_back(ssrcFID);
        addCNAME(mline, relatedSSRC, cname);
      }

      //-----------------------------------------------------------------------
      void SDPParser::createSDPMediaLevel(
                                          const GeneratorOptions &options,
                                          const Description &description,
                                          SDP &ioSDP
                                          )
      {
        auto &result = ioSDP;

        if ((description.mRTPMediaLines.size() < 1) &&
            (description.mSCTPMediaLines.size() < 1)) return;

        size_t largestIndex {};
        for (auto iter = description.mRTPMediaLines.begin(); iter != description.mRTPMediaLines.end(); ++iter) {
          auto &mediaLine = *(*iter);
          ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);
          ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails->mInternalIndex.hasValue());
          auto index = mediaLine.mDetails->mInternalIndex.value();
          if (index > largestIndex) largestIndex = index;
        }
        for (auto iter = description.mSCTPMediaLines.begin(); iter != description.mSCTPMediaLines.end(); ++iter) {
          auto &mediaLine = *(*iter);
          ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);
          ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails->mInternalIndex.hasValue());
          auto index = mediaLine.mDetails->mInternalIndex.value();
          if (index > largestIndex) largestIndex = index;
        }

        for (size_t index = 0; index <= largestIndex; ++index) {
          {
            // scope: check rtp media lines for entry
            {
              for (auto iter = description.mRTPMediaLines.begin(); iter != description.mRTPMediaLines.end(); ++iter) {
                auto &mediaLine = *(*iter);
                auto value = mediaLine.mDetails->mInternalIndex.value();
                if (index != value) continue;
                auto mline = make_shared<ISDPTypes::MLine>(Noop{});
                createSDPRTPMediaLine(description, mediaLine, result, *mline);
                result.mMLines.push_back(mline);
                fillMediaLine(options, description, mediaLine, result, *mline);
                fillRTPMediaLine(options, description, mediaLine, result, *mline);

                if (mediaLine.mReceiverCapabilities) {
                  for (auto iterCodec = mediaLine.mReceiverCapabilities->mCodecs.begin(); iterCodec != mediaLine.mReceiverCapabilities->mCodecs.end(); ++iterCodec) {
                    auto &codec = (*iterCodec);
                    for (auto iterFB = codec.mRTCPFeedback.begin(); iterFB != codec.mRTCPFeedback.end(); ++iterFB) {
                      auto &fb = (*iterFB);
                      auto aFB = make_shared<ISDPTypes::ARTCPFBLine>(Noop{});
                      aFB->mPayloadType = codec.mPreferredPayloadType;
                      aFB->mID = fb.mType;
                      aFB->mParam1 = fb.mParameter;
                      mline->mARTCPFBLines.push_back(aFB);
                    }
                  }

                  for (auto iterHeaderExt = mediaLine.mReceiverCapabilities->mHeaderExtensions.begin(); iterHeaderExt != mediaLine.mReceiverCapabilities->mHeaderExtensions.end(); ++iterHeaderExt) {
                    auto &headerExt = (*iterHeaderExt);
                    auto aExt = make_shared<ISDPTypes::AExtmapLine>(Noop{});
                    aExt->mURI = headerExt.mURI;
                    aExt->mDirection = ISDPTypes::Direction_SendReceive;
                    aExt->mID = headerExt.mPreferredID;
                    mline->mAExtmapLines.push_back(aExt);
                  }
                }

                for (auto iterSender = description.mRTPSenders.begin(); iterSender != description.mRTPSenders.end(); ++iterSender) {
                  auto &sender = *(*iterSender);

                  if (sender.mRTPMediaLineID != mediaLine.mID)  continue; // do not output for wrong media line

                  if (!sender.mParameters) continue;
                  if (sender.mParameters->mEncodings.size() < 1) continue;
                  auto &encoding = *(sender.mParameters->mEncodings.begin());
                  if (!encoding.mSSRC.mHasValue) continue;

                  addCNAME(*mline, encoding.mSSRC.value(), sender.mParameters->mRTCP.mCName);

                  auto aMSLabel = make_shared<ISDPTypes::ASSRCLine>(Noop{});
                  aMSLabel->mSSRC = encoding.mSSRC.value();

                  if (sender.mMediaStreamTrackID.hasData()) {
                    if (sender.mMediaStreamIDs.size() > 0) {
                      for (auto iterMSID = sender.mMediaStreamIDs.begin(); iterMSID != sender.mMediaStreamIDs.end(); ++iterMSID) {
                        auto &msid = (*iterMSID);
                        auto ssrcMSID = make_shared<ISDPTypes::ASSRCLine>(Noop{});
                        auto aMSID = make_shared<ISDPTypes::AMSIDLine>(Noop{});
                        ssrcMSID->mSSRC = encoding.mSSRC.value();
                        ssrcMSID->mAttributeValues.push_back(ISDPTypes::KeyValuePair("msid", msid));
                        ssrcMSID->mAttributeValues.push_back(ISDPTypes::KeyValuePair(sender.mMediaStreamTrackID, String()));
                        aMSID->mID = msid;
                        aMSID->mAppData = sender.mMediaStreamTrackID;

                        if (aMSLabel->mAttributeValues.size() < 1) {
                          aMSLabel->mAttributeValues.push_back(ISDPTypes::KeyValuePair("mslabel", msid));
                        } else {
                          aMSLabel->mAttributeValues.push_back(ISDPTypes::KeyValuePair(msid, String()));
                        }

                        mline->mASSRCLines.push_back(ssrcMSID);
                        mline->mAMSIDLines.push_back(aMSID);
                      }
                    }

#define NOTE_THIS_IS_FOR_LEGACY_REMOVE_WHEN_POSSIBLE 1
#define NOTE_THIS_IS_FOR_LEGACY_REMOVE_WHEN_POSSIBLE 2

                    // NOTE: LEGACY SUPPORT ONLY
                    auto ssrcLabel = make_shared<ISDPTypes::ASSRCLine>(Noop{});
                    ssrcLabel->mSSRC = encoding.mSSRC.value();
                    ssrcLabel->mAttributeValues.push_back(ISDPTypes::KeyValuePair("label", sender.mMediaStreamTrackID));
                    mline->mASSRCLines.push_back(ssrcLabel);
                    if (aMSLabel->mAttributeValues.size() > 0) {
                      mline->mASSRCLines.push_back(aMSLabel);
                    }
                  }
                  if (encoding.mRTX.hasValue()) {
                    if (encoding.mRTX.value().mSSRC.hasValue()) {
                      addSSRCGroup(*mline, "FID", encoding.mSSRC.value(), encoding.mRTX.value().mSSRC.value(), sender.mParameters->mRTCP.mCName);
                    }
                  }
                  if (encoding.mFEC.hasValue()) {
                    if (encoding.mFEC.value().mSSRC.hasValue()) {
                      addSSRCGroup(*mline, "FEC-FR", encoding.mSSRC.value(), encoding.mFEC.value().mSSRC.value(), sender.mParameters->mRTCP.mCName);
                    }
                  }
                }
                goto found;
              }
            }

            // scope: check sctp media lines for entry
            {
              for (auto iter = description.mSCTPMediaLines.begin(); iter != description.mSCTPMediaLines.end(); ++iter) {
                auto &mediaLine = *(*iter);
                auto value = mediaLine.mDetails->mInternalIndex.value();
                if (index != value) continue;
                auto mline = make_shared<ISDPTypes::MLine>(Noop{});
                createSDPSCTPMediaLine(description, mediaLine, result, *mline);
                result.mMLines.push_back(mline);
                fillMediaLine(options, description, mediaLine, result, *mline);
                if (mediaLine.mCapabilities) {
                  auto sctpPort = make_shared<ASCTPPortLine>(Noop{});
                  sctpPort->mPort = mediaLine.mPort;
                  auto maxMessageSize = make_shared<AMaxMessageSizeLine>(Noop{});
                  maxMessageSize->mMaxMessageSize = mediaLine.mCapabilities->mMaxMessageSize;
                  mline->mASCTPPortLine = sctpPort;
                  mline->mAMaxMessageSize = maxMessageSize;
                  auto format = make_shared<ISDPTypes::AFMTPLine>(Noop{});
                  format->mFormatStr = "webrtc-datachannel";
                  format->mFormatSpecific.push_back(String("max-message-size=") + string(mediaLine.mCapabilities->mMaxMessageSize));
                  mline->mAFMTPLines.push_back(format);
                }
                goto found;
              }
            }
            goto not_found;
          }

        not_found:
          {
            auto mline = make_shared<ISDPTypes::MLine>(Noop{});
            mline->mMedia = "application";
            mline->mPort = 0;
            mline->mProto = ISDPTypes::ProtocolType_Unknown;
            mline->mProtoStr = "UDP/UNKNOWN";
            mline->mFmts.push_back("unknown");
            auto format = make_shared<ISDPTypes::AFMTPLine>(Noop{});
            format->mFormatStr = "unknown";
            format->mFormatSpecific.push_back(String("unknown=0"));
            mline->mAFMTPLines.push_back(format);
            continue;
          }

        found:
          {
          }
        }
      }

      //-----------------------------------------------------------------------
      ISDPTypes::SDPPtr SDPParser::createSDP(
                                             const GeneratorOptions &options,
                                             const Description &description
                                             )
      {
        SDPPtr result(make_shared<SDP>());

        try {
          createSDPSessionLevel(options, description, *result);
          createSDPMediaLevel(options, description, *result);
        } catch (const SafeIntException &e) {
          ORTC_THROW_INVALID_PARAMETERS("value found out of legal value range" + string(e.m_code));
        }

        return result;
      }

      //-----------------------------------------------------------------------
      String SDPParser::getCandidateSDP(const IICETypes::Candidate &candidate)
      {
        auto result = internal::fillCandidate(candidate);
        if (!result) {
          ZS_LOG_WARNING(Debug, internal::slog("unable to convert candidate to string") + candidate.toDebug());
          return String();
        }
        return result->toString();
      }

    }  // namespace internal
  } // namespace adapter
} // namespace ortc
