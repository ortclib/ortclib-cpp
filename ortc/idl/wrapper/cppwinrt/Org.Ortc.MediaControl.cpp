
#include "pch.h"
#include "cppwinrt_Helpers.h"
#include "Org.Ortc.MediaControl.h"

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaControl > Org::Ortc::MediaControl::ToCppWinrtImpl(wrapper::org::ortc::MediaControlPtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Ortc::implementation::MediaControl>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaControl > Org::Ortc::MediaControl::ToCppWinrtImpl(Org::Ortc::MediaControl const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::MediaControl > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::MediaControl>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaControl > Org::Ortc::MediaControl::ToCppWinrtImpl(Org::Ortc::IMediaControl const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::MediaControl > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::MediaControl>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl Org::Ortc::MediaControl::ToCppWinrt(wrapper::org::ortc::MediaControlPtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl Org::Ortc::MediaControl::ToCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaControl > const & value)
{
  return value.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl Org::Ortc::MediaControl::ToCppWinrt(Org::Ortc::IMediaControl const & value)
{
  return value.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaControl Org::Ortc::MediaControl::ToCppWinrtInterface(wrapper::org::ortc::MediaControlPtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::IMediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaControl Org::Ortc::MediaControl::ToCppWinrtInterface(Org::Ortc::MediaControl const & value)
{
  return value.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaControl Org::Ortc::MediaControl::ToCppWinrtInterface(winrt::com_ptr< Org::Ortc::implementation::MediaControl > const & value)
{
  return value.as< Org::Ortc::MediaControl >();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaControlPtr Org::Ortc::MediaControl::FromCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaControl > const & value)
{
  if (!value) return wrapper::org::ortc::MediaControlPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaControlPtr Org::Ortc::MediaControl::FromCppWinrt(Org::Ortc::MediaControl const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaControlPtr Org::Ortc::MediaControl::FromCppWinrt(Org::Ortc::IMediaControl const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Ortc::MediaControl ::Org::Ortc::MediaControl::CastFromIMediaControl(Org::Ortc::IMediaControl const & value)
{
  if (!source) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Ortc_MediaControl(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::ortc::MediaControl >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Windows::Foundation::IInspectable Org::Ortc::MediaControl::DisplayOrientation()
{
  return ::Internal::Helper::ToCppWinrt(wrapper::org::ortc::MediaControl::get_displayOrientation());
}

//------------------------------------------------------------------------------
void Org::Ortc::MediaControl::DisplayOrientation(Windows::Foundation::IInspectable const & value)
{
  wrapper::org::ortc::MediaControl::set_displayOrientation(::Internal::Helper::FromCppWinrt(value));
}
