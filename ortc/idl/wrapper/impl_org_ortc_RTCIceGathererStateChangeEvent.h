
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGathererStateChangeEvent.h"

#include <ortc/IICEGatherer.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGathererStateChangeEvent : public wrapper::org::ortc::RTCIceGathererStateChangeEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(RTCIceGathererStateChangeEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceGathererStateChangeEvent, WrapperType);
          RTCIceGathererStateChangeEventWeakPtr thisWeak_;
          ::ortc::IICEGathererTypes::States state_ { ::ortc::IICEGathererTypes::State_First};

          RTCIceGathererStateChangeEvent();
          virtual ~RTCIceGathererStateChangeEvent();

          // properties RTCIceGathererStateChangeEvent
          virtual wrapper::org::ortc::RTCIceGathererState get_state() override;

          static WrapperImplTypePtr toWrapper(::ortc::IICEGathererTypes::States state);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

