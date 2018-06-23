
#pragma once

#ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_EVENTQUEUE

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/Ortc/EventQueue.g.h"
#include <wrapper/generated/org_ortc_EventQueue.h>

namespace winrt {
  namespace Org {
    namespace Ortc {
      namespace implementation {


        /// <summary>
        /// A message queue for receiving delegate events.
        /// </summary>
        struct EventQueue : EventQueueT<EventQueue>
        {
          // internal
          wrapper::org::ortc::EventQueuePtr native_;

          struct WrapperCreate {};
          EventQueue(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::Ortc::implementation::EventQueue > ToCppWinrtImpl(wrapper::org::ortc::EventQueuePtr value);
          static winrt::com_ptr< Org::Ortc::implementation::EventQueue > ToCppWinrtImpl(Org::Ortc::EventQueue const & value);
          static winrt::com_ptr< Org::Ortc::implementation::EventQueue > ToCppWinrtImpl(winrt::com_ptr< Org::Ortc::implementation::EventQueue > const & value);
          static winrt::com_ptr< Org::Ortc::implementation::EventQueue > ToCppWinrtImpl(Org::Ortc::IEventQueue const & value);

          // ToCppWinrt
          static Org::Ortc::EventQueue ToCppWinrt(wrapper::org::ortc::EventQueuePtr value);
          static Org::Ortc::EventQueue ToCppWinrt(Org::Ortc::EventQueue const & value);
          static Org::Ortc::EventQueue ToCppWinrt(winrt::com_ptr< Org::Ortc::implementation::EventQueue > const & value);
          static Org::Ortc::EventQueue ToCppWinrt(Org::Ortc::IEventQueue const & value);

          // ToCppWinrtInterface
          static Org::Ortc::IEventQueue ToCppWinrtInterface(wrapper::org::ortc::EventQueuePtr value);
          static Org::Ortc::IEventQueue ToCppWinrtInterface(Org::Ortc::EventQueue const & value);
          static Org::Ortc::IEventQueue ToCppWinrtInterface(winrt::com_ptr< Org::Ortc::implementation::EventQueue > const & value);
          static Org::Ortc::IEventQueue ToCppWinrtInterface(Org::Ortc::IEventQueue const & value);

          // FromCppWinrt
          static wrapper::org::ortc::EventQueuePtr FromCppWinrt(wrapper::org::ortc::EventQueuePtr value);
          static wrapper::org::ortc::EventQueuePtr FromCppWinrt(winrt::com_ptr< Org::Ortc::implementation::EventQueue > const & value);
          static wrapper::org::ortc::EventQueuePtr FromCppWinrt(Org::Ortc::EventQueue const & value);
          static wrapper::org::ortc::EventQueuePtr FromCppWinrt(Org::Ortc::IEventQueue const & value);




        public:
          /// <summary>
          /// Cast from Org::Ortc::IEventQueue to EventQueue
          /// </summary>
          static Org::Ortc::EventQueue CastFromIEventQueue(Org::Ortc::IEventQueue const & value);

          // ::org::ortc::EventQueue

          /// <summary>
          /// The default windows message queue for the system GUI thread.
          /// </summary>
          static Org::Ortc::IEventQueue GetDefaultForUi();

          /// <summary>
          /// Gets or sets the default system dispatcher object.
          /// </summary>
          static Org::Ortc::IEventQueue Singleton();
          static void Singleton(Org::Ortc::IEventQueue const & value);

        };

      } // namepsace implementation

      namespace factory_implementation {

        struct EventQueue : EventQueueT<EventQueue, implementation::EventQueue>
        {
        };

      } // namespace factory_implementation

    } // namespace Ortc
  } // namespace Org
} // namespace winrt

#endif //ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_EVENTQUEUE
