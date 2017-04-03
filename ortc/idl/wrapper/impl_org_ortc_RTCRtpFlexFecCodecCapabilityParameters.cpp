
#include "impl_org_ortc_RTCRtpFlexFecCodecCapabilityParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::RTCRtpFlexFecCodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersPtr wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::~RTCRtpFlexFecCodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters(wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersPtr source)
{
  if (!source) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::wrapper_init_org_ortc_RTCRtpFlexFecCodecCapabilityParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpFlexFecCodecCapabilityParameters::hash()
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

  pThis->repairWindow = native.mRepairWindow;
  Helper::optionalSafeIntConvert(native.mL, pThis->l);
  Helper::optionalSafeIntConvert(native.mD, pThis->d);
  if (native.mToP.hasValue()) {
    pThis->toP = Helper::toWrapper(native.mToP.value());
  }
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mRepairWindow = wrapper->repairWindow;
  Helper::optionalSafeIntConvert(wrapper->l, result->mL);
  Helper::optionalSafeIntConvert(wrapper->d, result->mD);
  if (wrapper->toP.hasValue()) {
    result->mToP = Helper::toNative(wrapper->toP.value());
  }
  return result;
}
