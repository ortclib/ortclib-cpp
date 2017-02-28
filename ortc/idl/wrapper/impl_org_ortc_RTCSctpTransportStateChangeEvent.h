
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSctpTransportStateChangeEvent.h"

#include <ortc/ISCTPTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSctpTransportStateChangeEvent : public wrapper::org::ortc::RTCSctpTransportStateChangeEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransportTypes::States, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCSctpTransportStateChangeEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSctpTransportStateChangeEvent, WrapperType);
          RTCSctpTransportStateChangeEventWeakPtr thisWeak_;
          NativeType native_;

          RTCSctpTransportStateChangeEvent();
          virtual ~RTCSctpTransportStateChangeEvent();

          // properties RTCSctpTransportStateChangeEvent
          virtual wrapper::org::ortc::RTCSctpTransportState get_state() override;

          static WrapperImplTypePtr toWrapper(NativeType native);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

