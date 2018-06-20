
#include "impl_org_ortc_adapter_RTCOfferAnswerOptions.h"

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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCOfferAnswerOptions::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCOfferAnswerOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCOfferAnswerOptions::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCOfferAnswerOptions::RTCOfferAnswerOptions() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCOfferAnswerOptionsPtr wrapper::org::ortc::adapter::RTCOfferAnswerOptions::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCOfferAnswerOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCOfferAnswerOptions::~RTCOfferAnswerOptions() noexcept
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return RTCOfferAnswerOptionsPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCOfferAnswerOptions>();
  pThis->thisWeak_ = pThis;
  pThis->voiceActivityDetection = native.mVoiceActivityDetection;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();
  auto result = make_shared < NativeType >();
  result->mVoiceActivityDetection = wrapper->voiceActivityDetection;
  return result;
}
