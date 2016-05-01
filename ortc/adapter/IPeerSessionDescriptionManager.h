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

namespace ortc
{
  namespace adapter
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerSessionDescriptionManagerTypes
    #pragma mark

    interaction IPeerSessionDescriptionManagerTypes
    {
      ZS_DECLARE_STRUCT_PTR(Configuration)
      ZS_DECLARE_STRUCT_PTR(MediaTrackConfiguration)
      ZS_DECLARE_STRUCT_PTR(ICECandidate)
      ZS_DECLARE_STRUCT_PTR(ICECandidateErrorEvent)
      ZS_DECLARE_STRUCT_PTR(MediaStreamTrackEvent)
       
      ZS_DECLARE_TYPEDEF_PTR(PromiseWith<String>, PromiseWithDescription)
      ZS_DECLARE_TYPEDEF_PTR(std::list<ICertificate>, CertificateList)
      ZS_DECLARE_TYPEDEF_PTR(std::list<IRTPSender>, SenderList)
      ZS_DECLARE_TYPEDEF_PTR(std::list<IRTPReceiver>, ReceiverList)
      ZS_DECLARE_TYPEDEF_PTR(ISessionDescription::MediaStreamSet, MediaStreamSet)

      enum BundlePolicies
      {
        BundlePolicy_Balanced,
        BundlePolity_MaxCompat,
        BundlePolicy_MaxBundle,
      };

      static const char *toString(BundlePolicies policy);
      static BundlePolicies toBundlePolicy(const char *policy);

      enum RTCPMuxPolicies
      {
        RTCPMuxPolicy_Negotiated,
        RTCPMuxPolicy_Require,
      };

      static const char *toString(RTCPMuxPolicies policy);
      static RTCPMuxPolicies toRTCPMuxPolicy(const char *policy);

      enum ConnectionStates
      {
        ConnectionState_New,
        ConnectionState_Connecting,
        ConnectionState_Connected,
        ConnectionState_Disconnected,
        ConnectionState_Failed,
      };

      struct Configruation
      {
        IICEGathererTypes::Options mGatherOptions;

        BundlePolicies mBundlePolicy;
        RTCPMuxPolicies mRTCPMuxPolicy;
        CertificateList mCertificates;

        IRTPTypes::CapabilitiesPtr mCodecPreferences;
      };

      struct MediaTrackConfiguration
      {
        IRTPTypes::ParametersPtr mParameters;
        MediaStreamSet mMediaStreams;
      };

      struct ICECandidate
      {
        String mMid;
        Optional<unsigned short> mMLineIndex;
        IICETypes::GatherCandidatePtr mCandidate;
      };

      struct ICECandidateErrorEvent
      {
        typedef WORD ErrorCode;
        ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate)
        ZS_DECLARE_TYPEDEF_PTR(IICEGathererTypes::ErrorEvent, ErrorEvent)

        String mMid;
        Optional<unsigned short> mMLineIndex;

        CandidatePtr    mHostCandidate;
        String          mURL;
        ErrorCode       mErrorCode;
        String          mErrorText;
      };

      struct MediaStreamTrackEvent
      {
        IRTPReceiverPtr mReceiver;
        IMediaStreamTrackPtr mTrack;
        MediaStreamSet mMediaStreams;
      };

    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerSessionDescriptionManager
    #pragma mark

    interface IPeerSessionDescriptionManager : public IPeerSessionDescriptionManagerTypes
    {
      static IPeerSessionDescriptionManagerPtr create(const Configruation &configuration);

      virtual PromiseWithDescriptionPtr getLocalDescription() = 0;

      virtual PromisePtr setLocalDescription(const char *description) = 0;
      virtual PromisePtr setRemoteDescription(const char *description) = 0;

      virtual void addRemoteIceCandidate(const ICECandidate &candidate) = 0;

      virtual IICEGathererTypes::States iceGatheringState() const = 0;
      virtual IICETransportTypes::States iceTransportState() const = 0;
      virtual ConnectionStates connectionState() const = 0;

      virtual ConfigurationPtr configuration() const = 0;
      virtual void configuration(const Configruation &configuration) = 0;

      virtual IRTPTypes::CapabilitiesPtr getCurrentReceivingCapabilities(IMediaStreamTrack::Kinds kind) const = 0;
      virtual IRTPTypes::CapabilitiesPtr getCurrentSendingCapabilities(IMediaStreamTrack::Kinds kind) const = 0;
      virtual IRTPTypes::ParametersPtr getCurrentReceivingParameters(IMediaStreamTrack::Kinds kind) const = 0;
      virtual IRTPTypes::ParametersPtr getCurrentSendingParameters(IMediaStreamTrack::Kinds kind) const = 0;

      virtual void close() = 0;

      virtual SenderList getSenders() const = 0;
      virtual ReceiverList getReceivers() const = 0;
      virtual IRTPSender addTrack(
                                  IMediaStreamTrackPtr track,
                                  const MediaTrackConfiguration &configuration = MediaTrackConfiguration()
                                  ) = 0;

      virtual void removeTrack(IRTPSenderPtr sender) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerSessionDescriptionManagerDelegate
    #pragma mark

    interface IPeerSessionDescriptionManagerDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IPeerSessionDescriptionManagerTypes::ICECandidate, ICECandidate)
      ZS_DECLARE_TYPEDEF_PTR(IPeerSessionDescriptionManagerTypes::ICECandidateErrorEvent, ICECandidateErrorEvent)
      ZS_DECLARE_TYPEDEF_PTR(IPeerSessionDescriptionManagerTypes::MediaStreamTrackEvent, MediaStreamTrackEvent)
      ZS_DECLARE_TYPEDEF_PTR(IPeerSessionDescriptionManagerTypes::ConnectionStates, ConnectionStates)

      virtual void onPeerSessionDescriptionManagerGetLocalDescriptionNeeded(IPeerSessionDescriptionManagerPtr manager) = 0;
      virtual void onPeerSessionDescriptionManagerIceCandidate(
                                                               IPeerSessionDescriptionManagerPtr manager,
                                                               ICECandidatePtr candidate
                                                               ) = 0;
      virtual void onPeerSessionDescriptionManagerIceCandidateError(
                                                                    IPeerSessionDescriptionManagerPtr manager,
                                                                    ICECandidateErrorEventPtr error
                                                                    ) = 0;
      virtual void onPeerSessionDescriptionManagerIceGatheringStateChange(
                                                                          IPeerSessionDescriptionManagerPtr manager,
                                                                          IICEGathererTypes::States state
                                                                          ) = 0;
      virtual void onPeerSessionDescriptionManagerIceTransportStateChange(
                                                                          IPeerSessionDescriptionManagerPtr manager,
                                                                          IICETransportTypes::States state
                                                                          ) = 0;
      virtual void onPeerSessionDescriptionManagerConnectionStateChange(
                                                                        IPeerSessionDescriptionManagerPtr manager,
                                                                        ConnectionStates state
                                                                        ) = 0;
      virtual void onPeerSessionDescriptionManagerTrack(
                                                        IPeerSessionDescriptionManagerPtr manager,
                                                        MediaStreamTrackEventPtr event
                                                        ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IPeerSessionDescriptionManagerSubscription
    #pragma mark

    interaction IPeerSessionDescriptionManagerSubscription
    {
      virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
    };
  }
}


ZS_DECLARE_PROXY_BEGIN(ortc::adapter::IPeerSessionDescriptionManagerDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerPtr, IPeerSessionDescriptionManagerPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerTypes::ICECandidatePtr, ICECandidatePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerTypes::ICECandidateErrorEventPtr, ICECandidateErrorEventPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGathererTypes::States, ICEGathererStates)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransportTypes::States, ICETransportStates)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerTypes::ConnectionStates, ConnectionStates)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerTypes::MediaStreamTrackEventPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_METHOD_1(onPeerSessionDescriptionManagerGetLocalDescriptionNeeded, IPeerSessionDescriptionManagerPtr)
ZS_DECLARE_PROXY_METHOD_2(onPeerSessionDescriptionManagerIceCandidate, IPeerSessionDescriptionManagerPtr, ICECandidatePtr)
ZS_DECLARE_PROXY_METHOD_2(onPeerSessionDescriptionManagerIceCandidateError, IPeerSessionDescriptionManagerPtr, ICECandidateErrorEventPtr)
ZS_DECLARE_PROXY_METHOD_2(onPeerSessionDescriptionManagerIceGatheringStateChange, IPeerSessionDescriptionManagerPtr, ICEGathererStates)
ZS_DECLARE_PROXY_METHOD_2(onPeerSessionDescriptionManagerIceTransportStateChange, IPeerSessionDescriptionManagerPtr, ICETransportStates)
ZS_DECLARE_PROXY_METHOD_2(onPeerSessionDescriptionManagerConnectionStateChange, IPeerSessionDescriptionManagerPtr, ConnectionStates)
ZS_DECLARE_PROXY_METHOD_2(onPeerSessionDescriptionManagerTrack, IPeerSessionDescriptionManagerPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_END()


ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::adapter::IPeerSessionDescriptionManagerDelegate, ortc::adapter::IPeerSessionDescriptionManagerSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerPtr, IPeerSessionDescriptionManagerPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerTypes::ICECandidatePtr, ICECandidatePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerTypes::ICECandidateErrorEventPtr, ICECandidateErrorEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGathererTypes::States, ICEGathererStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransportTypes::States, ICETransportStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerTypes::ConnectionStates, ConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IPeerSessionDescriptionManagerTypes::MediaStreamTrackEventPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onPeerSessionDescriptionManagerGetLocalDescriptionNeeded, IPeerSessionDescriptionManagerPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerSessionDescriptionManagerIceCandidate, IPeerSessionDescriptionManagerPtr, ICECandidatePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerSessionDescriptionManagerIceCandidateError, IPeerSessionDescriptionManagerPtr, ICECandidateErrorEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerSessionDescriptionManagerIceGatheringStateChange, IPeerSessionDescriptionManagerPtr, ICEGathererStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerSessionDescriptionManagerIceTransportStateChange, IPeerSessionDescriptionManagerPtr, ICETransportStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerSessionDescriptionManagerConnectionStateChange, IPeerSessionDescriptionManagerPtr, ConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onPeerSessionDescriptionManagerTrack, IPeerSessionDescriptionManagerPtr, MediaStreamTrackEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
