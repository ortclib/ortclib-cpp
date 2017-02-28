
#include "impl_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::RTCSessionDescriptionMediaLineDetails()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLineDetailsPtr wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::~RTCSessionDescriptionMediaLineDetails()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails(wrapper::org::ortc::adapter::RTCSessionDescriptionMediaLineDetailsPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::wrapper_init_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescriptionMediaLineDetails::hash()
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
  Helper::optionalSafeIntConvert(native.mInternalIndex, pThis->internalIndex);
  pThis->privateTransportID = native.mPrivateTransportID;
  pThis->protocol = native.mProtocol;
  pThis->connectionData = RTCSessionDescriptionConnectionData::toWrapper(native.mConnectionData);
  pThis->mediaDirection = Helper::toWrapper(native.mMediaDirection);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  Helper::optionalSafeIntConvert(wrapper->internalIndex, result->mInternalIndex);
  result->mPrivateTransportID = wrapper->privateTransportID;
  result->mProtocol = wrapper->protocol;
  result->mConnectionData = RTCSessionDescriptionConnectionData::toNative(wrapper->connectionData);
  result->mMediaDirection = Helper::toNative(wrapper->mediaDirection);
  return result;
}
