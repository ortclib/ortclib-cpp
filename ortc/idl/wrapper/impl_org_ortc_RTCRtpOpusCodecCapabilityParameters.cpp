
#include "impl_org_ortc_RTCRtpOpusCodecCapabilityParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::RTCRtpOpusCodecCapabilityParameters() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpOpusCodecCapabilityParametersPtr wrapper::org::ortc::RTCRtpOpusCodecCapabilityParameters::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::~RTCRtpOpusCodecCapabilityParameters() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters(wrapper::org::ortc::RTCRtpOpusCodecCapabilityParametersPtr source) noexcept
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json) noexcept
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::toJson() noexcept
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpOpusCodecCapabilityParameters::hash() noexcept
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(AnyPtr native) noexcept
{
  return toWrapper(std::dynamic_pointer_cast<NativeType>(native));
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  Helper::optionalSafeIntConvert(native.mMaxPlaybackRate, pThis->maxPlaybackRate);
  Helper::optionalSafeIntConvert(native.mMaxAverageBitrate, pThis->maxAverageBitrate);
  pThis->stereo = native.mStereo;
  pThis->cbr = native.mCBR;
  pThis->useInbandFec = native.mUseInbandFEC;
  pThis->useDtx = native.mUseDTX;
  Helper::optionalSafeIntConvert(native.mSPropMaxCaptureRate, pThis->sPropMaxCaptureRate);
  pThis->sPropStereo = native.mSPropStereo;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  Helper::optionalSafeIntConvert(wrapper->maxPlaybackRate, result->mMaxPlaybackRate);
  Helper::optionalSafeIntConvert(wrapper->maxAverageBitrate, result->mMaxAverageBitrate);
  result->mStereo = wrapper->stereo;
  result->mCBR = wrapper->cbr;
  result->mUseInbandFEC = wrapper->useInbandFec;
  result->mUseDTX = wrapper->useDtx;
  Helper::optionalSafeIntConvert(wrapper->sPropMaxCaptureRate, result->mSPropMaxCaptureRate);
  result->mSPropStereo = wrapper->sPropStereo;

  return result;
}
