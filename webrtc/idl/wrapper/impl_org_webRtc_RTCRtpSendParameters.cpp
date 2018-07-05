
#include "impl_org_webRtc_RTCRtpSendParameters.h"
#include "impl_org_webRtc_RTCRtpParameters.h"
#include "impl_org_webRtc_RTCRtpEncodingParameters.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSendParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpParameters, UseRtpParameters);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpEncodingParameters, UseRtpEncodingParameters);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpSendParameters::RTCRtpSendParameters() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSendParametersPtr wrapper::org::webRtc::RTCRtpSendParameters::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpSendParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpSendParameters::~RTCRtpSendParameters() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpSendParameters::wrapper_init_org_webRtc_RTCRtpSendParameters() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.encodings = make_shared< decltype(to.encodings)::element_type >();

  UseRtpParameters::apply(from, to);

  to.transactionId = from.transaction_id;
  to.degradationPreference = UseEnum::toWrapper(from.degradation_preference);
  for (auto iter = from.encodings.begin(); iter != from.encodings.end(); ++iter) {
    auto converted = UseRtpEncodingParameters::toWrapper(*iter);
    if (!converted) continue;
    to.encodings->push_back(converted);
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  UseRtpParameters::apply(from, to);

  to.transaction_id = from.transactionId;
  to.degradation_preference = UseEnum::toNative(from.degradationPreference);
  if (from.encodings) {
    for (auto iter = from.encodings->begin(); iter != from.encodings->end(); ++iter) {
      auto converted = UseRtpEncodingParameters::toNative(*iter);
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

