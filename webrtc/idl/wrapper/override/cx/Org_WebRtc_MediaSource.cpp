

#ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#include <wrapper/generated/cx/cx_Helpers.h>
#include <wrapper/generated/cx/Org_WebRtc_MediaSource.h>

#include <wrapper/impl_org_webRtc_MediaSource.h>

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
Org::WebRtc::MediaSource::MediaSource(Windows::Media::Core::IMediaSource^ source)
 : native_(wrapper::org::webRtc::MediaSource::wrapper_create())
{
  if (!native_) {throw ref new Platform::NullReferenceException();}

  auto wrapperSource = wrapper::impl::org::webRtc::MediaSource::toWrapper(source);
  ZS_ASSERT(wrapperSource);
  native_->wrapper_init_org_webRtc_MediaSource(wrapperSource->source_);
}

//------------------------------------------------------------------------------
Windows::Media::Core::IMediaSource^ Org::WebRtc::MediaSource::Source::get()
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  return  wrapper::impl::org::webRtc::MediaSource::toNative_cx(native_);
}


#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE
