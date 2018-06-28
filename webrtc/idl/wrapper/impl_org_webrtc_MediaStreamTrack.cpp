
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

#include "impl_org_webrtc_helpers.h"
#include "impl_org_webrtc_MediaElement.h"
#include "impl_org_webrtc_MediaSource.h"
#include "impl_org_webrtc_AudioOptions.h"
#include "impl_org_webrtc_MediaConstraints.h"
#include "impl_org_webrtc_WebrtcLib.h"

#include "impl_org_webrtc_pre_include.h"
#include "pc/audiotrack.h"
#include "pc/videotrack.h"
#include "api/mediastreamtrackproxy.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webrtc_post_include.h"

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

// borrow types from call defintions
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaStreamTrack::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaStreamTrack::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaStreamTrack::NativeType, NativeType);

typedef wrapper::impl::org::webrtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::WebRtcLib, UseWebrtcLib);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaConstraints, UseMediaConstraints);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::AudioOptions, UseAudioOptions);

//------------------------------------------------------------------------------
static UseWrapperMapper &mapperSingleton()
{
  static UseWrapperMapper singleton;
  return singleton;
}

//------------------------------------------------------------------------------
static ::webrtc::AudioTrackInterface *unproxyAudioTrack(NativeType *native)
{
  if (!native) return nullptr;
  auto converted = dynamic_cast<::webrtc::AudioTrack *>(native);
  if (!converted) return nullptr;

  return WRAPPER_DEPROXIFY_CLASS(::webrtc, AudioTrack, converted);
}

//------------------------------------------------------------------------------
static ::webrtc::VideoTrackInterface *unproxyVideoTrack(NativeType *native)
{
  if (!native) return nullptr;
  auto converted = dynamic_cast<::webrtc::VideoTrack *>(native);
  if (!converted) return nullptr;

  return WRAPPER_DEPROXIFY_CLASS(::webrtc, VideoTrack, converted);
}

#ifdef WINUWP

#ifdef __cplusplus_winrt
//------------------------------------------------------------------------------
static void notifyAboutNewMediaSource(WrapperImplType &wrapper, Windows::Media::Core::IMediaSource^ newSource)
{
  typedef WrapperImplType::UseMediaSourceImpl UseMediaSourceImpl;
  auto source = UseMediaSourceImpl::toWrapper(newSource);
  wrapper.notifySourceChanged(source);
}
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION
//------------------------------------------------------------------------------
static void notifyAboutNewMediaSource(WrapperImplType &wrapper, winrt::Windows::Media::Core::IMediaSource const & newSource)
{
  typedef WrapperImplType::UseMediaSourceImpl UseMediaSourceImpl;
  auto source = UseMediaSourceImpl::toWrapper(newSource);
  wrapper.notifySourceChanged(source);
}
#endif //CPPWINRT_VERSION

#endif //WINUWP

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
  thisWeak_.reset();
  teardownObserver();
  mapperSingleton().remove(native_.get());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::wrapper_init_org_webrtc_MediaStreamTrack() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::AudioTrackSourcePtr wrapper::org::webrtc::MediaStreamTrack::createAudioTrackSource(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept
{
  //HERE

  auto factory = UseWebrtcLib::peerConnectionFactory();
  if (!factory) return WrapperTypePtr();

  auto converted = UseMediaConstraints::toNative(constraints);

  auto source = factory->CreateAudioSource(converted.get());
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::AudioTrackSourcePtr wrapper::org::webrtc::MediaStreamTrack::createAudioTrackSource(wrapper::org::webrtc::AudioOptionsPtr options) noexcept
{
  wrapper::org::webrtc::AudioTrackSourcePtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::VideoTrackSourcePtr wrapper::org::webrtc::MediaStreamTrack::createVideoTrackSource(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept
{
  wrapper::org::webrtc::VideoTrackSourcePtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::VideoTrackSourcePtr wrapper::org::webrtc::MediaStreamTrack::createVideoTrackSource(wrapper::org::webrtc::VideoCapturerPtr capturer) noexcept
{
  wrapper::org::webrtc::VideoTrackSourcePtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackPtr wrapper::org::webrtc::MediaStreamTrack::createAudioTrack(wrapper::org::webrtc::AudioTrackSourcePtr source) noexcept
{
  wrapper::org::webrtc::MediaStreamTrackPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackPtr wrapper::org::webrtc::MediaStreamTrack::createVideoTrack(wrapper::org::webrtc::VideoTrackSourcePtr source) noexcept
{
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
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackState wrapper::impl::org::webrtc::MediaStreamTrack::get_state() noexcept
{
#pragma ZS_BUILD_NOTE("IMPLEMENT","(robin)")
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
void WrapperImplType::notifySourceChanged(UseMediaSourcePtr source)
{
  {
    zsLib::AutoLock lock(lock_);
    source_ = source;
  }

  // notify subscribers of event
  onMediaSourceChanged();

  // auto attach media source to media element
  autoAttachSourceToElement();
}

//------------------------------------------------------------------------------
void WrapperImplType::autoAttachSourceToElement()
{
#pragma ZS_BUILD_NOTE("VERIFY","(mosa) verify this is correct behaviour (REMOVE IF GOOD)")

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

  {
    zsLib::AutoLock lock(lock_);

    ZS_MAYBE_USED() bool didAttachment = false;
    ZS_MAYBE_USED(didAttachment);

#ifdef __cplusplus_winrt
    if (!didAttachment) {
      auto nativeElement = UseMediaElementImpl::toNative_cx(element);
      if (nativeElement) {
        auto nativeSource = UseMediaSourceImpl::toNative_cx(source);
        nativeElement->SetMediaStreamSource(nativeSource);
        didAttachment = true;
      }
    }
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION
    if (!didAttachment) {
      auto nativeElement = UseMediaElementImpl::toNative_winrt(element);
      if (nativeElement) {
        auto nativeSource = UseMediaSourceImpl::toNative_winrt(source);
        nativeElement.SetMediaStreamSource(nativeSource);
        didAttachment = true;
      }
    }
#endif //CPPWINRT_VERSION
  }
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver()
{
  auto converted = dynamic_cast<::webrtc::VideoTrackInterface *>(native_.get());
  ZS_ASSERT(converted);
  if (!converted) return;

  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock());

  rtc::VideoSinkWants wants;

#pragma ZS_BUILD_NOTE("TODO","(mosa) you may want to tweak these properties -- not sure")

  // wants.rotation_applied = ;
  // wants.black_frames = ;
  // wants.max_pixel_count = ;
  // wants.target_pixel_count = ;
  // wants.max_framerate_fps = ;

  converted->AddOrUpdateSink(observer_.get(), wants);
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver()
{
  if (!observer_) return;

  auto converted = dynamic_cast<::webrtc::VideoTrackInterface *>(native_.get());
  ZS_ASSERT(converted);
  if (!converted) return;

  converted->RemoveSink(observer_.get());
  observer_.reset();
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyFrame(const ::webrtc::VideoFrame& frame) noexcept
{
#pragma ZS_BUILD_NOTE("TODO","(mosa) call this static method with new cppwinrt IMediaSource object and the appropriate events will fire to upper layers")
  // winrt::Windows::Media::Core::IMediaSource source = some_value; // pick one definition
  // Windows::Media::Core::IMediaSource ^source = some_value;       // pick one definition
  // notifyAboutNewMediaSource(*this, source);
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyDiscardedFrame() noexcept
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
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
    result->setupObserver();
    return result;
  });
  return wrapper;
}

//------------------------------------------------------------------------------
rtc::scoped_refptr<NativeType> WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return rtc::scoped_refptr<NativeType>();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return rtc::scoped_refptr<NativeType>();
  return converted->native_;
}
