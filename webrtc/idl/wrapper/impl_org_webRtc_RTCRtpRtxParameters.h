
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpRtxParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpRtxParameters : public wrapper::org::webRtc::RTCRtpRtxParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpRtxParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpRtxParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpRtxParameters, NativeType);

          RTCRtpRtxParametersWeakPtr thisWeak_;

          RTCRtpRtxParameters() noexcept;
          virtual ~RTCRtpRtxParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtpRtxParameters() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

