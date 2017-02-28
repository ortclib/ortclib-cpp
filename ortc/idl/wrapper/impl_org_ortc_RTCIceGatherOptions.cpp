
#include "impl_org_ortc_RTCIceGatherOptions.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_RTCIceGatherInterfacePolicy.h"
#include "impl_org_ortc_RTCIceServer.h"

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
wrapper::impl::org::ortc::RTCIceGatherOptions::RTCIceGatherOptions()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGatherOptionsPtr wrapper::org::ortc::RTCIceGatherOptions::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCIceGatherOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherOptions::~RTCIceGatherOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGatherOptions::wrapper_init_org_ortc_RTCIceGatherOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGatherOptions::wrapper_init_org_ortc_RTCIceGatherOptions(wrapper::org::ortc::RTCIceGatherOptionsPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCIceGatherOptions::wrapper_init_org_ortc_RTCIceGatherOptions(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCIceGatherOptions::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCIceGatherOptions"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCIceGatherOptions::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherOptions::WrapperImplTypePtr wrapper::impl::org::ortc::RTCIceGatherOptions::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherOptions::WrapperImplTypePtr wrapper::impl::org::ortc::RTCIceGatherOptions::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->continuousGathering = native.mContinuousGathering;
  pThis->interfacePolicies = make_shared< list<wrapper::org::ortc::RTCIceGatherInterfacePolicyPtr> >();
  for (auto iter = native.mInterfacePolicies.begin(); iter != native.mInterfacePolicies.end(); ++iter) {
    pThis->interfacePolicies->push_back(RTCIceGatherInterfacePolicy::toWrapper(*iter));
  }
  pThis->iceServers = make_shared< list<wrapper::org::ortc::RTCIceServerPtr> >();
  for (auto iter = native.mICEServers.begin(); iter != native.mICEServers.end(); ++iter) {
    pThis->iceServers->push_back(RTCIceServer::toWrapper(*iter));
  }
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCIceGatherOptions::NativeTypePtr wrapper::impl::org::ortc::RTCIceGatherOptions::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mContinuousGathering = wrapper->continuousGathering;
  if (wrapper->interfacePolicies) {
    for (auto iter = wrapper->interfacePolicies->begin(); iter != wrapper->interfacePolicies->end(); ++iter) {
      auto value = RTCIceGatherInterfacePolicy::toNative(*iter);
      if (!value) continue;
      result->mInterfacePolicies.push_back(*value);
    }
  }
  if (wrapper->iceServers) {
    for (auto iter = wrapper->iceServers->begin(); iter != wrapper->iceServers->end(); ++iter) {
      auto value = RTCIceServer::toNative(*iter);
      if (!value) continue;
      result->mICEServers.push_back(*value);
    }
  }
  return result;
}
