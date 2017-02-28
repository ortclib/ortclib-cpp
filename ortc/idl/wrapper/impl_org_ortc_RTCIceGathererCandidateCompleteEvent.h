
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGathererCandidateCompleteEvent.h"

#include <ortc/IICEGatherer.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGathererCandidateCompleteEvent : public wrapper::org::ortc::RTCIceGathererCandidateCompleteEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(RTCIceGathererCandidateCompleteEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceGathererCandidateCompleteEvent, WrapperType);
          RTCIceGathererCandidateCompleteEventWeakPtr thisWeak_;
          wrapper::org::ortc::RTCIceCandidateCompletePtr candidate_;

          RTCIceGathererCandidateCompleteEvent();
          virtual ~RTCIceGathererCandidateCompleteEvent();

          // properties RTCIceGathererCandidateCompleteEvent
          virtual wrapper::org::ortc::RTCIceCandidateCompletePtr get_candidate() override;

          static WrapperImplTypePtr toWrapper(::ortc::IICEGathererTypes::CandidateCompletePtr candidate);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

