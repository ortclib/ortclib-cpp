
#pragma once

#include "generated/types.h"

#include <zsLib/Exception.h>

namespace wrapper {
  namespace impl {

    namespace org {
      namespace webRtc {

        ZS_DECLARE_TYPEDEF_PTR(::zsLib::Exceptions::InvalidArgument, InvalidParameters);
        ZS_DECLARE_TYPEDEF_PTR(::zsLib::IMessageQueue, IMessageQueue);

        ZS_DECLARE_STRUCT_PTR(AudioOptions);
        ZS_DECLARE_STRUCT_PTR(AudioTrackSource);
        ZS_DECLARE_STRUCT_PTR(Constraint);
        ZS_DECLARE_STRUCT_PTR(EventQueue);
        ZS_DECLARE_STRUCT_PTR(EventQueueMaker);
        ZS_DECLARE_STRUCT_PTR(MediaConstraints);
        ZS_DECLARE_STRUCT_PTR(MediaElement);
        ZS_DECLARE_STRUCT_PTR(MediaSource);
        ZS_DECLARE_STRUCT_PTR(MediaStreamTrack);
        ZS_DECLARE_STRUCT_PTR(MediaTrackSource);
        ZS_DECLARE_STRUCT_PTR(MessageEvent);
        ZS_DECLARE_STRUCT_PTR(MillisecondIntervalRange);
        ZS_DECLARE_STRUCT_PTR(RTCAnswerOptions);
        ZS_DECLARE_STRUCT_PTR(RTCAudioHandlerStats);
        ZS_DECLARE_STRUCT_PTR(RTCAudioReceiverStats);
        ZS_DECLARE_STRUCT_PTR(RTCAudioSenderStats);
        ZS_DECLARE_STRUCT_PTR(RTCBitrateParameters);
        ZS_DECLARE_STRUCT_PTR(RTCCertificate);
        ZS_DECLARE_STRUCT_PTR(RTCCertificateStats);
        ZS_DECLARE_STRUCT_PTR(RTCCodecStats);
        ZS_DECLARE_STRUCT_PTR(RTCConfiguration);
        ZS_DECLARE_STRUCT_PTR(RTCDataChannel);
        ZS_DECLARE_STRUCT_PTR(RTCDataChannelEvent);
        ZS_DECLARE_STRUCT_PTR(RTCDataChannelInit);
        ZS_DECLARE_STRUCT_PTR(RTCDataChannelStats);
        ZS_DECLARE_STRUCT_PTR(RTCDtlsFingerprint);
        ZS_DECLARE_STRUCT_PTR(RTCDtmfSender);
        ZS_DECLARE_STRUCT_PTR(RTCDtmfToneChangeEvent);
        ZS_DECLARE_STRUCT_PTR(RTCError);
        ZS_DECLARE_STRUCT_PTR(RTCIceCandidate);
        ZS_DECLARE_STRUCT_PTR(RTCIceCandidateInit);
        ZS_DECLARE_STRUCT_PTR(RTCIceCandidatePairStats);
        ZS_DECLARE_STRUCT_PTR(RTCIceCandidateStats);
        ZS_DECLARE_STRUCT_PTR(RTCIceServer);
        ZS_DECLARE_STRUCT_PTR(RTCInboundRtpStreamStats);
        ZS_DECLARE_STRUCT_PTR(RTCKeyParams);
        ZS_DECLARE_STRUCT_PTR(RTCMediaHandlerStats);
        ZS_DECLARE_STRUCT_PTR(RTCMediaStreamStats);
        ZS_DECLARE_STRUCT_PTR(RTCOAuthCredential);
        ZS_DECLARE_STRUCT_PTR(RTCOfferAnswerOptions);
        ZS_DECLARE_STRUCT_PTR(RTCOfferOptions);
        ZS_DECLARE_STRUCT_PTR(RTCOutboundRtpStreamStats);
        ZS_DECLARE_STRUCT_PTR(RTCPeerConnection);
        ZS_DECLARE_STRUCT_PTR(RTCPeerConnectionIceErrorEvent);
        ZS_DECLARE_STRUCT_PTR(RTCPeerConnectionIceEvent);
        ZS_DECLARE_STRUCT_PTR(RTCPeerConnectionStats);
        ZS_DECLARE_STRUCT_PTR(RTCRSAParams);
        ZS_DECLARE_STRUCT_PTR(RTCReceivedRtpStreamStats);
        ZS_DECLARE_STRUCT_PTR(RTCRemoteInboundRtpStreamStats);
        ZS_DECLARE_STRUCT_PTR(RTCRemoteOutboundRtpStreamStats);
        ZS_DECLARE_STRUCT_PTR(RTCRtcpFeedback);
        ZS_DECLARE_STRUCT_PTR(RTCRtcpParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpCapabilities);
        ZS_DECLARE_STRUCT_PTR(RTCRtpCodecCapability);
        ZS_DECLARE_STRUCT_PTR(RTCRtpCodecParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpCodingParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpContributingSource);
        ZS_DECLARE_STRUCT_PTR(RTCRtpContributingSourceStats);
        ZS_DECLARE_STRUCT_PTR(RTCRtpDecodingParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpEncodingParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpFecParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpHeaderExtensionCapability);
        ZS_DECLARE_STRUCT_PTR(RTCRtpHeaderExtensionParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpReceiveParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpReceiver);
        ZS_DECLARE_STRUCT_PTR(RTCRtpRtxParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpSendParameters);
        ZS_DECLARE_STRUCT_PTR(RTCRtpSender);
        ZS_DECLARE_STRUCT_PTR(RTCRtpStreamStats);
        ZS_DECLARE_STRUCT_PTR(RTCRtpSynchronizationSource);
        ZS_DECLARE_STRUCT_PTR(RTCRtpTransceiver);
        ZS_DECLARE_STRUCT_PTR(RTCRtpTransceiverInit);
        ZS_DECLARE_STRUCT_PTR(RTCSenderAudioTrackAttachmentStats);
        ZS_DECLARE_STRUCT_PTR(RTCSenderVideoTrackAttachmentStats);
        ZS_DECLARE_STRUCT_PTR(RTCSentRtpStreamStats);
        ZS_DECLARE_STRUCT_PTR(RTCSessionDescription);
        ZS_DECLARE_STRUCT_PTR(RTCSessionDescriptionInit);
        ZS_DECLARE_STRUCT_PTR(RTCStats);
        ZS_DECLARE_STRUCT_PTR(RTCStatsProvider);
        ZS_DECLARE_STRUCT_PTR(RTCStatsReport);
        ZS_DECLARE_STRUCT_PTR(RTCStatsTypeSet);
        ZS_DECLARE_STRUCT_PTR(RTCTrackEvent);
        ZS_DECLARE_STRUCT_PTR(RTCTransportStats);
        ZS_DECLARE_STRUCT_PTR(RTCVideoHandlerStats);
        ZS_DECLARE_STRUCT_PTR(RTCVideoReceiverStats);
        ZS_DECLARE_STRUCT_PTR(RTCVideoSenderStats);
        ZS_DECLARE_STRUCT_PTR(VideoCapturer);
        ZS_DECLARE_STRUCT_PTR(VideoCapturerInputSize);
        ZS_DECLARE_STRUCT_PTR(VideoDeviceInfo);
        ZS_DECLARE_STRUCT_PTR(VideoFormat);
        ZS_DECLARE_STRUCT_PTR(VideoTrackSource);
        ZS_DECLARE_STRUCT_PTR(VideoTrackSourceStats);
        ZS_DECLARE_STRUCT_PTR(WebRtcLib);

      } // namespace webRtc
    } // namespace org
  } // namespace impl
} // namespace wrapper

