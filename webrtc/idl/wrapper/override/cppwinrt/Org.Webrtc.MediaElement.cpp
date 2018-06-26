
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/override/cppwinrt/Org.Webrtc.MediaElement.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaElement > Org::Webrtc::implementation::MediaElement::ToCppWinrtImpl(wrapper::org::webrtc::MediaElementPtr value)
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
Org::Webrtc::MediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrt(wrapper::org::webrtc::MediaElementPtr value)
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
Org::Webrtc::IMediaElement Org::Webrtc::implementation::MediaElement::ToCppWinrtInterface(wrapper::org::webrtc::MediaElementPtr value)
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
wrapper::org::webrtc::MediaElementPtr Org::Webrtc::implementation::MediaElement::FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaElement > const & value)
{
  if (!value) return wrapper::org::webrtc::MediaElementPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaElementPtr Org::Webrtc::implementation::MediaElement::FromCppWinrt(Org::Webrtc::MediaElement const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaElementPtr Org::Webrtc::implementation::MediaElement::FromCppWinrt(wrapper::org::webrtc::MediaElementPtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaElementPtr Org::Webrtc::implementation::MediaElement::FromCppWinrt(Org::Webrtc::IMediaElement const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Webrtc::implementation::MediaElement::MediaElement()
  : native_(wrapper::org::webrtc::MediaElement::wrapper_create())
{
  native_->wrapper_init_org_webrtc_MediaElement();
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaElement Org::Webrtc::implementation::MediaElement::CastFromIMediaElement(Org::Webrtc::IMediaElement const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Webrtc_MediaElement(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webrtc::MediaElement >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Windows::Foundation::IInspectable Org::Webrtc::implementation::MediaElement::Element()
{
  if (!native_) {throw hresult_error(E_POINTER);}
  return ::Internal::Helper::ToCppWinrt(native_->get_element());
}

//------------------------------------------------------------------------------
void Org::Webrtc::implementation::MediaElement::Element(Windows::Foundation::IInspectable const & value)
{
  if (!native_) {throw hresult_error(E_POINTER);}
  native_->set_element(::Internal::Helper::FromCppWinrt(value));
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
