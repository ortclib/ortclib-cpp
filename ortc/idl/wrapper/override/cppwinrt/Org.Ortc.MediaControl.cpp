
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_MEDIACONTROL
#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/override/cppwinrt/Org.Ortc.MediaControl.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaControl > Org::Ortc::implementation::MediaControl::ToCppWinrtImpl(wrapper::org::ortc::MediaControlPtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Ortc::implementation::MediaControl>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaControl > Org::Ortc::implementation::MediaControl::ToCppWinrtImpl(Org::Ortc::MediaControl const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::MediaControl > impl{ nullptr };
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::MediaControl>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaControl > Org::Ortc::implementation::MediaControl::ToCppWinrtImpl(winrt::com_ptr< Org::Ortc::implementation::MediaControl > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaControl > Org::Ortc::implementation::MediaControl::ToCppWinrtImpl(Org::Ortc::IMediaControl const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::MediaControl > impl{ nullptr };
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::MediaControl>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl Org::Ortc::implementation::MediaControl::ToCppWinrt(wrapper::org::ortc::MediaControlPtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl Org::Ortc::implementation::MediaControl::ToCppWinrt(Org::Ortc::MediaControl const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl Org::Ortc::implementation::MediaControl::ToCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaControl > const & value)
{
  return value.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl Org::Ortc::implementation::MediaControl::ToCppWinrt(Org::Ortc::IMediaControl const & value)
{
  return value.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaControl Org::Ortc::implementation::MediaControl::ToCppWinrtInterface(wrapper::org::ortc::MediaControlPtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::IMediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaControl Org::Ortc::implementation::MediaControl::ToCppWinrtInterface(Org::Ortc::MediaControl const & value)
{
  return value.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaControl Org::Ortc::implementation::MediaControl::ToCppWinrtInterface(winrt::com_ptr< Org::Ortc::implementation::MediaControl > const & value)
{
  return value.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaControl Org::Ortc::implementation::MediaControl::ToCppWinrtInterface(Org::Ortc::IMediaControl const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaControlPtr Org::Ortc::implementation::MediaControl::FromCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaControl > const & value)
{
  if (!value) return wrapper::org::ortc::MediaControlPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaControlPtr Org::Ortc::implementation::MediaControl::FromCppWinrt(Org::Ortc::MediaControl const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaControlPtr Org::Ortc::implementation::MediaControl::FromCppWinrt(wrapper::org::ortc::MediaControlPtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaControlPtr Org::Ortc::implementation::MediaControl::FromCppWinrt(Org::Ortc::IMediaControl const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl Org::Ortc::implementation::MediaControl::CastFromIMediaControl(Org::Ortc::IMediaControl const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Ortc_MediaControl(value);
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::ortc::MediaControl >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Windows::Foundation::IInspectable Org::Ortc::implementation::MediaControl::DisplayOrientation()
{
  return nullptr; // ::Internal::Helper::ToCppWinrt(wrapper::org::ortc::MediaControl::get_displayOrientation());
}

//------------------------------------------------------------------------------
void Org::Ortc::implementation::MediaControl::DisplayOrientation(Windows::Foundation::IInspectable const & value)
{
  //wrapper::org::ortc::MediaControl::set_displayOrientation(::Internal::Helper::FromCppWinrt(value));
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_MEDIACONTROL
