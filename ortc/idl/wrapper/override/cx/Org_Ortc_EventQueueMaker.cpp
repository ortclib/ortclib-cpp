
#ifndef CX_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER

#include <wrapper/generated/cx/cx_Helpers.h>

#include "Org_Ortc_EventQueueMaker.h"
#include "Org_Ortc_EventQueue.h"

//------------------------------------------------------------------------------
::Org::Ortc::EventQueueMaker^ Org::Ortc::EventQueueMaker::ToCx(wrapper::org::ortc::EventQueueMakerPtr value)
{
  if (!value) return nullptr;
  auto result = ref new EventQueueMaker(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueueMakerPtr Org::Ortc::EventQueueMaker::FromCx(::Org::Ortc::EventQueueMaker^ value)
{
  if (nullptr == value) return wrapper::org::ortc::EventQueueMakerPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
::Org::Ortc::EventQueue^ Org::Ortc::EventQueueMaker::BindQueue(Windows::UI::Core::CoreDispatcher^ queue)
{
  ::Org::Ortc::EventQueue^ result {};
  result = ::Org::Ortc::EventQueue::ToCx(wrapper::org::ortc::EventQueueMaker::bindQueue(queue));
  return result;
}

//------------------------------------------------------------------------------
Windows::UI::Core::CoreDispatcher^ Org::Ortc::EventQueueMaker::ExtractQueue(::Org::Ortc::EventQueue^ queue)
{
  Windows::UI::Core::CoreDispatcher^ result {};
  result = wrapper::org::ortc::EventQueueMaker::extractQueue(::Org::Ortc::EventQueue::FromCx(queue));
  return result;
}


#endif //ifndef CX_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER
