
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpVp8CodecCapabilityParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpVp8CodecCapabilityParameters : public wrapper::org::ortc::RTCRtpVp8CodecCapabilityParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::VP8CodecCapabilityParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpVp8CodecCapabilityParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpVp8CodecCapabilityParameters, WrapperType);
          RTCRtpVp8CodecCapabilityParametersWeakPtr thisWeak_;

          RTCRtpVp8CodecCapabilityParameters();
          virtual ~RTCRtpVp8CodecCapabilityParameters();

          // methods RTCRtpVp8CodecCapabilityParameters
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters(wrapper::org::ortc::RTCRtpVp8CodecCapabilityParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) override;
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

