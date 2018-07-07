
#include "pch.h"

#ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#include <wrapper/generated/cppwinrt/cppwinrt_Helpers.h>
#include <wrapper/override/cppwinrt/Org.WebRtc.MediaSource.h>

#include <wrapper/impl_org_webRtc_MediaSource.h>

using namespace winrt;

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaSource > Org::WebRtc::implementation::MediaSource::ToCppWinrtImpl(wrapper::org::webRtc::MediaSourcePtr value)
{
  if (!value) return nullptr;
  auto result = winrt::make_self<Org::WebRtc::implementation::MediaSource>(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaSource > Org::WebRtc::implementation::MediaSource::ToCppWinrtImpl(Org::WebRtc::MediaSource const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::MediaSource > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::MediaSource>(value));
  return impl;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaSource > Org::WebRtc::implementation::MediaSource::ToCppWinrtImpl(winrt::com_ptr< Org::WebRtc::implementation::MediaSource > const & value)
{
  return value;
}

//------------------------------------------------------------------------------
winrt::com_ptr< Org::WebRtc::implementation::MediaSource > Org::WebRtc::implementation::MediaSource::ToCppWinrtImpl(Org::WebRtc::IMediaSource const & value)
{
  winrt::com_ptr< Org::WebRtc::implementation::MediaSource > impl {nullptr};
  impl.copy_from(winrt::from_abi<Org::WebRtc::implementation::MediaSource>(value));
  return impl;
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaSource Org::WebRtc::implementation::MediaSource::ToCppWinrt(wrapper::org::webRtc::MediaSourcePtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::MediaSource {nullptr};
  return result.as< Org::WebRtc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaSource Org::WebRtc::implementation::MediaSource::ToCppWinrt(Org::WebRtc::MediaSource const & value)
{
  return value;
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaSource Org::WebRtc::implementation::MediaSource::ToCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaSource > const & value)
{
  if (!value) return Org::WebRtc::MediaSource {nullptr};
  return value.as< Org::WebRtc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaSource Org::WebRtc::implementation::MediaSource::ToCppWinrt(Org::WebRtc::IMediaSource const & value)
{
  if (!value) return Org::WebRtc::MediaSource {nullptr};
  return value.as< Org::WebRtc::MediaSource >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaSource Org::WebRtc::implementation::MediaSource::ToCppWinrtInterface(wrapper::org::webRtc::MediaSourcePtr value)
{
  auto result = ToCppWinrtImpl(value);
  if (!result) return Org::WebRtc::IMediaSource {nullptr};
  return result.as< Org::WebRtc::IMediaSource >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaSource Org::WebRtc::implementation::MediaSource::ToCppWinrtInterface(Org::WebRtc::MediaSource const & value)
{
  if (!value) return Org::WebRtc::IMediaSource {nullptr};
  return value.as< Org::WebRtc::IMediaSource >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaSource Org::WebRtc::implementation::MediaSource::ToCppWinrtInterface(winrt::com_ptr< Org::WebRtc::implementation::MediaSource > const & value)
{
  if (!value) return Org::WebRtc::IMediaSource {nullptr};
  return value.as< Org::WebRtc::IMediaSource >();
}

//------------------------------------------------------------------------------
Org::WebRtc::IMediaSource Org::WebRtc::implementation::MediaSource::ToCppWinrtInterface(Org::WebRtc::IMediaSource const & value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaSourcePtr Org::WebRtc::implementation::MediaSource::FromCppWinrt(winrt::com_ptr< Org::WebRtc::implementation::MediaSource > const & value)
{
  if (!value) return wrapper::org::webRtc::MediaSourcePtr();
  return value->native_;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaSourcePtr Org::WebRtc::implementation::MediaSource::FromCppWinrt(Org::WebRtc::MediaSource const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaSourcePtr Org::WebRtc::implementation::MediaSource::FromCppWinrt(wrapper::org::webRtc::MediaSourcePtr value)
{
  return value;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaSourcePtr Org::WebRtc::implementation::MediaSource::FromCppWinrt(Org::WebRtc::IMediaSource const & value)
{
  return FromCppWinrt(ToCppWinrtImpl(value));
}

//------------------------------------------------------------------------------
Org::WebRtc::MediaSource Org::WebRtc::implementation::MediaSource::CastFromIMediaSource(Org::WebRtc::IMediaSource const & value)
{
  if (!value) return nullptr;
  auto nativeObject = ::Internal::Helper::FromCppWinrt_Org_WebRtc_MediaSource(value);  
  if (!nativeObject) return nullptr;
  auto result = std::dynamic_pointer_cast< wrapper::org::webRtc::MediaSource >(nativeObject);
  if (!result) return nullptr;
  return ToCppWinrt(result);
}

//------------------------------------------------------------------------------
Org::WebRtc::implementation::MediaSource::MediaSource(Windows::Media::Core::IMediaSource const & source)
 : native_(wrapper::org::webRtc::MediaSource::wrapper_create())
{
  ZS_ASSERT(source);
  if (!native_) { throw hresult_error(E_POINTER); }
  auto wrapperSource = wrapper::impl::org::webRtc::MediaSource::toWrapper(source);
  ZS_ASSERT(wrapperSource);

  native_->wrapper_init_org_webRtc_MediaSource(wrapperSource->source_);
}

//------------------------------------------------------------------------------
Windows::Media::Core::IMediaSource Org::WebRtc::implementation::MediaSource::Source()
{
  if (!native_) {throw hresult_error(E_POINTER);}
  return  wrapper::impl::org::webRtc::MediaSource::toNative_winrt(native_);
}


#endif //ifndef CPPWINRT_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE
