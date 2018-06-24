
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpFecParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpFecParameters : public wrapper::org::ortc::RTCRtpFecParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::FECParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpFecParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpFecParameters, WrapperType);
          RTCRtpFecParametersWeakPtr thisWeak_;

          RTCRtpFecParameters() noexcept;
          virtual ~RTCRtpFecParameters() noexcept;

          // methods RTCRtpFecParameters
          void wrapper_init_org_ortc_RTCRtpFecParameters() noexcept override;
          void wrapper_init_org_ortc_RTCRtpFecParameters(wrapper::org::ortc::RTCRtpFecParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpFecParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

