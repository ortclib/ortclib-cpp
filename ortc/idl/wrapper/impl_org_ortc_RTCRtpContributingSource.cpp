
#include "impl_org_ortc_RTCRtpContributingSource.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpContributingSource::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpContributingSource::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpContributingSource::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpContributingSource::RTCRtpContributingSource()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpContributingSourcePtr wrapper::org::ortc::RTCRtpContributingSource::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpContributingSource>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpContributingSource::~RTCRtpContributingSource()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpContributingSource::wrapper_init_org_ortc_RTCRtpContributingSource()
{
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpContributingSource::hash()
{
  return toNative(thisWeak_.lock())->hash();
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
  pThis->timestamp = native.mTimestamp;
  pThis->csrc = SafeInt<decltype(pThis->csrc)>(native.mCSRC);
  pThis->audioLevel = SafeInt<decltype(pThis->audioLevel)>(native.mAudioLevel);
  pThis->voiceActivityFlag = native.mVoiceActivityFlag;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mTimestamp = wrapper->timestamp;
  result->mCSRC = SafeInt<decltype(result->mCSRC)>(wrapper->csrc);
  result->mAudioLevel = SafeInt<decltype(result->mAudioLevel)>(wrapper->audioLevel);
  result->mVoiceActivityFlag = wrapper->voiceActivityFlag;
  return result;
}
