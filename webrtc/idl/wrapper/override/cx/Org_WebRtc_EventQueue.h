

#ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#pragma once

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_webRtc_EventQueue.h>

namespace Org {
  namespace WebRtc {



    /// <summary>
    /// A message queue for receiving delegate events.
    /// </summary>
    public ref class EventQueue sealed
    {
    internal:
      wrapper::org::webRtc::EventQueuePtr native_;

      struct WrapperCreate {};
      EventQueue(const WrapperCreate &) {}

      static EventQueue^ ToCx(wrapper::org::webRtc::EventQueuePtr value);
      static wrapper::org::webRtc::EventQueuePtr FromCx(EventQueue^ value);



    public:
      // ::org::webRtc::EventQueue

#if 0
      /// <summary>
      /// Constructs an event queue object from a native queue type.
      /// </summary>
      [Windows::Foundation::Metadata::DefaultOverloadAttribute, Windows::Foundation::Metadata::OverloadAttribute("EventQueueWithNativeQueue")]
      EventQueue(Platform::Object^ queue);
#endif //0
      /// <summary>
      /// The default windows message queue for the system GUI thread.
      /// </summary>
      static ::Org::WebRtc::EventQueue^ GetDefaultForUi();

      /// <summary>
      /// Gets or sets the default system dispatcher object.
      /// </summary>
      static property ::Org::WebRtc::EventQueue^ Singleton
      {
        ::Org::WebRtc::EventQueue^ get();
        void set(::Org::WebRtc::EventQueue^ value);
      }
#if 0
      /// <summary>
      /// Gets a native queue from an event queue object.
      /// </summary>
      property Platform::Object^ Queue { Platform::Object^ get(); }
#endif //0

    };

  } // namespace WebRtc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
