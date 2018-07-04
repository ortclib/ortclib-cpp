
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCKeyParams.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCKeyParams : public wrapper::org::webRtc::RTCKeyParams
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCKeyParams, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCKeyParams, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::rtc::KeyParams, NativeType);

          NativeTypePtr native_;
          RTCKeyParamsWeakPtr thisWeak_;

          RTCKeyParams() noexcept;
          virtual ~RTCKeyParams() noexcept;

          // methods RTCKeyParams
          void wrapper_init_org_webRtc_RTCKeyParams() noexcept override;
          void wrapper_init_org_webRtc_RTCKeyParams(wrapper::org::webRtc::RTCKeyType key_type) noexcept override;

          // properties RTCKeyParams
          bool get_valid() noexcept override;
          wrapper::org::webRtc::RTCRSAParamsPtr get_rsa() noexcept override;
          wrapper::org::webRtc::RTCECCurve get_ec() noexcept override;
          wrapper::org::webRtc::RTCKeyType get_type() noexcept override;

          static WrapperImplTypePtr createDefault();

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

