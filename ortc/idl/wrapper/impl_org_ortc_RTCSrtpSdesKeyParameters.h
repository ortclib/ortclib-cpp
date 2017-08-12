
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

          RTCSrtpSdesKeyParameters();
          virtual ~RTCSrtpSdesKeyParameters();

          // methods RTCSrtpSdesKeyParameters
          virtual void wrapper_init_org_ortc_RTCSrtpSdesKeyParameters() override;
          virtual void wrapper_init_org_ortc_RTCSrtpSdesKeyParameters(wrapper::org::ortc::RTCSrtpSdesKeyParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCSrtpSdesKeyParameters(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

