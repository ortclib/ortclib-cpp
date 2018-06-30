
#pragma once

#include "types.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "api/mediastreaminterface.h"
#include "api/datachannelinterface.h"
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

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCDataChannelState toWrapper(::webrtc::DataChannelInterface::DataState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::DataChannelInterface::DataState toNative(wrapper::org::webrtc::RTCDataChannelState value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCDegradationPreference toWrapper(::webrtc::DegradationPreference value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::DegradationPreference toNative(wrapper::org::webrtc::RTCDegradationPreference value) noexcept;

          ZS_NO_DISCARD() static const char *toWrapper(::webrtc::FecMechanism value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::FecMechanism toNativeFecMechanism(const char *value) noexcept(false); // throws InvalidParameters

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCDtxStatus toWrapper(::webrtc::DtxStatus value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::DtxStatus toNative(wrapper::org::webrtc::RTCDtxStatus value) noexcept;

          ZS_NO_DISCARD() static const char *toWrapper(::webrtc::RtcpFeedbackType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::RtcpFeedbackType toNativeRtcpFeedbackType(const char *value) noexcept(false); // throws InvalidParameters

          ZS_NO_DISCARD() static const char *toWrapper(::webrtc::RtcpFeedbackMessageType value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::RtcpFeedbackMessageType toNativeRtcpFeedbackMessageType(const char *value) noexcept(false); // throws InvalidParameters

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCECCurve toWrapper(::rtc::ECCurve value) noexcept;
          ZS_NO_DISCARD() static ::rtc::ECCurve toNative(wrapper::org::webrtc::RTCECCurve value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCKeyType toWrapper(::rtc::KeyType value) noexcept;
          ZS_NO_DISCARD() static ::rtc::KeyType toNative(wrapper::org::webrtc::RTCKeyType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCIceCandidateType toWrapperRTCIceCandidateType(const char *value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static const char *toNative(wrapper::org::webrtc::RTCIceCandidateType value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCIceComponent toWrapperRTCIceComponent(int value) noexcept(false); // throws InvalidParameters
          ZS_NO_DISCARD() static int toNative(wrapper::org::webrtc::RTCIceComponent value) noexcept;

          ZS_NO_DISCARD() static wrapper::org::webrtc::RTCIceConnectionState toWrapper(::webrtc::PeerConnectionInterface::IceConnectionState value) noexcept;
          ZS_NO_DISCARD() static ::webrtc::PeerConnectionInterface::IceConnectionState toNative(wrapper::org::webrtc::RTCIceConnectionState value) noexcept;

#if 0


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

          PeerConnectionInterface
          enum SignalingState {
            kStable,
            kHaveLocalOffer,
            kHaveLocalPrAnswer,
            kHaveRemoteOffer,
            kHaveRemotePrAnswer,
            kClosed,
  };
          PeerConnectionInterface
          enum IceGatheringState {
            kIceGatheringNew,
            kIceGatheringGathering,
            kIceGatheringComplete
          };






          ///// StatsReport


          enum Direction {
            kSend = 0,
            kReceive,
  };

          enum StatsType {
            // StatsReport types.
            // A StatsReport of |type| = "googSession" contains overall information
            // about the thing libjingle calls a session (which may contain one
            // or more RTP sessions.
            kStatsReportTypeSession,

            // A StatsReport of |type| = "googTransport" contains information
            // about a libjingle "transport".
            kStatsReportTypeTransport,

            // A StatsReport of |type| = "googComponent" contains information
            // about a libjingle "channel" (typically, RTP or RTCP for a transport).
            // This is intended to be the same thing as an ICE "Component".
            kStatsReportTypeComponent,

            // A StatsReport of |type| = "googCandidatePair" contains information
            // about a libjingle "connection" - a single source/destination port pair.
            // This is intended to be the same thing as an ICE "candidate pair".
            kStatsReportTypeCandidatePair,

            // A StatsReport of |type| = "VideoBWE" is statistics for video Bandwidth
            // Estimation, which is global per-session.  The |id| field is "bweforvideo"
            // (will probably change in the future).
            kStatsReportTypeBwe,

            // A StatsReport of |type| = "ssrc" is statistics for a specific rtp stream.
            // The |id| field is the SSRC in decimal form of the rtp stream.
            kStatsReportTypeSsrc,

            // A StatsReport of |type| = "remoteSsrc" is statistics for a specific
            // rtp stream, generated by the remote end of the connection.
            kStatsReportTypeRemoteSsrc,

            // A StatsReport of |type| = "googTrack" is statistics for a specific media
            // track. The |id| field is the track id.
            kStatsReportTypeTrack,

            // A StatsReport of |type| = "localcandidate" or "remotecandidate" is
            // attributes on a specific ICE Candidate. It links to its connection pair
            // by candidate id. The string value is taken from
            // http://w3c.github.io/webrtc-stats/#rtcstatstype-enum*.
            kStatsReportTypeIceLocalCandidate,
            kStatsReportTypeIceRemoteCandidate,

            // A StatsReport of |type| = "googCertificate" contains an SSL certificate
            // transmitted by one of the endpoints of this connection.  The |id| is
            // controlled by the fingerprint, and is used to identify the certificate in
            // the Channel stats (as "googLocalCertificateId" or
            // "googRemoteCertificateId") and in any child certificates (as
            // "googIssuerId").
            kStatsReportTypeCertificate,

            // A StatsReport of |type| = "datachannel" with statistics for a
            // particular DataChannel.
            kStatsReportTypeDataChannel,
          };

          enum StatsValueName {
            kStatsValueNameActiveConnection,
            kStatsValueNameAecDivergentFilterFraction,
            kStatsValueNameAudioInputLevel,
            kStatsValueNameAudioOutputLevel,
            kStatsValueNameBytesReceived,
            kStatsValueNameBytesSent,
            kStatsValueNameCodecImplementationName,
            kStatsValueNameConcealedSamples,
            kStatsValueNameConcealmentEvents,
            kStatsValueNameDataChannelId,
            kStatsValueNameFramesDecoded,
            kStatsValueNameFramesEncoded,
            kStatsValueNameJitterBufferDelay,
            kStatsValueNameMediaType,
            kStatsValueNamePacketsLost,
            kStatsValueNamePacketsReceived,
            kStatsValueNamePacketsSent,
            kStatsValueNameProtocol,
            kStatsValueNameQpSum,
            kStatsValueNameReceiving,
            kStatsValueNameSelectedCandidatePairId,
            kStatsValueNameSsrc,
            kStatsValueNameState,
            kStatsValueNameTotalAudioEnergy,
            kStatsValueNameTotalSamplesDuration,
            kStatsValueNameTotalSamplesReceived,
            kStatsValueNameTransportId,
            kStatsValueNameSentPingRequestsTotal,
            kStatsValueNameSentPingRequestsBeforeFirstResponse,
            kStatsValueNameSentPingResponses,
            kStatsValueNameRecvPingRequests,
            kStatsValueNameRecvPingResponses,
            kStatsValueNameSentStunKeepaliveRequests,
            kStatsValueNameRecvStunKeepaliveResponses,
            kStatsValueNameStunKeepaliveRttTotal,
            kStatsValueNameStunKeepaliveRttSquaredTotal,

            // Internal StatsValue names.
            kStatsValueNameAccelerateRate,
            kStatsValueNameActualEncBitrate,
            kStatsValueNameAdaptationChanges,
            kStatsValueNameAvailableReceiveBandwidth,
            kStatsValueNameAvailableSendBandwidth,
            kStatsValueNameAvgEncodeMs,
            kStatsValueNameBandwidthLimitedResolution,
            kStatsValueNameBucketDelay,
            kStatsValueNameCaptureStartNtpTimeMs,
            kStatsValueNameCandidateIPAddress,
            kStatsValueNameCandidateNetworkType,
            kStatsValueNameCandidatePortNumber,
            kStatsValueNameCandidatePriority,
            kStatsValueNameCandidateTransportType,
            kStatsValueNameCandidateType,
            kStatsValueNameChannelId,
            kStatsValueNameCodecName,
            kStatsValueNameComponent,
            kStatsValueNameContentName,
            kStatsValueNameContentType,
            kStatsValueNameCpuLimitedResolution,
            kStatsValueNameCurrentDelayMs,
#ifdef WEBRTC_FEATURE_END_TO_END_DELAY
            kStatsValueNameCurrentEndToEndDelayMs,
#endif /* WEBRTC_FEATURE_END_TO_END_DELAY */
            kStatsValueNameDecodeMs,
            kStatsValueNameDecodingCNG,
            kStatsValueNameDecodingCTN,
            kStatsValueNameDecodingCTSG,
            kStatsValueNameDecodingMutedOutput,
            kStatsValueNameDecodingNormal,
            kStatsValueNameDecodingPLC,
            kStatsValueNameDecodingPLCCNG,
            kStatsValueNameDer,
            kStatsValueNameDtlsCipher,
            kStatsValueNameEchoDelayMedian,
            kStatsValueNameEchoDelayStdDev,
            kStatsValueNameEchoReturnLoss,
            kStatsValueNameEchoReturnLossEnhancement,
            kStatsValueNameEncodeUsagePercent,
            kStatsValueNameExpandRate,
            kStatsValueNameFingerprint,
            kStatsValueNameFingerprintAlgorithm,
            kStatsValueNameFirsReceived,
            kStatsValueNameFirsSent,
            kStatsValueNameFrameHeightInput,
            kStatsValueNameFrameHeightReceived,
            kStatsValueNameFrameHeightSent,
            kStatsValueNameFrameRateDecoded,
            kStatsValueNameFrameRateInput,
            kStatsValueNameFrameRateOutput,
            kStatsValueNameFrameRateReceived,
            kStatsValueNameFrameRateSent,
            kStatsValueNameFrameWidthInput,
            kStatsValueNameFrameWidthReceived,
            kStatsValueNameFrameWidthSent,
            kStatsValueNameHasEnteredLowResolution,
            kStatsValueNameHugeFramesSent,
            kStatsValueNameInitiator,
            kStatsValueNameInterframeDelayMaxMs,  // Max over last 10 seconds.
            kStatsValueNameIssuerId,
            kStatsValueNameJitterBufferMs,
            kStatsValueNameJitterReceived,
            kStatsValueNameLabel,
            kStatsValueNameLocalAddress,
            kStatsValueNameLocalCandidateId,
            kStatsValueNameLocalCandidateType,
            kStatsValueNameLocalCertificateId,
            kStatsValueNameMaxDecodeMs,
            kStatsValueNameMinPlayoutDelayMs,
            kStatsValueNameNacksReceived,
            kStatsValueNameNacksSent,
            kStatsValueNamePlisReceived,
            kStatsValueNamePlisSent,
            kStatsValueNamePreemptiveExpandRate,
            kStatsValueNamePreferredJitterBufferMs,
            kStatsValueNameRemoteAddress,
            kStatsValueNameRemoteCandidateId,
            kStatsValueNameRemoteCandidateType,
            kStatsValueNameRemoteCertificateId,
            kStatsValueNameRenderDelayMs,
            kStatsValueNameResidualEchoLikelihood,
            kStatsValueNameResidualEchoLikelihoodRecentMax,
            kStatsValueNameAnaBitrateActionCounter,
            kStatsValueNameAnaChannelActionCounter,
            kStatsValueNameAnaDtxActionCounter,
            kStatsValueNameAnaFecActionCounter,
            kStatsValueNameAnaFrameLengthIncreaseCounter,
            kStatsValueNameAnaFrameLengthDecreaseCounter,
            kStatsValueNameAnaUplinkPacketLossFraction,
            kStatsValueNameRetransmitBitrate,
            kStatsValueNameRtt,
            kStatsValueNameSecondaryDecodedRate,
            kStatsValueNameSecondaryDiscardedRate,
            kStatsValueNameSendPacketsDiscarded,
            kStatsValueNameSpeechExpandRate,
            kStatsValueNameSrtpCipher,
            kStatsValueNameTargetDelayMs,
            kStatsValueNameTargetEncBitrate,
            kStatsValueNameTimingFrameInfo,  // Result of |TimingFrameInfo::ToString|
            kStatsValueNameTrackId,
            kStatsValueNameTransmitBitrate,
            kStatsValueNameTransportType,
            kStatsValueNameTypingNoiseState,
            kStatsValueNameWritable,
          };

          struct Value {
            enum Type {
              kInt,           // int.
              kInt64,         // int64_t.
              kFloat,         // float.
              kString,        // std::string
              kStaticString,  // const char*.
              kBool,          // bool.
              kId,            // Id.
    };


          ///// StatsReport



            enum RTCIceCandidatePairState {
              RTCIceCandidatePairState_frozen,
              RTCIceCandidatePairState_waiting,
              RTCIceCandidatePairState_inProgress,
              RTCIceCandidatePairState_failed,
              RTCIceCandidatePairState_succeeded,
              RTCIceCandidatePairState_cancelled,
          };

#endif //0

        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

