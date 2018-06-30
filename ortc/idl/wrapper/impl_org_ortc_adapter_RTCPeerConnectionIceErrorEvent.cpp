
#include "impl_org_ortc_adapter_RTCPeerConnectionIceErrorEvent.h"
#include "impl_org_ortc_adapter_RTCIceCandidate.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::RTCPeerConnectionIceErrorEvent() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCPeerConnectionIceErrorEventPtr wrapper::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::~RTCPeerConnectionIceErrorEvent() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCIceCandidatePtr wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::get_hostCandidate() noexcept
{
  return RTCIceCandidate::toWrapper(native_->mHostCandidate);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::get_url() noexcept
{
  return native_->mURL;
}

//------------------------------------------------------------------------------
Optional< uint16_t > wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::get_errorCode() noexcept
{
  Optional< uint16_t > result {};
  Helper::optionalSafeIntConvert(native_->mErrorCode, result);
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceErrorEvent::get_errorText() noexcept
{
  return native_->mErrorText;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();

  auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!impl) return NativeTypePtr();

  return impl->native_;
}

