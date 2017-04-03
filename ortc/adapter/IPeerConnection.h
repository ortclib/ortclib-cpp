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
#include <ortc/adapter/ISessionDescription.h>

#include <ortc/IRTPTypes.h>
#include <ortc/IICEGatherer.h>
#include <ortc/IICETransport.h>
#include <ortc/IDataChannel.h>

namespace ortc
{
  namespace adapter
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionTypes
    #pragma mark

    interaction IPeerConnectionTypes
    {
      typedef IICEGatherer::States ICEGatheringStates;
      typedef IICETransport::States ICEConnectionStates;
      typedef ISessionDescription::SignalingTypes SignalingTypes;

      ZS_DECLARE_STRUCT_PTR(Configuration);
      ZS_DECLARE_STRUCT_PTR(OfferAnswerOptions);
      ZS_DECLARE_STRUCT_PTR(OfferOptions);
      ZS_DECLARE_STRUCT_PTR(AnswerOptions);
      ZS_DECLARE_STRUCT_PTR(CapabilityOptions);
      ZS_DECLARE_STRUCT_PTR(MediaStreamTrackConfiguration);
      ZS_DECLARE_STRUCT_PTR(ICECandidateErrorEvent);
      ZS_DECLARE_STRUCT_PTR(MediaStreamTrackEvent);
       
      ZS_DECLARE_TYPEDEF_PTR(PromiseWith<ISessionDescription>, PromiseWithDescription);
      ZS_DECLARE_TYPEDEF_PTR(PromiseWith<IRTPSender>, PromiseWithSender);
      ZS_DECLARE_TYPEDEF_PTR(PromiseWith<IDataChannel>, PromiseWithDataChannel);
      ZS_DECLARE_TYPEDEF_PTR(std::list<ICertificatePtr>, CertificateList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<IRTPSenderPtr>, SenderList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<IRTPReceiverPtr>, ReceiverList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<IRTPTypes::ParametersPtr>, RTPParametersList);
      ZS_DECLARE_TYPEDEF_PTR(ISessionDescription::MediaStreamSet, MediaStreamSet);
      ZS_DECLARE_TYPEDEF_PTR(std::list<IMediaStreamPtr>, MediaStreamList);
      ZS_DECLARE_TYPEDEF_PTR(ortc::IICEGatherer::ServerList, ServerList);
      ZS_DECLARE_TYPEDEF_PTR(ISessionDescription::ICECandidate, ICECandidate);

      enum BundlePolicies
      {
        BundlePolicy_First,

        BundlePolicy_Balanced = BundlePolicy_First,
        BundlePolicy_MaxCompat,
        BundlePolicy_MaxBundle,

        BundlePolicy_Last = BundlePolicy_MaxBundle,
      };

      static const char *toString(BundlePolicies policy);

      enum RTCPMuxPolicies
      {
        RTCPMuxPolicy_First,

        RTCPMuxPolicy_Negotiated = RTCPMuxPolicy_First,
        RTCPMuxPolicy_Require,

        RTCPMuxPolicy_Last = RTCPMuxPolicy_Require,
      };

      static const char *toString(RTCPMuxPolicies policy);

      enum SignalingModes
      {
        SignalingMode_First,

        SignalingMode_JSON = SignalingMode_First,
        SignalingMode_SDP,

        SignalingMode_Last = SignalingMode_SDP,
      };

      static const char *toString(SignalingModes mode);
      static SignalingModes toSignalingMode(const char *mode);
      static bool isCompatible(
                               SignalingModes mode,
                               ISessionDescriptionTypes::SignalingTypes signalingType
                               );

      enum SignalingStates
      {
        SignalingState_First,

        SignalingState_Stable = SignalingState_First,
        SignalingState_HaveLocalOffer,
        SignalingState_HaveRemoteOffer,
        SignalingState_HaveLocalPranswer,
        SignalingState_HaveRemotePranswer,
        SignalingState_Closed,

        SignalingState_Last = SignalingState_Closed,
      };

      static const char *toString(SignalingStates state);

      enum PeerConnectionStates
      {
        PeerConnectionState_First,

        PeerConnectionState_New = PeerConnectionState_First,
        PeerConnectionState_Connecting,
        PeerConnectionState_Connected,
        PeerConnectionState_Disconnected,
        PeerConnectionState_Failed,
        PeerConnectionState_Closed,

        PeerConnectionState_Last = PeerConnectionState_Closed,
      };

      static const char *toString(PeerConnectionStates state);

      struct Configuration
      {
        IICEGathererTypes::OptionsPtr mGatherOptions;

        SignalingModes mSignalingMode {IPeerConnectionTypes::SignalingMode_First};
        bool mNegotiateSRTPSDES {false};

        BundlePolicies mBundlePolicy {BundlePolicy_Balanced};
        RTCPMuxPolicies mRTCPMuxPolicy {RTCPMuxPolicy_Require};
        CertificateList mCertificates;
        size_t mICECandidatePoolSize {};

        Configuration() {}
        Configuration(const Configuration &op2);
        ElementPtr toDebug() const;
      };

      struct OfferAnswerOptions
      {
        bool mVoiceActivityDetection {true};

        ElementPtr toDebug() const;
      };

      struct OfferOptions : public OfferAnswerOptions
      {
        bool mICERestart {false};

        ElementPtr toDebug() const;
      };

      struct AnswerOptions : public OfferAnswerOptions
      {
        ElementPtr toDebug() const;
      };

      struct CapabilityOptions : public OfferAnswerOptions
      {
        ElementPtr toDebug() const;
      };

      struct MediaStreamTrackConfiguration
      {
        IRTPTypes::CapabilitiesPtr mCapabilities;
        IRTPTypes::ParametersPtr mParameters;

        MediaStreamTrackConfiguration() {}
        MediaStreamTrackConfiguration(const MediaStreamTrackConfiguration &op2);

        MediaStreamTrackConfiguration &operator=(const MediaStreamTrackConfiguration &op2);

        ElementPtr toDebug() const;
      };

      struct ICECandidateErrorEvent
      {
        typedef WORD ErrorCode;
        ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate);
        ZS_DECLARE_TYPEDEF_PTR(IICEGathererTypes::ErrorEvent, ErrorEvent);

        String mMid;
        Optional<unsigned short> mMLineIndex;

        ICECandidatePtr       mHostCandidate;
        String                mURL;
        Optional<ErrorCode>   mErrorCode;
        String                mErrorText;

        ElementPtr toDebug() const;
      };

      struct MediaStreamTrackEvent
      {
        ZS_DECLARE_TYPEDEF_PTR(std::list<IMediaStreamPtr>, MediaStreamList);

        IRTPReceiverPtr mReceiver;
        IMediaStreamTrackPtr mTrack;
        MediaStreamList mMediaStreams;

        ElementPtr toDebug() const;
      };

    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnection
    #pragma mark

    interface IPeerConnection : public IPeerConnectionTypes,
                                public IStatsProvider
    {
      static IPeerConnectionPtr create(
                                       IPeerConnectionDelegatePtr delegate,
                                       const Optional<Configuration> &configuration = Optional<Configuration>()
                                       );

      virtual PUID getID() const = 0;

      virtual IPeerConnectionSubscriptionPtr subscribe(IPeerConnectionDelegatePtr delegate) = 0;

      virtual PromiseWithDescriptionPtr createOffer(const Optional<OfferOptions> &configuration = Optional<OfferOptions>()) = 0;
      virtual PromiseWithDescriptionPtr createAnswer(const Optional<AnswerOptions> &configuration = Optional<AnswerOptions>()) = 0;

      virtual PromiseWithDescriptionPtr createCapabilities(const Optional<CapabilityOptions> &configuration = Optional<CapabilityOptions>()) = 0;

      virtual PromisePtr setLocalDescription(ISessionDescriptionPtr description) = 0;

      virtual ISessionDescriptionPtr localDescription() const = 0;
      virtual ISessionDescriptionPtr currentDescription() const = 0;
      virtual ISessionDescriptionPtr pendingDescription() const = 0;

      virtual PromisePtr setRemoteDescription(ISessionDescriptionPtr description) = 0;
      virtual ISessionDescriptionPtr remoteDescription() const = 0;
      virtual ISessionDescriptionPtr currentRemoteDescription() const = 0;
      virtual ISessionDescriptionPtr pendingRemoteDescription() const = 0;
      virtual void addICECandidate(const ICECandidate &candidate) = 0;

      virtual SignalingStates signalingState() const = 0;
      virtual ICEGatheringStates iceGatheringState() const = 0;
      virtual ICEConnectionStates iceConnectionState() const = 0;
      virtual PeerConnectionStates connectionState() const = 0;
      virtual bool canTrickleCandidates() const = 0;

      static ServerListPtr getDefaultIceServers();

      virtual ConfigurationPtr getConfiguration() const = 0;
      virtual void setConfiguration(const Configuration &configuration) = 0;

      virtual void close() = 0;

      virtual SenderListPtr getSenders() const = 0;
      virtual ReceiverListPtr getReceivers() const = 0;
      virtual PromiseWithSenderPtr addTrack(
                                            IMediaStreamTrackPtr track,
                                            const MediaStreamTrackConfiguration &configuration = MediaStreamTrackConfiguration()
                                            ) = 0;
      virtual PromiseWithSenderPtr addTrack(
                                            IMediaStreamTrackPtr track,
                                            const MediaStreamList &mediaStreams,
                                            const MediaStreamTrackConfiguration &configuration = MediaStreamTrackConfiguration()
                                            ) = 0;

      virtual void removeTrack(IRTPSenderPtr sender) = 0;

      virtual PromiseWithDataChannelPtr createDataChannel(const IDataChannelTypes::Parameters &parameters) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionDelegate
    #pragma mark

    interface IPeerConnectionDelegate
    {
      typedef IICEGatherer::States ICEGatheringStates;
      typedef IICETransport::States ICEConnectionStates;
      typedef IPeerConnectionTypes::PeerConnectionStates PeerConnectionStates;

      ZS_DECLARE_TYPEDEF_PTR(IPeerConnectionTypes::ICECandidate, ICECandidate);
      ZS_DECLARE_TYPEDEF_PTR(IPeerConnectionTypes::ICECandidateErrorEvent, ICECandidateErrorEvent);
      ZS_DECLARE_TYPEDEF_PTR(IPeerConnectionTypes::MediaStreamTrackEvent, MediaStreamTrackEvent);
      ZS_DECLARE_TYPEDEF_PTR(IPeerConnectionTypes::SignalingStates, SignalingStates);

      virtual void onPeerConnectionNegotiationNeeded(IPeerConnectionPtr connection) = 0;
      virtual void onPeerConnectionIceCandidate(
                                                IPeerConnectionPtr connection,
                                                ICECandidatePtr candidate,
                                                const char *url
                                                ) = 0;
      virtual void onPeerConnectionIceCandidateError(
                                                     IPeerConnectionPtr connection,
                                                     ICECandidateErrorEventPtr error
                                                     ) = 0;
      virtual void onPeerConnectionSignalingStateChange(
                                                        IPeerConnectionPtr connection,
                                                        SignalingStates state
                                                        ) = 0;
      virtual void onPeerConnectionICEGatheringStateChange(
                                                           IPeerConnectionPtr connection,
                                                           ICEGatheringStates state
                                                           ) = 0;
      virtual void onPeerConnectionICEConnectionStateChange(
                                                            IPeerConnectionPtr connection,
                                                            ICEConnectionStates state
                                                            ) = 0;
      virtual void onPeerConnectionConnectionStateChange(
                                                         IPeerConnectionPtr connection,
                                                         PeerConnectionStates state
                                                         ) = 0;
      virtual void onPeerConnectionTrack(
                                         IPeerConnectionPtr connection,
                                         MediaStreamTrackEventPtr event
                                         ) = 0;

      virtual void onPeerConnectionTrackGone(
                                             IPeerConnectionPtr connection,
                                             MediaStreamTrackEventPtr event
                                             ) = 0;

      virtual void onPeerConnectionDataChannel(
                                               IPeerConnectionPtr connection,
                                               IDataChannelPtr dataChannel
                                               ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerConnectionSubscription
    #pragma mark

    interaction IPeerConnectionSubscription
    {
      virtual PUID getID() const = 0;

      virtual void cancel() = 0;

      virtual void background() = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::adapter::IPeerConnectionDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerConnectionPtr, IPeerConnectionPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerConnectionTypes::ICECandidatePtr, ICECandidatePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerConnectionTypes::ICECandidateErrorEventPtr, ICECandidateErrorEventPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerConnectionTypes::ICEGatheringStates, ICEGatheringStates)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerConnectionTypes::ICEConnectionStates, ICEConnectionStates)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerConnectionTypes::SignalingStates, SignalingStates)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerConnectionTypes::PeerConnectionStates, PeerConnectionStates)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerConnectionTypes::MediaStreamTrackEventPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDataChannelPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_METHOD_1(onPeerConnectionNegotiationNeeded, IPeerConnectionPtr)
ZS_DECLARE_PROXY_METHOD_3(onPeerConnectionIceCandidate, IPeerConnectionPtr, ICECandidatePtr, const char *)
ZS_DECLARE_PROXY_METHOD_2(onPeerConnectionIceCandidateError, IPeerConnectionPtr, ICECandidateErrorEventPtr)
ZS_DECLARE_PROXY_METHOD_2(onPeerConnectionSignalingStateChange, IPeerConnectionPtr, SignalingStates)
ZS_DECLARE_PROXY_METHOD_2(onPeerConnectionICEGatheringStateChange, IPeerConnectionPtr, ICEGatheringStates)
ZS_DECLARE_PROXY_METHOD_2(onPeerConnectionICEConnectionStateChange, IPeerConnectionPtr, ICEConnectionStates)
ZS_DECLARE_PROXY_METHOD_2(onPeerConnectionConnectionStateChange, IPeerConnectionPtr, PeerConnectionStates)
ZS_DECLARE_PROXY_METHOD_2(onPeerConnectionTrack, IPeerConnectionPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_METHOD_2(onPeerConnectionTrackGone, IPeerConnectionPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_METHOD_2(onPeerConnectionDataChannel, IPeerConnectionPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_END()


ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::adapter::IPeerConnectionDelegate, ortc::adapter::IPeerConnectionSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerConnectionPtr, IPeerConnectionPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerConnectionTypes::ICECandidatePtr, ICECandidatePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerConnectionTypes::ICECandidateErrorEventPtr, ICECandidateErrorEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerConnectionTypes::ICEGatheringStates, ICEGatheringStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerConnectionTypes::ICEConnectionStates, ICEConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerConnectionTypes::PeerConnectionStates, PeerConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerConnectionTypes::MediaStreamTrackEventPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDataChannelPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onPeerConnectionNegotiationNeeded, IPeerConnectionPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_3(onPeerConnectionIceCandidate, IPeerConnectionPtr, ICECandidatePtr, const char *)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerConnectionIceCandidateError, IPeerConnectionPtr, ICECandidateErrorEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerConnectionSignalingStateChange, IPeerConnectionPtr, SignalingStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerConnectionICEGatheringStateChange, IPeerConnectionPtr, ICEGatheringStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerConnectionICEConnectionStateChange, IPeerConnectionPtr, ICEConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerConnectionConnectionStateChange, IPeerConnectionPtr, PeerConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerConnectionTrack, IPeerConnectionPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerConnectionTrackGone, IPeerConnectionPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerConnectionDataChannel, IPeerConnectionPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
