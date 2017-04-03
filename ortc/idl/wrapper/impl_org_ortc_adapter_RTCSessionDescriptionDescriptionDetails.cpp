
#include "impl_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionConnectionDataDetails.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionConnectionData.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::RTCSessionDescriptionDescriptionDetails()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionDetailsPtr wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::~RTCSessionDescriptionDescriptionDetails()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionDetailsPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescriptionDetails::hash()
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
  pThis->username = native.mUsername;
  pThis->sessionId = SafeInt<decltype(pThis->sessionId)>(native.mSessionID);
  pThis->sessionVersion = SafeInt<decltype(pThis->sessionVersion)>(native.mSessionVersion);
  pThis->unicaseAddress = RTCSessionDescriptionConnectionDataDetails::toWrapper(native.mUnicaseAddress);
  pThis->sessionName = native.mSessionName;
  pThis->startTime = SafeInt<decltype(pThis->startTime)>(native.mStartTime);
  pThis->endTime = SafeInt<decltype(pThis->endTime)>(native.mEndTime);
  pThis->connectionData = RTCSessionDescriptionConnectionData::toWrapper(native.mConnectionData);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mUsername = wrapper->username;
  result->mSessionID = SafeInt<decltype(result->mSessionID)>(wrapper->sessionId);
  result->mSessionVersion = SafeInt<decltype(result->mSessionVersion)>(wrapper->sessionVersion);
  result->mUnicaseAddress = RTCSessionDescriptionConnectionDataDetails::toNative(wrapper->unicaseAddress);
  result->mSessionName = wrapper->sessionName;
  result->mStartTime = SafeInt<decltype(result->mStartTime)>(wrapper->startTime);
  result->mEndTime = SafeInt<decltype(result->mEndTime)>(wrapper->endTime);
  result->mConnectionData = RTCSessionDescriptionConnectionData::toNative(wrapper->connectionData);
  return result;
}
