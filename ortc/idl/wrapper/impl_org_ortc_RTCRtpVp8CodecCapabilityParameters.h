
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

          RTCRtpVp8CodecCapabilityParameters() noexcept;
          virtual ~RTCRtpVp8CodecCapabilityParameters() noexcept;

          // methods RTCRtpVp8CodecCapabilityParameters
          void wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters() noexcept override;
          void wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters(wrapper::org::ortc::RTCRtpVp8CodecCapabilityParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

