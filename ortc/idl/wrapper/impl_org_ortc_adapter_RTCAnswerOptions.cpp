
#include "impl_org_ortc_adapter_RTCAnswerOptions.h"

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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCAnswerOptions::RTCAnswerOptions()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCAnswerOptionsPtr wrapper::org::ortc::adapter::RTCAnswerOptions::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCAnswerOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCAnswerOptions::~RTCAnswerOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCAnswerOptions::wrapper_init_org_ortc_adapter_RTCAnswerOptions()
{
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCAnswerOptions::WrapperImplTypePtr wrapper::impl::org::ortc::adapter::RTCAnswerOptions::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCAnswerOptions::WrapperImplTypePtr wrapper::impl::org::ortc::adapter::RTCAnswerOptions::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->voiceActivityDetection = native.mVoiceActivityDetection;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCAnswerOptions::NativeTypePtr wrapper::impl::org::ortc::adapter::RTCAnswerOptions::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  auto result = make_shared < NativeType >();
  result->mVoiceActivityDetection = wrapper->voiceActivityDetection;
  return result;
}
