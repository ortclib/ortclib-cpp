
#include "impl_org_webRtc_RTCRtpReceiver.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpreceiverinterface.h"
#include "pc/rtpreceiver.h"
#include "impl_org_webRtc_post_include.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver::NativeType, NativeType);

typedef WrapperImplType::NativeTypeScopedPtr NativeTypeScopedPtr;

typedef wrapper::impl::org::webRtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);

//------------------------------------------------------------------------------
static UseWrapperMapper &mapperSingleton()
{
  static UseWrapperMapper singleton;
  return singleton;
}

//------------------------------------------------------------------------------
static NativeType *unproxyAudioNative(NativeType *native)
{
  if (!native) return nullptr;
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::RtpReceiver, ::webrtc::AudioRtpReceiver,  native);
}

//------------------------------------------------------------------------------
static NativeType *unproxyVideoNative(NativeType *native)
{
  if (!native) return nullptr;
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::RtpReceiver, ::webrtc::VideoRtpReceiver, native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpReceiver::RTCRtpReceiver() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpReceiverPtr wrapper::org::webRtc::RTCRtpReceiver::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpReceiver>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpReceiver::~RTCRtpReceiver() noexcept
{
  thisWeak_.reset();
  mapperSingleton().remove(native_.get());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpCapabilitiesPtr wrapper::org::webRtc::RTCRtpReceiver::getCapabilities(String kind) noexcept
{
  wrapper::org::webRtc::RTCRtpCapabilitiesPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpReceiveParametersPtr wrapper::impl::org::webRtc::RTCRtpReceiver::getParameters() noexcept
{
  wrapper::org::webRtc::RTCRtpReceiveParametersPtr result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::RTCRtpContributingSourcePtr > > wrapper::impl::org::webRtc::RTCRtpReceiver::getContributingSources() noexcept
{
  shared_ptr< list< wrapper::org::webRtc::RTCRtpContributingSourcePtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::RTCRtpSynchronizationSourcePtr > > wrapper::impl::org::webRtc::RTCRtpReceiver::getSynchronizationSources() noexcept
{
  shared_ptr< list< wrapper::org::webRtc::RTCRtpSynchronizationSourcePtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaStreamTrackPtr wrapper::impl::org::webRtc::RTCRtpReceiver::get_track() noexcept
{
  wrapper::org::webRtc::MediaStreamTrackPtr result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpReceiver::set_track(wrapper::org::webRtc::MediaStreamTrackPtr value) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver()
{
//  if (!native_) return;
//  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock(), UseWebrtcLib::delegateQueue());
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver()
{
//  if (!observer_) return;
//  if (!native_) return;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  NativeType *original = unproxyAudioNative(native);
  original = unproxyVideoNative(original);

  // search for original non-proxied pointer in map
  auto wrapper = mapperSingleton().getExistingOrCreateNew(original, [native]() {
    auto result = make_shared<WrapperImplType>();
    result->thisWeak_ = result;
    result->native_ = rtc::scoped_refptr<NativeType>(native); // only use proxy and never original pointer
    result->setupObserver();
    return result;
  });
  return wrapper;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypeScopedPtr native) noexcept
{
  return toWrapper(native.get());
}

//------------------------------------------------------------------------------
NativeTypeScopedPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return rtc::scoped_refptr<NativeType>();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return rtc::scoped_refptr<NativeType>();
  return converted->native_;
}

