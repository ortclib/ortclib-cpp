
#ifndef WRAPPER_USE_GENERATED_ORG_ORTC_EVENTQUEUE

#pragma once

#include <wrapper/generated/types.h>

namespace wrapper {
  namespace org {
    namespace ortc {

      struct EventQueue
      {
        static EventQueuePtr wrapper_create() noexcept;
        virtual ~EventQueue() noexcept {}

        virtual void wrapper_init_org_ortc_EventQueue(AnyPtr queue) noexcept { ZS_MAYBE_USED(queue); }

        static wrapper::org::ortc::EventQueuePtr getDefaultForUi() noexcept;

        static wrapper::org::ortc::EventQueuePtr get_singleton() noexcept;
        static void set_singleton(wrapper::org::ortc::EventQueuePtr value) noexcept;

        virtual AnyPtr get_queue() noexcept = 0;

      };

    } // ortc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_ORTC_EVENTQUEUE
