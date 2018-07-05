
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpHeaderExtensionCapability.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpHeaderExtensionCapability : public wrapper::org::webRtc::RTCRtpHeaderExtensionCapability
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpHeaderExtensionCapability, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpHeaderExtensionCapability, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpHeaderExtensionCapability, NativeType);

          RTCRtpHeaderExtensionCapabilityWeakPtr thisWeak_;

          RTCRtpHeaderExtensionCapability() noexcept;
          virtual ~RTCRtpHeaderExtensionCapability() noexcept;
          void wrapper_init_org_webRtc_RTCRtpHeaderExtensionCapability() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

