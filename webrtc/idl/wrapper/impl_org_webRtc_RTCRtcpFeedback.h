
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtcpFeedback.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtcpFeedback : public wrapper::org::webRtc::RTCRtcpFeedback
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtcpFeedback, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtcpFeedback, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtcpFeedback, NativeType);

          RTCRtcpFeedbackWeakPtr thisWeak_;

          RTCRtcpFeedback() noexcept;
          virtual ~RTCRtcpFeedback() noexcept;
          void wrapper_init_org_webRtc_RTCRtcpFeedback() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

