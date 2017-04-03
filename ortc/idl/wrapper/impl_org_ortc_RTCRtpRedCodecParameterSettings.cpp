
#include "impl_org_ortc_RTCRtpRedCodecParameterSettings.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::RTCRtpRedCodecParameterSettings()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpRedCodecParameterSettingsPtr wrapper::org::ortc::RTCRtpRedCodecParameterSettings::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::~RTCRtpRedCodecParameterSettings()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings(wrapper::org::ortc::RTCRtpRedCodecParameterSettingsPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::wrapper_init_org_ortc_RTCRtpRedCodecParameterSettings(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpRedCodecParameterSettings::hash()
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
  pThis->payloadTypes = make_shared< list< uint8_t > >();
  for (auto iter = native.mPayloadTypes.begin(); iter != native.mPayloadTypes.end(); ++iter) {
    pThis->payloadTypes->push_back(SafeInt<uint8_t>(*iter));
  }
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  if (wrapper->payloadTypes) {
    for (auto iter = wrapper->payloadTypes->begin(); iter != wrapper->payloadTypes->end(); ++iter) {
      result->mPayloadTypes.push_back(SafeInt< decltype(result->mPayloadTypes)::value_type >(*iter));
    }
  }

  return result;
}

