

#ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
#include <wrappers/generated/cx/cx_Helpers.h>
#include <wrappers/generated/cx/Org_WebRtc_EventQueueMaker.h>
#include <wrappers/generated/cx/Org_WebRtc_EventQueue.h>

//------------------------------------------------------------------------------
::Org::WebRtc::EventQueueMaker^ Org::WebRtc::EventQueueMaker::ToCx(wrapper::org::webRtc::EventQueueMakerPtr value)
{
  if (!value) return nullptr;
  auto result = ref new EventQueueMaker(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::EventQueueMakerPtr Org::WebRtc::EventQueueMaker::FromCx(::Org::WebRtc::EventQueueMaker^ value)
{
  if (nullptr == value) return wrapper::org::webRtc::EventQueueMakerPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
::Org::WebRtc::EventQueue^ Org::WebRtc::EventQueueMaker::Bind(Platform::Object^ queue)
{
  ::Org::WebRtc::EventQueue^ result {};
  result = ::Internal::Helper::ToCx_Org_WebRtc_EventQueue(wrapper::org::webRtc::EventQueueMaker::bind(::Internal::Helper::FromCx(queue)));
  return result;
}

//------------------------------------------------------------------------------
Platform::Object^ Org::WebRtc::EventQueueMaker::Extract(::Org::WebRtc::EventQueue^ queue)
{
  Platform::Object^ result {};
  result = ::Internal::Helper::ToCx(wrapper::org::webRtc::EventQueueMaker::extract(::Internal::Helper::FromCx_Org_WebRtc_EventQueue(queue)));
  return result;
}


#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
