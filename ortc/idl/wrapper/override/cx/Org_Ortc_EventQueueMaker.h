

#ifndef CX_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER

#pragma once

#include <wrapper/generated/cx/types.h>
#include <wrapper/generated/org_ortc_EventQueueMaker.h>

namespace Org {
  namespace Ortc {



    [Windows::Foundation::Metadata::WebHostHiddenAttribute]
    public ref class EventQueueMaker sealed
    {
    internal:
      wrapper::org::ortc::EventQueueMakerPtr native_;

      struct WrapperCreate {};
      EventQueueMaker(const WrapperCreate &) {}

      static EventQueueMaker^ ToCx(wrapper::org::ortc::EventQueueMakerPtr value);
      static wrapper::org::ortc::EventQueueMakerPtr FromCx(EventQueueMaker^ value);

    public:
      // ::org::ortc::EventQueueMaker

      static ::Org::Ortc::EventQueue^ BindQueue(Windows::UI::Core::CoreDispatcher^ queue);
      static Windows::UI::Core::CoreDispatcher^ ExtractQueue(::Org::Ortc::EventQueue^ queue);

    };

  } // namespace Ortc
} // namespace Org
#endif //ifndef CX_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER
