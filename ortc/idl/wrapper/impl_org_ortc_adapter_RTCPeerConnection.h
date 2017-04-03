
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCPeerConnection.h"

#include <ortc/adapter/IPeerConnection.h>
#include <ortc/adapter/ISessionDescription.h>
#include <ortc/IDataChannel.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCPeerConnection : public wrapper::org::ortc::adapter::RTCPeerConnection,
                                     public ::ortc::adapter::IPeerConnectionDelegate
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnection, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnection, IPeerConnection);
            ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataChannel, IDataChannel);
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnectionSubscription, NativeTypeSubscription);
            ZS_DECLARE_TYPEDEF_PTR(RTCPeerConnection, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCPeerConnection, WrapperType);

            ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith< ::ortc::adapter::ISessionDescription >, NativePromiseWithSessionDescription);
            ZS_DECLARE_TYPEDEF_PTR(PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr >, WrapperPromiseWithSessionDescription);
            ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith< ::ortc::IRTPSender >, NativePromiseWithSender);
            ZS_DECLARE_TYPEDEF_PTR(PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr >, WrapperPromiseWithSender);
            ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith< ::ortc::IDataChannel >, NativePromiseWithDataChannel);
            ZS_DECLARE_TYPEDEF_PTR(PromiseWithHolderPtr< wrapper::org::ortc::RTCDataChannelPtr >, WrapperPromiseWithDataChannel);

            RTCPeerConnectionWeakPtr thisWeak_;
            NativeTypePtr native_;
            zsLib::Lock lock_;
            std::atomic<size_t> subscriptionCount_{};
            bool defaultSubscription_{ true };
            NativeTypeSubscriptionPtr subscription_;

            RTCPeerConnection();
            virtual ~RTCPeerConnection();

            // methods RTCStatsProvider
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

            // methods RTCPeerConnection
            virtual void wrapper_init_org_ortc_adapter_RTCPeerConnection() override;
            virtual void wrapper_init_org_ortc_adapter_RTCPeerConnection(wrapper::org::ortc::adapter::RTCConfigurationPtr configuration) override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createOffer() override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createOffer(wrapper::org::ortc::adapter::RTCOfferOptionsPtr options) override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createAnswer() override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createAnswer(wrapper::org::ortc::adapter::RTCAnswerOptionsPtr options) override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createCapabilities() override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createCapabilities(wrapper::org::ortc::adapter::RTCCapabilityOptionsPtr options) override;
            virtual PromisePtr setLocalDescription(wrapper::org::ortc::adapter::RTCSessionDescriptionPtr description) override;
            virtual PromisePtr setRemoteDescription(wrapper::org::ortc::adapter::RTCSessionDescriptionPtr description) override;
            virtual void addIceCandidate(wrapper::org::ortc::adapter::RTCIceCandidatePtr candidate) override;
            virtual shared_ptr< list< wrapper::org::ortc::RTCIceServerPtr > > getDefaultIceServers() override;
            virtual void close() override;
            virtual shared_ptr< list< wrapper::org::ortc::RTCRtpSenderPtr > > getSenders() override;
            virtual shared_ptr< list< wrapper::org::ortc::RTCRtpReceiverPtr > > getReceivers() override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > addTrack(wrapper::org::ortc::MediaStreamTrackPtr track) override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > addTrack(
              wrapper::org::ortc::MediaStreamTrackPtr track,
              wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr config
              ) override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > addTrack(
              wrapper::org::ortc::MediaStreamTrackPtr track,
              shared_ptr< list< wrapper::org::ortc::adapter::MediaStreamPtr > > mediaStreams,
              wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr config
              ) override;
            virtual void removeTrack(wrapper::org::ortc::RTCRtpSenderPtr sender) override;
            virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCDataChannelPtr > > createDataChannel(wrapper::org::ortc::RTCDataChannelParametersPtr parameters) override;

            // properties RTCPeerConnection
            virtual uint64_t get_objectId() override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_localDescription() override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_currentDescription() override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_pendingDescription() override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_remoteDescription() override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_currentRemoteDescription() override;
            virtual wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_pendingRemoteDescription() override;
            virtual wrapper::org::ortc::adapter::RTCSignalingState get_signalingState() override;
            virtual wrapper::org::ortc::RTCIceGathererState get_iceGatheringState() override;
            virtual wrapper::org::ortc::adapter::RTCIceConnectionState get_iceConnectionState() override;
            virtual wrapper::org::ortc::adapter::RTCPeerConnectionState get_connectionState() override;
            virtual bool get_canTrickleCandidates() override;
            virtual wrapper::org::ortc::adapter::RTCConfigurationPtr get_getConfiguration() override;
            virtual void set_getConfiguration(wrapper::org::ortc::adapter::RTCConfigurationPtr value) override;

            virtual void wrapper_onObserverCountChanged(size_t count) override;

            virtual void onPeerConnectionNegotiationNeeded(IPeerConnectionPtr connection) override;
            virtual void onPeerConnectionIceCandidate(
              IPeerConnectionPtr connection,
              ICECandidatePtr candidate,
              const char *url
            ) override;
            virtual void onPeerConnectionIceCandidateError(
              IPeerConnectionPtr connection,
              ICECandidateErrorEventPtr error
            ) override;
            virtual void onPeerConnectionSignalingStateChange(
              IPeerConnectionPtr connection,
              SignalingStates state
            ) override;
            virtual void onPeerConnectionICEGatheringStateChange(
              IPeerConnectionPtr connection,
              ICEGatheringStates state
            ) override;
            virtual void onPeerConnectionICEConnectionStateChange(
              IPeerConnectionPtr connection,
              ICEConnectionStates state
            ) override;
            virtual void onPeerConnectionConnectionStateChange(
              IPeerConnectionPtr connection,
              PeerConnectionStates state
            ) override;
            virtual void onPeerConnectionTrack(
              IPeerConnectionPtr connection,
              MediaStreamTrackEventPtr event
            ) override;

            virtual void onPeerConnectionTrackGone(
              IPeerConnectionPtr connection,
              MediaStreamTrackEventPtr event
            ) override;

            virtual void onPeerConnectionDataChannel(
              IPeerConnectionPtr connection,
              IDataChannelPtr dataChannel
            ) override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr track);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);

            void subscribe();

            static WrapperPromiseWithSessionDescriptionPtr toWrapper(NativePromiseWithSessionDescriptionPtr promise);
            static WrapperPromiseWithSenderPtr toWrapper(NativePromiseWithSenderPtr promise);
            static WrapperPromiseWithDataChannelPtr toWrapper(NativePromiseWithDataChannelPtr promise);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

