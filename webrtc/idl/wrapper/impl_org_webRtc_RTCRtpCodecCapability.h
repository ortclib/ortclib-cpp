
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpCodecCapability.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpCodecCapability : public wrapper::org::webRtc::RTCRtpCodecCapability
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpCodecCapability, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodecCapability, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpCodecCapability, NativeType);

          RTCRtpCodecCapabilityWeakPtr thisWeak_;

          RTCRtpCodecCapability() noexcept;
          virtual ~RTCRtpCodecCapability() noexcept;
          void wrapper_init_org_webRtc_RTCRtpCodecCapability() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

