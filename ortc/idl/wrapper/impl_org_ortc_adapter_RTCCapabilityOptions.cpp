
#include "impl_org_ortc_adapter_RTCCapabilityOptions.h"

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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCCapabilityOptions::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCCapabilityOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCCapabilityOptions::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCCapabilityOptions::RTCCapabilityOptions()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCCapabilityOptionsPtr wrapper::org::ortc::adapter::RTCCapabilityOptions::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCCapabilityOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCCapabilityOptions::~RTCCapabilityOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCCapabilityOptions::wrapper_init_org_ortc_adapter_RTCCapabilityOptions()
{
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
  pThis->voiceActivityDetection = native.mVoiceActivityDetection;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  auto result = make_shared < NativeType >();
  result->mVoiceActivityDetection = wrapper->voiceActivityDetection;
  return result;
}

