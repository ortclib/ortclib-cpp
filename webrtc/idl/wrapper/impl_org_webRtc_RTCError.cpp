
#include "impl_org_webRtc_RTCError.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtcerror.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCError::RTCError() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCErrorPtr wrapper::org::webRtc::RTCError::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCError>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCError::~RTCError() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCError::wrapper_init_org_webRtc_RTCError() noexcept
{
  native_ = make_shared<NativeType>();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCErrorType wrapper::impl::org::webRtc::RTCError::get_type() noexcept
{
  if (!native_) return wrapper::org::webRtc::RTCErrorType::RTCErrorType_none;
  return IEnum::toWrapper(native_->type());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCError::get_message() noexcept
{
  if (!native_) return String();
  return native_->message();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCError::get_ok() noexcept
{
  if (!native_) return false;
  return native_->ok();
}

//------------------------------------------------------------------------------
void WrapperImplType::reject(PromisePtr promise) noexcept
{
  if (!promise) return;

  if (native_) {
    if (native_->ok()) {
      promise->resolve();
      return;
    }
  }

  auto pThis = thisWeak_.lock();
  ZS_ASSERT(pThis);

  auto holder = make_shared< ::zsLib::AnyHolder< wrapper::org::webRtc::RTCErrorPtr > >();
  holder->value_ = pThis;
  promise->reject(holder);
}

//------------------------------------------------------------------------------
void WrapperImplType::rejectPromise(PromisePtr promise, const NativeType &native) noexcept
{
  if (!promise) return;
  if (native.ok()) {
    promise->resolve();
    return;
  }

  auto wrapper = toWrapper(native);

  auto holder = make_shared< ::zsLib::AnyHolder< wrapper::org::webRtc::RTCErrorPtr > >();
  holder->value_ = wrapper;

  promise->reject(holder);
}

//------------------------------------------------------------------------------
void WrapperImplType::rejectPromise(PromisePtr promise, NativeTypePtr native) noexcept
{
  if (!promise) return;
  if (!native) {
    ::webrtc::RTCError error(::webrtc::RTCErrorType::INTERNAL_ERROR);
    rejectPromise(promise, error);
    return;
  }
  rejectPromise(promise, *native);
}

//------------------------------------------------------------------------------
PromisePtr WrapperImplType::toPromise(const NativeType &native) noexcept
{
  if (native.ok()) return Promise::createResolved(UseWebrtcLib::delegateQueue());
  auto wrapper = toWrapper(native);

  auto holder = make_shared< ::zsLib::AnyHolder< wrapper::org::webRtc::RTCErrorPtr > >();
  holder->value_ = wrapper;

  return Promise::createRejected(holder, UseWebrtcLib::delegateQueue());
}

//------------------------------------------------------------------------------
PromisePtr WrapperImplType::toPromise(NativeTypePtr native) noexcept
{
  if (!native) {
    ::webrtc::RTCError error(::webrtc::RTCErrorType::INTERNAL_ERROR);
    return toPromise(error);
  }
  return toPromise(native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(native.type(), native.message());
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypePtr();
  return converted->native_;
}
