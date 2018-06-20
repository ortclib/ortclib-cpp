
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpEncodingParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpEncodingParameters : public wrapper::org::ortc::RTCRtpEncodingParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::EncodingParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpEncodingParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpEncodingParameters, WrapperType);
          RTCRtpEncodingParametersWeakPtr thisWeak_;

          RTCRtpEncodingParameters() noexcept;
          virtual ~RTCRtpEncodingParameters() noexcept;

          // methods RTCRtpEncodingParameters
          virtual void wrapper_init_org_ortc_RTCRtpEncodingParameters() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpEncodingParameters(wrapper::org::ortc::RTCRtpEncodingParametersPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpEncodingParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

