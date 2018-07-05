
#include "impl_org_webRtc_RTCRtpCodecParameters.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCRtcpFeedback.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpparameters.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/SafeInt.h>
#include <zsLib/IHelper.h>

#include <sstream>

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodecParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtcpFeedback, UseRtcpFeedback);

ZS_DECLARE_TYPEDEF_PTR(::zsLib::IHelper, UseHelper);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpCodecParameters::RTCRtpCodecParameters() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpCodecParametersPtr wrapper::org::webRtc::RTCRtpCodecParameters::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpCodecParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpCodecParameters::~RTCRtpCodecParameters() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpCodecParameters::wrapper_init_org_webRtc_RTCRtpCodecParameters() noexcept
{
}

//------------------------------------------------------------------------------
String WrapperImplType::toFmtp(const std::unordered_map<std::string, std::string> &input) noexcept
{
  if (input.size() < 1) return String();

  std::stringstream ss;
  bool first{ true };
  for (auto iter = input.begin(); iter != input.end(); ++iter) {
    auto &name = (*iter).first;
    auto &value = (*iter).second;

    if (!first) {
      ss << "; ";
    }
    ss << name;
    if (value.size() > 0) {
      ss << "=";
      ss << value;
    }
  }
  return ss.str();
}

//------------------------------------------------------------------------------
void WrapperImplType::fromFmtp(const String &input, std::unordered_map<std::string, std::string> &output) noexcept
{
  if (!input.hasData()) return;

  UseHelper::SplitMap splits;
  UseHelper::split(input, splits, " ");

  for (auto iter = splits.begin(); iter != splits.end(); ++iter) {
    UseHelper::SplitMap splitsSemi;
    UseHelper::split((*iter).second, splitsSemi, ";");
    for (auto iterSemi = splitsSemi.begin(); iterSemi != splitsSemi.end(); ++iter) {
      UseHelper::SplitMap splitsEquals;
      UseHelper::split((*iterSemi).second, splitsEquals, "=");

      if (splitsEquals.size() > 0) {
        String name = splitsEquals[0];
        splitsEquals.erase(splitsEquals.begin());

        String combined;
        if (splitsEquals.size() > 1) {
          combined = UseHelper::combine(splitsEquals, "=");
        }
        if (name.hasData()) {
          output[name] = combined;
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.payloadType = SafeInt<decltype(to.payloadType)>(from.payload_type);
  to.name = from.name;
  to.mimeType = from.mime_type();
  if (from.clock_rate.has_value()) {
    to.clockRate = SafeInt<decltype(to.clockRate)::value_type>(from.clock_rate.value());
  }
  if (from.num_channels.has_value()) {
    to.channels = SafeInt<decltype(to.channels)::value_type>(from.num_channels.value());
  }
  to.sdpFmtpLine = WrapperImplType::toFmtp(from.parameters);
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
  to.payload_type = SafeInt<decltype(to.payload_type)>(from.payloadType);
  to.name = from.name;

  {
    UseHelper::SplitMap splits;
    UseHelper::split(from.mimeType, splits, "/");
    if (splits.size() > 0) {
      try {
        to.kind = UseEnum::toNativeMediaType(splits[0]);
      } catch (const ::zsLib::Exceptions::InvalidArgument &) {
      }
    }
  }
  if (from.clockRate.has_value()) {
    to.clock_rate = (int)SafeInt<int>(from.clockRate.value());
  }
  if (from.channels.has_value()) {
    to.num_channels = (int)SafeInt<int>(from.channels.value());
  }
  WrapperImplType::fromFmtp(from.sdpFmtpLine, to.parameters);
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
