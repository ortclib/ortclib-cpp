
#include "Org_Ortc_MediaControl.h"

#include <modules/video_capture/windows/video_capture_winuwp.h>

//------------------------------------------------------------------------------
::Org::Ortc::MediaControl^ Org::Ortc::MediaControl::ToCx(wrapper::org::ortc::MediaControlPtr value)
{
  if (!value) return nullptr;
  auto result = ref new MediaControl(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaControlPtr Org::Ortc::MediaControl::FromCx(::Org::Ortc::MediaControl^ value)
{
  if (nullptr == value) return wrapper::org::ortc::MediaControlPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
Windows::Graphics::Display::DisplayOrientations Org::Ortc::MediaControl::DisplayOrientation::get()
{
  //return webrtc::videocapturemodule::AppStateDispatcher::Instance()->GetOrientation();
  return Windows::Graphics::Display::DisplayOrientations::Landscape;
}

//------------------------------------------------------------------------------
void Org::Ortc::MediaControl::DisplayOrientation::set(Windows::Graphics::Display::DisplayOrientations value)
{
  //webrtc::videocapturemodule::AppStateDispatcher::Instance()->DisplayOrientationChanged(value);
}
