
#include "impl_org_webrtc_RTCError.h"
#include "impl_org_webrtc_enums.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/rtcerror.h"
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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCError::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCError::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCError::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCError::RTCError() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCErrorPtr wrapper::org::webrtc::RTCError::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCError>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCError::~RTCError() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCError::wrapper_init_org_webrtc_RTCError() noexcept
{
  native_ = make_shared<NativeType>();
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCErrorType wrapper::impl::org::webrtc::RTCError::get_type() noexcept
{
  if (!native_) return wrapper::org::webrtc::RTCErrorType::RTCErrorType_none;
  return IEnum::toWrapper(native_->type());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webrtc::RTCError::get_message() noexcept
{
  if (!native_) return String();
  return native_->message();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCError::get_ok() noexcept
{
  if (!native_) return false;
  return native_->ok();
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
