
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpCodecParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpCodecParameters : public wrapper::org::ortc::RTCRtpCodecParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::CodecParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpCodecParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpCodecParameters, WrapperType);
          RTCRtpCodecParametersWeakPtr thisWeak_;

          RTCRtpCodecParameters() noexcept;
          virtual ~RTCRtpCodecParameters() noexcept;

          // methods RTCRtpCodecParameters
          virtual void wrapper_init_org_ortc_RTCRtpCodecParameters() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpCodecParameters(wrapper::org::ortc::RTCRtpCodecParametersPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpCodecParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

