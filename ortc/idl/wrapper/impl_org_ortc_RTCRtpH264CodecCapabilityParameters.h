
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

          RTCRtpH264CodecCapabilityParameters() noexcept;
          virtual ~RTCRtpH264CodecCapabilityParameters() noexcept;

          // methods RTCRtpH264CodecCapabilityParameters
          void wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters() noexcept override;
          void wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters(wrapper::org::ortc::RTCRtpH264CodecCapabilityParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

