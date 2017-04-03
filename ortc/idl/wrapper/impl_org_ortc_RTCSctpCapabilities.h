
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

          RTCSctpCapabilities();
          virtual ~RTCSctpCapabilities();

          // methods RTCSctpCapabilities
          virtual void wrapper_init_org_ortc_RTCSctpCapabilities() override;
          virtual void wrapper_init_org_ortc_RTCSctpCapabilities(wrapper::org::ortc::RTCSctpCapabilitiesPtr source) override;
          virtual void wrapper_init_org_ortc_RTCSctpCapabilities(wrapper::org::ortc::JsonPtr json) override;
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

