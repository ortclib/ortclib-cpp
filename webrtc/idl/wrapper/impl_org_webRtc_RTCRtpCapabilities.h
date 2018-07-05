
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpCapabilities.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpCapabilities : public wrapper::org::webRtc::RTCRtpCapabilities
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpCapabilities, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCapabilities, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpCapabilities, NativeType);

          RTCRtpCapabilitiesWeakPtr thisWeak_;

          RTCRtpCapabilities() noexcept;
          virtual ~RTCRtpCapabilities() noexcept;
          void wrapper_init_org_webRtc_RTCRtpCapabilities() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

