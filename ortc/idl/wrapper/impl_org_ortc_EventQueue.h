
#pragma once

#include "types.h"
#include "generated/org_ortc_EventQueue.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct EventQueue : public wrapper::org::ortc::EventQueue
        {
          static EventQueuePtr singleton_;
          EventQueueWeakPtr thisWeak_;

#ifdef WINUWP
#ifdef __cplusplus_winrt
          Windows::UI::Core::CoreDispatcher^ queue_{};
#else // __cplusplus_winrt
          winrt::Windows::UI::Core::CoreDispatcher queue_ {nullptr};
#endif //__cplusplus_winrt
#else
          ::zsLib::IMessageQueuePtr queue_{};
#endif //WINUWP

          EventQueue() noexcept;
          virtual ~EventQueue() noexcept;

          // methods EventQueue
#ifdef WINUWP
#ifdef __cplusplus_winrt
          void wrapper_init_org_ortc_EventQueue(Windows::UI::Core::CoreDispatcher^ queue) noexcept override { queue_ = queue; }
#else // __cplusplus_winrt
          void wrapper_init_org_ortc_EventQueue(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept override { queue_ = queue; }
#endif //__cplusplus_winrt
#else
          void wrapper_init_org_ortc_EventQueue(::zsLib::IMessageQueuePtr queue) noexcept override { queue_ = queue; }
#endif //WINUWP

          // properties EventQueue
#ifdef WINUWP
#ifdef __cplusplus_winrt
          Windows::UI::Core::CoreDispatcher^ get_queue() noexcept override { return queue_; }
#else // __cplusplus_winrt
          winrt::Windows::UI::Core::CoreDispatcher get_queue() noexcept override { return queue_; }
#endif //__cplusplus_winrt
#else
          ::zsLib::IMessageQueuePtr get_queue() noexcept override { return queue_; }
#endif //WINUWP

#ifdef WINUWP
#ifdef __cplusplus_winrt
          static Windows::UI::Core::CoreDispatcher^ toNative(wrapper::org::ortc::EventQueuePtr queue) { if (!queue) return nullptr; return queue->get_queue(); }
#else // __cplusplus_winrt
          static winrt::Windows::UI::Core::CoreDispatcher toNative(wrapper::org::ortc::EventQueuePtr queue) { if (!queue) return nullptr; return queue->get_queue(); }
#endif //__cplusplus_winrt
#else
          static ::zsLib::IMessageQueuePtr toNative(wrapper::org::ortc::EventQueuePtr queue) { if (!queue) return nullptr; return queue->get_queue(); }
#endif //WINUWP
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

