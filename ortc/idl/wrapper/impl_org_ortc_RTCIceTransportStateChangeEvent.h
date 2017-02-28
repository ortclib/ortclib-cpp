
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceTransportStateChangeEvent.h"

#include <ortc/IICETransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceTransportStateChangeEvent : public wrapper::org::ortc::RTCIceTransportStateChangeEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(RTCIceTransportStateChangeEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceTransportStateChangeEvent, WrapperType);
          RTCIceTransportStateChangeEventWeakPtr thisWeak_;
          ::ortc::IICETransportTypes::States state_ { ::ortc::IICETransportTypes::State_First};

          RTCIceTransportStateChangeEvent();
          virtual ~RTCIceTransportStateChangeEvent();

          // properties RTCIceTransportStateChangeEvent
          virtual wrapper::org::ortc::RTCIceTransportState get_state() override;

          static WrapperImplTypePtr toWrapper(::ortc::IICETransportTypes::States state);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

