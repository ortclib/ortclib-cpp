
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSctpTransportListenerEvent.h"

#include <ortc/types.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSctpTransportListenerEvent : public wrapper::org::ortc::RTCSctpTransportListenerEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransport, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCSctpTransportListenerEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSctpTransportListenerEvent, WrapperType);
          RTCSctpTransportListenerEventWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCSctpTransportListenerEvent() noexcept;
          virtual ~RTCSctpTransportListenerEvent() noexcept;

          // properties RTCSctpTransportListenerEvent
          wrapper::org::ortc::RTCSctpTransportPtr get_transport() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

