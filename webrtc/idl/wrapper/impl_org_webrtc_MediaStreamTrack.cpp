
#ifdef WINUWP

#ifdef __cplusplus_winrt
#include <windows.ui.xaml.controls.h>
#endif //__cplusplus_winrt

#ifdef __has_include
#if __has_include(<winrt/Windows.UI.Xaml.Controls.h>)
#include <winrt/Windows.UI.Xaml.Controls.h>
#endif //__has_include(<winrt/Windows.UI.Xaml.Controls.h>)
#endif //__has_include

#else

#ifdef _WIN32
#endif //_WIN32

#endif //WINUWP

#include "impl_org_webrtc_MediaStreamTrack.h"
#include "impl_org_webrtc_MediaElement.h"
#include "impl_org_webrtc_MediaSource.h"
#include "impl_org_webrtc_helpers.h"

using ::zsLib::String;
using ::zsLib::Optional;
using ::zsLib::Any;
using ::zsLib::AnyPtr;
using ::zsLib::AnyHolder;
using ::zsLib::Promise;
using ::zsLib::PromisePtr;
using ::zsLib::PromiseWithHolder;
using ::zsLib::PromiseWithHolderPtr;
using ::zsLib::eventing::SecureByteBlock;
using ::zsLib::eventing::SecureByteBlockPtr;
using ::std::shared_ptr;
using ::std::weak_ptr;
using ::std::make_shared;
using ::std::list;
using ::std::set;
using ::std::map;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaStreamTrack::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaStreamTrack::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaStreamTrack::NativeType, NativeType);

typedef wrapper::impl::org::webrtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

//------------------------------------------------------------------------------
static UseWrapperMapper &mapperSingleton()
{
  static UseWrapperMapper singleton;
  return singleton;
}

#include "impl_org_webrtc_pre_include.h"
#include "pc/audiotrack.h"
#include "pc/videotrack.h"
#include "api/mediastreamtrackproxy.h"
#include "impl_org_webrtc_post_include.h"

//------------------------------------------------------------------------------
static ::webrtc::AudioTrackInterface *unproxyAudioTrack(NativeType *track)
{
  if (!track) return nullptr;
  auto converted = dynamic_cast<::webrtc::AudioTrack *>(track);
  if (!converted) return nullptr;

  return WRAPPER_DEPROXIFY_CLASS(::webrtc, AudioTrack, converted);
}

//------------------------------------------------------------------------------
static ::webrtc::VideoTrackInterface *unproxyVideoTrack(NativeType *track)
{
  if (!track) return nullptr;
  auto converted = dynamic_cast<::webrtc::VideoTrack *>(track);
  if (!converted) return nullptr;

  return WRAPPER_DEPROXIFY_CLASS(::webrtc, VideoTrack, converted);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::MediaStreamTrack::MediaStreamTrack() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackPtr wrapper::org::webrtc::MediaStreamTrack::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::MediaStreamTrack>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::MediaStreamTrack::~MediaStreamTrack()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::wrapper_init_org_webrtc_MediaStreamTrack() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackPtr wrapper::org::webrtc::MediaStreamTrack::createAudioSource(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept
{
#pragma ZS_BUILD_NOTE("TODO","implement")
  wrapper::org::webrtc::MediaStreamTrackPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackPtr wrapper::org::webrtc::MediaStreamTrack::createVideoSource(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept
{
#pragma ZS_BUILD_NOTE("TODO","implement")
  wrapper::org::webrtc::MediaStreamTrackPtr result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webrtc::MediaStreamTrack::get_kind() noexcept
{
  if (!native_) return String();
  return native_->kind();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webrtc::MediaStreamTrack::get_id() noexcept
{
  if (!native_) return String();
  return native_->id();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::MediaStreamTrack::get_enabled() noexcept
{
  if (!native_) return false;
  return native_->enabled();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::set_enabled(bool value) noexcept
{
  if (!native_) return;

  native_->set_enabled(value);

#pragma ZS_BUILD_NOTE("EXAMPLE","how to obtain the video track interface out of the native pointer (REMOVE THIS EXAMPLE)")
  // example of what to do to get the video track
  auto converted = dynamic_cast<::webrtc::VideoTrackInterface *>(native_.get());
  ZS_ASSERT(converted);
  if (!converted) return;

}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackState wrapper::impl::org::webrtc::MediaStreamTrack::get_state() noexcept
{
#pragma ZS_BUILD_NOTE("TODO","implement")
  wrapper::org::webrtc::MediaStreamTrackState result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaSourcePtr wrapper::impl::org::webrtc::MediaStreamTrack::get_source() noexcept
{
  zsLib::AutoLock lock(lock_);
  return source_;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaElementPtr wrapper::impl::org::webrtc::MediaStreamTrack::get_element() noexcept
{
  zsLib::AutoLock lock(lock_);
  return element_;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::set_element(wrapper::org::webrtc::MediaElementPtr value) noexcept
{
  {
    zsLib::AutoLock lock(lock_);
    element_ = value;
  }

  autoAttachSourceToElement();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::wrapper_onObserverCountChanged(ZS_MAYBE_USED() size_t count) noexcept
{
  ZS_MAYBE_USED(count);
}

//------------------------------------------------------------------------------
void WrapperImplType::notifySourceChanged()
{
  // notify subscribers of event
  onMediaSourceChanged();

  // auto attach media source to media element
  autoAttachSourceToElement();
}

//------------------------------------------------------------------------------
void WrapperImplType::autoAttachSourceToElement()
{
  if (!native_) return;

  UseMediaElementPtr element;
  UseMediaSourcePtr source;

  // get mapping within lock
  {
    zsLib::AutoLock lock(lock_);
    element = element_;
    source = source_;
  }

  if (!element) return;

#ifdef CPPWINRT_VERSION
  auto winrtMediaElement = UseMediaElementImpl::toNative_winrt(element);
  if (!winrtMediaElement) return;

  auto winrtMediaSource = UseMediaSourceImpl::toNative_winrt(source);
  winrtMediaElement.SetMediaStreamSource(winrtMediaSource);
#pragma ZS_BUILD_NOTE("TODO","(mosa) verify this is correct behaviour")
#endif //CPPWINRT_VERSION

}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native)
{
  if (!native) return WrapperImplTypePtr();

  ::webrtc::MediaStreamTrackInterface *originalTrack {};
  if (!originalTrack) {
    originalTrack = unproxyAudioTrack(native);
  }
  if (!originalTrack) {
    originalTrack = unproxyVideoTrack(native);
  }
  if (!originalTrack) return WrapperImplTypePtr();

  // search for original non-proxied pointer in map
  auto wrapper = mapperSingleton().getExistingOrCreateNew(originalTrack, [native]() {
    auto result = make_shared<WrapperImplType>();
    result->thisWeak_ = result;
    result->native_ = rtc::scoped_refptr<NativeType>(native); // only use proxy and never original pointer
    return result;
  });
  return wrapper;
}

//------------------------------------------------------------------------------
rtc::scoped_refptr<NativeType> WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return rtc::scoped_refptr<NativeType>();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return rtc::scoped_refptr<NativeType>();
  return converted->native_;
}
