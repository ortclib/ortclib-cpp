
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCPeerConnection.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCPeerConnection : public wrapper::org::webRtc::RTCPeerConnection
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCPeerConnection, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCPeerConnection, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface, NativeType);

          typedef rtc::scoped_refptr<::webrtc::PeerConnectionInterface> NativeScopedPtr;

          ZS_DECLARE_STRUCT_PTR(WebrtcObserver);

          struct WebrtcObserver : public ::webrtc::PeerConnectionObserver
          {
            WebrtcObserver(WrapperImplTypePtr wrapper) noexcept : outer_(wrapper) {}

            void OnSignalingChange(NativeType::SignalingState new_state) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifySignalingChange(new_state);
            }

            // deprecated
            //virtual void OnAddStream(rtc::scoped_refptr<::webrtc::MediaStreamInterface> stream) {}
            //virtual void OnRemoveStream(rtc::scoped_refptr<::webrtc::MediaStreamInterface> stream) {}

            void OnDataChannel(rtc::scoped_refptr<::webrtc::DataChannelInterface> data_channel) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyDataChannel(data_channel);
            }

            void OnRenegotiationNeeded() override
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyRenegotiationNeeded();
            }

            void OnIceConnectionChange(NativeType::PeerConnectionInterface::IceConnectionState new_state) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyIceConnectionChange(new_state);
            }

            void OnIceGatheringChange(NativeType::PeerConnectionInterface::IceGatheringState new_state) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyIceGatheringChange(new_state);
            }

            void OnIceCandidate(const ::webrtc::IceCandidateInterface* candidate) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyIceCandidate(candidate);
            }

            void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyIceCandidatesRemoved(candidates);
            }

            void OnIceConnectionReceivingChange(bool receiving) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyIceConnectionReceivingChange(receiving);
            }

            void OnAddTrack(
                            rtc::scoped_refptr<::webrtc::RtpReceiverInterface> receiver,
                            const std::vector<rtc::scoped_refptr<::webrtc::MediaStreamInterface>>& streams
                            ) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyAddTrack(receiver, streams);
            }

            void OnTrack(rtc::scoped_refptr<::webrtc::RtpTransceiverInterface> transceiver) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyTrack(transceiver);
            }

            void OnRemoveTrack(rtc::scoped_refptr<::webrtc::RtpReceiverInterface> receiver) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyRemoveTrack(receiver);
            }

          private:
            WrapperImplTypeWeakPtr outer_;
          };

          WebrtcObserverUniPtr observer_;
          NativeScopedPtr native_;
          RTCPeerConnectionWeakPtr thisWeak_;

          RTCPeerConnection() noexcept;
          virtual ~RTCPeerConnection() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCStatsReportPtr > > getStats(wrapper::org::webRtc::RTCStatsTypeSetPtr statTypes) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr

          // methods RTCPeerConnection
          void wrapper_init_org_webRtc_RTCPeerConnection(wrapper::org::webRtc::RTCConfigurationPtr config) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > createOffer(wrapper::org::webRtc::RTCOfferOptionsPtr options) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > createOffer(wrapper::org::webRtc::MediaConstraintsPtr constraints) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > createAnswer(wrapper::org::webRtc::RTCAnswerOptionsPtr options) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > createAnswer(wrapper::org::webRtc::MediaConstraintsPtr constraints) noexcept override;
          PromisePtr setLocalDescription(wrapper::org::webRtc::RTCSessionDescriptionPtr constraints) noexcept override;
          PromisePtr setRemoteDescription(wrapper::org::webRtc::RTCSessionDescriptionPtr constraints) noexcept override;
          wrapper::org::webRtc::RTCConfigurationPtr getConfiguration() noexcept override;
          void setConfiguration(wrapper::org::webRtc::RTCConfigurationPtr config) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          PromisePtr addIceCandidate(wrapper::org::webRtc::RTCIceCandidatePtr candidate) noexcept override;
          void removeIceCandidates(shared_ptr< list< wrapper::org::webRtc::RTCIceCandidatePtr > > candidates) noexcept override;
          void setBitrate(wrapper::org::webRtc::RTCBitrateParametersPtr params) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          void close() noexcept override;
          wrapper::org::webRtc::RTCRtpSenderPtr addTrack(wrapper::org::webRtc::MediaStreamTrackPtr track) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          bool removeTrack(wrapper::org::webRtc::RTCRtpSenderPtr sender) noexcept override;
          wrapper::org::webRtc::RTCRtpTransceiverPtr addTransceiver(wrapper::org::webRtc::MediaStreamTrackPtr track) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          wrapper::org::webRtc::RTCRtpTransceiverPtr addTransceiver(String kind) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          wrapper::org::webRtc::RTCRtpSenderPtr addSender(
            String kind,
            String trackId
            ) noexcept override;
          shared_ptr< list< wrapper::org::webRtc::RTCRtpSenderPtr > > getSenders() noexcept override;
          shared_ptr< list< wrapper::org::webRtc::RTCRtpReceiverPtr > > getReceivers() noexcept override;
          shared_ptr< list< wrapper::org::webRtc::RTCRtpTransceiverPtr > > getTransceivers() noexcept override;
          wrapper::org::webRtc::RTCDataChannelPtr createDataChannel(
            String label,
            wrapper::org::webRtc::RTCDataChannelInitPtr init
            ) noexcept override;

          // properties RTCPeerConnection
          wrapper::org::webRtc::RTCSignalingState get_signalingState() noexcept override;
          wrapper::org::webRtc::RTCIceGatheringState get_iceGatheringState() noexcept override;
          wrapper::org::webRtc::RTCIceConnectionState get_iceConnectionState() noexcept override;
          wrapper::org::webRtc::RTCPeerConnectionState get_connectionState_NotAvailable() noexcept override;
          Optional< bool > get_canTrickleIceCandidates_NotAvailable() noexcept override;
          wrapper::org::webRtc::RTCSessionDescriptionPtr get_localDescription() noexcept override;
          wrapper::org::webRtc::RTCSessionDescriptionPtr get_currentLocalDescription() noexcept override;
          wrapper::org::webRtc::RTCSessionDescriptionPtr get_pendingLocalDescription() noexcept override;
          wrapper::org::webRtc::RTCSessionDescriptionPtr get_remoteDescription() noexcept override;
          wrapper::org::webRtc::RTCSessionDescriptionPtr get_currentRemoteDescription() noexcept override;
          wrapper::org::webRtc::RTCSessionDescriptionPtr get_pendingRemoteDescription() noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          void notifySignalingChange(NativeType::SignalingState new_state) noexcept;
          void notifyDataChannel(rtc::scoped_refptr<::webrtc::DataChannelInterface> data_channel) noexcept;
          void notifyRenegotiationNeeded() noexcept;
          void notifyIceConnectionChange(NativeType::PeerConnectionInterface::IceConnectionState new_state) noexcept;
          void notifyIceGatheringChange(NativeType::PeerConnectionInterface::IceGatheringState new_state) noexcept;
          void notifyIceCandidate(const ::webrtc::IceCandidateInterface* candidate) noexcept;
          void notifyIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) noexcept;
          void notifyIceConnectionReceivingChange(bool receiving) noexcept;
          void notifyAddTrack(
                              rtc::scoped_refptr<::webrtc::RtpReceiverInterface> receiver,
                              const std::vector<rtc::scoped_refptr<::webrtc::MediaStreamInterface>>& streams
                              ) noexcept;
          void notifyTrack(rtc::scoped_refptr<::webrtc::RtpTransceiverInterface> transceiver) noexcept;
          void notifyRemoveTrack(rtc::scoped_refptr<::webrtc::RtpReceiverInterface> receiver) noexcept;

          void setupObserver();
          void teardownObserver();

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;

          ZS_NO_DISCARD() static NativeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

