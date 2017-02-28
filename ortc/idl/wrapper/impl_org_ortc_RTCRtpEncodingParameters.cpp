
#include "impl_org_ortc_RTCRtpEncodingParameters.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_Helper.h"
#include "impl_org_ortc_RTCRtpFecParameters.h"
#include "impl_org_ortc_RTCRtpRtxParameters.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpEncodingParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpEncodingParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpEncodingParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpEncodingParameters::RTCRtpEncodingParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpEncodingParametersPtr wrapper::org::ortc::RTCRtpEncodingParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpEncodingParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpEncodingParameters::~RTCRtpEncodingParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpEncodingParameters::wrapper_init_org_ortc_RTCRtpEncodingParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpEncodingParameters::wrapper_init_org_ortc_RTCRtpEncodingParameters(wrapper::org::ortc::RTCRtpEncodingParametersPtr source)
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpEncodingParameters::wrapper_init_org_ortc_RTCRtpEncodingParameters(wrapper::org::ortc::JsonPtr json)
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpEncodingParameters::toJson()
{
  wrapper::org::ortc::JsonPtr result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpEncodingParameters::hash()
{
  String result {};
  return result;
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
  Helper::optionalSafeIntConvert(native.mSSRC, pThis->ssrc);
  Helper::optionalSafeIntConvert(native.mCodecPayloadType, pThis->codecPayloadType);
  if (native.mFEC.hasValue()) {
    pThis->fec = RTCRtpFecParameters::toWrapper(native.mFEC);
  }
  if (native.mRTX.hasValue()) {
    pThis->rtx = RTCRtpRtxParameters::toWrapper(native.mRTX);
  }
  if (native.mPriority.hasValue()) {
    pThis->priority = Helper::toWrapper(native.mPriority.value());
  }
  Helper::optionalSafeIntConvert(native.mMaxBitrate, pThis->maxBitrate);
  pThis->minQuality = native.mMinQuality;
  pThis->resolutionScale = native.mResolutionScale;
  pThis->framerateScale = native.mFramerateScale;
  pThis->active = native.mActive;
  pThis->encodingId = native.mEncodingID;
  pThis->dependencyEncodingIds = make_shared< list<String> >(native.mDependencyEncodingIDs);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  Helper::optionalSafeIntConvert(wrapper->ssrc, result->mSSRC);
  Helper::optionalSafeIntConvert(wrapper->codecPayloadType, result->mCodecPayloadType);
  if (wrapper->fec.hasValue()) {
    if (wrapper->fec.value()) {
      auto native = RTCRtpFecParameters::toNative(wrapper->fec.value());
      if (native) {
        result->mFEC = *native;
      }
    }
  }
  if (wrapper->rtx.hasValue()) {
    if (wrapper->rtx.value()) {
      auto native = RTCRtpRtxParameters::toNative(wrapper->rtx.value());
      if (native) {
        result->mRTX = *native;
      }
    }
  }
  if (wrapper->priority.hasValue()) {
    result->mPriority = Helper::toNative(wrapper->priority.value());
  }
  Helper::optionalSafeIntConvert(wrapper->maxBitrate, result->mMaxBitrate);
  result->mMinQuality = wrapper->minQuality;
  result->mResolutionScale = wrapper->resolutionScale;
  result->mFramerateScale = wrapper->framerateScale;
  result->mActive = wrapper->active;
  result->mEncodingID = wrapper->encodingId;
  if (wrapper->dependencyEncodingIds) {
    result->mDependencyEncodingIDs = (*wrapper->dependencyEncodingIds);
  }
  return result;
}
