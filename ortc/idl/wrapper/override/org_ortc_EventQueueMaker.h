
#ifndef WRAPPER_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER

#pragma once

#include <wrapper/generated/types.h>
#include <wrapper/generated/org_ortc_EventQueue.h>

#ifndef WINUWP
#include <zsLib/IMessageQueue.h>
#endif //ndef WINUWP

namespace wrapper {
  namespace org {
    namespace ortc {

      struct EventQueueMaker
      {
        virtual ~EventQueueMaker() noexcept {}

#ifdef WINUWP
#ifdef __cplusplus_winrt
        static wrapper::org::ortc::EventQueuePtr bindQueue(Windows::UI::Core::CoreDispatcher^ queue) noexcept;
        static Windows::UI::Core::CoreDispatcher^ extractQueue(wrapper::org::ortc::EventQueuePtr queue) noexcept;
#else // __cplusplus_winrt
        static wrapper::org::ortc::EventQueuePtr bindQueue(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept;
        static winrt::Windows::UI::Core::CoreDispatcher extractQueue(wrapper::org::ortc::EventQueuePtr queue) noexcept;
#endif //__cplusplus_winrt
#else //WINUWP
        static wrapper::org::ortc::EventQueuePtr bindQueue(::zsLib::IMessageQueuePtr queue) noexcept;
        static ::zsLib::IMessageQueuePtr extractQueue(wrapper::org::ortc::EventQueuePtr queue) noexcept;
#endif //WINUWP

      };

    } // ortc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER
