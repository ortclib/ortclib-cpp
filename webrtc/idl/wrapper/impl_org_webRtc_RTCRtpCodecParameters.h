
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCRtpCodecParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCRtpCodecParameters : public wrapper::org::webRtc::RTCRtpCodecParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCRtpCodecParameters, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodecParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RtpCodecParameters, NativeType);

          RTCRtpCodecParametersWeakPtr thisWeak_;

          RTCRtpCodecParameters() noexcept;
          virtual ~RTCRtpCodecParameters() noexcept;
          void wrapper_init_org_webRtc_RTCRtpCodecParameters() noexcept override;

          static String toFmtp(const std::unordered_map<std::string, std::string> &input) noexcept;
          static void fromFmtp(const String &input, std::unordered_map<std::string, std::string> &output) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

