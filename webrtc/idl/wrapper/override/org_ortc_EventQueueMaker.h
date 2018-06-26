
#ifndef WRAPPER_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER

#pragma once

#include <wrapper/generated/types.h>
#include <wrapper/generated/org_ortc_EventQueue.h>

namespace wrapper {
  namespace org {
    namespace ortc {

      struct EventQueueMaker
      {
        virtual ~EventQueueMaker() noexcept {}

#if 0
        static wrapper::org::ortc::EventQueuePtr bindQueue(AnyPtr queue) noexcept;
        static AnyPtr extractQueue(wrapper::org::ortc::EventQueuePtr queue) noexcept;
#endif //0

      };

    } // ortc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER
