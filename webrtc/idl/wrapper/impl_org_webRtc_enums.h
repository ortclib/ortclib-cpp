
#pragma once

#include "types.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "api/mediastreaminterface.h"
#include "api/datachannelinterface.h"
#include "api/statstypes.h"
#include "rtc_base/network_constants.h"
#include "impl_org_webRtc_post_include.h"


namespace webRtc
{
  enum class RTCErrorType;
}

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // (helper functions)
        //

        struct IEnum
        {
          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCErrorType toWrapper(::webrtc::RTCErrorType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::RTCErrorType toNative(wrapper::org::webRtc::RTCErrorType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCIceTransportPolicy toWrapper(::webrtc::PeerConnectionInterface::IceTransportsType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::IceTransportsType toNative(wrapper::org::webRtc::RTCIceTransportPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCBundlePolicy toWrapper(::webrtc::PeerConnectionInterface::BundlePolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::BundlePolicy toNative(wrapper::org::webRtc::RTCBundlePolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCRtcpMuxPolicy toWrapper(::webrtc::PeerConnectionInterface::RtcpMuxPolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::RtcpMuxPolicy toNative(wrapper::org::webRtc::RTCRtcpMuxPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCTcpCandidatePolicy toWrapper(::webrtc::PeerConnectionInterface::TcpCandidatePolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::TcpCandidatePolicy toNative(wrapper::org::webRtc::RTCTcpCandidatePolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCCandidateNetworkPolicy toWrapper(::webrtc::PeerConnectionInterface::CandidateNetworkPolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::CandidateNetworkPolicy toNative(wrapper::org::webRtc::RTCCandidateNetworkPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCContinualGatheringPolicy toWrapper(::webrtc::PeerConnectionInterface::ContinualGatheringPolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::ContinualGatheringPolicy toNative(wrapper::org::webRtc::RTCContinualGatheringPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCNetworkType toWrapper(::rtc::AdapterType value) noexcept;
          ZS_NO_DISCARD() static ::rtc::AdapterType toNative(wrapper::org::webRtc::RTCNetworkType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCNetworkType toWrapperRTCNetworkType(const char *value) noexcept(false);
          ZS_NO_DISCARD() static const char *toString(wrapper::org::webRtc::RTCNetworkType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCSdpSemantics toWrapper(::webrtc::SdpSemantics value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::SdpSemantics toNative(wrapper::org::webRtc::RTCSdpSemantics value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCTlsCertPolicy toWrapper(::webrtc::PeerConnectionInterface::TlsCertPolicy value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::TlsCertPolicy toNative(wrapper::org::webRtc::RTCTlsCertPolicy value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCSdpType toWrapper(::webrtc::SdpType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::SdpType toNative(wrapper::org::webRtc::RTCSdpType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::MediaSourceState toWrapper(::webrtc::MediaSourceInterface::SourceState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::MediaSourceInterface::SourceState toNative(wrapper::org::webRtc::MediaSourceState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::MediaStreamTrackState toWrapper(::webrtc::MediaStreamTrackInterface::TrackState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::MediaStreamTrackInterface::TrackState toNative(wrapper::org::webRtc::MediaStreamTrackState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCConfigurationType toWrapper(::webrtc::PeerConnectionInterface::RTCConfigurationType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::RTCConfigurationType toNative(wrapper::org::webRtc::RTCConfigurationType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCDataChannelState toWrapper(::webrtc::DataChannelInterface::DataState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::DataChannelInterface::DataState toNative(wrapper::org::webRtc::RTCDataChannelState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCDegradationPreference toWrapper(::webrtc::DegradationPreference value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::DegradationPreference toNative(wrapper::org::webRtc::RTCDegradationPreference value) noexcept;

          ZS_NO_DISCARD() static const char *toString(::webrtc::FecMechanism value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::FecMechanism toNativeFecMechanism(const char *value) noexcept(false); // throws InvalidParameters

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCDtxStatus toWrapper(::webrtc::DtxStatus value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::DtxStatus toNative(wrapper::org::webRtc::RTCDtxStatus value) noexcept;

          ZS_NO_DISCARD() static const char *toString(::webrtc::RtcpFeedbackType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::RtcpFeedbackType toNativeRtcpFeedbackType(const char *value) noexcept(false); // throws InvalidParameters

          ZS_NO_DISCARD() static const char *toString(::webrtc::RtcpFeedbackMessageType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::RtcpFeedbackMessageType toNativeRtcpFeedbackMessageType(const char *value) noexcept(false); // throws InvalidParameters

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCECCurve toWrapper(::rtc::ECCurve value) noexcept;
          ZS_NO_DISCARD() static ::rtc::ECCurve toNative(wrapper::org::webRtc::RTCECCurve value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCKeyType toWrapper(::rtc::KeyType value) noexcept;
          ZS_NO_DISCARD() static ::rtc::KeyType toNative(wrapper::org::webRtc::RTCKeyType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCIceCandidateType toWrapperRTCIceCandidateType(const char *value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static const char *toString(wrapper::org::webRtc::RTCIceCandidateType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCIceComponent toWrapperRTCIceComponent(int value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static int toNative(wrapper::org::webRtc::RTCIceComponent value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCIceConnectionState toWrapper(::webrtc::PeerConnectionInterface::IceConnectionState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::IceConnectionState toNative(wrapper::org::webRtc::RTCIceConnectionState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCIceGatheringState toWrapper(::webrtc::PeerConnectionInterface::IceGatheringState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::IceGatheringState toNative(wrapper::org::webRtc::RTCIceGatheringState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCSignalingState toWrapper(::webrtc::PeerConnectionInterface::SignalingState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::SignalingState toNative(wrapper::org::webRtc::RTCSignalingState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCIceProtocol toWrapperRTCIceProtocol(const char *value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static const char *toString(wrapper::org::webRtc::RTCIceProtocol value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCIceTcpCandidateType toWrapperRTCIceTcpCandidateType(const char *value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static const char *toString(wrapper::org::webRtc::RTCIceTcpCandidateType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCPriorityType toWrapperRTCPriorityType(double value) noexcept;
          ZS_NO_DISCARD() static double toNative(wrapper::org::webRtc::RTCPriorityType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCRtpTransceiverDirection toWrapper(::webrtc::RtpTransceiverDirection value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::RtpTransceiverDirection toNative(wrapper::org::webRtc::RTCRtpTransceiverDirection value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::VideoCaptureState toWrapper(::cricket::CaptureState value) noexcept;
          ZS_NO_DISCARD() static ::cricket::CaptureState toNative(wrapper::org::webRtc::VideoCaptureState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCStatsOutputLevel toWrapper(::webrtc::PeerConnectionInterface::StatsOutputLevel value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::StatsOutputLevel toNative(wrapper::org::webRtc::RTCStatsOutputLevel value) noexcept;

          ZS_NO_DISCARD() static zsLib::Optional<::webrtc::StatsReport::Direction> toWrapperDirection(const char *statId) noexcept;
          ZS_NO_DISCARD() static const char *toString(::webrtc::StatsReport::Direction value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCStatsType toWrapperRTCStatsType(const char *value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static const char *toString(wrapper::org::webRtc::RTCStatsType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCDtlsTransportState toWrapperRTCDtlsTransportState(const char *value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static const char *toString(wrapper::org::webRtc::RTCDtlsTransportState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCStatsIceCandidatePairState toWrapperRTCStatsIceCandidatePairState(const char *value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static const char *toString(wrapper::org::webRtc::RTCStatsIceCandidatePairState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webRtc::RTCDataChannelState toWrapperRTCDataChannelState(const char *value) noexcept(false);
          ZS_NO_DISCARD() static const char *toString(wrapper::org::webRtc::RTCDataChannelState value) noexcept;

          ZS_NO_DISCARD() static const char *toString(::cricket::MediaType value) noexcept;
          ZS_NO_DISCARD() static ::cricket::MediaType toNativeMediaType(const char *value) noexcept(false); // throws InvalidParameters


#if 0

          enum MediaType {
            MEDIA_TYPE_AUDIO,
            MEDIA_TYPE_VIDEO,
            MEDIA_TYPE_DATA
          };
          // not possible to convery
          enum RTCCodecType {
            RTCCodecType_encode,
            RTCCodecType_decode,
          };

          // not possible to convert
          enum RTCQualityLimitationReason {
            RTCQualityLimitationReason_none,
            RTCQualityLimitationReason_cpu,
            RTCQualityLimitationReason_bandwidth,
            RTCQualityLimitationReason_other,
          };

          // not possible to convert
          enum RTCIceRole {
            RTCIceRole_controlling,
            RTCIceRole_controlled,
          };

          // no conversion possible
          enum RTCPeerConnectionState {
            RTCPeerConnectionState_new,
            RTCPeerConnectionState_connecting,
            RTCPeerConnectionState_connected,
            RTCPeerConnectionState_disconnected,
            RTCPeerConnectionState_failed,
            RTCPeerConnectionState_closed,
          };

          // no conversion possible
          enum RTCIceCredentialType {
            RTCIceCredentialType_password,
            RTCIceCredentialType_oauth,
          };

#endif //0

        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

