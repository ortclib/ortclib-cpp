
#include "impl_org_webRtc_RTCDtmfSender.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_RTCDtmfToneChangeEvent.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "pc/dtmfsender.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/SafeInt.h>

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDtmfSender::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

typedef WrapperImplType::NativeTypeScopedPtr NativeTypeScopedPtr;

typedef wrapper::impl::org::webRtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDtmfToneChangeEvent, UseToneChangeEvent);

//------------------------------------------------------------------------------
static UseWrapperMapper &mapperSingleton()
{
  static UseWrapperMapper singleton;
  return singleton;
}

//------------------------------------------------------------------------------
static NativeType *unproxyNative(NativeType *native)
{
  if (!native) return nullptr;
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::DtmfSender, ::webrtc::DtmfSender, native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDtmfSender::RTCDtmfSender() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDtmfSenderPtr wrapper::org::webRtc::RTCDtmfSender::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCDtmfSender>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCDtmfSender::~RTCDtmfSender() noexcept
{
  thisWeak_.reset();
  teardownObserver();
  mapperSingleton().remove(native_.get());
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCDtmfSender::canInsertDtmf() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;
  return native_->CanInsertDtmf();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDtmfSender::insertDtmf(String tones) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return;

  native_->InsertDtmf(tones, 100, 70);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDtmfSender::insertDtmf(
  String tones,
  ::zsLib::Milliseconds duration
  ) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return;

  native_->InsertDtmf(tones, SafeInt<int>(duration.count()), 70);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDtmfSender::insertDtmf(
  String tones,
  ::zsLib::Milliseconds duration,
  ::zsLib::Milliseconds interToneGap
  ) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return;

  native_->InsertDtmf(tones, SafeInt<int>(duration.count()), SafeInt<int>(interToneGap.count()));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCDtmfSender::get_toneBuffer() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();

  return native_->tones();
}

//------------------------------------------------------------------------------
::zsLib::Milliseconds wrapper::impl::org::webRtc::RTCDtmfSender::get_duration() noexcept
{
  typedef ::zsLib::Milliseconds ResultType;

  ZS_ASSERT(native_);
  if (!native_) return ResultType();
  auto ms = native_->duration();

  return ResultType(SafeInt<::zsLib::Milliseconds::rep>(ms));
}

//------------------------------------------------------------------------------
::zsLib::Milliseconds wrapper::impl::org::webRtc::RTCDtmfSender::get_interToneGap() noexcept
{
  typedef ::zsLib::Milliseconds ResultType;

  ZS_ASSERT(native_);
  if (!native_) return ResultType();
  auto ms = native_->inter_tone_gap();

  return ResultType(SafeInt<::zsLib::Milliseconds::rep>(ms));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCDtmfSender::wrapper_onObserverCountChanged(size_t count) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverToneChange(const String &tones) noexcept
{
  String tone = tones.substr(0, 1);
  auto event = UseToneChangeEvent::toWrapper(tone);
  if (!event) return;
  onToneChange(event);
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver() noexcept
{
  if (!native_) return;
  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock(), UseWebrtcLib::delegateQueue());
  native_->RegisterObserver(observer_.get());
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver() noexcept
{
  if (!observer_) return;
  if (!native_) return;

  native_->UnregisterObserver();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  NativeType *original = unproxyNative(native);

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
