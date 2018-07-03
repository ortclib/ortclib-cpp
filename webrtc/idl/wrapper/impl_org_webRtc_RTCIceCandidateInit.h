
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCIceCandidateInit.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCIceCandidateInit : public wrapper::org::webRtc::RTCIceCandidateInit
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCIceCandidateInit, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidateInit, WrapperImplType);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidate, UseIceCandidate);

          RTCIceCandidateInitWeakPtr thisWeak_;

          RTCIceCandidateInit() noexcept;
          virtual ~RTCIceCandidateInit() noexcept;
          void wrapper_init_org_webRtc_RTCIceCandidateInit() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const UseIceCandidate &candidate) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(UseIceCandidatePtr candidate) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

