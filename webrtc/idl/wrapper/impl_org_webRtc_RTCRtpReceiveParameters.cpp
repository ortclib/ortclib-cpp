
#include "impl_org_webRtc_RTCRtpReceiveParameters.h"
#include "impl_org_webRtc_RTCRtpParameters.h"
#include "impl_org_webRtc_RTCRtpDecodingParameters.h"
#include "impl_org_webRtc_enums.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiveParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpParameters, UseRtpParameters);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpDecodingParameters, UseRtpDecodingParameters);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpReceiveParameters::RTCRtpReceiveParameters() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpReceiveParametersPtr wrapper::org::webRtc::RTCRtpReceiveParameters::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpReceiveParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpReceiveParameters::~RTCRtpReceiveParameters() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpReceiveParameters::wrapper_init_org_webRtc_RTCRtpReceiveParameters() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.encodings = make_shared< decltype(to.encodings)::element_type >();

  UseRtpParameters::apply(from, to);

  for (auto iter = from.encodings.begin(); iter != from.encodings.end(); ++iter) {
    auto converted = UseRtpDecodingParameters::toWrapper(*iter);
    if (!converted) continue;
    to.encodings->push_back(converted);
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  UseRtpParameters::apply(from, to);

  if (from.encodings) {
    for (auto iter = from.encodings->begin(); iter != from.encodings->end(); ++iter) {
      auto converted = UseRtpDecodingParameters::toNative(*iter);
      if (!converted) continue;
      to.encodings.push_back(*converted);
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

