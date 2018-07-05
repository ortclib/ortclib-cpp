
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpParameters : public wrapper::org::webRtc::RTCRtpParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpParameters, NativeType);

          RTCRtpParametersWeakPtr thisWeak_;

          RTCRtpParameters() noexcept;
          virtual ~RTCRtpParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtpParameters() noexcept override;

          static void apply(const NativeType &from, WrapperType &to);
          static void apply(const WrapperType &from, NativeType &to);

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

