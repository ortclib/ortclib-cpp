
#ifndef WRAPPER_USE_GENERATED_ORG_ORTC_EVENTQUEUE

#pragma once

#include <wrapper/generated/types.h>

#ifdef WINUWP
#ifdef __cplusplus_winrt
#include <windows.ui.core.h>
#else
#include <winrt/windows.ui.core.h>
#endif //__cplusplus_winrt
#else
#include <zsLib/IMessageQueue.h>
#endif //WINUWP


namespace wrapper {
  namespace org {
    namespace ortc {

      struct EventQueue
      {
        static EventQueuePtr wrapper_create() noexcept;
        virtual ~EventQueue() noexcept {}

#ifdef WINUWP
#ifdef __cplusplus_winrt
        virtual void wrapper_init_org_ortc_EventQueue(Windows::UI::Core::CoreDispatcher^ queue) noexcept { ZS_MAYBE_USED(queue); }
#else // __cplusplus_winrt
        virtual void wrapper_init_org_ortc_EventQueue(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept { ZS_MAYBE_USED(queue); }
#endif //__cplusplus_winrt
#else
        virtual void wrapper_init_org_ortc_EventQueue(::zsLib::IMessageQueuePtr queue) noexcept { ZS_MAYBE_USED(queue); }
#endif //WINUWP

        static wrapper::org::ortc::EventQueuePtr getDefaultForUi() noexcept;

        static wrapper::org::ortc::EventQueuePtr get_singleton() noexcept;
        static void set_singleton(wrapper::org::ortc::EventQueuePtr value) noexcept;
#ifdef WINUWP
#ifdef __cplusplus_winrt
        virtual Windows::UI::Core::CoreDispatcher^ get_queue() noexcept = 0;
#else // __cplusplus_winrt
        virtual winrt::Windows::UI::Core::CoreDispatcher get_queue() noexcept = 0;
#endif //__cplusplus_winrt
#else
        virtual ::zsLib::IMessageQueuePtr get_queue() noexcept = 0;
#endif //WINUWP

      };

    } // ortc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_ORTC_EVENTQUEUE
