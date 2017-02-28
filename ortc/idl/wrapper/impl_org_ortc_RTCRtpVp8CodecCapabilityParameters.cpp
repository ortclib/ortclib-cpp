
#include "impl_org_ortc_RTCRtpVp8CodecCapabilityParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::RTCRtpVp8CodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpVp8CodecCapabilityParametersPtr wrapper::org::ortc::RTCRtpVp8CodecCapabilityParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::~RTCRtpVp8CodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters(wrapper::org::ortc::RTCRtpVp8CodecCapabilityParametersPtr source)
{
  if (!source) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpVp8CodecCapabilityParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpVp8CodecCapabilityParameters::hash()
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

  Helper::optionalSafeIntConvert(native.mMaxFR, pThis->maxFr);
  Helper::optionalSafeIntConvert(native.mMaxFS, pThis->maxFs);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  Helper::optionalSafeIntConvert(wrapper->maxFr, result->mMaxFR);
  Helper::optionalSafeIntConvert(wrapper->maxFs, result->mMaxFS);
  return result;
}

