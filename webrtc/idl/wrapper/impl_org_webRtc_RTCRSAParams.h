
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRSAParams.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRSAParams : public wrapper::org::webRtc::RTCRSAParams
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRSAParams, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRSAParams, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::rtc::RSAParams, NativeType);

          RTCRSAParamsWeakPtr thisWeak_;

          RTCRSAParams() noexcept;
          virtual ~RTCRSAParams() noexcept;
          void wrapper_init_org_webRtc_RTCRSAParams() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

