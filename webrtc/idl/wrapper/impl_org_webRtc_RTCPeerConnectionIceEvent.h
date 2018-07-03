
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCPeerConnectionIceEvent.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCPeerConnectionIceEvent : public wrapper::org::webRtc::RTCPeerConnectionIceEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCPeerConnectionIceEvent, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent, WrapperImplType);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidate, UseIceCandidate);

          UseIceCandidatePtr candidate_;
          RTCPeerConnectionIceEventWeakPtr thisWeak_;

          RTCPeerConnectionIceEvent() noexcept;
          virtual ~RTCPeerConnectionIceEvent() noexcept;
          void wrapper_init_org_webRtc_RTCPeerConnectionIceEvent() noexcept override;

          // properties RTCPeerConnectionIceEvent
          wrapper::org::webRtc::RTCIceCandidatePtr get_candidate() noexcept override;
          String get_url() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(UseIceCandidatePtr value) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

