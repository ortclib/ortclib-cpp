
#ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#pragma once

#include <wrapper/generated/types.h>
#include <wrapper/generated/org_webrtc_EventQueue.h>

namespace wrapper {
  namespace org {
    namespace webrtc {

      struct EventQueueMaker
      {
        virtual ~EventQueueMaker() noexcept {}

#if 0
        static wrapper::org::webrtc::EventQueuePtr bindQueue(AnyPtr queue) noexcept;
        static AnyPtr extractQueue(wrapper::org::webrtc::EventQueuePtr queue) noexcept;
#endif //0

      };

    } // webrtc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
