
#include "impl_org_ortc_RTCRtpRtxCodecCapabilityParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::RTCRtpRtxCodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpRtxCodecCapabilityParametersPtr wrapper::org::ortc::RTCRtpRtxCodecCapabilityParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::~RTCRtpRtxCodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters(wrapper::org::ortc::RTCRtpRtxCodecCapabilityParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpRtxCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpRtxCodecCapabilityParameters::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(AnyPtr native)
{
  return toWrapper(std::dynamic_pointer_cast<NativeType>(native));
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
  pThis->apt = SafeInt<decltype(pThis->apt)>(native.mApt);
  pThis->rtxTime = native.mRTXTime;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mApt = SafeInt<decltype(result->mApt)>(wrapper->apt);
  result->mRTXTime = wrapper->rtxTime;
  return result;
}

