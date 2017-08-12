
#include "impl_org_ortc_RTCRtpCodecCapabilityParameters.h"
#include "impl_org_ortc_RTCRtpOpusCodecCapabilityParameters.h"
#include "impl_org_ortc_RTCRtpVp8CodecCapabilityParameters.h"
#include "impl_org_ortc_RTCRtpH264CodecCapabilityParameters.h"
#include "impl_org_ortc_RTCRtpRtxCodecCapabilityParameters.h"
#include "impl_org_ortc_RTCRtpFlexFecCodecCapabilityParameters.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapabilityParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapabilityParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecCapabilityParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecCapabilityParameters::RTCRtpCodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCodecCapabilityParametersPtr wrapper::org::ortc::RTCRtpCodecCapabilityParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpCodecCapabilityParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecCapabilityParameters::~RTCRtpCodecCapabilityParameters()
{
}

//------------------------------------------------------------------------------
WrapperTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();

  {
    auto wrapper = RTCRtpOpusCodecCapabilityParameters::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpVp8CodecCapabilityParameters::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpH264CodecCapabilityParameters::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpRtxCodecCapabilityParameters::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpFlexFecCodecCapabilityParameters::toWrapper(native);
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
    auto impl = std::dynamic_pointer_cast<RTCRtpOpusCodecCapabilityParameters>(wrapper);
    if (impl) return RTCRtpOpusCodecCapabilityParameters::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpVp8CodecCapabilityParameters>(wrapper);
    if (impl) return RTCRtpVp8CodecCapabilityParameters::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpH264CodecCapabilityParameters>(wrapper);
    if (impl) return RTCRtpH264CodecCapabilityParameters::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpRtxCodecCapabilityParameters>(wrapper);
    if (impl) return RTCRtpRtxCodecCapabilityParameters::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpFlexFecCodecCapabilityParameters>(wrapper);
    if (impl) return RTCRtpFlexFecCodecCapabilityParameters::toNative(impl);
  }

  {
    auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
    if (impl) return impl->native_;
  }
  return NativeTypePtr();
}

