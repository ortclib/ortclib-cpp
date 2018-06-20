
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtcpParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtcpParameters : public wrapper::org::ortc::RTCRtcpParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::RTCPParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtcpParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtcpParameters, WrapperType);
          RTCRtcpParametersWeakPtr thisWeak_;

          RTCRtcpParameters() noexcept;
          virtual ~RTCRtcpParameters() noexcept;

          // methods RTCRtcpParameters
          virtual void wrapper_init_org_ortc_RTCRtcpParameters() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtcpParameters(wrapper::org::ortc::RTCRtcpParametersPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtcpParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

