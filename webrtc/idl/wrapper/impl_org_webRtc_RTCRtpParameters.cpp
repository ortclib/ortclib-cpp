
#include "impl_org_webRtc_RTCRtpParameters.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCRtpCodecParameters.h"
#include "impl_org_webRtc_RTCRtpHeaderExtensionParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpparameters.h"
#include "impl_org_webRtc_post_include.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodecParameters, UseRtpCodecParameters);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpHeaderExtensionParameters, UseRtpHeaderExtensionParameters);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpParameters::RTCRtpParameters() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpParametersPtr wrapper::org::webRtc::RTCRtpParameters::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpParameters::~RTCRtpParameters() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpParameters::wrapper_init_org_webRtc_RTCRtpParameters() noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::apply(const NativeType &from, WrapperType &to)
{
  to.muxId = from.mid;
  to.codecs = make_shared< decltype(to.codecs)::element_type >();
  to.headerExtensions = make_shared< decltype(to.headerExtensions)::element_type >();

#pragma ZS_BUILD_NOTE("LATER","no rtcp parameters")

  for (auto iter = from.codecs.begin(); iter != from.codecs.end(); ++iter) {
    auto wrapper = UseRtpCodecParameters::toWrapper(*iter);
    if (!wrapper) continue;
    to.codecs->push_back(wrapper);
  }

  for (auto iter = from.header_extensions.begin(); iter != from.header_extensions.end(); ++iter) {
    auto wrapper = UseRtpHeaderExtensionParameters::toWrapper(*iter);
    if (!wrapper) continue;
    to.headerExtensions->push_back(wrapper);
  }
}

//------------------------------------------------------------------------------
void WrapperImplType::apply(const WrapperType &from, NativeType &to)
{
  if (from.codecs) {
    for (auto iter = from.codecs->begin(); iter != from.codecs->end(); ++iter) {
      auto native = UseRtpCodecParameters::toNative(*iter);
      if (!native) continue;
      to.codecs.push_back(*native);
    }
  }

  if (from.headerExtensions) {
    for (auto iter = from.headerExtensions->begin(); iter != from.headerExtensions->end(); ++iter) {
      auto native = UseRtpHeaderExtensionParameters::toNative(*iter);
      if (!native) continue;
      to.header_extensions.push_back(*native);
    }
  }
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  apply(native, *result);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeTypePtr();
  auto cloned = make_shared<NativeType>();
  apply(*converted, *cloned);
  return cloned;
}

