
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpDecodingParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpDecodingParameters : public wrapper::org::webRtc::RTCRtpDecodingParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpDecodingParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpDecodingParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpEncodingParameters, NativeType);

          RTCRtpDecodingParametersWeakPtr thisWeak_;

          RTCRtpDecodingParameters() noexcept;
          virtual ~RTCRtpDecodingParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtpDecodingParameters() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

