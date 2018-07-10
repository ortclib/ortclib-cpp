

#ifndef CX_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO

#include <wrapper/generated/cx/cx_Helpers.h>
#include <wrapper/generated/cx/Org_WebRtc_VideoDeviceInfo.h>

#include <wrapper/impl_org_webRtc_VideoDeviceInfo.h>

//------------------------------------------------------------------------------
::Org::WebRtc::VideoDeviceInfo^ Org::WebRtc::VideoDeviceInfo::ToCx(wrapper::org::webRtc::VideoDeviceInfoPtr value)
{
  if (!value) return nullptr;
  auto result = ref new VideoDeviceInfo(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoDeviceInfoPtr Org::WebRtc::VideoDeviceInfo::FromCx(::Org::WebRtc::VideoDeviceInfo^ value)
{
  if (nullptr == value) return wrapper::org::webRtc::VideoDeviceInfoPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
Windows::Devices::Enumeration::DeviceInformation^ Org::WebRtc::VideoDeviceInfo::Info::get()
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  return wrapper::impl::org::webRtc::VideoDeviceInfo::toNative_cx(native_);
}


#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO
