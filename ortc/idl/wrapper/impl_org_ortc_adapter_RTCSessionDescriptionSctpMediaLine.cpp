
#include "impl_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionMediaLineDetails.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::RTCSessionDescriptionSctpMediaLine()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionSctpMediaLinePtr wrapper::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::~RTCSessionDescriptionSctpMediaLine()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine(wrapper::org::ortc::adapter::RTCSessionDescriptionSctpMediaLinePtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionSctpMediaLine::wrapper_init_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
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
  pThis->id = native.mID;
  pThis->transportId = native.mTransportID;
  pThis->mediaType = native.mMediaType;
  pThis->details = RTCSessionDescriptionMediaLineDetails::toWrapper(native.mDetails);
  pThis->capabilities = RTCSctpCapabilities::toWrapper(native.mCapabilities);
  Helper::optionalSafeIntConvert(native.mPort, pThis->port);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mID = wrapper->id;
  result->mTransportID = wrapper->transportId;
  result->mMediaType = wrapper->mediaType;
  result->mDetails = RTCSessionDescriptionMediaLineDetails::toNative(wrapper->details);
  result->mCapabilities = RTCSctpCapabilities::toNative(wrapper->capabilities);
  Helper::optionalSafeIntConvert(wrapper->port, result->mPort);
  return result;
}
