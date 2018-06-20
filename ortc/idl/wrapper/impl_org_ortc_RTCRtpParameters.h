
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpParameters : public wrapper::org::ortc::RTCRtpParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::Parameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpParameters, WrapperType);
          RTCRtpParametersWeakPtr thisWeak_;

          RTCRtpParameters() noexcept;
          virtual ~RTCRtpParameters() noexcept;

          // methods RTCRtpParameters
          virtual void wrapper_init_org_ortc_RTCRtpParameters() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpParameters(wrapper::org::ortc::RTCRtpParametersPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

