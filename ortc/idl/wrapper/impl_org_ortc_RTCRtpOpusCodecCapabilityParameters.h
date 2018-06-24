
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpOpusCodecCapabilityParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpOpusCodecCapabilityParameters : public wrapper::org::ortc::RTCRtpOpusCodecCapabilityParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::OpusCodecCapabilityParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpOpusCodecCapabilityParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpOpusCodecCapabilityParameters, WrapperType);
          RTCRtpOpusCodecCapabilityParametersWeakPtr thisWeak_;

          RTCRtpOpusCodecCapabilityParameters() noexcept;
          virtual ~RTCRtpOpusCodecCapabilityParameters() noexcept;

          // methods RTCRtpOpusCodecCapabilityParameters
          void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters() noexcept override;
          void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters(wrapper::org::ortc::RTCRtpOpusCodecCapabilityParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(AnyPtr native) noexcept;
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

