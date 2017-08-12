
#include "impl_org_ortc_adapter_RTCMediaStreamTrackConfiguration.h"
#include "impl_org_ortc_RTCRtpCapabilities.h"
#include "impl_org_ortc_RTCRtpParameters.h"

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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCMediaStreamTrackConfiguration::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCMediaStreamTrackConfiguration::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCMediaStreamTrackConfiguration::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCMediaStreamTrackConfiguration::RTCMediaStreamTrackConfiguration()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr wrapper::org::ortc::adapter::RTCMediaStreamTrackConfiguration::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCMediaStreamTrackConfiguration>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCMediaStreamTrackConfiguration::~RTCMediaStreamTrackConfiguration()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCMediaStreamTrackConfiguration::wrapper_init_org_ortc_adapter_RTCMediaStreamTrackConfiguration()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCMediaStreamTrackConfiguration::wrapper_init_org_ortc_adapter_RTCMediaStreamTrackConfiguration(wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
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
  pThis->capabilities = RTCRtpCapabilities::toWrapper(native.mCapabilities);
  pThis->parameters = RTCRtpParameters::toWrapper(native.mParameters);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mCapabilities = RTCRtpCapabilities::toNative(wrapper->capabilities);
  result->mParameters = RTCRtpParameters::toNative(wrapper->parameters);
  return result;
}
