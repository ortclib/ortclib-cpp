
#ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#pragma once

#include <wrapper/generated/types.h>

namespace wrapper {
  namespace org {
    namespace webrtc {

      struct EventQueue
      {
        static EventQueuePtr wrapper_create() noexcept;
        virtual ~EventQueue() noexcept {}

        virtual void wrapper_init_org_webrtc_EventQueue(AnyPtr queue) noexcept { ZS_MAYBE_USED(queue); }

        static wrapper::org::webrtc::EventQueuePtr getDefaultForUi() noexcept;

        static wrapper::org::webrtc::EventQueuePtr get_singleton() noexcept;
        static void set_singleton(wrapper::org::webrtc::EventQueuePtr value) noexcept;

        virtual AnyPtr get_queue() noexcept = 0;

      };

    } // webrtc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
