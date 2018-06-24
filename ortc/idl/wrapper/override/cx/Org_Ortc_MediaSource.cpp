
#include "Org_Ortc_MediaSource.h"
//#include "cx_custom_WebRtcMediaSource.h"

#include <wrapper/generated/cx/cx_Helpers.h>

//------------------------------------------------------------------------------
::Org::Ortc::MediaSource^ Org::Ortc::MediaSource::ToCx(wrapper::org::ortc::MediaSourcePtr value)
{
  if (!value) return nullptr;
  auto result = ref new MediaSource(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr Org::Ortc::MediaSource::FromCx(::Org::Ortc::MediaSource^ value)
{
  if (nullptr == value) return wrapper::org::ortc::MediaSourcePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
Windows::Media::Core::IMediaSource^ Org::Ortc::MediaSource::Source::get()
{
  return nullptr;
}
