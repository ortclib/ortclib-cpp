
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/override/cppwinrt/Org.Webrtc.MediaSource.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaSource > Org::Webrtc::implementation::MediaSource::ToCppWinrtImpl(wrapper::org::webrtc::MediaSourcePtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::Webrtc::implementation::MediaSource>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaSource > Org::Webrtc::implementation::MediaSource::ToCppWinrtImpl(Org::Webrtc::MediaSource const & value)
{
  winrt::com_ptr< Org::Webrtc::implementation::MediaSource > impl{ nullptr };
  impl.copy_from(winrt::from_abi<Org::Webrtc::implementation::MediaSource>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaSource > Org::Webrtc::implementation::MediaSource::ToCppWinrtImpl(winrt::com_ptr< Org::Webrtc::implementation::MediaSource > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::Webrtc::implementation::MediaSource > Org::Webrtc::implementation::MediaSource::ToCppWinrtImpl(Org::Webrtc::IMediaSource const & value)
{
  winrt::com_ptr< Org::Webrtc::implementation::MediaSource > impl{ nullptr };
  impl.copy_from(winrt::from_abi<Org::Webrtc::implementation::MediaSource>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaSource Org::Webrtc::implementation::MediaSource::ToCppWinrt(wrapper::org::webrtc::MediaSourcePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Webrtc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaSource Org::Webrtc::implementation::MediaSource::ToCppWinrt(Org::Webrtc::MediaSource const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaSource Org::Webrtc::implementation::MediaSource::ToCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaSource > const & value)
{
  return value.as< Org::Webrtc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaSource Org::Webrtc::implementation::MediaSource::ToCppWinrt(Org::Webrtc::IMediaSource const & value)
{
  return value.as< Org::Webrtc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IMediaSource Org::Webrtc::implementation::MediaSource::ToCppWinrtInterface(wrapper::org::webrtc::MediaSourcePtr value)
{
  auto result = ToCppWinrtImpl(value);
  return result.as< Org::Webrtc::IMediaSource >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IMediaSource Org::Webrtc::implementation::MediaSource::ToCppWinrtInterface(Org::Webrtc::MediaSource const & value)
{
  return value.as< Org::Webrtc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IMediaSource Org::Webrtc::implementation::MediaSource::ToCppWinrtInterface(winrt::com_ptr< Org::Webrtc::implementation::MediaSource > const & value)
{
  return value.as< Org::Webrtc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::Webrtc::IMediaSource Org::Webrtc::implementation::MediaSource::ToCppWinrtInterface(Org::Webrtc::IMediaSource const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaSourcePtr Org::Webrtc::implementation::MediaSource::FromCppWinrt(winrt::com_ptr< Org::Webrtc::implementation::MediaSource > const & value)
{
  if (!value) return wrapper::org::webrtc::MediaSourcePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaSourcePtr Org::Webrtc::implementation::MediaSource::FromCppWinrt(Org::Webrtc::MediaSource const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaSourcePtr Org::Webrtc::implementation::MediaSource::FromCppWinrt(wrapper::org::webrtc::MediaSourcePtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaSourcePtr Org::Webrtc::implementation::MediaSource::FromCppWinrt(Org::Webrtc::IMediaSource const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::Webrtc::implementation::MediaSource::MediaSource()
  : native_(wrapper::org::webrtc::MediaSource::wrapper_create())
{
  native_->wrapper_init_org_webrtc_MediaSource();
}

//------------------------------------------------------------------------------
Org::Webrtc::MediaSource Org::Webrtc::implementation::MediaSource::CastFromIMediaSource(Org::Webrtc::IMediaSource const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_Webrtc_MediaSource(value);
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webrtc::MediaSource >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Windows::Media::Core::IMediaSource Org::Webrtc::implementation::MediaSource::Source()
{
  if (!native_) { throw hresult_error(E_POINTER); }
  return nullptr; // ::Internal::Helper::ToCppWinrt(native_->get_source());
}

//------------------------------------------------------------------------------
void Org::Webrtc::implementation::MediaSource::Source(Windows::Media::Core::IMediaSource const & )
{
  if (!native_) { throw hresult_error(E_POINTER); }
  //native_->set_source(::Internal::Helper::FromCppWinrt(value));
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE
