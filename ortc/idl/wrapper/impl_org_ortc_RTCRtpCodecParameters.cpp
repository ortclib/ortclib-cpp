
#include "impl_org_ortc_RTCRtpCodecParameters.h"
#include "impl_org_ortc_RTCRtcpFeedback.h"
#include "impl_org_ortc_RTCRtpCodecParameterSettings.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecParameters::RTCRtpCodecParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCodecParametersPtr wrapper::org::ortc::RTCRtpCodecParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpCodecParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecParameters::~RTCRtpCodecParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCodecParameters::wrapper_init_org_ortc_RTCRtpCodecParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCodecParameters::wrapper_init_org_ortc_RTCRtpCodecParameters(wrapper::org::ortc::RTCRtpCodecParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCodecParameters::wrapper_init_org_ortc_RTCRtpCodecParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpCodecParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCRtpCodecParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpCodecParameters::hash()
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
  pThis->name = native.mName;
  pThis->payloadType = SafeInt<decltype(pThis->payloadType)>(native.mPayloadType);
  Helper::optionalSafeIntConvert(native.mClockRate, pThis->clockRate);
  pThis->pTime = native.mPTime;
  pThis->maxPTime = native.mMaxPTime;
  Helper::optionalSafeIntConvert(native.mNumChannels, pThis->numChannels);
  pThis->rtcpFeedback = make_shared< list <wrapper::org::ortc::RTCRtcpFeedbackPtr > >();
  for (auto iter = native.mRTCPFeedback.begin(); iter != native.mRTCPFeedback.end(); ++iter) {
    auto wrapper = RTCRtcpFeedback::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->rtcpFeedback->push_back(wrapper);
  }
  pThis->parameters = RTCRtpCodecParameterSettings::toWrapper(native.mParameters);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mName = wrapper->name;
  result->mPayloadType = SafeInt<decltype(result->mPayloadType)>(wrapper->payloadType);
  Helper::optionalSafeIntConvert(wrapper->clockRate, result->mClockRate);
  result->mPTime = wrapper->pTime;
  result->mMaxPTime = wrapper->maxPTime;
  Helper::optionalSafeIntConvert(wrapper->numChannels, result->mNumChannels);
  if (wrapper->rtcpFeedback) {
    for (auto iter = wrapper->rtcpFeedback->begin(); iter != wrapper->rtcpFeedback->end(); ++iter) {
      auto native = RTCRtcpFeedback::toNative(*iter);
      if (!native) continue;
      result->mRTCPFeedback.push_back(*native);
    }
  }
  if (wrapper->parameters) {
    result->mParameters = RTCRtpCodecParameterSettings::toNative(wrapper->parameters);
  }
  return result;
}
