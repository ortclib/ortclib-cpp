
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpH264CodecCapabilityParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpH264CodecCapabilityParameters : public wrapper::org::ortc::RTCRtpH264CodecCapabilityParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::H264CodecCapabilityParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpH264CodecCapabilityParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpH264CodecCapabilityParameters, WrapperType);
          RTCRtpH264CodecCapabilityParametersWeakPtr thisWeak_;

          RTCRtpH264CodecCapabilityParameters();
          virtual ~RTCRtpH264CodecCapabilityParameters();

          // methods RTCRtpH264CodecCapabilityParameters
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters(wrapper::org::ortc::RTCRtpH264CodecCapabilityParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) override;
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

