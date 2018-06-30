
#pragma once

#ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_webRtc_EventQueue.h>

namespace Org {
  namespace Webrtc {



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
      // ::org::webrtc::EventQueue

      /// <summary>
      /// The default windows message queue for the system GUI thread.
      /// </summary>
      static ::Org::Webrtc::EventQueue^ GetDefaultForUi();

      /// <summary>
      /// Gets or sets the default system dispatcher object.
      /// </summary>
      static property ::Org::Webrtc::EventQueue^ Singleton
      {
        ::Org::Webrtc::EventQueue^ get();
        void set(::Org::Webrtc::EventQueue^ value);
      }

    };

  } // namespace Webrtc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
