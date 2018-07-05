
#include "impl_org_webRtc_RTCRtpCodecCapability.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCRtcpFeedback.h"
#include "impl_org_webRtc_RTCRtpCodecParameters.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpparameters.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/SafeInt.h>
#include <zsLib/IHelper.h>

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodecCapability::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtcpFeedback, UseRtcpFeedback);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodecParameters, UseRtpCodecParameters);

ZS_DECLARE_TYPEDEF_PTR(::zsLib::IHelper, UseHelper);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpCodecCapability::RTCRtpCodecCapability() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpCodecCapabilityPtr wrapper::org::webRtc::RTCRtpCodecCapability::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpCodecCapability>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpCodecCapability::~RTCRtpCodecCapability() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpCodecCapability::wrapper_init_org_webRtc_RTCRtpCodecCapability() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  if (from.preferred_payload_type.has_value()) {
    to.preferredPayloadType = SafeInt<decltype(to.preferredPayloadType)::value_type>(from.preferred_payload_type.value());
  }
  to.name = from.name;
  to.mimeType = from.mime_type();
  if (from.clock_rate.has_value()) {
    to.clockRate = SafeInt<decltype(to.clockRate)::value_type>(from.clock_rate.value());
  }
  if (from.num_channels.has_value()) {
    to.channels = SafeInt<decltype(to.channels)::value_type>(from.num_channels.value());
  }
  to.sdpFmtpLine = UseRtpCodecParameters::toFmtp(from.parameters);
  if (from.ptime.has_value()) {
    to.ptime = decltype(to.ptime)(SafeInt<decltype(to.ptime)::rep>(from.ptime.value()));
  }
  if (from.max_ptime.has_value()) {
    to.maxPtime = decltype(to.maxPtime)(SafeInt<decltype(to.maxPtime)::rep>(from.max_ptime.value()));
  }
  to.rtcpFeedback = make_shared<decltype(to.rtcpFeedback)::element_type>();
  for (auto iter = from.rtcp_feedback.begin(); iter != from.rtcp_feedback.end(); ++iter) {
    auto wrapper = UseRtcpFeedback::toWrapper(*iter);
    if (!wrapper) continue;
    to.rtcpFeedback->push_back(wrapper);
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to)
{
  if (from.preferredPayloadType.has_value()) {
    to.preferred_payload_type = (int)SafeInt<int>(from.preferredPayloadType.value());
  }
  to.name = from.name;

  {
    UseHelper::SplitMap splits;
    UseHelper::split(from.mimeType, splits, "/");
    if (splits.size() > 0) {
      try {
        to.kind = UseEnum::toNativeMediaType(splits[0]);
      }
      catch (const ::zsLib::Exceptions::InvalidArgument &) {
      }
    }
  }
  if (from.clockRate.has_value()) {
    to.clock_rate = (int)SafeInt<int>(from.clockRate.value());
  }
  if (from.channels.has_value()) {
    to.num_channels = (int)SafeInt<int>(from.channels.value());
  }
  UseRtpCodecParameters::fromFmtp(from.sdpFmtpLine, to.parameters);
  if (decltype(from.ptime)() != from.ptime) {
    to.ptime = (int)SafeInt<int>(from.ptime.count());
  }
  if (decltype(from.maxPtime)() != from.maxPtime) {
    to.max_ptime = (int)SafeInt<int>(from.maxPtime.count());
  }
  if (from.rtcpFeedback) {
    for (auto iter = from.rtcpFeedback->begin(); iter != from.rtcpFeedback->end(); ++iter) {
      auto native = UseRtcpFeedback::toNative(*iter);
      if (!native) continue;
      to.rtcp_feedback.push_back(*native);
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
