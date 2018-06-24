
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

          RTCSrtpSdesCryptoParameters() noexcept;
          virtual ~RTCSrtpSdesCryptoParameters() noexcept;

          // methods RTCSrtpSdesCryptoParameters
          void wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters() noexcept override;
          void wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters(wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

