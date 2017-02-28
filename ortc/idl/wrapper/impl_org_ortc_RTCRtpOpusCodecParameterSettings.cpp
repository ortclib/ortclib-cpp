
#include "impl_org_ortc_RTCRtpOpusCodecParameterSettings.h"
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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::RTCRtpOpusCodecParameterSettings()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpOpusCodecParameterSettingsPtr wrapper::org::ortc::RTCRtpOpusCodecParameterSettings::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::~RTCRtpOpusCodecParameterSettings()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings(wrapper::org::ortc::RTCRtpOpusCodecParameterSettingsPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::wrapper_init_org_ortc_RTCRtpOpusCodecParameterSettings(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpOpusCodecParameterSettings::hash()
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
  Helper::optionalSafeIntConvert(native.mMaxPlaybackRate, pThis->maxPlaybackRate);
  Helper::optionalSafeIntConvert(native.mMaxAverageBitrate, pThis->maxAverageBitrate);
  pThis->stereo = native.mStereo;
  pThis->cbr = native.mCBR;
  pThis->useInbandFec = native.mUseInbandFEC;
  pThis->useDtx = native.mUseDTX;
  Helper::optionalSafeIntConvert(native.mComplexity, pThis->complexity);
  if (native.mSignal.hasValue()) {
    pThis->signal = Helper::toWrapper(native.mSignal.value());
  }
  if (native.mApplication.hasValue()) {
    pThis->application = Helper::toWrapper(native.mApplication.value());
  }
  Helper::optionalSafeIntConvert(native.mPacketLossPerc, pThis->packetLossPerc);
  pThis->predictionDisabled = native.mPredictionDisabled;
  Helper::optionalSafeIntConvert(native.mSPropMaxCaptureRate, pThis->sPropMaxCaptureRate);
  pThis->sPropStereo = native.mSPropStereo;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  Helper::optionalSafeIntConvert(wrapper->maxPlaybackRate, result->mMaxPlaybackRate);
  Helper::optionalSafeIntConvert(wrapper->maxAverageBitrate, result->mMaxAverageBitrate);
  result->mStereo = wrapper->stereo;
  result->mCBR = wrapper->cbr;
  result->mUseInbandFEC = wrapper->useInbandFec;
  result->mUseDTX = wrapper->useDtx;
  Helper::optionalSafeIntConvert(wrapper->complexity, result->mComplexity);
  if (wrapper->signal.hasValue()) {
    result->mSignal = Helper::toNative(wrapper->signal.value());
  }
  if (wrapper->application.hasValue()) {
    result->mApplication = Helper::toNative(wrapper->application.value());
  }
  Helper::optionalSafeIntConvert(wrapper->packetLossPerc, result->mPacketLossPerc);
  result->mPacketLossPerc = wrapper->predictionDisabled;
  Helper::optionalSafeIntConvert(wrapper->sPropMaxCaptureRate, result->mSPropMaxCaptureRate);
  result->mSPropStereo = wrapper->sPropStereo;

  return result;
}
