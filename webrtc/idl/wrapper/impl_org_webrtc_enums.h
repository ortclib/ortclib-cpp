
#pragma once

#include "types.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "api/mediastreaminterface.h"
#include "rtc_base/network_constants.h"
#include "impl_org_webrtc_post_include.h"


namespace webrtc
{
  enum class RTCErrorType;
}

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // (helper functions)
        //

        struct IEnum
        {
          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCErrorType toWrapper(::webrtc::RTCErrorType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::RTCErrorType toNative(wrapper::org::webrtc::RTCErrorType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCIceTransportPolicy toWrapper(::webrtc::PeerConnectionInterface::IceTransportsType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::IceTransportsType toNative(wrapper::org::webrtc::RTCIceTransportPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCBundlePolicy toWrapper(::webrtc::PeerConnectionInterface::BundlePolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::BundlePolicy toNative(wrapper::org::webrtc::RTCBundlePolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCRtcpMuxPolicy toWrapper(::webrtc::PeerConnectionInterface::RtcpMuxPolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::RtcpMuxPolicy toNative(wrapper::org::webrtc::RTCRtcpMuxPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCTcpCandidatePolicy toWrapper(::webrtc::PeerConnectionInterface::TcpCandidatePolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::TcpCandidatePolicy toNative(wrapper::org::webrtc::RTCTcpCandidatePolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCCandidateNetworkPolicy toWrapper(::webrtc::PeerConnectionInterface::CandidateNetworkPolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::CandidateNetworkPolicy toNative(wrapper::org::webrtc::RTCCandidateNetworkPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCContinualGatheringPolicy toWrapper(::webrtc::PeerConnectionInterface::ContinualGatheringPolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::ContinualGatheringPolicy toNative(wrapper::org::webrtc::RTCContinualGatheringPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCAdapterType toWrapper(::rtc::AdapterType value) noexcept;
          ZS_NO_DISCARD() static ::rtc::AdapterType toNative(wrapper::org::webrtc::RTCAdapterType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCSdpSemantics toWrapper(::webrtc::SdpSemantics value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::SdpSemantics toNative(wrapper::org::webrtc::RTCSdpSemantics value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCTlsCertPolicy toWrapper(::webrtc::PeerConnectionInterface::TlsCertPolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::TlsCertPolicy toNative(wrapper::org::webrtc::RTCTlsCertPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCSdpType toWrapper(::webrtc::SdpType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::SdpType toNative(wrapper::org::webrtc::RTCSdpType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::MediaSourceState toWrapper(::webrtc::MediaSourceInterface::SourceState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::MediaSourceInterface::SourceState toNative(wrapper::org::webrtc::MediaSourceState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::MediaStreamTrackState toWrapper(::webrtc::MediaStreamTrackInterface::TrackState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::MediaStreamTrackInterface::TrackState toNative(wrapper::org::webrtc::MediaStreamTrackState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCConfigurationType toWrapper(::webrtc::PeerConnectionInterface::RTCConfigurationType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::RTCConfigurationType toNative(wrapper::org::webrtc::RTCConfigurationType value) noexcept;

#if 0
          enum RTCConfigurationType {
            RTCConfigurationType_safe,
            RTCConfigurationType_aggressive,
          };

          PeerConnectionInterface::RTCConfigurationType {
            // A configuration that is safer to use, despite not having the best
            // performance. Currently this is the default configuration.
            kSafe,
            // An aggressive configuration that has better performance, although it
            // may be riskier and may need extra support in the application.
            kAggressive
          };

          enum RTCDataChannelState {
            RTCDataChannelState_connecting,
            RTCDataChannelState_open,
            RTCDataChannelState_closing,
            RTCDataChannelState_closed,
          };
          enum RTCDegradationPreference {
            RTCDegradationPreference_maintainFramerate,
            RTCDegradationPreference_maintainResolution,
            RTCDegradationPreference_balanced,
          };
          enum RTCDtxStatus {
            RTCDtxStatus_disabled,
            RTCDtxStatus_enabled,
          };
          enum RTCECCurve {
            RTCECCurve_nistP256,
          };
     
          enum RTCIceCandidatePairState {
            RTCIceCandidatePairState_frozen,
            RTCIceCandidatePairState_waiting,
            RTCIceCandidatePairState_inProgress,
            RTCIceCandidatePairState_failed,
            RTCIceCandidatePairState_succeeded,
            RTCIceCandidatePairState_cancelled,
          };
          enum RTCIceCandidateType {
            RTCIceCandidateType_host,
            RTCIceCandidateType_srflex,
            RTCIceCandidateType_prflx,
            RTCIceCandidateType_relay,
          };
          enum RTCIceComponent {
            RTCIceComponent_rtp = 1,
            RTCIceComponent_rtcp = 2,
          };
          enum RTCIceConnectionState {
            RTCIceConnectionState_new,
            RTCIceConnectionState_checking,
            RTCIceConnectionState_connected,
            RTCIceConnectionState_completed,
            RTCIceConnectionState_failed,
            RTCIceConnectionState_disconnected,
            RTCIceConnectionState_closed,
          };
          enum RTCIceCredentialType {
            RTCIceCredentialType_password,
            RTCIceCredentialType_oauth,
          };
          enum RTCIceGatheringState {
            RTCIceGatheringState_new,
            RTCIceGatheringState_gathering,
            RTCIceGatheringState_complete,
          };
          enum RTCIceProtocol {
            RTCIceProtocol_udp,
            RTCIceProtocol_tcp,
          };
          enum RTCIceTcpCandidateType {
            RTCIceTcpCandidateType_active,
            RTCIceTcpCandidateType_passive,
            RTCIceTcpCandidateType_so,
          };
          enum RTCKeyType {
            RTCKeyType_rsa,
            RTCKeyType_ecdsa,
          };
          enum RTCPeerConnectionState {
            RTCPeerConnectionState_new,
            RTCPeerConnectionState_connecting,
            RTCPeerConnectionState_connected,
            RTCPeerConnectionState_disconnected,
            RTCPeerConnectionState_failed,
            RTCPeerConnectionState_closed,
          };
          enum RTCPriorityType {
            RTCPriorityType_veryLow,
            RTCPriorityType_low,
            RTCPriorityType_medium,
            RTCPriorityType_high,
          };
          enum RTCRtpTransceiverDirection {
            RTCRtpTransceiverDirection_sendrecv,
            RTCRtpTransceiverDirection_sendonly,
            RTCRtpTransceiverDirection_recvonly,
            RTCRtpTransceiverDirection_inactive,
          };
          enum RTCSignalingState {
            RTCSignalingState_stable,
            RTCSignalingState_haveLocalOffer,
            RTCSignalingState_haveLocalPranswer,
            RTCSignalingState_haveRemoteOffer,
            RTCSignalingState_haveRemotePranswer,
            RTCSignalingState_closed,
          };
          enum RTCStatsOutputLevel {
            RTCStatsOutputLevel_standard,
            RTCStatsOutputLevel_debug,
          };
          enum RTCStatsType {
            RTCStatsType_inboundRtp,
            RTCStatsType_outboundRtp,
            RTCStatsType_codec,
            RTCStatsType_sctpTransport,
            RTCStatsType_dataChannel,
            RTCStatsType_stream,
            RTCStatsType_track,
            RTCStatsType_iceGatherer,
            RTCStatsType_iceTransport,
            RTCStatsType_dtlsTransport,
            RTCStatsType_srtpTransport,
            RTCStatsType_certificate,
            RTCStatsType_candidate,
            RTCStatsType_candidatePair,
            RTCStatsType_localCandidate,
            RTCStatsType_remoteCandidate,
          };
          enum VideoCaptureState {
            VideoCaptureState_stopped,
            VideoCaptureState_starting,
            VideoCaptureState_running,
            VideoCaptureState_failed,
          };
#endif //0

        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

