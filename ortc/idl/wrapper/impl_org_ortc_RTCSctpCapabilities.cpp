
#include "impl_org_ortc_RTCSctpCapabilities.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpCapabilities::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpCapabilities::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSctpCapabilities::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpCapabilities::RTCSctpCapabilities()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSctpCapabilitiesPtr wrapper::org::ortc::RTCSctpCapabilities::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSctpCapabilities>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSctpCapabilities::~RTCSctpCapabilities()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpCapabilities::wrapper_init_org_ortc_RTCSctpCapabilities()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpCapabilities::wrapper_init_org_ortc_RTCSctpCapabilities(wrapper::org::ortc::RTCSctpCapabilitiesPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSctpCapabilities::wrapper_init_org_ortc_RTCSctpCapabilities(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCSctpCapabilities::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCSctpCapabilities"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSctpCapabilities::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->maxMessageSize = SafeInt<decltype(pThis->maxMessageSize)>(native.mMaxMessageSize);
  pThis->minPort = SafeInt<decltype(pThis->minPort)>(native.mMinPort);
  pThis->maxPort = SafeInt<decltype(pThis->maxPort)>(native.mMaxPort);
  pThis->maxUsablePorts = SafeInt<decltype(pThis->maxUsablePorts)>(native.mMaxUsablePorts);
  pThis->maxSessionsPerPort = SafeInt<decltype(pThis->maxSessionsPerPort)>(native.mMaxSessionsPerPort);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mMaxMessageSize = SafeInt<decltype(result->mMaxMessageSize)>(wrapper->maxMessageSize);
  result->mMinPort = SafeInt<decltype(result->mMinPort)>(wrapper->minPort);
  result->mMaxPort = SafeInt<decltype(result->mMaxPort)>(wrapper->maxPort);
  result->mMaxUsablePorts = SafeInt<decltype(result->mMaxUsablePorts)>(wrapper->maxUsablePorts);
  result->mMaxSessionsPerPort = SafeInt<decltype(result->mMaxSessionsPerPort)>(wrapper->maxSessionsPerPort);
  return result;
}
