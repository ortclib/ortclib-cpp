
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSctpCapabilities.h"

#include <ortc/ISCTPTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSctpCapabilities : public wrapper::org::ortc::RTCSctpCapabilities
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransportTypes::Capabilities, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCSctpCapabilities, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSctpCapabilities, WrapperType);
          RTCSctpCapabilitiesWeakPtr thisWeak_;

          RTCSctpCapabilities() noexcept;
          virtual ~RTCSctpCapabilities() noexcept;

          // methods RTCSctpCapabilities
          void wrapper_init_org_ortc_RTCSctpCapabilities() noexcept override;
          void wrapper_init_org_ortc_RTCSctpCapabilities(wrapper::org::ortc::RTCSctpCapabilitiesPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCSctpCapabilities(wrapper::org::ortc::JsonPtr json) noexcept override;
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

