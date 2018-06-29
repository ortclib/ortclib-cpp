
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCOfferAnswerOptions.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCOfferAnswerOptions : public wrapper::org::webrtc::RTCOfferAnswerOptions
        {
          virtual ~RTCOfferAnswerOptions() noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

