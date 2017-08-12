
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSrtpSdesParameters.h"

#include <ortc/ISRTPSDESTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSrtpSdesParameters : public wrapper::org::ortc::RTCSrtpSdesParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISRTPSDESTransport::Parameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCSrtpSdesParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSrtpSdesParameters, WrapperType);
          RTCSrtpSdesParametersWeakPtr thisWeak_;

          RTCSrtpSdesParameters();
          virtual ~RTCSrtpSdesParameters();

          // methods RTCSrtpSdesParameters
          virtual void wrapper_init_org_ortc_RTCSrtpSdesParameters() override;
          virtual void wrapper_init_org_ortc_RTCSrtpSdesParameters(wrapper::org::ortc::RTCSrtpSdesParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCSrtpSdesParameters(wrapper::org::ortc::JsonPtr json) override;
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

