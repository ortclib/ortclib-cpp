
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpReceiveParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpReceiveParameters : public wrapper::org::webRtc::RTCRtpReceiveParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpReceiveParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiveParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpParameters, NativeType);

          RTCRtpReceiveParametersWeakPtr thisWeak_;

          RTCRtpReceiveParameters() noexcept;
          virtual ~RTCRtpReceiveParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtpReceiveParameters() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

