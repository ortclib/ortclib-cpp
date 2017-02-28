
#include "impl_org_ortc_RTCRtpCodecCapability.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_Helper.h"
#include "impl_org_ortc_RTCRtcpFeedback.h"
#include "impl_org_ortc_RTCRtpCodecCapabilityParameters.h"
#include "impl_org_ortc_RTCRtpCodecCapabilityOptions.h"

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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapability::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapability::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapability::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecCapability::RTCRtpCodecCapability()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCodecCapabilityPtr wrapper::org::ortc::RTCRtpCodecCapability::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpCodecCapability>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecCapability::~RTCRtpCodecCapability()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCodecCapability::wrapper_init_org_ortc_RTCRtpCodecCapability()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCodecCapability::wrapper_init_org_ortc_RTCRtpCodecCapability(wrapper::org::ortc::RTCRtpCodecCapabilityPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCodecCapability::wrapper_init_org_ortc_RTCRtpCodecCapability(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpCodecCapability::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpCodecCapability::hash()
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
  pThis->kind = native.mKind;
  Helper::optionalSafeIntConvert(native.mClockRate, pThis->clockRate);
  pThis->preferredPayloadType = native.mPreferredPayloadType;
  pThis->pTime = native.mPTime;
  pThis->maxPTime = native.mMaxPTime;
  Helper::optionalSafeIntConvert(native.mNumChannels, pThis->numChannels);
  pThis->rtcpFeedback = make_shared< list< wrapper::org::ortc::RTCRtcpFeedbackPtr > >();
  for (auto iter = native.mRTCPFeedback.begin(); iter != native.mRTCPFeedback.end(); ++iter) {
    auto wrapper = RTCRtcpFeedback::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->rtcpFeedback->push_back(wrapper);
  }
  pThis->parameters = RTCRtpCodecCapabilityParameters::toWrapper(native.mParameters);
  pThis->options = RTCRtpCodecCapabilityOptions::toWrapper(native.mOptions);
  pThis->maxTemporalLayers = native.mMaxTemporalLayers;
  pThis->maxSpatialLayers = native.mMaxSpatialLayers;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();

  result->mName = wrapper->name;
  result->mKind = wrapper->kind;
  Helper::optionalSafeIntConvert(wrapper->clockRate, result->mClockRate);
  result->mPreferredPayloadType = wrapper->preferredPayloadType;
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
  result->mParameters = RTCRtpCodecCapabilityParameters::toNative(wrapper->parameters);
  result->mOptions = RTCRtpCodecCapabilityOptions::toNative(wrapper->options);
  result->mMaxTemporalLayers = wrapper->maxTemporalLayers;
  result->mMaxSpatialLayers = wrapper->maxSpatialLayers;

  return result;
}

