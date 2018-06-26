
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/Ortc/EventQueueMaker.g.h"
#include <wrapper/generated/org_ortc_EventQueueMaker.h>

namespace winrt {
  namespace Org {
    namespace Ortc {
      namespace implementation {


        struct EventQueueMaker : EventQueueMakerT<EventQueueMaker>
        {
          // internal
          wrapper::org::ortc::EventQueueMakerPtr native_;

          struct WrapperCreate {};
          EventQueueMaker(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::Ortc::implementation::EventQueueMaker > ToCppWinrtImpl(wrapper::org::ortc::EventQueueMakerPtr value);
          static winrt::com_ptr< Org::Ortc::implementation::EventQueueMaker > ToCppWinrtImpl(Org::Ortc::EventQueueMaker const & value);
          static winrt::com_ptr< Org::Ortc::implementation::EventQueueMaker > ToCppWinrtImpl(winrt::com_ptr< Org::Ortc::implementation::EventQueueMaker > const & value);
          static winrt::com_ptr< Org::Ortc::implementation::EventQueueMaker > ToCppWinrtImpl(Org::Ortc::IEventQueueMaker const & value);

          // ToCppWinrt
          static Org::Ortc::EventQueueMaker ToCppWinrt(wrapper::org::ortc::EventQueueMakerPtr value);
          static Org::Ortc::EventQueueMaker ToCppWinrt(Org::Ortc::EventQueueMaker const & value);
          static Org::Ortc::EventQueueMaker ToCppWinrt(winrt::com_ptr< Org::Ortc::implementation::EventQueueMaker > const & value);
          static Org::Ortc::EventQueueMaker ToCppWinrt(Org::Ortc::IEventQueueMaker const & value);

          // ToCppWinrtInterface
          static Org::Ortc::IEventQueueMaker ToCppWinrtInterface(wrapper::org::ortc::EventQueueMakerPtr value);
          static Org::Ortc::IEventQueueMaker ToCppWinrtInterface(Org::Ortc::EventQueueMaker const & value);
          static Org::Ortc::IEventQueueMaker ToCppWinrtInterface(winrt::com_ptr< Org::Ortc::implementation::EventQueueMaker > const & value);
          static Org::Ortc::IEventQueueMaker ToCppWinrtInterface(Org::Ortc::IEventQueueMaker const & value);

          // FromCppWinrt
          static wrapper::org::ortc::EventQueueMakerPtr FromCppWinrt(wrapper::org::ortc::EventQueueMakerPtr value);
          static wrapper::org::ortc::EventQueueMakerPtr FromCppWinrt(winrt::com_ptr< Org::Ortc::implementation::EventQueueMaker > const & value);
          static wrapper::org::ortc::EventQueueMakerPtr FromCppWinrt(Org::Ortc::EventQueueMaker const & value);
          static wrapper::org::ortc::EventQueueMakerPtr FromCppWinrt(Org::Ortc::IEventQueueMaker const & value);




        public:
          /// <summary>
          /// Cast from Org::Ortc::IEventQueueMaker to EventQueueMaker
          /// </summary>
          static Org::Ortc::EventQueueMaker CastFromIEventQueueMaker(Org::Ortc::IEventQueueMaker const & value);

          // ::org::ortc::EventQueueMaker

          /// <summary>
          /// Creates an event queue object from a native queue type.
          /// </summary>
          static Org::Ortc::IEventQueue BindQueue(Windows::UI::Core::CoreDispatcher const & queue);
          /// <summary>
          /// Extracts a native queue from an event queue object.
          /// </summary>
          static Windows::UI::Core::CoreDispatcher ExtractQueue(Org::Ortc::IEventQueue const & queue);


        };

      } // namepsace implementation

      namespace factory_implementation {

        struct EventQueueMaker : EventQueueMakerT<EventQueueMaker, implementation::EventQueueMaker>
        {
        };

      } // namespace factory_implementation

    } // namespace Ortc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER
