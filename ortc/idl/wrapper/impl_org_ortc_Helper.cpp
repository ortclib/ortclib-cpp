
#include "impl_org_ortc_Helper.h"

#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_RTCStatsTypeSet.h"
#include "impl_org_ortc_RTCStatsReport.h"
#include "impl_org_ortc_OverconstrainedError.h"
#include "impl_org_ortc_Error.h"

#include <ortc/IStatsProvider.h>
#include <zsLib/Log.h>
#include <zsLib/IMessageQueueThread.h>

namespace wrapper { namespace impl { namespace org { namespace ortc { ZS_DECLARE_SUBSYSTEM(ortc_wrapper); } } } }
namespace wrapper { namespace impl { namespace org { namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortc_adapter_wrapper); } } } } }

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {
        } // namespace adapater

        //---------------------------------------------------------------------
        zsLib::IMessageQueuePtr Helper::getGuiQueue()
        {
          return zsLib::IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue();
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::log::Level Helper::toWrapper(zsLib::Log::Level level)
        {
          switch (level)
          {
            case zsLib::Log::Level::None:   return wrapper::org::ortc::log::Level_none;
            case zsLib::Log::Level::Basic:  return wrapper::org::ortc::log::Level_basic;
            case zsLib::Log::Level::Detail: return wrapper::org::ortc::log::Level_detail;
            case zsLib::Log::Level::Debug:  return wrapper::org::ortc::log::Level_debug;
            case zsLib::Log::Level::Trace:  return wrapper::org::ortc::log::Level_trace;
            case zsLib::Log::Level::Insane: return wrapper::org::ortc::log::Level_insane;
          }
          ZS_THROW_INVALID_ARGUMENT("level is unknown");
          return toWrapper(zsLib::Log::Level_First);
        }

        //---------------------------------------------------------------------
        zsLib::Log::Level Helper::toNative(wrapper::org::ortc::log::Level level)
        {
          switch (level)
          {
            case wrapper::org::ortc::log::Level_none:   return zsLib::Log::Level::None;
            case wrapper::org::ortc::log::Level_basic:  return zsLib::Log::Level::Basic;
            case wrapper::org::ortc::log::Level_detail: return zsLib::Log::Level::Detail;
            case wrapper::org::ortc::log::Level_debug:  return zsLib::Log::Level::Debug;
            case wrapper::org::ortc::log::Level_trace:  return zsLib::Log::Level::Trace;
            case wrapper::org::ortc::log::Level_insane: return zsLib::Log::Level::Insane;
          }
          ZS_THROW_INVALID_ARGUMENT("level is unknown");
          return zsLib::Log::Level_First;
        }

        //---------------------------------------------------------------------
        const char *Helper::toNative(wrapper::org::ortc::log::Component component)
        {
          const char *componentStr{};
          switch (component) {
            case wrapper::org::ortc::log::Component_zsLib:                          componentStr = "zsLib"; break;
            case wrapper::org::ortc::log::Component_zsLibSocket:                    componentStr = "zsLib_socket"; break;
            case wrapper::org::ortc::log::Component_services:                       componentStr = "ortc_services"; break;
            case wrapper::org::ortc::log::Component_servicesDns:                    componentStr = "ortc_services_dns"; break;
            case wrapper::org::ortc::log::Component_servicesTurn:                   componentStr = "ortc_services_turn"; break;
            case wrapper::org::ortc::log::Component_servicesHttp:                   componentStr = "ortc_services_http"; break;
            case wrapper::org::ortc::log::Component_servicesWire:                   componentStr = "ortc_services_wire"; break;
            case wrapper::org::ortc::log::Component_servicesStun:                   componentStr = "ortc_services_stun"; break;
            case wrapper::org::ortc::log::Component_ortcLib:                        componentStr = "ortclib"; break;
            case wrapper::org::ortc::log::Component_ortcLibWebrtc:                  componentStr = "ortclib_webrtc"; break;
            case wrapper::org::ortc::log::Component_ortcLibDtlsTransport:           componentStr = "ortclib_dtlstransport"; break;
            case wrapper::org::ortc::log::Component_ortcLibIceGatherer:             componentStr = "ortclib_icegatherer"; break;
            case wrapper::org::ortc::log::Component_ortcLibIceGathererRouter:       componentStr = "ortclib_icegatherer_router"; break;
            case wrapper::org::ortc::log::Component_ortcLibIceTransport:            componentStr = "ortclib_icetransport"; break;
            case wrapper::org::ortc::log::Component_ortcLibIceTransportController:  componentStr = "ortclib_icetransport_controller"; break;
            case wrapper::org::ortc::log::Component_ortcLibMediaDevices:            componentStr = "ortclib_mediadevices"; break;
            case wrapper::org::ortc::log::Component_ortcLibMediaStreamTrack:        componentStr = "ortclib_mediastreamtrack"; break;
            case wrapper::org::ortc::log::Component_ortcLibRtpRtcpPacket:           componentStr = "ortclib_rtp_rtcp_packet"; break;
            case wrapper::org::ortc::log::Component_ortcLibRtpListener:             componentStr = "ortclib_rtplistener"; break;
            case wrapper::org::ortc::log::Component_ortcLibRtpMediaEngine:          componentStr = "ortclib_rtpmediaengine"; break;
            case wrapper::org::ortc::log::Component_ortcLibRtpReceiver:             componentStr = "ortclib_rtpreceiver"; break;
            case wrapper::org::ortc::log::Component_ortcLibRtpSender:               componentStr = "ortclib_rtpsender"; break;
            case wrapper::org::ortc::log::Component_ortcLibRtpTypes:                componentStr = "ortclib_rtptypes"; break;
            case wrapper::org::ortc::log::Component_ortcLibSctpDataChannel:         componentStr = "ortclib_sctp_datachannel"; break;
            case wrapper::org::ortc::log::Component_ortcLibSrtp:                    componentStr = "ortclib_srtp"; break;
            case wrapper::org::ortc::log::Component_ortcLibStats:                   componentStr = "ortclib_stats"; break;
            case wrapper::org::ortc::log::Component_ortcLibAdapter:                 componentStr = "ortclib_adapter"; break;
          };
          ZS_THROW_INVALID_ARGUMENT_IF(!componentStr);
          return componentStr;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::MediaStreamTrackKind Helper::toWrapper(IMediaStreamTrackTypes::Kinds kind)
        {
          switch (kind)
          {
            case IMediaStreamTrackTypes::Kind_Audio: return wrapper::org::ortc::MediaStreamTrackKind::MediaStreamTrackKind_audio;
            case IMediaStreamTrackTypes::Kind_Video: return wrapper::org::ortc::MediaStreamTrackKind::MediaStreamTrackKind_video;
          }
          ZS_THROW_INVALID_ARGUMENT("kind is unknown");
          return toWrapper(IMediaStreamTrackTypes::Kind_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IMediaStreamTrackTypes::Kinds Helper::toNative(wrapper::org::ortc::MediaStreamTrackKind kind)
        {
          switch (kind)
          {
            case wrapper::org::ortc::MediaStreamTrackKind::MediaStreamTrackKind_audio: return IMediaStreamTrackTypes::Kind_Audio;
            case wrapper::org::ortc::MediaStreamTrackKind::MediaStreamTrackKind_video: return IMediaStreamTrackTypes::Kind_Video;
          }
          ZS_THROW_INVALID_ARGUMENT("kind is unknown");
          return IMediaStreamTrackTypes::Kind_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::MediaStreamTrackState Helper::toWrapper(IMediaStreamTrackTypes::States state)
        {
          switch (state)
          {
            case IMediaStreamTrackTypes::State_Live: return wrapper::org::ortc::MediaStreamTrackState::MediaStreamTrackState_live;
            case IMediaStreamTrackTypes::State_Ended: return wrapper::org::ortc::MediaStreamTrackState::MediaStreamTrackState_ended;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return wrapper::org::ortc::MediaStreamTrackState::MediaStreamTrackState_ended;
        }

        //---------------------------------------------------------------------
        ::ortc::IMediaDevicesTypes::DeviceKinds Helper::toNative(wrapper::org::ortc::MediaDeviceKind kind)
        {
          switch (kind)
          {
            case wrapper::org::ortc::MediaDeviceKind_audioInput:       return ::ortc::IMediaDevicesTypes::DeviceKind_AudioInput;
            case wrapper::org::ortc::MediaDeviceKind_audioOutput:      return ::ortc::IMediaDevicesTypes::DeviceKind_AudioOutput;
            case wrapper::org::ortc::MediaDeviceKind_videoInput:       return ::ortc::IMediaDevicesTypes::DeviceKind_VideoInput;
          }

          ZS_THROW_INVALID_ARGUMENT("kind is unknown");
          return ::ortc::IMediaDevicesTypes::DeviceKind_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::MediaDeviceKind Helper::toWrapper(::ortc::IMediaDevicesTypes::DeviceKinds kind)
        {
          switch (kind)
          {
            case ::ortc::IMediaDevicesTypes::DeviceKind_AudioInput:     return wrapper::org::ortc::MediaDeviceKind_audioInput;
            case ::ortc::IMediaDevicesTypes::DeviceKind_AudioOutput:    return wrapper::org::ortc::MediaDeviceKind_audioOutput;
            case ::ortc::IMediaDevicesTypes::DeviceKind_VideoInput:     return wrapper::org::ortc::MediaDeviceKind_videoInput;
          }
          ZS_THROW_INVALID_ARGUMENT("kind is unknown");
          return toWrapper(::ortc::IMediaDevicesTypes::DeviceKind_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IStatsReportTypes::StatsTypes Helper::toNative(wrapper::org::ortc::RTCStatsType type)
        {
          switch (type)
          {
            case wrapper::org::ortc::RTCStatsType_inboundRtp:       return ::ortc::IStatsReportTypes::StatsType_InboundRTP;
            case wrapper::org::ortc::RTCStatsType_outboundRtp:      return ::ortc::IStatsReportTypes::StatsType_OutboundRTP;
            case wrapper::org::ortc::RTCStatsType_codec:            return ::ortc::IStatsReportTypes::StatsType_Codec;
            case wrapper::org::ortc::RTCStatsType_sctpTransport:    return ::ortc::IStatsReportTypes::StatsType_SCTPTransport;
            case wrapper::org::ortc::RTCStatsType_dataChannel:      return ::ortc::IStatsReportTypes::StatsType_DataChannel;
            case wrapper::org::ortc::RTCStatsType_stream:           return ::ortc::IStatsReportTypes::StatsType_Stream;
            case wrapper::org::ortc::RTCStatsType_track:            return ::ortc::IStatsReportTypes::StatsType_Track;
            case wrapper::org::ortc::RTCStatsType_iceGatherer:      return ::ortc::IStatsReportTypes::StatsType_ICEGatherer;
            case wrapper::org::ortc::RTCStatsType_iceTransport:     return ::ortc::IStatsReportTypes::StatsType_ICETransport;
            case wrapper::org::ortc::RTCStatsType_dtlsTransport:    return ::ortc::IStatsReportTypes::StatsType_DTLSTransport;
            case wrapper::org::ortc::RTCStatsType_srtpTransport:    return ::ortc::IStatsReportTypes::StatsType_SRTPTransport;
            case wrapper::org::ortc::RTCStatsType_certificate:      return ::ortc::IStatsReportTypes::StatsType_Certificate;
            case wrapper::org::ortc::RTCStatsType_candidate:        return ::ortc::IStatsReportTypes::StatsType_Candidate;
            case wrapper::org::ortc::RTCStatsType_candidatePair:    return ::ortc::IStatsReportTypes::StatsType_CandidatePair;
            case wrapper::org::ortc::RTCStatsType_localCandidate:   return ::ortc::IStatsReportTypes::StatsType_LocalCandidate;
            case wrapper::org::ortc::RTCStatsType_remoteCandidate:  return ::ortc::IStatsReportTypes::StatsType_RemoteCandidate;
          }

          ZS_THROW_INVALID_ARGUMENT("stats type is unknown");
          return ::ortc::IStatsReportTypes::StatsType_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCStatsType Helper::toWrapper(::ortc::IStatsReportTypes::StatsTypes type)
        {
          switch (type)
          {
            case ::ortc::IStatsReportTypes::StatsType_InboundRTP:     return wrapper::org::ortc::RTCStatsType_inboundRtp;
            case ::ortc::IStatsReportTypes::StatsType_OutboundRTP:    return wrapper::org::ortc::RTCStatsType_outboundRtp;
            case ::ortc::IStatsReportTypes::StatsType_Codec:          return wrapper::org::ortc::RTCStatsType_codec;
            case ::ortc::IStatsReportTypes::StatsType_SCTPTransport:  return wrapper::org::ortc::RTCStatsType_sctpTransport;
            case ::ortc::IStatsReportTypes::StatsType_DataChannel:    return wrapper::org::ortc::RTCStatsType_dataChannel;
            case ::ortc::IStatsReportTypes::StatsType_Stream:         return wrapper::org::ortc::RTCStatsType_stream;
            case ::ortc::IStatsReportTypes::StatsType_Track:          return wrapper::org::ortc::RTCStatsType_track;
            case ::ortc::IStatsReportTypes::StatsType_ICEGatherer:    return wrapper::org::ortc::RTCStatsType_iceGatherer;
            case ::ortc::IStatsReportTypes::StatsType_ICETransport:   return wrapper::org::ortc::RTCStatsType_iceTransport;
            case ::ortc::IStatsReportTypes::StatsType_DTLSTransport:  return wrapper::org::ortc::RTCStatsType_dtlsTransport;
            case ::ortc::IStatsReportTypes::StatsType_SRTPTransport:  return wrapper::org::ortc::RTCStatsType_srtpTransport;
            case ::ortc::IStatsReportTypes::StatsType_Certificate:      return wrapper::org::ortc::RTCStatsType_certificate;
            case ::ortc::IStatsReportTypes::StatsType_Candidate:        return wrapper::org::ortc::RTCStatsType_candidate;
            case ::ortc::IStatsReportTypes::StatsType_CandidatePair:    return wrapper::org::ortc::RTCStatsType_candidatePair;
            case ::ortc::IStatsReportTypes::StatsType_LocalCandidate:   return wrapper::org::ortc::RTCStatsType_localCandidate;
            case ::ortc::IStatsReportTypes::StatsType_RemoteCandidate:  return wrapper::org::ortc::RTCStatsType_remoteCandidate;
          }
          ZS_THROW_INVALID_ARGUMENT("stats type is unknown");
          return toWrapper(::ortc::IStatsReportTypes::StatsType_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCSctpTransportState Helper::toWrapper(::ortc::ISCTPTransportTypes::States state)
        {
          switch (state)
          {
            case ::ortc::ISCTPTransportTypes::State_New:            return wrapper::org::ortc::RTCSctpTransportState_new;
            case ::ortc::ISCTPTransportTypes::State_Connecting:     return wrapper::org::ortc::RTCSctpTransportState_connecting;
            case ::ortc::ISCTPTransportTypes::State_Connected:      return wrapper::org::ortc::RTCSctpTransportState_connected;
            case ::ortc::ISCTPTransportTypes::State_Closed:         return wrapper::org::ortc::RTCSctpTransportState_closed;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return toWrapper(::ortc::ISCTPTransportTypes::State_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCDataChannelState Helper::toWrapper(::ortc::IDataChannelTypes::States state)
        {
          switch (state)
          {
            case ::ortc::IDataChannelTypes::State_Connecting: return wrapper::org::ortc::RTCDataChannelState_connecting;
            case ::ortc::IDataChannelTypes::State_Open:       return wrapper::org::ortc::RTCDataChannelState_open;
            case ::ortc::IDataChannelTypes::State_Closing:    return wrapper::org::ortc::RTCDataChannelState_closing;
            case ::ortc::IDataChannelTypes::State_Closed:     return wrapper::org::ortc::RTCDataChannelState_closed;
          }
          ZS_THROW_INVALID_ARGUMENT("data channel state is unknown");
          return toWrapper(::ortc::IDataChannelTypes::State_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceCandidatePairState Helper::toWrapper(::ortc::IStatsReportTypes::StatsICECandidatePairStates state)
        {
          switch (state)
          {
            case ::ortc::IStatsReportTypes::StatsICECandidatePairState_Frozen:      return wrapper::org::ortc::RTCIceCandidatePairState_frozen;
            case ::ortc::IStatsReportTypes::StatsICECandidatePairState_Waiting:     return wrapper::org::ortc::RTCIceCandidatePairState_waiting;
            case ::ortc::IStatsReportTypes::StatsICECandidatePairState_InProgress:  return wrapper::org::ortc::RTCIceCandidatePairState_inProgress;
            case ::ortc::IStatsReportTypes::StatsICECandidatePairState_Failed:      return wrapper::org::ortc::RTCIceCandidatePairState_failed;
            case ::ortc::IStatsReportTypes::StatsICECandidatePairState_Succeeded:   return wrapper::org::ortc::RTCIceCandidatePairState_succeeded;
            case ::ortc::IStatsReportTypes::StatsICECandidatePairState_Cancelled:   return wrapper::org::ortc::RTCIceCandidatePairState_failed;
          }
          ZS_THROW_INVALID_ARGUMENT("ICE candidate pair state is unknown");
          return toWrapper(::ortc::IStatsReportTypes::StatsICECandidatePairState_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode Helper::toWrapper(::ortc::adapter::IPeerConnectionTypes::SignalingModes mode)
        {
          switch (mode)
          {
            case ::ortc::adapter::IPeerConnectionTypes::SignalingMode_JSON:         return wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode_json;
            case ::ortc::adapter::IPeerConnectionTypes::SignalingMode_SDP:          return wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode_sdp;
          }
          ZS_THROW_INVALID_ARGUMENT("signaling mode is unknown");
          return toWrapper(::ortc::adapter::IPeerConnectionTypes::SignalingMode_First);
        }

        //---------------------------------------------------------------------
        ::ortc::adapter::IPeerConnectionTypes::SignalingModes Helper::toNative(wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode mode)
        {
          switch (mode)
          {
            case wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode_json:         return ::ortc::adapter::IPeerConnectionTypes::SignalingMode_JSON;
            case wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode_sdp:          return ::ortc::adapter::IPeerConnectionTypes::SignalingMode_SDP;
          }
          ZS_THROW_INVALID_ARGUMENT("signaling mode is unknown");
          return ::ortc::adapter::IPeerConnectionTypes::SignalingMode_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::adapter::RTCRtcpMuxPolicy Helper::toWrapper(::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicies policy)
        {
          switch (policy)
          {
            case ::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicy_Negotiated:       return wrapper::org::ortc::adapter::RTCRtcpMuxPolicy_negotiated;
            case ::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicy_Require:          return wrapper::org::ortc::adapter::RTCRtcpMuxPolicy_require;
          }
          ZS_THROW_INVALID_ARGUMENT("mux policy mode is unknown");
          return toWrapper(::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicy_First);
        }

        //---------------------------------------------------------------------
        ::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicies Helper::toNative(wrapper::org::ortc::adapter::RTCRtcpMuxPolicy policy)
        {
          switch (policy)
          {
            case wrapper::org::ortc::adapter::RTCRtcpMuxPolicy_negotiated:       return ::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicy_Negotiated;
            case wrapper::org::ortc::adapter::RTCRtcpMuxPolicy_require:          return ::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicy_Require;
          }
          ZS_THROW_INVALID_ARGUMENT("mux policy is unknown");
          return ::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicy_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::adapter::RTCBundlePolicy Helper::toWrapper(::ortc::adapter::IPeerConnectionTypes::BundlePolicies policy)
        {
          switch (policy)
          {
            case ::ortc::adapter::IPeerConnectionTypes::BundlePolicy_Balanced:    return wrapper::org::ortc::adapter::RTCBundlePolicy_balanced;
            case ::ortc::adapter::IPeerConnectionTypes::BundlePolicy_MaxCompat:   return wrapper::org::ortc::adapter::RTCBundlePolicy_maxCompat;
            case ::ortc::adapter::IPeerConnectionTypes::BundlePolicy_MaxBundle:   return wrapper::org::ortc::adapter::RTCBundlePolicy_maxBundle;
          }
          ZS_THROW_INVALID_ARGUMENT("bundle policy mode is unknown");
          return toWrapper(::ortc::adapter::IPeerConnectionTypes::BundlePolicy_First);
        }

        //---------------------------------------------------------------------
        ::ortc::adapter::IPeerConnectionTypes::BundlePolicies Helper::toNative(wrapper::org::ortc::adapter::RTCBundlePolicy policy)
        {
          switch (policy)
          {
            case wrapper::org::ortc::adapter::RTCBundlePolicy_balanced:       return ::ortc::adapter::IPeerConnectionTypes::BundlePolicy_Balanced;
            case wrapper::org::ortc::adapter::RTCBundlePolicy_maxCompat:      return ::ortc::adapter::IPeerConnectionTypes::BundlePolicy_MaxCompat;
            case wrapper::org::ortc::adapter::RTCBundlePolicy_maxBundle:      return ::ortc::adapter::IPeerConnectionTypes::BundlePolicy_MaxBundle;
          }
          ZS_THROW_INVALID_ARGUMENT("bundle policy is unknown");
          return ::ortc::adapter::IPeerConnectionTypes::BundlePolicy_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::adapter::RTCSignalingState Helper::toWrapper(::ortc::adapter::IPeerConnectionTypes::SignalingStates state)
        {
          switch (state)
          {
            case ::ortc::adapter::IPeerConnectionTypes::SignalingState_Stable:              return wrapper::org::ortc::adapter::RTCSignalingState_stable;
            case ::ortc::adapter::IPeerConnectionTypes::SignalingState_HaveLocalOffer:      return wrapper::org::ortc::adapter::RTCSignalingState_haveLocalOffer;
            case ::ortc::adapter::IPeerConnectionTypes::SignalingState_HaveRemoteOffer:     return wrapper::org::ortc::adapter::RTCSignalingState_haveRemoteOffer;
            case ::ortc::adapter::IPeerConnectionTypes::SignalingState_HaveLocalPranswer:   return wrapper::org::ortc::adapter::RTCSignalingState_haveLocalPranswer;
            case ::ortc::adapter::IPeerConnectionTypes::SignalingState_HaveRemotePranswer:  return wrapper::org::ortc::adapter::RTCSignalingState_haveRemotePranswer;
            case ::ortc::adapter::IPeerConnectionTypes::SignalingState_Closed:              return wrapper::org::ortc::adapter::RTCSignalingState_closed;
           }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return toWrapper(::ortc::adapter::IPeerConnectionTypes::SignalingState_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::adapter::RTCIceConnectionState Helper::toPeerConnectionWrapper(::ortc::IICETransportTypes::States state)
        {
         switch (state)
          {
            case ::ortc::IICETransportTypes::State_New:           return wrapper::org::ortc::adapter::RTCIceConnectionState_new;
            case ::ortc::IICETransportTypes::State_Checking:      return wrapper::org::ortc::adapter::RTCIceConnectionState_checking;
            case ::ortc::IICETransportTypes::State_Connected:     return wrapper::org::ortc::adapter::RTCIceConnectionState_connected;
            case ::ortc::IICETransportTypes::State_Completed:     return wrapper::org::ortc::adapter::RTCIceConnectionState_completed;
            case ::ortc::IICETransportTypes::State_Disconnected:  return wrapper::org::ortc::adapter::RTCIceConnectionState_disconnected;
            case ::ortc::IICETransportTypes::State_Failed:        return wrapper::org::ortc::adapter::RTCIceConnectionState_failed;
            case ::ortc::IICETransportTypes::State_Closed:        return wrapper::org::ortc::adapter::RTCIceConnectionState_closed;
         }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return toPeerConnectionWrapper(::ortc::IICETransportTypes::State_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::adapter::RTCPeerConnectionState Helper::toWrapper(::ortc::adapter::IPeerConnectionTypes::PeerConnectionStates state)
        {
          switch (state)
          {
            case ::ortc::adapter::IPeerConnectionTypes::PeerConnectionState_New:          return wrapper::org::ortc::adapter::RTCPeerConnectionState_new;
            case ::ortc::adapter::IPeerConnectionTypes::PeerConnectionState_Connecting:   return wrapper::org::ortc::adapter::RTCPeerConnectionState_connecting;
            case ::ortc::adapter::IPeerConnectionTypes::PeerConnectionState_Connected:    return wrapper::org::ortc::adapter::RTCPeerConnectionState_connected;
            case ::ortc::adapter::IPeerConnectionTypes::PeerConnectionState_Disconnected: return wrapper::org::ortc::adapter::RTCPeerConnectionState_disconnected;
            case ::ortc::adapter::IPeerConnectionTypes::PeerConnectionState_Failed:       return wrapper::org::ortc::adapter::RTCPeerConnectionState_failed;
            case ::ortc::adapter::IPeerConnectionTypes::PeerConnectionState_Closed:       return wrapper::org::ortc::adapter::RTCPeerConnectionState_closed;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return toWrapper(::ortc::adapter::IPeerConnectionTypes::PeerConnectionState_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceGatherFilterPolicy Helper::toWrapper(::ortc::IICEGathererTypes::FilterPolicies policy)
        {
          return static_cast<wrapper::org::ortc::RTCIceGatherFilterPolicy>(static_cast<std::underlying_type<::ortc::IICEGathererTypes::FilterPolicies>::type>(policy));
        }

        //---------------------------------------------------------------------
        ::ortc::IICEGathererTypes::FilterPolicies Helper::toNative(wrapper::org::ortc::RTCIceGatherFilterPolicy policy)
        {
          return static_cast<::ortc::IICEGathererTypes::FilterPolicies>(static_cast<std::underlying_type<wrapper::org::ortc::RTCIceGatherFilterPolicy>::type>(policy));
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceCredentialType Helper::toWrapper(::ortc::IICEGathererTypes::CredentialTypes type)
        {
          switch (type)
          {
            case ::ortc::IICEGathererTypes::CredentialType_Password:  return wrapper::org::ortc::RTCIceCredentialType_password;
            case ::ortc::IICEGathererTypes::CredentialType_Token:     return wrapper::org::ortc::RTCIceCredentialType_token;
          }
          ZS_THROW_INVALID_ARGUMENT("credential type is unknown");
          return toWrapper(::ortc::IICEGathererTypes::CredentialType_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IICEGathererTypes::CredentialTypes Helper::toNative(wrapper::org::ortc::RTCIceCredentialType type)
        {
          switch (type)
          {
            case wrapper::org::ortc::RTCIceCredentialType_password:   return ::ortc::IICEGathererTypes::CredentialType_Password;
            case wrapper::org::ortc::RTCIceCredentialType_token:      return ::ortc::IICEGathererTypes::CredentialType_Token;
          }
          ZS_THROW_INVALID_ARGUMENT("credential type is unknown");
          return ::ortc::IICEGathererTypes::CredentialType_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceGathererState Helper::toWrapper(::ortc::IICEGathererTypes::States state)
        {
          switch (state)
          {
            case ::ortc::IICEGathererTypes::State_New:        return wrapper::org::ortc::RTCIceGathererState_new;
            case ::ortc::IICEGathererTypes::State_Gathering:  return wrapper::org::ortc::RTCIceGathererState_gathering;
            case ::ortc::IICEGathererTypes::State_Complete:   return wrapper::org::ortc::RTCIceGathererState_complete;
            case ::ortc::IICEGathererTypes::State_Closed:     return wrapper::org::ortc::RTCIceGathererState_closed;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return toWrapper(::ortc::IICEGathererTypes::State_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IICEGathererTypes::States Helper::toNative(wrapper::org::ortc::RTCIceGathererState state)
        {
          switch (state)
          {
            case wrapper::org::ortc::RTCIceGathererState_new:         return ::ortc::IICEGathererTypes::State_New;
            case wrapper::org::ortc::RTCIceGathererState_gathering:   return ::ortc::IICEGathererTypes::State_Gathering;
            case wrapper::org::ortc::RTCIceGathererState_complete:    return ::ortc::IICEGathererTypes::State_Complete;
            case wrapper::org::ortc::RTCIceGathererState_closed:      return ::ortc::IICEGathererTypes::State_Closed;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return ::ortc::IICEGathererTypes::State_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceCandidateType Helper::toWrapper(::ortc::IICETypes::CandidateTypes type)
        {
          switch (type)
          {
            case ::ortc::IICETypes::CandidateType_Host:   return wrapper::org::ortc::RTCIceCandidateType_host;
            case ::ortc::IICETypes::CandidateType_Srflex:   return wrapper::org::ortc::RTCIceCandidateType_srflex;
            case ::ortc::IICETypes::CandidateType_Prflx:    return wrapper::org::ortc::RTCIceCandidateType_prflx;
            case ::ortc::IICETypes::CandidateType_Relay:    return wrapper::org::ortc::RTCIceCandidateType_relay;
          }
          ZS_THROW_INVALID_ARGUMENT("ice candidate type is unknown");
          return toWrapper(::ortc::IICETypes::CandidateType_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceComponent Helper::toWrapper(::ortc::IICETypes::Components component)
        {
          switch (component)
          {
            case ::ortc::IICETypes::Component_RTP:    return wrapper::org::ortc::RTCIceComponent_rtp;
            case ::ortc::IICETypes::Component_RTCP:   return wrapper::org::ortc::RTCIceComponent_rtcp;
          }
          ZS_THROW_INVALID_ARGUMENT("component is unknown");
          return toWrapper(::ortc::IICETypes::Component_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceProtocol Helper::toWrapper(::ortc::IICETypes::Protocols protocol)
        {
          switch (protocol)
          {
            case ::ortc::IICETypes::Protocol_UDP:  return wrapper::org::ortc::RTCIceProtocol_udp;
            case ::ortc::IICETypes::Protocol_TCP:  return wrapper::org::ortc::RTCIceProtocol_tcp;
           }
          ZS_THROW_INVALID_ARGUMENT("protocol is unknown");
          return toWrapper(::ortc::IICETypes::Protocol_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceTcpCandidateType Helper::toWrapper(::ortc::IICETypes::TCPCandidateTypes type)
        {
          switch (type)
          {
            case ::ortc::IICETypes::TCPCandidateType_Active:  return wrapper::org::ortc::RTCIceTcpCandidateType_active;
            case ::ortc::IICETypes::TCPCandidateType_Passive: return wrapper::org::ortc::RTCIceTcpCandidateType_passive;
            case ::ortc::IICETypes::TCPCandidateType_SO:      return wrapper::org::ortc::RTCIceTcpCandidateType_so;
          }
          ZS_THROW_INVALID_ARGUMENT("tcp candidate type is unknown");
          return toWrapper(::ortc::IICETypes::TCPCandidateType_First);
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceTransportState Helper::toWrapper(::ortc::IICETransportTypes::States state)
        {
          switch (state)
          {
            case ::ortc::IICETransportTypes::State_New:           return wrapper::org::ortc::RTCIceTransportState_new;
            case ::ortc::IICETransportTypes::State_Checking:      return wrapper::org::ortc::RTCIceTransportState_checking;
            case ::ortc::IICETransportTypes::State_Connected:     return wrapper::org::ortc::RTCIceTransportState_connected;
            case ::ortc::IICETransportTypes::State_Completed:     return wrapper::org::ortc::RTCIceTransportState_completed;
            case ::ortc::IICETransportTypes::State_Disconnected:  return wrapper::org::ortc::RTCIceTransportState_disconnected;
            case ::ortc::IICETransportTypes::State_Failed:        return wrapper::org::ortc::RTCIceTransportState_failed;
            case ::ortc::IICETransportTypes::State_Closed:        return wrapper::org::ortc::RTCIceTransportState_closed;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return toWrapper(::ortc::IICETransportTypes::State_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IICETransportTypes::States Helper::toNative(wrapper::org::ortc::RTCIceTransportState state)
        {
          switch (state)
          {
            case wrapper::org::ortc::RTCIceTransportState_new:          return ::ortc::IICETransportTypes::State_New;
            case wrapper::org::ortc::RTCIceTransportState_checking:     return ::ortc::IICETransportTypes::State_Checking;
            case wrapper::org::ortc::RTCIceTransportState_connected:    return ::ortc::IICETransportTypes::State_Connected;
            case wrapper::org::ortc::RTCIceTransportState_completed:    return ::ortc::IICETransportTypes::State_Completed;
            case wrapper::org::ortc::RTCIceTransportState_disconnected: return ::ortc::IICETransportTypes::State_Disconnected;
            case wrapper::org::ortc::RTCIceTransportState_failed:       return ::ortc::IICETransportTypes::State_Failed;
            case wrapper::org::ortc::RTCIceTransportState_closed:       return ::ortc::IICETransportTypes::State_Closed;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return ::ortc::IICETransportTypes::State_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCIceRole Helper::toWrapper(::ortc::IICETypes::Roles role)
        {
          switch (role)
          {
            case ::ortc::IICETypes::Role_Controlling:           return wrapper::org::ortc::RTCIceRole_controlling;
            case ::ortc::IICETypes::Role_Controlled:            return wrapper::org::ortc::RTCIceRole_controlled;
          }
          ZS_THROW_INVALID_ARGUMENT("role is unknown");
          return toWrapper(::ortc::IICETypes::Role_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IICETypes::Roles Helper::toNative(wrapper::org::ortc::RTCIceRole role)
        {
          switch (role)
          {
            case wrapper::org::ortc::RTCIceRole_controlling:    return ::ortc::IICETypes::Role_Controlling;
            case wrapper::org::ortc::RTCIceRole_controlled:     return ::ortc::IICETypes::Role_Controlled;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return ::ortc::IICETypes::Role_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal Helper::toWrapper(::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signals signal)
        {
          switch (signal)
          {
            case ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signal_Auto:  return wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal_auto;
            case ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signal_Music:  return wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal_music;
            case ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signal_Voice:  return wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal_voice;
          }
          ZS_THROW_INVALID_ARGUMENT("signal type is unknown");
          return toWrapper(::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signal_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signals Helper::toNative(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal signal)
        {
          switch (signal)
          {
            case wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal_auto:  return ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signal_Auto;
            case wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal_music: return ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signal_Music;
            case wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal_voice: return ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signal_Voice;
          }
          ZS_THROW_INVALID_ARGUMENT("signal type is unknown");
          return ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signal_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication Helper::toWrapper(::ortc::IRTPTypes::OpusCodecCapabilityOptions::Applications application)
        {
          switch (application)
          {
            case ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Application_VoIP:       return wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication_voip;
            case ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Application_Audio:      return wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication_audio;
            case ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Application_LowDelay:   return wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication_lowDelay;
          }
          ZS_THROW_INVALID_ARGUMENT("application type is unknown");
          return toWrapper(::ortc::IRTPTypes::OpusCodecCapabilityOptions::Application_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Applications Helper::toNative(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication application)
        {
          switch (application)
          {
            case wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication_voip:      return ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Application_VoIP;
            case wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication_audio:     return ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Application_Audio;
            case wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication_lowDelay:  return ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Application_LowDelay;
          }
          ZS_THROW_INVALID_ARGUMENT("application type is unknown");
          return ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Application_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP Helper::toWrapper(::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToPs top)
        {
          switch (top)
          {
            case ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_1DInterleavedFEC:     return wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP_n1DInterleavedFEC;
            case ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_1DNonInterleavedFEC:  return wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP_n1DNonInterleavedFEC;
            case ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_2DParityFEEC:         return wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP_n2DParityFEEC;
            case ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_Reserved:             return wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP_reserved;
          }
          ZS_THROW_INVALID_ARGUMENT("ToP is unknown");
          return toWrapper(::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToPs Helper::toNative(wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP top)
        {
          switch (top)
          {
            case wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP_n1DInterleavedFEC:     return ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_1DInterleavedFEC;
            case wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP_n1DNonInterleavedFEC:  return ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_1DNonInterleavedFEC;
            case wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP_n2DParityFEEC:         return ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_2DParityFEEC;
            case wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP_reserved:              return ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_Reserved;
          }
          ZS_THROW_INVALID_ARGUMENT("ToP is unknown");
          return ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToP_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCRtpDegradationPreference Helper::toWrapper(::ortc::IRTPTypes::DegradationPreferences preference)
        {
          switch (preference)
          {
            case ::ortc::IRTPTypes::DegradationPreference_MaintainFramerate:    return wrapper::org::ortc::RTCRtpDegradationPreference_maintainFramerate;
            case ::ortc::IRTPTypes::DegradationPreference_MaintainResolution:   return wrapper::org::ortc::RTCRtpDegradationPreference_maintainResolution;
            case ::ortc::IRTPTypes::DegradationPreference_Balanced:             return wrapper::org::ortc::RTCRtpDegradationPreference_balanced;
          }
          ZS_THROW_INVALID_ARGUMENT("preference is unknown");
          return toWrapper(::ortc::IRTPTypes::DegradationPreference_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IRTPTypes::DegradationPreferences Helper::toNative(wrapper::org::ortc::RTCRtpDegradationPreference preference)
        {
          switch (preference)
          {
            case wrapper::org::ortc::RTCRtpDegradationPreference_maintainFramerate:     return ::ortc::IRTPTypes::DegradationPreference_MaintainFramerate;
            case wrapper::org::ortc::RTCRtpDegradationPreference_maintainResolution:    return ::ortc::IRTPTypes::DegradationPreference_MaintainResolution;
            case wrapper::org::ortc::RTCRtpDegradationPreference_balanced:              return ::ortc::IRTPTypes::DegradationPreference_Balanced;
          }
          ZS_THROW_INVALID_ARGUMENT("preference is unknown");
          return ::ortc::IRTPTypes::DegradationPreference_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCRtpPriorityType Helper::toWrapper(::ortc::IRTPTypes::PriorityTypes type)
        {
          switch (type)
          {
            case ::ortc::IRTPTypes::PriorityType_VeryLow:     return wrapper::org::ortc::RTCRtpPriorityType_veryLow;
            case ::ortc::IRTPTypes::PriorityType_Low:         return wrapper::org::ortc::RTCRtpPriorityType_low;
            case ::ortc::IRTPTypes::PriorityType_Medium:      return wrapper::org::ortc::RTCRtpPriorityType_medium;
            case ::ortc::IRTPTypes::PriorityType_High:        return wrapper::org::ortc::RTCRtpPriorityType_high;
          }
          ZS_THROW_INVALID_ARGUMENT("type is unknown");
          return toWrapper(::ortc::IRTPTypes::PriorityType_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IRTPTypes::PriorityTypes Helper::toNative(wrapper::org::ortc::RTCRtpPriorityType type)
        {
          switch (type)
          {
            case wrapper::org::ortc::RTCRtpPriorityType_veryLow:    return ::ortc::IRTPTypes::PriorityType_VeryLow;
            case wrapper::org::ortc::RTCRtpPriorityType_low:        return ::ortc::IRTPTypes::PriorityType_Low;
            case wrapper::org::ortc::RTCRtpPriorityType_medium:     return ::ortc::IRTPTypes::PriorityType_Medium;
            case wrapper::org::ortc::RTCRtpPriorityType_high:       return ::ortc::IRTPTypes::PriorityType_High;
           }
          ZS_THROW_INVALID_ARGUMENT("type is unknown");
          return ::ortc::IRTPTypes::PriorityType_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType Helper::toWrapper(::ortc::adapter::ISessionDescriptionTypes::SignalingTypes type)
        {
          switch (type)
          {
            case ::ortc::adapter::ISessionDescriptionTypes::SignalingType_JSON:         return wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_json;
            case ::ortc::adapter::ISessionDescriptionTypes::SignalingType_SDPOffer:     return wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_sdpOffer;
            case ::ortc::adapter::ISessionDescriptionTypes::SignalingType_SDPPranswer:  return wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_sdpPranswer;
            case ::ortc::adapter::ISessionDescriptionTypes::SignalingType_SDPAnswer:    return wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_sdpAnswer;
            case ::ortc::adapter::ISessionDescriptionTypes::SignalingType_SDPRollback:  return wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_sdpRollback;
          }
          ZS_THROW_INVALID_ARGUMENT("type is unknown");
          return toWrapper(::ortc::adapter::ISessionDescriptionTypes::SignalingType_First);
        }

        //---------------------------------------------------------------------
        ::ortc::adapter::ISessionDescriptionTypes::SignalingTypes Helper::toNative(wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type)
        {
          switch (type)
          {
            case wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_json:        return ::ortc::adapter::ISessionDescriptionTypes::SignalingType_JSON;
            case wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_sdpOffer:    return ::ortc::adapter::ISessionDescriptionTypes::SignalingType_SDPOffer;
            case wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_sdpPranswer: return ::ortc::adapter::ISessionDescriptionTypes::SignalingType_SDPPranswer;
            case wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_sdpAnswer:   return ::ortc::adapter::ISessionDescriptionTypes::SignalingType_SDPAnswer;
            case wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType_sdpRollback: return ::ortc::adapter::ISessionDescriptionTypes::SignalingType_SDPRollback;
          }
          ZS_THROW_INVALID_ARGUMENT("type is unknown");
          return ::ortc::adapter::ISessionDescriptionTypes::SignalingType_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCDtlsRole Helper::toWrapper(::ortc::IDTLSTransportTypes::Roles role)
        {  
          switch (role)
          {
            case ::ortc::IDTLSTransportTypes::Role_Auto:         return wrapper::org::ortc::RTCDtlsRole_auto;
            case ::ortc::IDTLSTransportTypes::Role_Client:       return wrapper::org::ortc::RTCDtlsRole_client;
            case ::ortc::IDTLSTransportTypes::Role_Server:       return wrapper::org::ortc::RTCDtlsRole_server;
          }
          ZS_THROW_INVALID_ARGUMENT("role is unknown");
          return toWrapper(::ortc::IDTLSTransportTypes::Role_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IDTLSTransportTypes::Roles Helper::toNative(wrapper::org::ortc::RTCDtlsRole role)
        {
          switch (role)
          {
            case wrapper::org::ortc::RTCDtlsRole_auto:        return ::ortc::IDTLSTransportTypes::Role_Auto;
            case wrapper::org::ortc::RTCDtlsRole_client:      return ::ortc::IDTLSTransportTypes::Role_Client;
            case wrapper::org::ortc::RTCDtlsRole_server:      return ::ortc::IDTLSTransportTypes::Role_Server;
          }
          ZS_THROW_INVALID_ARGUMENT("role is unknown");
          return ::ortc::IDTLSTransportTypes::Role_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::RTCDtlsTransportState Helper::toWrapper(::ortc::IDTLSTransportTypes::States state)
        {
          switch (state)
          {
            case ::ortc::IDTLSTransportTypes::State_New:          return wrapper::org::ortc::RTCDtlsTransportState_new;
            case ::ortc::IDTLSTransportTypes::State_Connecting:   return wrapper::org::ortc::RTCDtlsTransportState_connecting;
            case ::ortc::IDTLSTransportTypes::State_Connected:    return wrapper::org::ortc::RTCDtlsTransportState_connected;
            case ::ortc::IDTLSTransportTypes::State_Closed:       return wrapper::org::ortc::RTCDtlsTransportState_closed;
            case ::ortc::IDTLSTransportTypes::State_Failed:       return wrapper::org::ortc::RTCDtlsTransportState_failed;
          }
          ZS_THROW_INVALID_ARGUMENT("state is unknown");
          return toWrapper(::ortc::IDTLSTransportTypes::State_First);
        }

        //---------------------------------------------------------------------
        ::ortc::IDTLSTransportTypes::States Helper::toNative(wrapper::org::ortc::RTCDtlsTransportState state)
        {
          switch (state)
          {
            case wrapper::org::ortc::RTCDtlsTransportState_new:         return ::ortc::IDTLSTransportTypes::State_New;
            case wrapper::org::ortc::RTCDtlsTransportState_connecting:  return ::ortc::IDTLSTransportTypes::State_Connecting;
            case wrapper::org::ortc::RTCDtlsTransportState_connected:   return ::ortc::IDTLSTransportTypes::State_Connected;
            case wrapper::org::ortc::RTCDtlsTransportState_closed:      return ::ortc::IDTLSTransportTypes::State_Closed;
            case wrapper::org::ortc::RTCDtlsTransportState_failed:      return ::ortc::IDTLSTransportTypes::State_Failed;
          }
          ZS_THROW_INVALID_ARGUMENT("role is unknown");
          return ::ortc::IDTLSTransportTypes::State_First;
        }

        //---------------------------------------------------------------------
        wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection Helper::toWrapper(::ortc::adapter::ISessionDescriptionTypes::MediaDirections direction)
        {
          switch (direction)
          {
            case ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_SendReceive:   return wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection_sendReceive;
            case ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_SendOnly:      return wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection_sendOnly;
            case ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_ReceiveOnly:   return wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection_receiveOnly;
            case ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_Inactive:      return wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection_inactive;
          }
          ZS_THROW_INVALID_ARGUMENT("direction is unknown");
          return toWrapper(::ortc::adapter::ISessionDescriptionTypes::MediaDirection_First);
        }

        //---------------------------------------------------------------------
        ::ortc::adapter::ISessionDescriptionTypes::MediaDirections Helper::toNative(wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection direction)
        {
          switch (direction)
          {
            case wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection_sendReceive:     return ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_SendReceive;
            case wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection_sendOnly:        return ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_SendOnly;
            case wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection_receiveOnly:     return ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_ReceiveOnly;
            case wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection_inactive:        return ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_Inactive;
          }
          ZS_THROW_INVALID_ARGUMENT("direction is unknown");
          return ::ortc::adapter::ISessionDescriptionTypes::MediaDirection_First;
        }

        //---------------------------------------------------------------------
        Helper::WrapperMediaStreamTrackListPtr Helper::toWrapper(MediaStreamTrackListPtr tracks)
        {
          auto result = make_shared<WrapperMediaStreamTrackList>();
          if (!tracks) return result;
          for (auto iter = tracks->begin(); iter != tracks->end(); ++iter)
          {
            auto native = wrapper::impl::org::ortc::MediaStreamTrack::toWrapper(*iter);
            if (!native) continue;
            result->push_back(native);
          }
          return result;
        }

        //---------------------------------------------------------------------
        Helper::MediaStreamTrackListPtr Helper::toNative(WrapperMediaStreamTrackListPtr tracks)
        {
          if (!tracks) return MediaStreamTrackListPtr();

          MediaStreamTrackListPtr result = make_shared<MediaStreamTrackList>();
          for (auto iter = tracks->begin(); iter != tracks->end(); ++iter)
          {
            auto native = wrapper::impl::org::ortc::MediaStreamTrack::toNative(*iter);
            if (!native) continue;
            result->push_back(native);
          }
          return result;
        }

        //---------------------------------------------------------------------
        PromisePtr Helper::toWrapper(PromisePtr promise)
        {
          if (!promise) return promise;

          auto result = Promise::create(getGuiQueue());

          promise->thenClosure([promise, result] {
            if (promise->isRejected()) {
              reject(promise, result);
              return;
            }

            result->resolve();
          });
          promise->background();
          return result;
        }

        //---------------------------------------------------------------------
        void Helper::reject(
                            PromisePtr nativePromise,
                            PromisePtr wrapperPromise
                            )
        {
          {
            auto reason = nativePromise->reason<::ortc::ErrorAny>();
            if (reason) {
              auto wrapper = Error::toWrapper(reason);
              auto holder = make_shared< AnyHolder<ErrorPtr> >();
              holder->value_ = wrapper;
              wrapperPromise->reject(holder);
              return;
            }
          }
          {
            auto reason = nativePromise->reason<::ortc::IMediaStreamTrackTypes::OverconstrainedError>();
            if (reason) {
              auto wrapper = OverconstrainedError::toWrapper(reason);
              auto holder = make_shared< AnyHolder<OverconstrainedErrorPtr> >();
              holder->value_ = wrapper;
              wrapperPromise->reject(holder);
              return;
            }
          }
          wrapperPromise->reject();
          return;
        }

        //---------------------------------------------------------------------
        Helper::PromiseWithStatsReportPtr Helper::getStats(IStatsProviderPtr native, wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
        {
          ZS_THROW_INVALID_ARGUMENT_IF(!native);
          ZS_THROW_INVALID_ARGUMENT_IF(!statTypes);

          auto value = wrapper::impl::org::ortc::RTCStatsTypeSet::toNative(statTypes);
          ZS_THROW_INVALID_ARGUMENT_IF(!value);

          auto promise = native->getStats(*value);
          if (!promise) return PromiseWithStatsReportPtr();

          auto result = PromiseWithStatsReport::create();

          promise->thenClosure([result, promise]()
          {
            if (promise->isRejected())
            {
              result->reject();
              return;
            }
            result->resolve(wrapper::impl::org::ortc::RTCStatsReport::toWrapper(promise->value()));
          });
          promise->background();
          return result;
        }

      } // namespace ortc
    } // namespace org
  } // namespace impl
} // namespace wrapper
