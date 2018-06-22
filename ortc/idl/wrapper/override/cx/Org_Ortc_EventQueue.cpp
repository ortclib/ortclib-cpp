

#ifndef CX_USE_GENERATED_ORG_ORTC_EVENTQUEUE

#include <wrapper/generated/cx/cx_Helpers.h>
#include "Org_Ortc_EventQueue.h"

//------------------------------------------------------------------------------
::Org::Ortc::EventQueue^ Org::Ortc::EventQueue::ToCx(wrapper::org::ortc::EventQueuePtr value)
{
  if (!value) return nullptr;
  auto result = ref new EventQueue(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::EventQueuePtr Org::Ortc::EventQueue::FromCx(::Org::Ortc::EventQueue^ value)
{
  if (nullptr == value) return wrapper::org::ortc::EventQueuePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
Org::Ortc::EventQueue::EventQueue(Windows::UI::Core::CoreDispatcher^ queue)
 : native_(wrapper::org::ortc::EventQueue::wrapper_create())
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  native_->wrapper_init_org_ortc_EventQueue(queue);
}

//------------------------------------------------------------------------------
::Org::Ortc::EventQueue^ Org::Ortc::EventQueue::GetDefaultForUi()
{
  ::Org::Ortc::EventQueue^ result {};
  result = ::Internal::Helper::ToCx_Org_Ortc_EventQueue(wrapper::org::ortc::EventQueue::getDefaultForUi());
  return result;
}

//------------------------------------------------------------------------------
::Org::Ortc::EventQueue^ Org::Ortc::EventQueue::Singleton::get()
{
  return ::Internal::Helper::ToCx_Org_Ortc_EventQueue(wrapper::org::ortc::EventQueue::get_singleton());
}

//------------------------------------------------------------------------------
void Org::Ortc::EventQueue::Singleton::set(::Org::Ortc::EventQueue^ value)
{
  wrapper::org::ortc::EventQueue::set_singleton(::Internal::Helper::FromCx_Org_Ortc_EventQueue(value));
}

//------------------------------------------------------------------------------
Windows::UI::Core::CoreDispatcher^ Org::Ortc::EventQueue::Queue::get()
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  return native_->get_queue();
}


#endif //ifndef CX_USE_GENERATED_ORG_ORTC_EVENTQUEUE
