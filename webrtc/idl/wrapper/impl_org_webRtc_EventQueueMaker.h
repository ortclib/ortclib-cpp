
#pragma once

#include "types.h"
#include "generated/org_webRtc_EventQueueMaker.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct EventQueueMaker : public wrapper::org::webRtc::EventQueueMaker
        {
          virtual ~EventQueueMaker() noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

