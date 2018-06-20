
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSrtpSdesKeyParameters.h"

#include <ortc/ISRTPSDESTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSrtpSdesKeyParameters : public wrapper::org::ortc::RTCSrtpSdesKeyParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISRTPSDESTransportTypes::KeyParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCSrtpSdesKeyParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSrtpSdesKeyParameters, WrapperType);
          RTCSrtpSdesKeyParametersWeakPtr thisWeak_;

          RTCSrtpSdesKeyParameters() noexcept;
          virtual ~RTCSrtpSdesKeyParameters() noexcept;

          // methods RTCSrtpSdesKeyParameters
          virtual void wrapper_init_org_ortc_RTCSrtpSdesKeyParameters() noexcept override;
          virtual void wrapper_init_org_ortc_RTCSrtpSdesKeyParameters(wrapper::org::ortc::RTCSrtpSdesKeyParametersPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCSrtpSdesKeyParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

