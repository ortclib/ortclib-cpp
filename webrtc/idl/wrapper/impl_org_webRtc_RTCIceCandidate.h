// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCIceCandidate.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCIceCandidate : public wrapper::org::webRtc::RTCIceCandidate
        {
          RTCIceCandidateWeakPtr thisWeak_;

          RTCIceCandidate() noexcept;
          virtual ~RTCIceCandidate() noexcept;

          // methods RTCIceCandidate
          void wrapper_init_org_webRtc_RTCIceCandidate() noexcept override;
          void wrapper_init_org_webRtc_RTCIceCandidate(wrapper::org::webRtc::RTCIceCandidateInitPtr init) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          wrapper::org::webRtc::RTCIceCandidateInitPtr toJson() noexcept override;

          // properties RTCIceCandidate
          String get_candidate() noexcept override;
          String get_sdpMid() noexcept override;
          Optional< unsigned short > get_sdpMLineIndex() noexcept override;
          String get_foundation() noexcept override;
          Optional< wrapper::org::webRtc::RTCIceComponent > get_component() noexcept override;
          Optional< unsigned long > get_priority() noexcept override;
          String get_ip() noexcept override;
          Optional< wrapper::org::webRtc::RTCIceProtocol > get_protocol() noexcept override;
          Optional< uint16_t > get_port() noexcept override;
          Optional< wrapper::org::webRtc::RTCIceCandidateType > get_type() noexcept override;
          Optional< wrapper::org::webRtc::RTCIceTcpCandidateType > get_tcpType() noexcept override;
          String get_relatedAddress() noexcept override;
          Optional< uint16_t > get_relatedPort() noexcept override;
          String get_usernameFragment() noexcept override;
          void set_usernameFragment(String value) noexcept override;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper
