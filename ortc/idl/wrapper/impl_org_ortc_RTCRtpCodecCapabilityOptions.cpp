
#include "impl_org_ortc_RTCRtpCodecCapabilityOptions.h"
#include "impl_org_ortc_RTCRtpOpusCodecCapabilityOptions.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapabilityOptions::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapabilityOptions::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapabilityOptions::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecCapabilityOptions::RTCRtpCodecCapabilityOptions()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCodecCapabilityOptionsPtr wrapper::org::ortc::RTCRtpCodecCapabilityOptions::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpCodecCapabilityOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecCapabilityOptions::~RTCRtpCodecCapabilityOptions()
{
}

//------------------------------------------------------------------------------
WrapperTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperTypePtr();

  {
    auto wrapper = RTCRtpOpusCodecCapabilityOptions::toWrapper(native);
    if (wrapper) return wrapper;
  }

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpOpusCodecCapabilityOptions>(wrapper);
    if (impl) return RTCRtpOpusCodecCapabilityOptions::toNative(impl);
  }

  {
    auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
    if (impl) return impl->native_;
  }

  return NativeTypePtr();
}
