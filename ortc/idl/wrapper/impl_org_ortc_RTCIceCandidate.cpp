
#include "impl_org_ortc_RTCIceCandidate.h"
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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidate::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidate::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidate::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidate::RTCIceCandidate()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePtr wrapper::org::ortc::RTCIceCandidate::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidate>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidate::~RTCIceCandidate()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidate::wrapper_init_org_ortc_RTCIceCandidate()
{
  native_ = make_shared< NativeType >();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceCandidate::toJson()
{
  return Json::toWrapper(native_->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidate::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceComponent wrapper::impl::org::ortc::RTCIceCandidate::get_component()
{
  return Helper::toWrapper(native_->mComponent);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidate::get_interfaceType()
{
  return native_->mInterfaceType;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidate::get_foundation()
{
  return native_->mFoundation;
}

//------------------------------------------------------------------------------
uint32_t wrapper::impl::org::ortc::RTCIceCandidate::get_priority()
{
  return native_->mPriority;
}

//------------------------------------------------------------------------------
uint32_t wrapper::impl::org::ortc::RTCIceCandidate::get_unfreezePriority()
{
  return native_->mUnfreezePriority;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceProtocol wrapper::impl::org::ortc::RTCIceCandidate::get_protocol()
{
  return Helper::toWrapper(native_->mProtocol);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidate::get_ip()
{
  return native_->mIP;
}

//------------------------------------------------------------------------------
uint16_t wrapper::impl::org::ortc::RTCIceCandidate::get_port()
{
  return native_->mPort;
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidateType wrapper::impl::org::ortc::RTCIceCandidate::get_candidateType()
{
  return Helper::toWrapper(native_->mCandidateType);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceTcpCandidateType wrapper::impl::org::ortc::RTCIceCandidate::get_tcpType()
{
  return Helper::toWrapper(native_->mTCPType);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidate::get_relatedAddress()
{
  return native_->mRelatedAddress;
}

//------------------------------------------------------------------------------
uint16_t wrapper::impl::org::ortc::RTCIceCandidate::get_relatedPort()
{
  return native_->mRelatedPort;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  return std::dynamic_pointer_cast<WrapperImplType>(wrapper)->native_;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(wrapper::org::ortc::RTCIceGathererCandidatePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!result) return NativeTypePtr();

  return result->native_;
}
