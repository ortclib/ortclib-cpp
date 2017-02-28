
#include "impl_org_ortc_RTCDataChannelParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannelParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannelParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDataChannelParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannelParameters::RTCDataChannelParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDataChannelParametersPtr wrapper::org::ortc::RTCDataChannelParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDataChannelParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDataChannelParameters::~RTCDataChannelParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannelParameters::wrapper_init_org_ortc_RTCDataChannelParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannelParameters::wrapper_init_org_ortc_RTCDataChannelParameters(wrapper::org::ortc::RTCDataChannelParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDataChannelParameters::wrapper_init_org_ortc_RTCDataChannelParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCDataChannelParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCDataChannelParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDataChannelParameters::hash()
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
  pThis->label = native.mLabel;
  pThis->ordered = native.mOrdered;
  pThis->maxPacketLifetime = native.mMaxPacketLifetime;
  pThis->maxRetransmits = native.mMaxRetransmits;
  pThis->protocol = native.mProtocol;
  pThis->negotiated = native.mNegotiated;
  Helper::optionalSafeIntConvert(native.mID, pThis->id);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mLabel = wrapper->label;
  result->mOrdered = wrapper->ordered;
  result->mMaxPacketLifetime = wrapper->maxPacketLifetime;
  result->mMaxRetransmits = wrapper->maxRetransmits;
  result->mProtocol = wrapper->protocol;
  result->mNegotiated = wrapper->negotiated;
  Helper::optionalSafeIntConvert(wrapper->id, result->mID);
  return result;
}
