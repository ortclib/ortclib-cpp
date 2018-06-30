// Generated by zsLibEventingTool

#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCPeerConnectionIceEvent.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCPeerConnectionIceEvent : public wrapper::org::webRtc::RTCPeerConnectionIceEvent
        {
          RTCPeerConnectionIceEventWeakPtr thisWeak_;

          RTCPeerConnectionIceEvent() noexcept;
          virtual ~RTCPeerConnectionIceEvent() noexcept;
          void wrapper_init_org_webRtc_RTCPeerConnectionIceEvent() noexcept override;

          // properties RTCPeerConnectionIceEvent
          wrapper::org::webRtc::RTCIceCandidatePtr get_candidate() noexcept override;
          String get_url() noexcept override;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

