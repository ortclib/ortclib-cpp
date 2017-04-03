
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceCandidatePairChangeEvent.h"

#include <ortc/IICETransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceCandidatePairChangeEvent : public wrapper::org::ortc::RTCIceCandidatePairChangeEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(RTCIceCandidatePairChangeEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceCandidatePairChangeEvent, WrapperType);
          RTCIceCandidatePairChangeEventWeakPtr thisWeak_;
          ::ortc::IICETransportTypes::CandidatePairPtr candidatePair_;

          RTCIceCandidatePairChangeEvent();
          virtual ~RTCIceCandidatePairChangeEvent();

          // properties RTCIceCandidatePairChangeEvent
          virtual wrapper::org::ortc::RTCIceCandidatePairPtr get_candidatePair() override;

          static WrapperImplTypePtr toWrapper(::ortc::IICETransportTypes::CandidatePairPtr candidatePair);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

