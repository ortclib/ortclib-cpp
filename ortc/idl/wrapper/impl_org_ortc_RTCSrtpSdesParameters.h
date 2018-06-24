
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

          RTCSrtpSdesParameters() noexcept;
          virtual ~RTCSrtpSdesParameters() noexcept;

          // methods RTCSrtpSdesParameters
          void wrapper_init_org_ortc_RTCSrtpSdesParameters() noexcept override;
          void wrapper_init_org_ortc_RTCSrtpSdesParameters(wrapper::org::ortc::RTCSrtpSdesParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCSrtpSdesParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

