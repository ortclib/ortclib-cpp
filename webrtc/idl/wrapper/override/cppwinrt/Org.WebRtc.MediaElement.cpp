
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/override/cppwinrt/Org.Webrtc.MediaElement.h>

#include <wrapper/impl_org_webRtc_MediaElement.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaElement > Org::Webrtc::implementation::MediaElement::ToCppWinrtImpl(wrapper::org::webRtc::MediaElementPtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Webrtc::implementation::MediaElement>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaElement > Org::Webrtc::implementation::MediaElement::ToCppWinrtImpl(Org::Webrtc::MediaElement const & value)
{
  winrt::com_ptr< Org::Webrtc::implementation::MediaElement > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Webrtc::implementation::MediaElement>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaElement > Org::Webrtc::implementation::MediaElement::ToCppWinrtImpl(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaElement > Org::Webrtc::implementation::MediaElement::ToCppWinrtImpl(Org::Webrtc::IMediaElement const & value)
{
  winrt::com_ptr< Org::Webrtc::implementation::MediaElement > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Webrtc::implementation::MediaElement>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrt(wrapper::org::webRtc::MediaElementPtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Webrtc::MediaElement >();
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrt(Org::Webrtc::MediaElement const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value)
{
  return value.as< Org::Webrtc::MediaElement >();
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrt(Org::Webrtc::IMediaElement const & value)
{
  return value.as< Org::Webrtc::MediaElement >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IMediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrtInterface(wrapper::org::webRtc::MediaElementPtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Webrtc::IMediaElement >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IMediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrtInterface(Org::Webrtc::MediaElement const & value)
{
  return value.as< Org::Webrtc::MediaElement >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IMediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrtInterface(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value)
{
  return value.as< Org::Webrtc::MediaElement >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IMediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrtInterface(Org::Webrtc::IMediaElement const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::Webrtc::implementation::MediaElement::FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value)
{
  if (!value) return wrapper::org::webRtc::MediaElementPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::Webrtc::implementation::MediaElement::FromCppWinrt(Org::Webrtc::MediaElement const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::Webrtc::implementation::MediaElement::FromCppWinrt(wrapper::org::webRtc::MediaElementPtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::Webrtc::implementation::MediaElement::FromCppWinrt(Org::Webrtc::IMediaElement const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Webrtc::implementation::MediaElement::MediaElement()
  : native_(wrapper::org::webRtc::MediaElement::wrapper_create())
{
  native_->wrapper_init_org_webRtc_MediaElement();
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaElement Org::Webrtc::implementation::MediaElement::CastFromIMediaElement(Org::Webrtc::IMediaElement const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Webrtc_MediaElement(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webRtc::MediaElement >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Windows::UI::Xaml::Controls::MediaElement Org::Webrtc::implementation::MediaElement::Element()
{
  if (!native_) {throw hresult_error(E_POINTER);}

  Windows::UI::Xaml::Controls::MediaElement result{ nullptr };
  result = wrapper::impl::org::webRtc::MediaElement::toNative_winrt(native_);
  return result;
}

//------------------------------------------------------------------------------
void Org::Webrtc::implementation::MediaElement::Element(Windows::UI::Xaml::Controls::MediaElement const & element)
{
  native_ = wrapper::impl::org::webRtc::MediaElement::toWrapper(element);
}

#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
