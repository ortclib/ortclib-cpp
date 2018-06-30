
#pragma once

#include "types.h"
#include "generated/org_ortc_EventQueue.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct EventQueue : public wrapper::org::ortc::EventQueue
        {
          EventQueueWeakPtr thisWeak_;

          AnyPtr queue_{};

          EventQueue() noexcept;
          virtual ~EventQueue() noexcept;

          // methods EventQueue
          void wrapper_init_org_ortc_EventQueue(AnyPtr queue) noexcept override { queue_ = queue; }

          // properties EventQueue
          AnyPtr get_queue() noexcept override { return queue_; }

#ifdef WINUWP
#ifdef __cplusplus_winrt
          static wrapper::org::ortc::EventQueuePtr toWrapper(Windows::UI::Core::CoreDispatcher^ queue) noexcept;
          static Windows::UI::Core::CoreDispatcher^ toNative_cx(wrapper::org::ortc::EventQueuePtr queue) noexcept;
#endif //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
          static wrapper::org::ortc::EventQueuePtr toWrapper(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept;
          static winrt::Windows::UI::Core::CoreDispatcher toNative_winrt(wrapper::org::ortc::EventQueuePtr queue) noexcept;
#endif // CPPWINRT_VERSION
#else
          static wrapper::org::ortc::EventQueuePtr toWrapper(::zsLib::IMessageQueuePtr queue) noexcept;
          static ::zsLib::IMessageQueuePtr toNative(wrapper::org::ortc::EventQueuePtr queue) noexcept;
#endif //WINUWP

        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

