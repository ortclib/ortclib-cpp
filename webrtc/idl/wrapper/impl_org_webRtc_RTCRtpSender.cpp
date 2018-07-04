
#include "impl_org_webRtc_RTCRtpSender.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpsenderinterface.h"
#include "pc/rtpsender.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSender::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

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
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::RtpSender, ::webrtc::AudioRtpSender, native);
}

//------------------------------------------------------------------------------
static NativeType *unproxyVideoNative(NativeType *native)
{
  if (!native) return nullptr;
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::RtpSender, ::webrtc::VideoRtpSender, native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpSender::RTCRtpSender() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSenderPtr wrapper::org::webRtc::RTCRtpSender::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpSender>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpSender::~RTCRtpSender() noexcept
{
  thisWeak_.reset();
  teardownObserver();
  mapperSingleton().remove(native_.get());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpCapabilitiesPtr wrapper::org::webRtc::RTCRtpSender::getCapabilities(String kind) noexcept
{
  wrapper::org::webRtc::RTCRtpCapabilitiesPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSendParametersPtr wrapper::impl::org::webRtc::RTCRtpSender::getParameters() noexcept
{
  wrapper::org::webRtc::RTCRtpSendParametersPtr result {};
  return result;
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCRtpSender::setParameters(wrapper::org::webRtc::RTCRtpSendParametersPtr parameters) noexcept(false)
{
  PromisePtr result {};
  return result;
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCRtpSender::replaceTrack(wrapper::org::webRtc::MediaStreamTrackPtr withTrack) noexcept
{
  PromisePtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaStreamTrackPtr wrapper::impl::org::webRtc::RTCRtpSender::get_track() noexcept
{
  wrapper::org::webRtc::MediaStreamTrackPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDtmfSenderPtr wrapper::impl::org::webRtc::RTCRtpSender::get_dtmf() noexcept
{
  wrapper::org::webRtc::RTCDtmfSenderPtr result {};
  return result;
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver() noexcept
{
  //  if (!native_) return;
  //  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock(), UseWebrtcLib::delegateQueue());
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver() noexcept
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
