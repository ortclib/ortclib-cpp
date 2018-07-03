
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpEncodingParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpEncodingParameters : public wrapper::org::webRtc::RTCRtpEncodingParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpEncodingParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpEncodingParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpEncodingParameters, NativeType);

          RTCRtpEncodingParametersWeakPtr thisWeak_;

          RTCRtpEncodingParameters() noexcept;
          virtual ~RTCRtpEncodingParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtpEncodingParameters() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

