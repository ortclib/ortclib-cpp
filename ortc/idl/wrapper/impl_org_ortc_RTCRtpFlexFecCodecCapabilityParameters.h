
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpFlexFecCodecCapabilityParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpFlexFecCodecCapabilityParameters : public wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::FlexFECCodecCapabilityParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpFlexFecCodecCapabilityParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParameters, WrapperType);
          RTCRtpFlexFecCodecCapabilityParametersWeakPtr thisWeak_;

          RTCRtpFlexFecCodecCapabilityParameters() noexcept;
          virtual ~RTCRtpFlexFecCodecCapabilityParameters() noexcept;

          // methods RTCRtpFlexFecCodecCapabilityParameters
          void wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters() noexcept override;
          void wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters(wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

