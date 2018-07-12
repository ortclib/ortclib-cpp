
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/override/cppwinrt/MediaElement.h>

#include <wrapper/impl_org_webRtc_MediaElement.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaElement > Org::WebRtc::implementation::MediaElement::ToCppWinrtImpl(wrapper::org::webRtc::MediaElementPtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::WebRtc::implementation::MediaElement>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaElement > Org::WebRtc::implementation::MediaElement::ToCppWinrtImpl(Org::WebRtc::MediaElement const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::MediaElement > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::MediaElement>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaElement > Org::WebRtc::implementation::MediaElement::ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::MediaElement > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaElement > Org::WebRtc::implementation::MediaElement::ToCppWinrtImpl(Org::WebRtc::IMediaElement const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::MediaElement > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::MediaElement>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElement Org::WebRtc::implementation::MediaElement::ToCppWinrt(wrapper::org::webRtc::MediaElementPtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::MediaElement {nullptr};
  return result.as< Org::WebRtc::MediaElement >();
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElement Org::WebRtc::implementation::MediaElement::ToCppWinrt(Org::WebRtc::MediaElement const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElement Org::WebRtc::implementation::MediaElement::ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaElement > const & value)
{
  if (!value) return Org::WebRtc::MediaElement {nullptr};
  return value.as< Org::WebRtc::MediaElement >();
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElement Org::WebRtc::implementation::MediaElement::ToCppWinrt(Org::WebRtc::IMediaElement const & value)
{
  if (!value) return Org::WebRtc::MediaElement {nullptr};
  return value.as< Org::WebRtc::MediaElement >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElement Org::WebRtc::implementation::MediaElement::ToCppWinrtInterface(wrapper::org::webRtc::MediaElementPtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::IMediaElement {nullptr};
  return result.as< Org::WebRtc::IMediaElement >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElement Org::WebRtc::implementation::MediaElement::ToCppWinrtInterface(Org::WebRtc::MediaElement const & value)
{
  if (!value) return Org::WebRtc::IMediaElement {nullptr};
  return value.as< Org::WebRtc::IMediaElement >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElement Org::WebRtc::implementation::MediaElement::ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::MediaElement > const & value)
{
  if (!value) return Org::WebRtc::IMediaElement {nullptr};
  return value.as< Org::WebRtc::IMediaElement >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaElement Org::WebRtc::implementation::MediaElement::ToCppWinrtInterface(Org::WebRtc::IMediaElement const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::WebRtc::implementation::MediaElement::FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaElement > const & value)
{
  if (!value) return wrapper::org::webRtc::MediaElementPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::WebRtc::implementation::MediaElement::FromCppWinrt(Org::WebRtc::MediaElement const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::WebRtc::implementation::MediaElement::FromCppWinrt(wrapper::org::webRtc::MediaElementPtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::WebRtc::implementation::MediaElement::FromCppWinrt(Org::WebRtc::IMediaElement const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaElement Org::WebRtc::implementation::MediaElement::Cast(Org::WebRtc::IMediaElement const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_WebRtc_MediaElement(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webRtc::MediaElement >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

#if 0
//------------------------------------------------------------------------------
Windows::UI::Xaml::Controls::MediaElement Org::WebRtc::implementation::MediaElement::Element()
{
  if (!native_) {throw hresult_error(E_POINTER);}

  Windows::UI::Xaml::Controls::MediaElement result{ nullptr };
  result = wrapper::impl::org::webRtc::MediaElement::toNative_winrt(native_);
  return result;
}

//------------------------------------------------------------------------------
void Org::WebRtc::implementation::MediaElement::Element(Windows::UI::Xaml::Controls::MediaElement const & element)
{
  native_ = wrapper::impl::org::webRtc::MediaElement::toWrapper(element);
}
#endif //0


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
