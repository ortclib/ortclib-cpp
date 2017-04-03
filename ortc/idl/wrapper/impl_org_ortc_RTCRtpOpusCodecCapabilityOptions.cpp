
#include "impl_org_ortc_RTCRtpOpusCodecCapabilityOptions.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::RTCRtpOpusCodecCapabilityOptions()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsPtr wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptions::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::~RTCRtpOpusCodecCapabilityOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsPtr source)
{
  if (!source) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityOptions::hash()
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
  Helper::optionalSafeIntConvert(native.mComplexity, pThis->complexity);
  if (native.mSignal.hasValue()) {
    pThis->signal = Helper::toWrapper(native.mSignal.value());
  }
  if (native.mApplication.hasValue()) {
    pThis->application = Helper::toWrapper(native.mApplication.value());
  }
  Helper::optionalSafeIntConvert(native.mPacketLossPerc, pThis->packetLossPerc);
  pThis->predictionDisabled = native.mPredictionDisabled;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  Helper::optionalSafeIntConvert(wrapper->complexity, result->mComplexity);
  if (wrapper->signal.hasValue()) {
    result->mSignal = Helper::toNative(wrapper->signal.value());
  }
  if (wrapper->application.hasValue()) {
    result->mApplication = Helper::toNative(wrapper->application.value());
  }
  Helper::optionalSafeIntConvert(wrapper->packetLossPerc, result->mPacketLossPerc);
  result->mPredictionDisabled = wrapper->predictionDisabled;

  return result;
}
