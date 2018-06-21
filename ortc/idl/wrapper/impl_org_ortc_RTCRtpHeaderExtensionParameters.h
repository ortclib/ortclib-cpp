
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpHeaderExtensionParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpHeaderExtensionParameters : public wrapper::org::ortc::RTCRtpHeaderExtensionParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::HeaderExtensionParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpHeaderExtensionParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpHeaderExtensionParameters, WrapperType);
          RTCRtpHeaderExtensionParametersWeakPtr thisWeak_;

          RTCRtpHeaderExtensionParameters() noexcept;
          virtual ~RTCRtpHeaderExtensionParameters() noexcept;

          // methods RTCRtpHeaderExtensionParameters
          void wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters() noexcept override;
          void wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters(wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpHeaderExtensionParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

