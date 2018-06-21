
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

          RTCIceGathererCandidateEvent() noexcept;
          virtual ~RTCIceGathererCandidateEvent() noexcept;

          // properties RTCIceGathererCandidateEvent
          wrapper::org::ortc::RTCIceCandidatePtr get_candidate() noexcept override;
          String get_url() noexcept override;

          static WrapperImplTypePtr toWrapper(
            ::ortc::IICEGathererTypes::CandidatePtr candidate,
            const String &url = String()
          ) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

