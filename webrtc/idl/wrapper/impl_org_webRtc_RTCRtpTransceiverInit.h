
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpTransceiverInit.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpTransceiverInit : public wrapper::org::webRtc::RTCRtpTransceiverInit
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpTransceiverInit, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiverInit, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpTransceiverInit, NativeType);

          RTCRtpTransceiverInitWeakPtr thisWeak_;

          RTCRtpTransceiverInit() noexcept;
          virtual ~RTCRtpTransceiverInit() noexcept;
          void wrapper_init_org_webRtc_RTCRtpTransceiverInit() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

