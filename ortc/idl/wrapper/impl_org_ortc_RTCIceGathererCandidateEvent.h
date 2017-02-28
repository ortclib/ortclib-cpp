
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceGathererCandidateEvent.h"

#include <ortc/IICEGatherer.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceGathererCandidateEvent : public wrapper::org::ortc::RTCIceGathererCandidateEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(RTCIceGathererCandidateEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceGathererCandidateEvent, WrapperType);
          RTCIceGathererCandidateEventWeakPtr thisWeak_;
          wrapper::org::ortc::RTCIceCandidatePtr candidate_;
          String url_;

          RTCIceGathererCandidateEvent();
          virtual ~RTCIceGathererCandidateEvent();

          // properties RTCIceGathererCandidateEvent
          virtual wrapper::org::ortc::RTCIceCandidatePtr get_candidate() override;
          virtual String get_url() override;

          static WrapperImplTypePtr toWrapper(
            ::ortc::IICEGathererTypes::CandidatePtr candidate,
            const String &url = String()
          );
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

