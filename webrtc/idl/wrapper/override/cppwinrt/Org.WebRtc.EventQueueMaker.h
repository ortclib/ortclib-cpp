
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/Webrtc/EventQueueMaker.g.h"
#include <wrapper/generated/org_webRtc_EventQueueMaker.h>

namespace winrt {
  namespace Org {
    namespace Webrtc {
      namespace implementation {


        struct EventQueueMaker : EventQueueMakerT<EventQueueMaker>
        {
          // internal
          wrapper::org::webRtc::EventQueueMakerPtr native_;

          struct WrapperCreate {};
          EventQueueMaker(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > ToCppWinrtImpl(wrapper::org::webRtc::EventQueueMakerPtr value);
          static winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > ToCppWinrtImpl(Org::Webrtc::EventQueueMaker const & value);
          static winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > ToCppWinrtImpl(winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > const & value);
          static winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > ToCppWinrtImpl(Org::Webrtc::IEventQueueMaker const & value);

          // ToCppWinrt
          static Org::Webrtc::EventQueueMaker ToCppWinrt(wrapper::org::webRtc::EventQueueMakerPtr value);
          static Org::Webrtc::EventQueueMaker ToCppWinrt(Org::Webrtc::EventQueueMaker const & value);
          static Org::Webrtc::EventQueueMaker ToCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > const & value);
          static Org::Webrtc::EventQueueMaker ToCppWinrt(Org::Webrtc::IEventQueueMaker const & value);

          // ToCppWinrtInterface
          static Org::Webrtc::IEventQueueMaker ToCppWinrtInterface(wrapper::org::webRtc::EventQueueMakerPtr value);
          static Org::Webrtc::IEventQueueMaker ToCppWinrtInterface(Org::Webrtc::EventQueueMaker const & value);
          static Org::Webrtc::IEventQueueMaker ToCppWinrtInterface(winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > const & value);
          static Org::Webrtc::IEventQueueMaker ToCppWinrtInterface(Org::Webrtc::IEventQueueMaker const & value);

          // FromCppWinrt
          static wrapper::org::webRtc::EventQueueMakerPtr FromCppWinrt(wrapper::org::webRtc::EventQueueMakerPtr value);
          static wrapper::org::webRtc::EventQueueMakerPtr FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::EventQueueMaker > const & value);
          static wrapper::org::webRtc::EventQueueMakerPtr FromCppWinrt(Org::Webrtc::EventQueueMaker const & value);
          static wrapper::org::webRtc::EventQueueMakerPtr FromCppWinrt(Org::Webrtc::IEventQueueMaker const & value);




        public:
          /// <summary>
          /// Cast from Org::Webrtc::IEventQueueMaker to EventQueueMaker
          /// </summary>
          static Org::Webrtc::EventQueueMaker CastFromIEventQueueMaker(Org::Webrtc::IEventQueueMaker const & value);

          // ::org::webrtc::EventQueueMaker

          /// <summary>
          /// Creates an event queue object from a native queue type.
          /// </summary>
          static Org::Webrtc::IEventQueue BindQueue(Windows::UI::Core::CoreDispatcher const & queue);
          /// <summary>
          /// Extracts a native queue from an event queue object.
          /// </summary>
          static Windows::UI::Core::CoreDispatcher ExtractQueue(Org::Webrtc::IEventQueue const & queue);


        };

      } // namepsace implementation

      namespace factory_implementation {

        struct EventQueueMaker : EventQueueMakerT<EventQueueMaker, implementation::EventQueueMaker>
        {
        };

      } // namespace factory_implementation

    } // namespace Webrtc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
