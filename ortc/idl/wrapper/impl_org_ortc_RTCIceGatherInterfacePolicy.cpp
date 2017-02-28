
#include "impl_org_ortc_RTCIceGatherInterfacePolicy.h"
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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::RTCIceGatherInterfacePolicy()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGatherInterfacePolicyPtr wrapper::org::ortc::RTCIceGatherInterfacePolicy::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::~RTCIceGatherInterfacePolicy()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::wrapper_init_org_ortc_RTCIceGatherInterfacePolicy()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::wrapper_init_org_ortc_RTCIceGatherInterfacePolicy(wrapper::org::ortc::RTCIceGatherInterfacePolicyPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::wrapper_init_org_ortc_RTCIceGatherInterfacePolicy(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::WrapperImplTypePtr wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::WrapperImplTypePtr wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->interfaceType = native.mInterfaceType;
  pThis->gatherPolicy = Helper::toWrapper(native.mGatherPolicy);
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::NativeTypePtr wrapper::impl::org::ortc::RTCIceGatherInterfacePolicy::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mInterfaceType = wrapper->interfaceType;
  result->mGatherPolicy = Helper::toNative(wrapper->gatherPolicy);
  return result;
}
