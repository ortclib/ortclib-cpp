
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/generated/cppwinrt/Org.WebRtc.MediaElementMaker.h>
#include <wrapper/generated/cppwinrt/Org.WebRtc.MediaElement.h>

#include <wrapper/impl_org_webRtc_MediaElement.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > Org::WebRtc::implementation::MediaElementMaker::ToCppWinrtImpl(wrapper::org::webRtc::MediaElementMakerPtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::WebRtc::implementation::MediaElementMaker>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > Org::WebRtc::implementation::MediaElementMaker::ToCppWinrtImpl(Org::WebRtc::MediaElementMaker const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::MediaElementMaker>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > Org::WebRtc::implementation::MediaElementMaker::ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > Org::WebRtc::implementation::MediaElementMaker::ToCppWinrtImpl(Org::WebRtc::IMediaElementMaker const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::MediaElementMaker>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElementMaker Org::WebRtc::implementation::MediaElementMaker::ToCppWinrt(wrapper::org::webRtc::MediaElementMakerPtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::MediaElementMaker {nullptr};
  return result.as< Org::WebRtc::MediaElementMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElementMaker Org::WebRtc::implementation::MediaElementMaker::ToCppWinrt(Org::WebRtc::MediaElementMaker const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElementMaker Org::WebRtc::implementation::MediaElementMaker::ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > const & value)
{
  if (!value) return Org::WebRtc::MediaElementMaker {nullptr};
  return value.as< Org::WebRtc::MediaElementMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElementMaker Org::WebRtc::implementation::MediaElementMaker::ToCppWinrt(Org::WebRtc::IMediaElementMaker const & value)
{
  if (!value) return Org::WebRtc::MediaElementMaker {nullptr};
  return value.as< Org::WebRtc::MediaElementMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElementMaker Org::WebRtc::implementation::MediaElementMaker::ToCppWinrtInterface(wrapper::org::webRtc::MediaElementMakerPtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::IMediaElementMaker {nullptr};
  return result.as< Org::WebRtc::IMediaElementMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElementMaker Org::WebRtc::implementation::MediaElementMaker::ToCppWinrtInterface(Org::WebRtc::MediaElementMaker const & value)
{
  if (!value) return Org::WebRtc::IMediaElementMaker {nullptr};
  return value.as< Org::WebRtc::IMediaElementMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElementMaker Org::WebRtc::implementation::MediaElementMaker::ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > const & value)
{
  if (!value) return Org::WebRtc::IMediaElementMaker {nullptr};
  return value.as< Org::WebRtc::IMediaElementMaker >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElementMaker Org::WebRtc::implementation::MediaElementMaker::ToCppWinrtInterface(Org::WebRtc::IMediaElementMaker const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementMakerPtr Org::WebRtc::implementation::MediaElementMaker::FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaElementMaker > const & value)
{
  if (!value) return wrapper::org::webRtc::MediaElementMakerPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementMakerPtr Org::WebRtc::implementation::MediaElementMaker::FromCppWinrt(Org::WebRtc::MediaElementMaker const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementMakerPtr Org::WebRtc::implementation::MediaElementMaker::FromCppWinrt(wrapper::org::webRtc::MediaElementMakerPtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementMakerPtr Org::WebRtc::implementation::MediaElementMaker::FromCppWinrt(Org::WebRtc::IMediaElementMaker const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElementMaker Org::WebRtc::implementation::MediaElementMaker::Cast(Org::WebRtc::IMediaElementMaker const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_WebRtc_MediaElementMaker(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webRtc::MediaElementMaker >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElement Org::WebRtc::implementation::MediaElementMaker::Bind(Windows::UI::Xaml::Controls::MediaElement const & element)
{
  Org::WebRtc::IMediaElement result {nullptr};
  result = Org::WebRtc::implementation::MediaElement::ToCppWinrtInterface(wrapper::impl::org::webRtc::MediaElement::toWrapper(element));
  return result;
}

//------------------------------------------------------------------------------
Windows::UI::Xaml::Controls::MediaElement Org::WebRtc::implementation::MediaElementMaker::Extract(Org::WebRtc::IMediaElement const & element)
{
  Windows::UI::Xaml::Controls::MediaElement result {nullptr};
  result = wrapper::impl::org::webRtc::MediaElement::toNative_winrt(Org::WebRtc::implementation::MediaElement::FromCppWinrt(element));
  return result;
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER
