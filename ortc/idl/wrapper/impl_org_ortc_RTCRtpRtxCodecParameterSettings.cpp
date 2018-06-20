
#include "impl_org_ortc_RTCRtpRtxCodecParameterSettings.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::RTCRtpRtxCodecParameterSettings() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpRtxCodecParameterSettingsPtr wrapper::org::ortc::RTCRtpRtxCodecParameterSettings::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::~RTCRtpRtxCodecParameterSettings() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::toJson() noexcept
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::hash() noexcept
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings(wrapper::org::ortc::RTCRtpRtxCodecParameterSettingsPtr source) noexcept
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRtxCodecParameterSettings::wrapper_init_org_ortc_RTCRtpRtxCodecParameterSettings(wrapper::org::ortc::JsonPtr json) noexcept
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
  pThis->apt = SafeInt<decltype(pThis->apt)>(native.mApt);
  pThis->rtxTime = native.mRTXTime;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mApt = SafeInt<decltype(result->mApt)>(wrapper->apt);
  result->mRTXTime = wrapper->rtxTime;
  return result;
}

