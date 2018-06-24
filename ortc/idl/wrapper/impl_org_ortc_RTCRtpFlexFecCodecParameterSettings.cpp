
#include "impl_org_ortc_RTCRtpFlexFecCodecParameterSettings.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::RTCRtpFlexFecCodecParameterSettings() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpFlexFecCodecParameterSettingsPtr wrapper::org::ortc::RTCRtpFlexFecCodecParameterSettings::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::~RTCRtpFlexFecCodecParameterSettings() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::toJson() noexcept
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::hash() noexcept
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings(wrapper::org::ortc::RTCRtpFlexFecCodecParameterSettingsPtr source) noexcept
{
  if (!source) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpFlexFecCodecParameterSettings::wrapper_init_org_ortc_RTCRtpFlexFecCodecParameterSettings(wrapper::org::ortc::JsonPtr json) noexcept
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
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

  pThis->repairWindow = native.mRepairWindow;
  Helper::optionalSafeIntConvert(native.mL, pThis->columns);
  Helper::optionalSafeIntConvert(native.mD, pThis->rows);
  if (native.mToP.hasValue()) {
    pThis->toP = Helper::toWrapper(native.mToP.value());
  }
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mRepairWindow = wrapper->repairWindow;
  Helper::optionalSafeIntConvert(wrapper->columns, result->mL);
  Helper::optionalSafeIntConvert(wrapper->rows, result->mD);
  if (wrapper->toP.hasValue()) {
    result->mToP = Helper::toNative(wrapper->toP.value());
  }
  return result;
}
