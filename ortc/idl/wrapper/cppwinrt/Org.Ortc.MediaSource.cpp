
#include "pch.h"
#include "cppwinrt_Helpers.h"
#include "Org.Ortc.MediaSource.h"

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaSource > Org::Ortc::MediaSource::ToCppWinrtImpl(wrapper::org::ortc::MediaSourcePtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Ortc::implementation::MediaSource>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaSource > Org::Ortc::MediaSource::ToCppWinrtImpl(Org::Ortc::MediaSource const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::MediaSource > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::MediaSource>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaSource > Org::Ortc::MediaSource::ToCppWinrtImpl(Org::Ortc::IMediaSource const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::MediaSource > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::MediaSource>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource Org::Ortc::MediaSource::ToCppWinrt(wrapper::org::ortc::MediaSourcePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource Org::Ortc::MediaSource::ToCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value)
{
  return value.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource Org::Ortc::MediaSource::ToCppWinrt(Org::Ortc::IMediaSource const & value)
{
  return value.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaSource Org::Ortc::MediaSource::ToCppWinrtInterface(wrapper::org::ortc::MediaSourcePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::IMediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaSource Org::Ortc::MediaSource::ToCppWinrtInterface(Org::Ortc::MediaSource const & value)
{
  return value.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaSource Org::Ortc::MediaSource::ToCppWinrtInterface(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value)
{
  return value.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr Org::Ortc::MediaSource::FromCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value)
{
  if (!value) return wrapper::org::ortc::MediaSourcePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr Org::Ortc::MediaSource::FromCppWinrt(Org::Ortc::MediaSource const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr Org::Ortc::MediaSource::FromCppWinrt(Org::Ortc::IMediaSource const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource::MediaSource()
  : native_(wrapper::org::ortc::MediaSource::wrapper_create())
{
  native_->wrapper_init_org_ortc_MediaSource();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource ::Org::Ortc::MediaSource::CastFromIMediaSource(Org::Ortc::IMediaSource const & value)
{
  if (!source) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Ortc_MediaSource(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::ortc::MediaSource >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Windows::Foundation::IInspectable Org::Ortc::MediaSource::Source()
{
  if (!native_) {throw hresult_error(E_POINTER);}
  return ::Internal::Helper::ToCppWinrt(native_->get_source());
}

//------------------------------------------------------------------------------
void Org::Ortc::MediaSource::Source(Windows::Foundation::IInspectable const & value)
{
  if (!native_) {throw hresult_error(E_POINTER);}
  native_->set_source(::Internal::Helper::FromCppWinrt(value));
}

//------------------------------------------------------------------------------
Windows::Foundation::IInspectable Org::Ortc::MediaSource::Track()
{
  if (!native_) {throw hresult_error(E_POINTER);}
  return ::Internal::Helper::ToCppWinrt(native_->get_track());
}
