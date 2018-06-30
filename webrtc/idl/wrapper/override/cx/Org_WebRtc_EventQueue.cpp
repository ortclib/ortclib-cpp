

#ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#include <wrapper/generated/cx/cx_Helpers.h>
#include "Org_Webrtc_EventQueue.h"

//------------------------------------------------------------------------------
::Org::Webrtc::EventQueue^ Org::Webrtc::EventQueue::ToCx(wrapper::org::webrtc::EventQueuePtr value)
{
  if (!value) return nullptr;
  auto result = ref new EventQueue(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::EventQueuePtr Org::Webrtc::EventQueue::FromCx(::Org::Webrtc::EventQueue^ value)
{
  if (nullptr == value) return wrapper::org::webrtc::EventQueuePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
::Org::Webrtc::EventQueue^ Org::Webrtc::EventQueue::GetDefaultForUi()
{
  ::Org::Webrtc::EventQueue^ result {};
  result = ::Internal::Helper::ToCx_Org_Webrtc_EventQueue(wrapper::org::webrtc::EventQueue::getDefaultForUi());
  return result;
}

//------------------------------------------------------------------------------
::Org::Webrtc::EventQueue^ Org::Webrtc::EventQueue::Singleton::get()
{
  return ::Internal::Helper::ToCx_Org_Webrtc_EventQueue(wrapper::org::webrtc::EventQueue::get_singleton());
}

//------------------------------------------------------------------------------
void Org::Webrtc::EventQueue::Singleton::set(::Org::Webrtc::EventQueue^ value)
{
  wrapper::org::webrtc::EventQueue::set_singleton(::Internal::Helper::FromCx_Org_Webrtc_EventQueue(value));
}


#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
