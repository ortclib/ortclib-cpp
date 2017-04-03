
#include "impl_org_ortc_RTCRtpH264CodecCapabilityParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::RTCRtpH264CodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpH264CodecCapabilityParametersPtr wrapper::org::ortc::RTCRtpH264CodecCapabilityParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::~RTCRtpH264CodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters(wrapper::org::ortc::RTCRtpH264CodecCapabilityParametersPtr source)
{
  if (!source) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpH264CodecCapabilityParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpH264CodecCapabilityParameters::hash()
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
  Helper::optionalSafeIntConvert(native.mProfileLevelID, pThis->profileLevelId);
  pThis->packetizationModes = make_shared< list< unsigned short > >();
  for (auto iter = native.mPacketizationModes.begin(); iter != native.mPacketizationModes.end(); ++iter) {
    pThis->packetizationModes->push_back(SafeInt<unsigned short>(*iter));
  }
  Helper::optionalSafeIntConvert(native.mMaxMBPS, pThis->maxMbps);
  Helper::optionalSafeIntConvert(native.mMaxSMBPS, pThis->maxSMbps);
  Helper::optionalSafeIntConvert(native.mMaxFS, pThis->maxFs);
  Helper::optionalSafeIntConvert(native.mMaxCPB, pThis->maxCpb);
  Helper::optionalSafeIntConvert(native.mMaxDPB, pThis->maxDpb);
  Helper::optionalSafeIntConvert(native.mMaxBR, pThis->maxBr);

  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  Helper::optionalSafeIntConvert(wrapper->profileLevelId, result->mProfileLevelID);
  if (wrapper->packetizationModes) {
    for (auto iter = wrapper->packetizationModes->begin(); iter != wrapper->packetizationModes->end(); ++iter) {
      result->mPacketizationModes.push_back(SafeInt<decltype(result->mPacketizationModes)::value_type>(*iter));
    }
  }
  Helper::optionalSafeIntConvert(wrapper->maxMbps, result->mMaxMBPS);
  Helper::optionalSafeIntConvert(wrapper->maxSMbps, result->mMaxSMBPS);
  Helper::optionalSafeIntConvert(wrapper->maxFs, result->mMaxFS);
  Helper::optionalSafeIntConvert(wrapper->maxCpb, result->mMaxCPB);
  Helper::optionalSafeIntConvert(wrapper->maxDpb, result->mMaxDPB);
  Helper::optionalSafeIntConvert(wrapper->maxBr, result->mMaxBR);

  return result;
}
