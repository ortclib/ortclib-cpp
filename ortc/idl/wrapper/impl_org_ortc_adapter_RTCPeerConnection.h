
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

            RTCPeerConnection() noexcept;
            virtual ~RTCPeerConnection() noexcept;

            // methods RTCStatsProvider
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState

            // methods RTCPeerConnection
            void wrapper_init_org_ortc_adapter_RTCPeerConnection() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCPeerConnection(wrapper::org::ortc::adapter::RTCConfigurationPtr configuration) noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createOffer() noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createOffer(wrapper::org::ortc::adapter::RTCOfferOptionsPtr options) noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createAnswer() noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createAnswer(wrapper::org::ortc::adapter::RTCAnswerOptionsPtr options) noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createCapabilities() noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > createCapabilities(wrapper::org::ortc::adapter::RTCCapabilityOptionsPtr options) noexcept override;
            PromisePtr setLocalDescription(wrapper::org::ortc::adapter::RTCSessionDescriptionPtr description) noexcept override;
            PromisePtr setRemoteDescription(wrapper::org::ortc::adapter::RTCSessionDescriptionPtr description) noexcept override;
            void addIceCandidate(wrapper::org::ortc::adapter::RTCIceCandidatePtr candidate) noexcept override;
            shared_ptr< list< wrapper::org::ortc::RTCIceServerPtr > > getDefaultIceServers() noexcept override;
            void close() noexcept override;
            shared_ptr< list< wrapper::org::ortc::RTCRtpSenderPtr > > getSenders() noexcept override;
            shared_ptr< list< wrapper::org::ortc::RTCRtpReceiverPtr > > getReceivers() noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > addTrack(wrapper::org::ortc::MediaStreamTrackPtr track) noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > addTrack(
              wrapper::org::ortc::MediaStreamTrackPtr track,
              wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr config
              ) noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > addTrack(
              wrapper::org::ortc::MediaStreamTrackPtr track,
              shared_ptr< list< wrapper::org::ortc::adapter::MediaStreamPtr > > mediaStreams,
              wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr config
              ) noexcept override;
            void removeTrack(wrapper::org::ortc::RTCRtpSenderPtr sender) noexcept override;
            shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCDataChannelPtr > > createDataChannel(wrapper::org::ortc::RTCDataChannelParametersPtr parameters) noexcept override;

            // properties RTCPeerConnection
            uint64_t get_objectId() noexcept override;
            wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_localDescription() noexcept override;
            wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_currentDescription() noexcept override;
            wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_pendingDescription() noexcept override;
            wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_remoteDescription() noexcept override;
            wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_currentRemoteDescription() noexcept override;
            wrapper::org::ortc::adapter::RTCSessionDescriptionPtr get_pendingRemoteDescription() noexcept override;
            wrapper::org::ortc::adapter::RTCSignalingState get_signalingState() noexcept override;
            wrapper::org::ortc::RTCIceGathererState get_iceGatheringState() noexcept override;
            wrapper::org::ortc::adapter::RTCIceConnectionState get_iceConnectionState() noexcept override;
            wrapper::org::ortc::adapter::RTCPeerConnectionState get_connectionState() noexcept override;
            bool get_canTrickleCandidates() noexcept override;
            wrapper::org::ortc::adapter::RTCConfigurationPtr get_getConfiguration() noexcept override;
            void set_getConfiguration(wrapper::org::ortc::adapter::RTCConfigurationPtr value) noexcept override;

            virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

            virtual void onPeerConnectionNegotiationNeeded(IPeerConnectionPtr connection) noexcept override;
            virtual void onPeerConnectionIceCandidate(
              IPeerConnectionPtr connection,
              ICECandidatePtr candidate,
              const char *url
            ) noexcept override;
            virtual void onPeerConnectionIceCandidateError(
              IPeerConnectionPtr connection,
              ICECandidateErrorEventPtr error
            ) noexcept override;
            virtual void onPeerConnectionSignalingStateChange(
              IPeerConnectionPtr connection,
              SignalingStates state
            ) noexcept override;
            virtual void onPeerConnectionICEGatheringStateChange(
              IPeerConnectionPtr connection,
              ICEGatheringStates state
            ) noexcept override;
            virtual void onPeerConnectionICEConnectionStateChange(
              IPeerConnectionPtr connection,
              ICEConnectionStates state
            ) noexcept override;
            virtual void onPeerConnectionConnectionStateChange(
              IPeerConnectionPtr connection,
              PeerConnectionStates state
            ) noexcept override;
            virtual void onPeerConnectionTrack(
              IPeerConnectionPtr connection,
              MediaStreamTrackEventPtr event
            ) noexcept override;

            virtual void onPeerConnectionTrackGone(
              IPeerConnectionPtr connection,
              MediaStreamTrackEventPtr event
            ) noexcept override;

            virtual void onPeerConnectionDataChannel(
              IPeerConnectionPtr connection,
              IDataChannelPtr dataChannel
            ) noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

            void subscribe() noexcept;

            static WrapperPromiseWithSessionDescriptionPtr toWrapper(NativePromiseWithSessionDescriptionPtr promise) noexcept;
            static WrapperPromiseWithSenderPtr toWrapper(NativePromiseWithSenderPtr promise) noexcept;
            static WrapperPromiseWithDataChannelPtr toWrapper(NativePromiseWithDataChannelPtr promise) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

