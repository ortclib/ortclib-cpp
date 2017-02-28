
#include "impl_org_ortc_RTCIceCandidatePair.h"
#include "impl_org_ortc_RTCIceCandidate.h"
#include "impl_org_ortc_Json.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidatePair::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidatePair::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCIceCandidatePair::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePair::RTCIceCandidatePair()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePairPtr wrapper::org::ortc::RTCIceCandidatePair::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceCandidatePair>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceCandidatePair::~RTCIceCandidatePair()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePair::wrapper_init_org_ortc_RTCIceCandidatePair()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePair::wrapper_init_org_ortc_RTCIceCandidatePair(wrapper::org::ortc::RTCIceCandidatePairPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePair::wrapper_init_org_ortc_RTCIceCandidatePair(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceCandidatePair::toJson()
{
  return Json::toWrapper(native_->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceCandidatePair::hash()
{
  return native_->hash();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePtr wrapper::impl::org::ortc::RTCIceCandidatePair::get_local()
{
  return RTCIceCandidate::toWrapper(native_->mLocal);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePair::set_local(wrapper::org::ortc::RTCIceCandidatePtr value)
{
  native_->mLocal = RTCIceCandidate::toNative(value);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceCandidatePtr wrapper::impl::org::ortc::RTCIceCandidatePair::get_remote()
{
  return RTCIceCandidate::toWrapper(native_->mRemote);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceCandidatePair::set_remote(wrapper::org::ortc::RTCIceCandidatePtr value)
{
  native_->mRemote = RTCIceCandidate::toNative(value);
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

  auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!impl) return NativeTypePtr();

  return impl->native_;
}
