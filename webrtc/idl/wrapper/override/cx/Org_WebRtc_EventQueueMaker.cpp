

#ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#include <wrapper/generated/cx/cx_Helpers.h>
#include <wrapper/generated/cx/Org_WebRtc_EventQueueMaker.h>
#include <wrapper/generated/cx/Org_WebRtc_EventQueue.h>

#include <wrapper/impl_org_webRtc_EventQueue.h>

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
::Org::WebRtc::EventQueue^ Org::WebRtc::EventQueueMaker::Bind(Windows::UI::Core::CoreDispatcher^ queue)
{
  ::Org::WebRtc::EventQueue^ result {};
  result = ::Internal::Helper::ToCx_Org_WebRtc_EventQueue(wrapper::impl::org::webRtc::EventQueue::toWrapper(queue));
  return result;
}

//------------------------------------------------------------------------------
Windows::UI::Core::CoreDispatcher^ Org::WebRtc::EventQueueMaker::Extract(::Org::WebRtc::EventQueue^ queue)
{
  return wrapper::impl::org::webRtc::EventQueue::toNative_cx(::Internal::Helper::FromCx_Org_WebRtc_EventQueue(queue));
}


#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER
