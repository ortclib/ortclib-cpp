
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO

#include "Org.WebRtc.VideoDeviceInfo.h"

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/override/cppwinrt/Org.WebRtc.VideoDeviceInfo.h>

#include <wrapper/impl_org_webRtc_VideoDeviceInfo.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrtImpl(wrapper::org::webRtc::VideoDeviceInfoPtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::WebRtc::implementation::VideoDeviceInfo>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrtImpl(Org::WebRtc::VideoDeviceInfo const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::VideoDeviceInfo>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrtImpl(Org::WebRtc::IVideoDeviceInfo const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::VideoDeviceInfo>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::WebRtc::VideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrt(wrapper::org::webRtc::VideoDeviceInfoPtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::VideoDeviceInfo {nullptr};
  return result.as< Org::WebRtc::VideoDeviceInfo >();
}

//------------------------------------------------------------------------------
Org::WebRtc::VideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrt(Org::WebRtc::VideoDeviceInfo const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::WebRtc::VideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > const & value)
{
  if (!value) return Org::WebRtc::VideoDeviceInfo {nullptr};
  return value.as< Org::WebRtc::VideoDeviceInfo >();
}

//------------------------------------------------------------------------------
Org::WebRtc::VideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrt(Org::WebRtc::IVideoDeviceInfo const & value)
{
  if (!value) return Org::WebRtc::VideoDeviceInfo {nullptr};
  return value.as< Org::WebRtc::VideoDeviceInfo >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IVideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrtInterface(wrapper::org::webRtc::VideoDeviceInfoPtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::IVideoDeviceInfo {nullptr};
  return result.as< Org::WebRtc::IVideoDeviceInfo >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IVideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrtInterface(Org::WebRtc::VideoDeviceInfo const & value)
{
  if (!value) return Org::WebRtc::IVideoDeviceInfo{nullptr};
  return value.as< Org::WebRtc::IVideoDeviceInfo >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IVideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > const & value)
{
  if (!value) return Org::WebRtc::IVideoDeviceInfo{nullptr};
  return value.as< Org::WebRtc::IVideoDeviceInfo >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IVideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::ToCppWinrtInterface(Org::WebRtc::IVideoDeviceInfo const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoDeviceInfoPtr Org::WebRtc::implementation::VideoDeviceInfo::FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::VideoDeviceInfo > const & value)
{
  if (!value) return wrapper::org::webRtc::VideoDeviceInfoPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoDeviceInfoPtr Org::WebRtc::implementation::VideoDeviceInfo::FromCppWinrt(Org::WebRtc::VideoDeviceInfo const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoDeviceInfoPtr Org::WebRtc::implementation::VideoDeviceInfo::FromCppWinrt(wrapper::org::webRtc::VideoDeviceInfoPtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoDeviceInfoPtr Org::WebRtc::implementation::VideoDeviceInfo::FromCppWinrt(Org::WebRtc::IVideoDeviceInfo const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::WebRtc::VideoDeviceInfo Org::WebRtc::implementation::VideoDeviceInfo::CastFromIVideoDeviceInfo(Org::WebRtc::IVideoDeviceInfo const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_WebRtc_VideoDeviceInfo(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webRtc::VideoDeviceInfo >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Windows::Devices::Enumeration::DeviceInformation Org::WebRtc::implementation::VideoDeviceInfo::Info()
{
  if (!native_) { throw hresult_error(E_POINTER); }

  Windows::Devices::Enumeration::DeviceInformation result{ nullptr };
  result = wrapper::impl::org::webRtc::VideoDeviceInfo::toNative_winrt(native_);
  return result;
}

#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_VIDEODEVICEINFO
