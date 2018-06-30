
#pragma once

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/Webrtc/EventQueue.g.h"
#include <wrapper/generated/org_webrtc_EventQueue.h>

namespace winrt {
  namespace Org {
    namespace Webrtc {
      namespace implementation {


        /// <summary>
        /// A message queue for receiving delegate events.
        /// </summary>
        struct EventQueue : EventQueueT<EventQueue>
        {
          // internal
          wrapper::org::webrtc::EventQueuePtr native_;

          struct WrapperCreate {};
          EventQueue(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::Webrtc::implementation::EventQueue > ToCppWinrtImpl(wrapper::org::webrtc::EventQueuePtr value);
          static winrt::com_ptr< Org::Webrtc::implementation::EventQueue > ToCppWinrtImpl(Org::Webrtc::EventQueue const & value);
          static winrt::com_ptr< Org::Webrtc::implementation::EventQueue > ToCppWinrtImpl(winrt::com_ptr< Org::Webrtc::implementation::EventQueue > const & value);
          static winrt::com_ptr< Org::Webrtc::implementation::EventQueue > ToCppWinrtImpl(Org::Webrtc::IEventQueue const & value);

          // ToCppWinrt
          static Org::Webrtc::EventQueue ToCppWinrt(wrapper::org::webrtc::EventQueuePtr value);
          static Org::Webrtc::EventQueue ToCppWinrt(Org::Webrtc::EventQueue const & value);
          static Org::Webrtc::EventQueue ToCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::EventQueue > const & value);
          static Org::Webrtc::EventQueue ToCppWinrt(Org::Webrtc::IEventQueue const & value);

          // ToCppWinrtInterface
          static Org::Webrtc::IEventQueue ToCppWinrtInterface(wrapper::org::webrtc::EventQueuePtr value);
          static Org::Webrtc::IEventQueue ToCppWinrtInterface(Org::Webrtc::EventQueue const & value);
          static Org::Webrtc::IEventQueue ToCppWinrtInterface(winrt::com_ptr< Org::Webrtc::implementation::EventQueue > const & value);
          static Org::Webrtc::IEventQueue ToCppWinrtInterface(Org::Webrtc::IEventQueue const & value);

          // FromCppWinrt
          static wrapper::org::webrtc::EventQueuePtr FromCppWinrt(wrapper::org::webrtc::EventQueuePtr value);
          static wrapper::org::webrtc::EventQueuePtr FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::EventQueue > const & value);
          static wrapper::org::webrtc::EventQueuePtr FromCppWinrt(Org::Webrtc::EventQueue const & value);
          static wrapper::org::webrtc::EventQueuePtr FromCppWinrt(Org::Webrtc::IEventQueue const & value);




        public:
          /// <summary>
          /// Cast from Org::Webrtc::IEventQueue to EventQueue
          /// </summary>
          static Org::Webrtc::EventQueue CastFromIEventQueue(Org::Webrtc::IEventQueue const & value);

          // ::org::webrtc::EventQueue

          /// <summary>
          /// The default windows message queue for the system GUI thread.
          /// </summary>
          static Org::Webrtc::IEventQueue GetDefaultForUi();

          /// <summary>
          /// Gets or sets the default system dispatcher object.
          /// </summary>
          static Org::Webrtc::IEventQueue Singleton();
          static void Singleton(Org::Webrtc::IEventQueue const & value);

        };

      } // namepsace implementation

      namespace factory_implementation {

        struct EventQueue : EventQueueT<EventQueue, implementation::EventQueue>
        {
        };

      } // namespace factory_implementation

    } // namespace Webrtc
  } // namespace Org
} // namespace winrt

#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
