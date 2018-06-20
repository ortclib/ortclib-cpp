
#include "impl_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_Helper.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::RTCSessionDescriptionRtpSenderDetails() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetailsPtr wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::~RTCSessionDescriptionRtpSenderDetails() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetailsPtr source) noexcept
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails(wrapper::org::ortc::JsonPtr json) noexcept
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::toJson() noexcept
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSenderDetails::hash() noexcept
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  Helper::optionalSafeIntConvert(native.mInternalRTPMediaLineIndex, pThis->internalRtpMediaLineIndex);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  Helper::optionalSafeIntConvert(wrapper->internalRtpMediaLineIndex, result->mInternalRTPMediaLineIndex);
  return result;
}
