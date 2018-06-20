
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

          RTCRtpRtxCodecCapabilityParameters() noexcept;
          virtual ~RTCRtpRtxCodecCapabilityParameters() noexcept;

          // methods RTCRtpRtxCodecCapabilityParameters
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters(wrapper::org::ortc::RTCRtpRtxCodecCapabilityParametersPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(AnyPtr native) noexcept;
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

