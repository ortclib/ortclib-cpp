
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpCodingParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpCodingParameters : public wrapper::org::webRtc::RTCRtpCodingParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpCodingParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodingParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpEncodingParameters, NativeType);

          RTCRtpCodingParametersWeakPtr thisWeak_;

          RTCRtpCodingParameters() noexcept;
          virtual ~RTCRtpCodingParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtpCodingParameters() noexcept override;

          static void apply(const NativeType &from, WrapperType &to) noexcept;
          static void apply(const WrapperType &from, NativeType &to) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

