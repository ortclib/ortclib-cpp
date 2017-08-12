
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

          RTCRtpFlexFecCodecCapabilityParameters();
          virtual ~RTCRtpFlexFecCodecCapabilityParameters();

          // methods RTCRtpFlexFecCodecCapabilityParameters
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters(wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static WrapperImplTypePtr toWrapper(AnyPtr native);
          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

