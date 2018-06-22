
#pragma once

#ifndef CX_USE_GENERATED_ORG_ORTC_EVENTQUEUE

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_ortc_EventQueue.h>

namespace Org {
  namespace Ortc {



    /// <summary>
    /// A message queue for receiving delegate events.
    /// </summary>
    public ref class EventQueue sealed
    {
    internal:
      wrapper::org::ortc::EventQueuePtr native_;

      struct WrapperCreate {};
      EventQueue(const WrapperCreate &) {}

      static EventQueue^ ToCx(wrapper::org::ortc::EventQueuePtr value);
      static wrapper::org::ortc::EventQueuePtr FromCx(EventQueue^ value);



    public:
      // ::org::ortc::EventQueue

      [Windows::Foundation::Metadata::DefaultOverloadAttribute]
      EventQueue(Windows::UI::Core::CoreDispatcher^ queue);
      /// <summary>
      /// The default windows message queue for the system GUI thread.
      /// </summary>
      static ::Org::Ortc::EventQueue^ GetDefaultForUi();

      /// <summary>
      /// Gets or sets the default system dispatcher object.
      /// </summary>
      static property ::Org::Ortc::EventQueue^ Singleton
      {
        ::Org::Ortc::EventQueue^ get();
        void set(::Org::Ortc::EventQueue^ value);
      }
      property Windows::UI::Core::CoreDispatcher^ Queue { Windows::UI::Core::CoreDispatcher^ get(); }

    };

  } // namespace Ortc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_ORTC_EVENTQUEUE
