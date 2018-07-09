
#include "impl_org_webRtc_RTCSessionDescription.h"
#include "impl_org_webRtc_RTCSessionDescriptionInit.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCError.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/jsep.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCSessionDescription::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError, UseError);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::UseSessionDescriptionInit, UseSessionDescriptionInit);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCSessionDescription::RTCSessionDescription() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSessionDescriptionPtr wrapper::org::webRtc::RTCSessionDescription::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCSessionDescription>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCSessionDescription::~RTCSessionDescription() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCSessionDescription::wrapper_init_org_webRtc_RTCSessionDescription(wrapper::org::webRtc::RTCSessionDescriptionInitPtr init) noexcept(false)
{
  if (!init) {
    ::webrtc::RTCError error(::webrtc::RTCErrorType::INVALID_PARAMETER);
    throw UseError::toWrapper(error);
  }

  if (init->sdp.isEmpty()) {
    ::webrtc::RTCError error(::webrtc::RTCErrorType::INVALID_PARAMETER);
    throw UseError::toWrapper(error);
  }

  ::webrtc::SdpParseError sdpError;
  auto native = ::webrtc::CreateSessionDescription(UseEnum::toNative(init->type), init->sdp, &sdpError);
  if (!native) {
    String message = "line= " + sdpError.line + " description=" + sdpError.description;
    ::webrtc::RTCError error(::webrtc::RTCErrorType::INVALID_PARAMETER, message);
    throw UseError::toWrapper(error);
  }

  native_ = std::move(native);
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSdpType wrapper::impl::org::webRtc::RTCSessionDescription::get_sdpType() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCSdpType::RTCSdpType_rollback;
  return UseEnum::toWrapper(native_->GetType());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCSessionDescription::get_sdp() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return String();

  String result;
  native_->ToString(&result);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  String sdp;
  native.ToString(&sdp);
  if (sdp.isEmpty()) return WrapperImplTypePtr();

  ::webrtc::SdpParseError error;
  auto cloned = ::webrtc::CreateSessionDescription(native.GetType(), sdp, &error);
  if (!cloned) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = std::move(cloned);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const UseSessionDescriptionInit &value) noexcept
{
  ::webrtc::SdpParseError error;
  auto native = ::webrtc::CreateSessionDescription(UseEnum::toNative(value.type), value.sdp, &error);

  if (!native) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = std::move(native);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(UseSessionDescriptionInitPtr value) noexcept
{
  if (!value) return WrapperImplTypePtr();
  return toWrapper(*value);
}

//------------------------------------------------------------------------------
NativeTypeUniPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypeUniPtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypeUniPtr();

  if (!converted->native_) return NativeTypeUniPtr();

  String sdp;
  converted->native_->ToString(&sdp);
  if (sdp.isEmpty()) return NativeTypeUniPtr();

  ::webrtc::SdpParseError error;
  return std::move(::webrtc::CreateSessionDescription(converted->native_->GetType(), sdp, &error));
}
