
#include "impl_org_ortc_adapter_RTCOfferOptions.h"

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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCOfferOptions::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCOfferOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCOfferOptions::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCOfferOptions::RTCOfferOptions()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCOfferOptionsPtr wrapper::org::ortc::adapter::RTCOfferOptions::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCOfferOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCOfferOptions::~RTCOfferOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCOfferOptions::wrapper_init_org_ortc_adapter_RTCOfferOptions()
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return RTCOfferOptionsPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCOfferOptions>();
  pThis->thisWeak_ = pThis;
  pThis->voiceActivityDetection = native.mVoiceActivityDetection;
  pThis->iceRestart = native.mICERestart;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(wrapper::org::ortc::adapter::RTCOfferOptionsPtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  auto result = make_shared < NativeType >();
  result->mVoiceActivityDetection = wrapper->voiceActivityDetection;
  result->mICERestart = wrapper->iceRestart;
  return result;
}
