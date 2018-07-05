
#include "impl_org_webRtc_RTCRtpCodingParameters.h"
#include "impl_org_webRtc_RTCRtpRtxParameters.h"
#include "impl_org_webRtc_RTCRtpFecParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpparameters.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/SafeInt.h>

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodingParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpRtxParameters, UseRtxParameters);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpFecParameters, UseFecParameters);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpCodingParameters::RTCRtpCodingParameters() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpCodingParametersPtr wrapper::org::webRtc::RTCRtpCodingParameters::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpCodingParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpCodingParameters::~RTCRtpCodingParameters() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpCodingParameters::wrapper_init_org_webRtc_RTCRtpCodingParameters() noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::apply(const NativeType &from, WrapperType &to) noexcept
{
  to.rid = from.rid;
  if (from.ssrc.has_value()) {
    to.ssrc = SafeInt<decltype(to.ssrc)::value_type>(from.ssrc.value());
  }
  if (from.codec_payload_type.has_value()) {
    to.codecPayloadType = SafeInt<decltype(to.codecPayloadType)::value_type>(from.codec_payload_type.value());
  }

  if (from.fec.has_value()) {
    auto temp = UseFecParameters::toWrapper(from.fec.value());
    if (temp) to.fec = temp;
  }
  if (from.rtx.has_value()) {
    auto temp = UseRtxParameters::toWrapper(from.rtx.value());
    if (temp) to.rtx = temp;
  }
  to.active = from.active;

}

//------------------------------------------------------------------------------
void WrapperImplType::apply(const WrapperType &from, NativeType &to) noexcept
{
  to.rid = from.rid;
  if (from.ssrc.has_value()) {
    to.ssrc = (uint32_t)SafeInt<uint32_t>(from.ssrc.value());
  }
  if (from.codecPayloadType.has_value()) {
    to.codec_payload_type = (int)SafeInt<int>(from.codecPayloadType.value());
  }
  if (from.fec.has_value()) {
    auto temp = UseFecParameters::toNative(from.fec.value());
    if (temp) to.fec = *temp;
  }
  if (from.rtx.has_value()) {
    auto temp = UseRtxParameters::toNative(from.rtx.value());
    if (temp) to.rtx = *temp;
  }
  to.active = from.active;
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

