
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_MEDIASOURCE

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/override/cppwinrt/MediaSource.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaSource > Org::Ortc::implementation::MediaSource::ToCppWinrtImpl(wrapper::org::ortc::MediaSourcePtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Ortc::implementation::MediaSource>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaSource > Org::Ortc::implementation::MediaSource::ToCppWinrtImpl(Org::Ortc::MediaSource const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::MediaSource > impl{ nullptr };
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::MediaSource>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaSource > Org::Ortc::implementation::MediaSource::ToCppWinrtImpl(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Ortc::implementation::MediaSource > Org::Ortc::implementation::MediaSource::ToCppWinrtImpl(Org::Ortc::IMediaSource const & value)
{
  winrt::com_ptr< Org::Ortc::implementation::MediaSource > impl{ nullptr };
  impl.copy_from(winrt::from_abi<Org::Ortc::implementation::MediaSource>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource Org::Ortc::implementation::MediaSource::ToCppWinrt(wrapper::org::ortc::MediaSourcePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource Org::Ortc::implementation::MediaSource::ToCppWinrt(Org::Ortc::MediaSource const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource Org::Ortc::implementation::MediaSource::ToCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value)
{
  return value.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource Org::Ortc::implementation::MediaSource::ToCppWinrt(Org::Ortc::IMediaSource const & value)
{
  return value.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaSource Org::Ortc::implementation::MediaSource::ToCppWinrtInterface(wrapper::org::ortc::MediaSourcePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Ortc::IMediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaSource Org::Ortc::implementation::MediaSource::ToCppWinrtInterface(Org::Ortc::MediaSource const & value)
{
  return value.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaSource Org::Ortc::implementation::MediaSource::ToCppWinrtInterface(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value)
{
  return value.as< Org::Ortc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Ortc::IMediaSource Org::Ortc::implementation::MediaSource::ToCppWinrtInterface(Org::Ortc::IMediaSource const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr Org::Ortc::implementation::MediaSource::FromCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value)
{
  if (!value) return wrapper::org::ortc::MediaSourcePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr Org::Ortc::implementation::MediaSource::FromCppWinrt(Org::Ortc::MediaSource const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr Org::Ortc::implementation::MediaSource::FromCppWinrt(wrapper::org::ortc::MediaSourcePtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr Org::Ortc::implementation::MediaSource::FromCppWinrt(Org::Ortc::IMediaSource const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Ortc::implementation::MediaSource::MediaSource()
  : native_(wrapper::org::ortc::MediaSource::wrapper_create())
{
  native_->wrapper_init_org_ortc_MediaSource();
}

//------------------------------------------------------------------------------
Org::Ortc::MediaSource Org::Ortc::implementation::MediaSource::Cast(Org::Ortc::IMediaSource const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Ortc_MediaSource(value);
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::ortc::MediaSource >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Windows::Media::Core::IMediaSource Org::Ortc::implementation::MediaSource::Source()
{
  if (!native_) { throw hresult_error(E_POINTER); }
  return nullptr; // ::Internal::Helper::ToCppWinrt(native_->get_source());
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_MEDIASOURCE
