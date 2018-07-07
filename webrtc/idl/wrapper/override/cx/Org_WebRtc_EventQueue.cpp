

#ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#include <wrapper/generated/cx/cx_Helpers.h>
#include "Org_Webrtc_EventQueue.h"

//------------------------------------------------------------------------------
::Org::WebRtc::EventQueue^ Org::WebRtc::EventQueue::ToCx(wrapper::org::webRtc::EventQueuePtr value)
{
  if (!value) return nullptr;
  auto result = ref new EventQueue(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueuePtr Org::WebRtc::EventQueue::FromCx(::Org::WebRtc::EventQueue^ value)
{
  if (nullptr == value) return wrapper::org::webRtc::EventQueuePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
Org::WebRtc::EventQueue::EventQueue(Platform::Object^ queue)
 : native_(wrapper::org::webRtc::EventQueue::wrapper_create())
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  native_->wrapper_init_org_webRtc_EventQueue(::Internal::Helper::FromCx(queue));
}

//------------------------------------------------------------------------------
::Org::WebRtc::EventQueue^ Org::WebRtc::EventQueue::GetDefaultForUi()
{
  ::Org::WebRtc::EventQueue^ result {};
  result = ::Internal::Helper::ToCx_Org_WebRtc_EventQueue(wrapper::org::webRtc::EventQueue::getDefaultForUi());
  return result;
}

//------------------------------------------------------------------------------
::Org::WebRtc::EventQueue^ Org::WebRtc::EventQueue::Singleton::get()
{
  return ::Internal::Helper::ToCx_Org_WebRtc_EventQueue(wrapper::org::webRtc::EventQueue::get_singleton());
}

//------------------------------------------------------------------------------
void Org::WebRtc::EventQueue::Singleton::set(::Org::WebRtc::EventQueue^ value)
{
  wrapper::org::webRtc::EventQueue::set_singleton(::Internal::Helper::FromCx_Org_WebRtc_EventQueue(value));
}

//------------------------------------------------------------------------------
Platform::Object^ Org::WebRtc::EventQueue::Queue::get()
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  return ::Internal::Helper::ToCx(native_->get_queue());
}


#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE
