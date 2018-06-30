
#ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#pragma once

#include <wrapper/generated/types.h>
#include <wrapper/generated/org_webRtc_EventQueue.h>

namespace wrapper {
  namespace org {
    namespace webRtc {

      struct EventQueueMaker
      {
        virtual ~EventQueueMaker() noexcept {}

#if 0
        static wrapper::org::webRtc::EventQueuePtr bindQueue(AnyPtr queue) noexcept;
        static AnyPtr extractQueue(wrapper::org::webRtc::EventQueuePtr queue) noexcept;
#endif //0

      };

    } // webRtc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
