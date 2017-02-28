
#include "impl_org_ortc_RTCRtpCodecParameterSettings.h"
#include "impl_org_ortc_RTCRtpOpusCodecParameterSettings.h"
#include "impl_org_ortc_RTCRtpVp8CodecParameterSettings.h"
#include "impl_org_ortc_RTCRtpH264CodecParameterSettings.h"
#include "impl_org_ortc_RTCRtpRtxCodecParameterSettings.h"
#include "impl_org_ortc_RTCRtpFlexFecCodecParameterSettings.h"
#include "impl_org_ortc_RTCRtpRedCodecParameterSettings.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecParameterSettings::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecParameterSettings::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCodecParameterSettings::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecParameterSettings::RTCRtpCodecParameterSettings()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCodecParameterSettingsPtr wrapper::org::ortc::RTCRtpCodecParameterSettings::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpCodecParameterSettings>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCodecParameterSettings::~RTCRtpCodecParameterSettings()
{
}

//------------------------------------------------------------------------------
WrapperTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();

  {
    auto wrapper = RTCRtpOpusCodecParameterSettings::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpVp8CodecParameterSettings::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpH264CodecParameterSettings::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpRtxCodecParameterSettings::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpFlexFecCodecParameterSettings::toWrapper(native);
    if (wrapper) return wrapper;
  }
  {
    auto wrapper = RTCRtpRedCodecParameterSettings::toWrapper(native);
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
    auto impl = std::dynamic_pointer_cast<RTCRtpOpusCodecParameterSettings>(wrapper);
    if (impl) return RTCRtpOpusCodecParameterSettings::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpVp8CodecParameterSettings>(wrapper);
    if (impl) return RTCRtpVp8CodecParameterSettings::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpH264CodecParameterSettings>(wrapper);
    if (impl) return RTCRtpH264CodecParameterSettings::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpRtxCodecParameterSettings>(wrapper);
    if (impl) return RTCRtpRtxCodecParameterSettings::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpFlexFecCodecParameterSettings>(wrapper);
    if (impl) return RTCRtpFlexFecCodecParameterSettings::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCRtpRedCodecParameterSettings>(wrapper);
    if (impl) return RTCRtpRedCodecParameterSettings::toNative(impl);
  }

  {
    auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
    if (impl) return impl->native_;
  }

  return NativeTypePtr();
}
