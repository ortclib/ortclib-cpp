
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpHeaderExtensionParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpHeaderExtensionParameters : public wrapper::org::webRtc::RTCRtpHeaderExtensionParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpHeaderExtensionParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpHeaderExtensionParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpExtension, NativeType);

          RTCRtpHeaderExtensionParametersWeakPtr thisWeak_;

          RTCRtpHeaderExtensionParameters() noexcept;
          virtual ~RTCRtpHeaderExtensionParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtpHeaderExtensionParameters() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

