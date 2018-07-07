

#ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#include <wrappers/generated/cx/cx_Helpers.h>
#include <wrappers/generated/cx/Org_WebRtc_MediaSource.h>

//------------------------------------------------------------------------------
::Org::WebRtc::MediaSource^ Org::WebRtc::MediaSource::ToCx(wrapper::org::webRtc::MediaSourcePtr value)
{
  if (!value) return nullptr;
  auto result = ref new MediaSource(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaSourcePtr Org::WebRtc::MediaSource::FromCx(::Org::WebRtc::MediaSource^ value)
{
  if (nullptr == value) return wrapper::org::webRtc::MediaSourcePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaSource::MediaSource(Platform::Object^ queue)
 : native_(wrapper::org::webRtc::MediaSource::wrapper_create())
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  native_->wrapper_init_org_webRtc_MediaSource(::Internal::Helper::FromCx(queue));
}

//------------------------------------------------------------------------------
Platform::Object^ Org::WebRtc::MediaSource::Source::get()
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  return ::Internal::Helper::ToCx(native_->get_source());
}

//------------------------------------------------------------------------------
void Org::WebRtc::MediaSource::Source::set(Platform::Object^ value)
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  native_->set_source(::Internal::Helper::FromCx(value));
}


#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE
