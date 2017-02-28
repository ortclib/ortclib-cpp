
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

          RTCRtpOpusCodecCapabilityParameters();
          virtual ~RTCRtpOpusCodecCapabilityParameters();

          // methods RTCRtpOpusCodecCapabilityParameters
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters(wrapper::org::ortc::RTCRtpOpusCodecCapabilityParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) override;
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

