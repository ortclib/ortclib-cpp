
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSrtpSdesCryptoParameters.h"

#include <ortc/ISRTPSDESTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSrtpSdesCryptoParameters : public wrapper::org::ortc::RTCSrtpSdesCryptoParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISRTPSDESTransport::CryptoParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCSrtpSdesCryptoParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSrtpSdesCryptoParameters, WrapperType);
          RTCSrtpSdesCryptoParametersWeakPtr thisWeak_;

          RTCSrtpSdesCryptoParameters();
          virtual ~RTCSrtpSdesCryptoParameters();

          // methods RTCSrtpSdesCryptoParameters
          virtual void wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters() override;
          virtual void wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters(wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters(wrapper::org::ortc::JsonPtr json) override;
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

