
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#include <wrapper/generated/cppwinrt/types.h>

#include "Org/WebRtc/EventQueueMaker.g.h"
#include <wrapper/generated/org_webRtc_EventQueueMaker.h>

namespace winrt {
  namespace Org {
    namespace WebRtc {
      namespace implementation {


        struct EventQueueMaker : EventQueueMakerT<EventQueueMaker>
        {
          // internal
          wrapper::org::webRtc::EventQueueMakerPtr native_;

          struct WrapperCreate {};
          EventQueueMaker(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > ToCppWinrtImpl(wrapper::org::webRtc::EventQueueMakerPtr value);
          static winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > ToCppWinrtImpl(Org::WebRtc::EventQueueMaker const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > const & value);
          static winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > ToCppWinrtImpl(Org::WebRtc::IEventQueueMaker const & value);

          // ToCppWinrt
          static Org::WebRtc::EventQueueMaker ToCppWinrt(wrapper::org::webRtc::EventQueueMakerPtr value);
          static Org::WebRtc::EventQueueMaker ToCppWinrt(Org::WebRtc::EventQueueMaker const & value);
          static Org::WebRtc::EventQueueMaker ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > const & value);
          static Org::WebRtc::EventQueueMaker ToCppWinrt(Org::WebRtc::IEventQueueMaker const & value);

          // ToCppWinrtInterface
          static Org::WebRtc::IEventQueueMaker ToCppWinrtInterface(wrapper::org::webRtc::EventQueueMakerPtr value);
          static Org::WebRtc::IEventQueueMaker ToCppWinrtInterface(Org::WebRtc::EventQueueMaker const & value);
          static Org::WebRtc::IEventQueueMaker ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > const & value);
          static Org::WebRtc::IEventQueueMaker ToCppWinrtInterface(Org::WebRtc::IEventQueueMaker const & value);

          // FromCppWinrt
          static wrapper::org::webRtc::EventQueueMakerPtr FromCppWinrt(wrapper::org::webRtc::EventQueueMakerPtr value);
          static wrapper::org::webRtc::EventQueueMakerPtr FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::EventQueueMaker > const & value);
          static wrapper::org::webRtc::EventQueueMakerPtr FromCppWinrt(Org::WebRtc::EventQueueMaker const & value);
          static wrapper::org::webRtc::EventQueueMakerPtr FromCppWinrt(Org::WebRtc::IEventQueueMaker const & value);




        public:
          /// <summary>
          /// Cast from Org::WebRtc::IEventQueueMaker to EventQueueMaker
          /// </summary>
          static Org::WebRtc::EventQueueMaker CastFromIEventQueueMaker(Org::WebRtc::IEventQueueMaker const & value);

          // ::org::webrtc::EventQueueMaker

          /// <summary>
          /// Creates an event queue object from a native queue type.
          /// </summary>
          static Org::WebRtc::IEventQueue BindQueue(Windows::UI::Core::CoreDispatcher const & queue);
          /// <summary>
          /// Extracts a native queue from an event queue object.
          /// </summary>
          static Windows::UI::Core::CoreDispatcher ExtractQueue(Org::WebRtc::IEventQueue const & queue);


        };

      } // namepsace implementation

      namespace factory_implementation {

        struct EventQueueMaker : EventQueueMakerT<EventQueueMaker, implementation::EventQueueMaker>
        {
        };

      } // namespace factory_implementation

    } // namespace WebRtc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
