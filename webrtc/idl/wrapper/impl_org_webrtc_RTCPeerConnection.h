
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCPeerConnection.h"

#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCPeerConnection : public wrapper::org::webrtc::RTCPeerConnection
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::RTCPeerConnection, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCPeerConnection, WrapperImplType);
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
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCStatsReportPtr > > getStats(wrapper::org::webrtc::RTCStatsTypeSetPtr statTypes) noexcept(false) override; // throws wrapper::org::webrtc::RTCErrorPtr

          // methods RTCPeerConnection
          void wrapper_init_org_webrtc_RTCPeerConnection(wrapper::org::webrtc::RTCConfigurationPtr config) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > createOffer(wrapper::org::webrtc::RTCOfferOptionsPtr options) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > createOffer(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > createAnswer(wrapper::org::webrtc::RTCAnswerOptionsPtr options) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > createAnswer(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept override;
          PromisePtr setLocalDescription(wrapper::org::webrtc::RTCSessionDescriptionPtr constraints) noexcept override;
          PromisePtr setRemoteDescription(wrapper::org::webrtc::RTCSessionDescriptionPtr constraints) noexcept override;
          wrapper::org::webrtc::RTCConfigurationPtr getConfiguration() noexcept override;
          void setConfiguration(wrapper::org::webrtc::RTCConfigurationPtr config) noexcept(false) override; // throws wrapper::org::webrtc::RTCErrorPtr
          PromisePtr addIceCandidate(wrapper::org::webrtc::RTCIceCandidatePtr candidate) noexcept override;
          void removeIceCandidates(shared_ptr< list< wrapper::org::webrtc::RTCIceCandidatePtr > > candidates) noexcept override;
          void setBitrate(wrapper::org::webrtc::RTCBitrateParametersPtr params) noexcept(false) override; // throws wrapper::org::webrtc::RTCErrorPtr
          void close() noexcept override;
          wrapper::org::webrtc::RTCRtpSenderPtr addTrack(wrapper::org::webrtc::MediaStreamTrackPtr track) noexcept(false) override; // throws wrapper::org::webrtc::RTCErrorPtr
          bool removeTrack(wrapper::org::webrtc::RTCRtpSenderPtr sender) noexcept override;
          wrapper::org::webrtc::RTCRtpTransceiverPtr addTransceiver(wrapper::org::webrtc::MediaStreamTrackPtr track) noexcept(false) override; // throws wrapper::org::webrtc::RTCErrorPtr
          wrapper::org::webrtc::RTCRtpTransceiverPtr addTransceiver(String kind) noexcept(false) override; // throws wrapper::org::webrtc::RTCErrorPtr
          wrapper::org::webrtc::RTCRtpSenderPtr addSender(
            String kind,
            String trackId
            ) noexcept override;
          shared_ptr< list< wrapper::org::webrtc::RTCRtpSenderPtr > > getSenders() noexcept override;
          shared_ptr< list< wrapper::org::webrtc::RTCRtpReceiverPtr > > getReceivers() noexcept override;
          shared_ptr< list< wrapper::org::webrtc::RTCRtpTransceiverPtr > > getTransceivers() noexcept override;
          wrapper::org::webrtc::RTCDataChannelPtr createDataChannel(
            String label,
            wrapper::org::webrtc::RTCDataChannelInitPtr init
            ) noexcept override;

          // properties RTCPeerConnection
          wrapper::org::webrtc::RTCSignalingState get_signalingState() noexcept override;
          wrapper::org::webrtc::RTCIceGatheringState get_iceGatheringState() noexcept override;
          wrapper::org::webrtc::RTCIceConnectionState get_iceConnectionState() noexcept override;
          wrapper::org::webrtc::RTCPeerConnectionState get_connectionState_NotAvailable() noexcept override;
          Optional< bool > get_canTrickleIceCandidates_NotAvailable() noexcept override;
          wrapper::org::webrtc::RTCSessionDescriptionPtr get_localDescription() noexcept override;
          wrapper::org::webrtc::RTCSessionDescriptionPtr get_currentLocalDescription() noexcept override;
          wrapper::org::webrtc::RTCSessionDescriptionPtr get_pendingLocalDescription() noexcept override;
          wrapper::org::webrtc::RTCSessionDescriptionPtr get_remoteDescription() noexcept override;
          wrapper::org::webrtc::RTCSessionDescriptionPtr get_currentRemoteDescription() noexcept override;
          wrapper::org::webrtc::RTCSessionDescriptionPtr get_pendingRemoteDescription() noexcept override;

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

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

