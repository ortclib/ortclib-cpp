
#pragma once

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/WebRtc/EventQueue.g.h"
#include <wrapper/generated/org_webRtc_EventQueue.h>

namespace winrt {
  namespace Org {
    namespace WebRtc {
      namespace implementation {


        /// <summary>
        /// A message queue for receiving delegate events.
        /// </summary>
        struct EventQueue : EventQueueT<EventQueue>
        {
          // internal
          wrapper::org::webRtc::EventQueuePtr native_;

          struct WrapperCreate {};
          EventQueue(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::WebRtc::implementation::EventQueue > ToCppWinrtImpl(wrapper::org::webRtc::EventQueuePtr value);
          static winrt::com_ptr< Org::WebRtc::implementation::EventQueue > ToCppWinrtImpl(Org::WebRtc::EventQueue const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::EventQueue > ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::EventQueue > const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::EventQueue > ToCppWinrtImpl(Org::WebRtc::IEventQueue const & value);

          // ToCppWinrt
          static Org::WebRtc::EventQueue ToCppWinrt(wrapper::org::webRtc::EventQueuePtr value);
          static Org::WebRtc::EventQueue ToCppWinrt(Org::WebRtc::EventQueue const & value);
          static Org::WebRtc::EventQueue ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::EventQueue > const & value);
          static Org::WebRtc::EventQueue ToCppWinrt(Org::WebRtc::IEventQueue const & value);

          // ToCppWinrtInterface
          static Org::WebRtc::IEventQueue ToCppWinrtInterface(wrapper::org::webRtc::EventQueuePtr value);
          static Org::WebRtc::IEventQueue ToCppWinrtInterface(Org::WebRtc::EventQueue const & value);
          static Org::WebRtc::IEventQueue ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::EventQueue > const & value);
          static Org::WebRtc::IEventQueue ToCppWinrtInterface(Org::WebRtc::IEventQueue const & value);

          // FromCppWinrt
          static wrapper::org::webRtc::EventQueuePtr FromCppWinrt(wrapper::org::webRtc::EventQueuePtr value);
          static wrapper::org::webRtc::EventQueuePtr FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::EventQueue > const & value);
          static wrapper::org::webRtc::EventQueuePtr FromCppWinrt(Org::WebRtc::EventQueue const & value);
          static wrapper::org::webRtc::EventQueuePtr FromCppWinrt(Org::WebRtc::IEventQueue const & value);




        public:
          /// <summary>
          /// Cast from Org::WebRtc::IEventQueue to EventQueue
          /// </summary>
          static Org::WebRtc::EventQueue Cast(Org::WebRtc::IEventQueue const & value);

          // ::org::webRtc::EventQueue

#if 0
          /// <summary>
          /// Constructs an event queue object from a native queue type.
          /// </summary>
          EventQueue(Windows::Foundation::IInspectable const & queue);
#endif //0
          /// <summary>
          /// The default windows message queue for the system GUI thread.
          /// </summary>
          static Org::WebRtc::IEventQueue GetDefaultForUi();

          /// <summary>
          /// Gets or sets the default system dispatcher object.
          /// </summary>
          static Org::WebRtc::IEventQueue Singleton();
          static void Singleton(Org::WebRtc::IEventQueue const & value);
#if 0
          /// <summary>
          /// Gets a native queue from an event queue object.
          /// </summary>
          Windows::Foundation::IInspectable Queue();
#endif //0

        };

      } // namepsace implementation

      namespace factory_implementation {

        struct EventQueue : EventQueueT<EventQueue, implementation::EventQueue>
        {
        };

      } // namespace factory_implementation

    } // namespace WebRtc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
