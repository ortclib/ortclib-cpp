/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#define ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

#include <ortc/internal/types.h>
#include <ortc/internal/ortc.h>

#include <ortc/adapter/internal/ortc_adapter_adapter.h>

#include <ortc/internal/ortc.events.h>
#include <zsLib/Log.h>

namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_webrtc) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_dtls_transport) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_ice_gatherer) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_ice_gatherer_router) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_ice_transport) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_ice_transport_controller) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_media_devices) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_media_engine) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_media_stream_track) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_rtp_rtcp_packet) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_rtp_listener) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_rtp_receiver) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_rtp_sender) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_rtp_types) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_sctp_data_channel) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_srtp) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(org_ortc_stats) }

ZS_EVENTING_EXCLUSIVE(OrtcLib);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_webrtc, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_dtlstransport, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_ice_gatherer, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_ice_gatherer_router, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_ice_transport, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_ice_transport_controller, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_media_devices, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_media_engine, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_media_stream_track, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_rtp_rtcp_packet, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_rtp_listener, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_rtp_receiver, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_rtp_sender, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_rtp_types, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_sctp_data_channel, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(org_ortc_srtp, Debug);
ZS_EVENTING_EXCLUSIVE(x);

ZS_EVENTING_EXCLUSIVE(OrtcLibStatsReport);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_stats, Debug);
ZS_EVENTING_EXCLUSIVE(x);

namespace ortc
{
  namespace internal
  {
    void initSubsystems()
    {
      ZS_GET_SUBSYSTEM_LOG_LEVEL(ZS_GET_OTHER_SUBSYSTEM(ortc, org_ortc));
      ZS_GET_SUBSYSTEM_LOG_LEVEL(ZS_GET_OTHER_SUBSYSTEM(ortc, org_ortc_webrtc));
    }
  }
}

ZS_DECLARE_TEAR_AWAY_IMPLEMENT(ortc::IDataChannel, ortc::internal::DataChannel::TearAwayData)
ZS_DECLARE_TEAR_AWAY_IMPLEMENT(ortc::IRTPListener, ortc::internal::RTPListener::TearAwayData)
ZS_DECLARE_TEAR_AWAY_IMPLEMENT(ortc::ISCTPTransport, ortc::internal::SCTPTransport::TearAwayData)

ZS_DECLARE_PROXY_IMPLEMENT(ortc::IDataChannelDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IDTLSTransportDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IDTMFSenderDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IICEGathererDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IICETransportDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IMediaDevicesDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IMediaStreamTrackDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IRTPListenerDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IRTPReceiverDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::IRTPSenderDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::ISCTPTransportDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::ISCTPTransportListenerDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::ISRTPSDESTransportDelegate)

ZS_DECLARE_PROXY_IMPLEMENT(ortc::adapter::IPeerConnectionDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::adapter::IMediaStreamDelegate)

ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IDataChannelAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IDTLSTransportAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IGathererAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IICETransportAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::ITransportControllerAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IMediaDeviceCaptureDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IMediaDeviceRenderDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IMediaDevicesAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IMediaStreamTrackAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IRTPDecoderDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IRTPDecoderAysncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IRTPEncoderAysncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IRTPEncoderDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IRTPListenerAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IRTPReceiverChannelAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IRTPSenderAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::IRTPSenderChannelAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::ISCTPTransportForDataChannelDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::ISecureTransportDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::ISRTPSDESTransportAsyncDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::ISRTPTransportDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(ortc::internal::ISCTPTransportAsyncDelegate)

ZS_DECLARE_PROXY_IMPLEMENT(ortc::adapter::internal::IPeerConnectionAsyncDelegate)

ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IDTLSTransportDelegate, ortc::IDTLSTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IDTMFSenderDelegate, ortc::IDTMFSenderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IDataChannelDelegate, ortc::IDataChannelSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IICEGathererDelegate, ortc::IICEGathererSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IICETransportDelegate, ortc::IICETransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IMediaDevicesDelegate, ortc::IMediaDevicesSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IMediaStreamTrackDelegate, ortc::IMediaStreamTrackSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IMediaElementDelegate, ortc::IMediaElementSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IMediaStreamTrackAsyncMediaDelegate, ortc::IMediaStreamTrackMediaSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IMediaStreamTrackSyncMediaDelegate, ortc::IMediaStreamTrackMediaSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IMediaStreamTrackSelectorDelegate, ortc::IMediaStreamTrackSelectorSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IRTPListenerDelegate, ortc::IRTPListenerSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IRTPReceiverDelegate, ortc::IRTPReceiverSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::IRTPSenderDelegate, ortc::IRTPSenderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::ISCTPTransportDelegate, ortc::ISCTPTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::ISCTPTransportListenerDelegate, ortc::ISCTPTransportListenerSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::ISRTPSDESTransportDelegate, ortc::ISRTPSDESTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::adapter::IMediaStreamDelegate, ortc::adapter::IMediaStreamSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::adapter::IPeerConnectionDelegate, ortc::adapter::IPeerConnectionSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::IMediaDeviceCaptureDelegate, ortc::internal::IMediaDeviceCaptureSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::IMediaDeviceRenderDelegate, ortc::internal::IMediaDeviceRenderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::IMediaStreamTrackRTPDelegate, ortc::internal::IMediaStreamTrackRTPSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::IMediaStreamTrackMonitorDelegate, ortc::internal::IMediaStreamTrackMonitorSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::IRTPDecoderDelegate, ortc::internal::IRTPDecoderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::IRTPEncoderDelegate, ortc::internal::IRTPEncoderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::ISRTPTransportDelegate, ortc::internal::ISRTPTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::ISecureTransportDelegate, ortc::internal::ISecureTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_IMPLEMENT(ortc::internal::ISCTPTransportForDataChannelDelegate, ortc::internal::ISCTPTransportForDataChannelSubscription)


