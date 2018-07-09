
#include "impl_org_webRtc_RTCRtpSender.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_RTCError.h"
#include "impl_org_webRtc_MediaStreamTrack.h"
#include "impl_org_webRtc_RTCDtmfSender.h"
#include "impl_org_webRtc_RTCRtpSendParameters.h"
#include "impl_org_webRtc_WebrtcLib.h"
#include "generated/org_webRtc_RTCRtpReceiver.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSendParameters, UseRtpSendParameters);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError, UseRtcError);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDtmfSender, UseDtmfSender);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaStreamTrack, UseMediaStreamTrack);
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
  wrapper_dispose();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpSender::wrapper_dispose() noexcept
{
  if (!native_) return;

  teardownObserver();
  mapperSingleton().remove(native_.get());
  native_ = NativeTypeScopedPtr();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpCapabilitiesPtr wrapper::org::webRtc::RTCRtpSender::getCapabilities(String kind) noexcept
{
#pragma ZS_BUILD_NOTE("NOTE","Sender and receiver share same capabilities within WebRTC (for now)")
  return wrapper::org::webRtc::RTCRtpReceiver::getCapabilities(kind);
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSendParametersPtr wrapper::impl::org::webRtc::RTCRtpSender::getParameters() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCRtpSendParametersPtr();

  return UseRtpSendParameters::toWrapper(native_->GetParameters());
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCRtpSender::setParameters(wrapper::org::webRtc::RTCRtpSendParametersPtr parameters) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return UseRtcError::toPromise(::webrtc::RTCError(::webrtc::RTCErrorType::INTERNAL_ERROR));

  auto nativeValue = UseRtpSendParameters::toNative(parameters);
  if (!nativeValue) return UseRtcError::toPromise(::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER));

  return UseRtcError::toPromise(native_->SetParameters(*nativeValue));
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCRtpSender::replaceTrack(wrapper::org::webRtc::MediaStreamTrackPtr withTrack) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return UseRtcError::toPromise(::webrtc::RTCError(::webrtc::RTCErrorType::INTERNAL_ERROR));

  auto nativeValue = UseMediaStreamTrack::toNative(withTrack);
  bool result = native_->SetTrack(nativeValue);

  if (!result) return UseRtcError::toPromise(::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER));

  return UseRtcError::toPromise(::webrtc::RTCError::OK());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaStreamTrackPtr wrapper::impl::org::webRtc::RTCRtpSender::get_track() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::MediaStreamTrackPtr();

  return UseMediaStreamTrack::toWrapper(native_->track());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDtmfSenderPtr wrapper::impl::org::webRtc::RTCRtpSender::get_dtmf() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCDtmfSenderPtr();

  return UseDtmfSender::toWrapper(native_->GetDtmfSender());
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
