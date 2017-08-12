
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpRtxCodecCapabilityParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpRtxCodecCapabilityParameters : public wrapper::org::ortc::RTCRtpRtxCodecCapabilityParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::RTXCodecCapabilityParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpRtxCodecCapabilityParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpRtxCodecCapabilityParameters, WrapperType);
          RTCRtpRtxCodecCapabilityParametersWeakPtr thisWeak_;

          RTCRtpRtxCodecCapabilityParameters();
          virtual ~RTCRtpRtxCodecCapabilityParameters();

          // methods RTCRtpRtxCodecCapabilityParameters
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters() override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters(wrapper::org::ortc::RTCRtpRtxCodecCapabilityParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) override;
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

