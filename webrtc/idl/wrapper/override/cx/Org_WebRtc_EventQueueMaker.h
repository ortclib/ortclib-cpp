

#ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#pragma once

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_webRtc_EventQueueMaker.h>

namespace Org {
  namespace WebRtc {



    [Windows::Foundation::Metadata::WebHostHiddenAttribute]
    public ref class EventQueueMaker sealed
    {
    internal:
      wrapper::org::webRtc::EventQueueMakerPtr native_;

      struct WrapperCreate {};
      EventQueueMaker(const WrapperCreate &) {}

      static EventQueueMaker^ ToCx(wrapper::org::webRtc::EventQueueMakerPtr value);
      static wrapper::org::webRtc::EventQueueMakerPtr FromCx(EventQueueMaker^ value);



    public:
      // ::org::webRtc::EventQueueMaker

      /// <summary>
      /// Creates an event queue object from a native queue type.
      /// </summary>
      static ::Org::WebRtc::EventQueue^ Bind(Platform::Object^ queue);
      /// <summary>
      /// Extracts a native queue from an event queue object.
      /// </summary>
      static Platform::Object^ Extract(::Org::WebRtc::EventQueue^ queue);

      static ::Org::Ortc::EventQueue^ BindQueue(Windows::UI::Core::CoreDispatcher^ queue);
      static Windows::UI::Core::CoreDispatcher^ ExtractQueue(::Org::Ortc::EventQueue^ queue);

    };

  } // namespace WebRtc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
