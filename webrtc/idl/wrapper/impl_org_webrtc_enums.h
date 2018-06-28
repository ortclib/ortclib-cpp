
#pragma once

#include "types.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/peerconnectioninterface.h"
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
          static wrapper::org::webrtc::RTCErrorType toWrapper(::webrtc::RTCErrorType value) noexcept;

          static wrapper::org::webrtc::RTCIceTransportPolicy toWrapper(::webrtc::PeerConnectionInterface::IceTransportsType value) noexcept;
          static ::webrtc::PeerConnectionInterface::IceTransportsType toNative(wrapper::org::webrtc::RTCIceTransportPolicy value) noexcept;

          static wrapper::org::webrtc::RTCBundlePolicy toWrapper(::webrtc::PeerConnectionInterface::BundlePolicy value) noexcept;
          static ::webrtc::PeerConnectionInterface::BundlePolicy toNative(wrapper::org::webrtc::RTCBundlePolicy value) noexcept;

          static wrapper::org::webrtc::RTCRtcpMuxPolicy toWrapper(::webrtc::PeerConnectionInterface::RtcpMuxPolicy value) noexcept;
          static ::webrtc::PeerConnectionInterface::RtcpMuxPolicy toNative(wrapper::org::webrtc::RTCRtcpMuxPolicy value) noexcept;

          static wrapper::org::webrtc::RTCTcpCandidatePolicy toWrapper(::webrtc::PeerConnectionInterface::TcpCandidatePolicy value) noexcept;
          static ::webrtc::PeerConnectionInterface::TcpCandidatePolicy toNative(wrapper::org::webrtc::RTCTcpCandidatePolicy value) noexcept;

          static wrapper::org::webrtc::RTCCandidateNetworkPolicy toWrapper(::webrtc::PeerConnectionInterface::CandidateNetworkPolicy value) noexcept;
          static ::webrtc::PeerConnectionInterface::CandidateNetworkPolicy toNative(wrapper::org::webrtc::RTCCandidateNetworkPolicy value) noexcept;

          static wrapper::org::webrtc::RTCContinualGatheringPolicy toWrapper(::webrtc::PeerConnectionInterface::ContinualGatheringPolicy value) noexcept;
          static ::webrtc::PeerConnectionInterface::ContinualGatheringPolicy toNative(wrapper::org::webrtc::RTCContinualGatheringPolicy value) noexcept;

          static wrapper::org::webrtc::RTCAdapterType toWrapper(::rtc::AdapterType value) noexcept;
          static ::rtc::AdapterType toNative(wrapper::org::webrtc::RTCAdapterType value) noexcept;

          static wrapper::org::webrtc::RTCSdpSemantics toWrapper(::webrtc::SdpSemantics value) noexcept;
          static ::webrtc::SdpSemantics toNative(wrapper::org::webrtc::RTCSdpSemantics value) noexcept;

          static wrapper::org::webrtc::RTCTlsCertPolicy toWrapper(::webrtc::PeerConnectionInterface::TlsCertPolicy value) noexcept;
          static ::webrtc::PeerConnectionInterface::TlsCertPolicy toNative(wrapper::org::webrtc::RTCTlsCertPolicy value) noexcept;


        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

