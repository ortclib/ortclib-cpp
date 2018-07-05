
#include "impl_org_webRtc_RTCRtpEncodingParameters.h"
#include "impl_org_webRtc_RTCRtpCodingParameters.h"
#include "impl_org_webRtc_enums.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpEncodingParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCodingParameters, UseRtpCodingParameters);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpEncodingParameters::RTCRtpEncodingParameters() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpEncodingParametersPtr wrapper::org::webRtc::RTCRtpEncodingParameters::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpEncodingParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpEncodingParameters::~RTCRtpEncodingParameters() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCRtpEncodingParameters::wrapper_init_org_webRtc_RTCRtpEncodingParameters() noexcept
{
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to) noexcept
{
  UseRtpCodingParameters::apply(from, to);

  if (from.dtx.has_value()) {
    to.dtx = UseEnum::toWrapper(from.dtx.value());
  }

  to.priority = UseEnum::toWrapperRTCPriorityType(from.bitrate_priority);

  if (from.ptime.has_value()) {
    to.ptime = decltype(to.ptime)(((decltype(to.ptime)::rep)SafeInt<decltype(to.ptime)::rep>(from.ptime.value())));
  }

  if (from.max_bitrate_bps.has_value()) {
    to.maxBitrate = SafeInt<decltype(to.maxBitrate)::value_type>(from.max_bitrate_bps.value());
  }

  if (from.max_framerate.has_value()) {
    to.maxFramerate = decltype(to.maxFramerate)::value_type(from.max_framerate.value());
  }

  to.scaleResolutionDownBy = from.scale_resolution_down_by;
  to.scaleFramerateDownBy = from.scale_framerate_down_by;

  to.dependencyEncodingIds = make_shared< list<String> >();
  if (from.dependency_rids.size() > 0) {
    for (auto iter = from.dependency_rids.begin(); iter != from.dependency_rids.end(); ++iter) {
      to.dependencyEncodingIds->push_back(*iter);
    }
  }
}

//------------------------------------------------------------------------------
static void apply(const WrapperImplType &from, NativeType &to) noexcept
{
  UseRtpCodingParameters::apply(from, to);

  if (from.dtx.has_value()) {
    to.dtx = UseEnum::toNative(from.dtx.value());
  }

  to.bitrate_priority = UseEnum::toNative(from.priority);

  if (from.ptime != decltype(from.ptime)()) {
    to.ptime = (int)SafeInt<int>(from.ptime.count());
  }

  if (from.maxBitrate.has_value()) {
    to.max_bitrate_bps = (int)SafeInt<int>(from.maxBitrate.value());
  }

  if (from.maxFramerate.has_value()) {
    to.max_framerate = int(from.maxFramerate.value());
  }

  to.scale_resolution_down_by = from.scaleResolutionDownBy;
  to.scale_framerate_down_by = from.scaleFramerateDownBy;

  if (from.dependencyEncodingIds) {
    for (auto iter = from.dependencyEncodingIds->begin(); iter != from.dependencyEncodingIds->end(); ++iter) {
      to.dependency_rids.push_back(*iter);
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

