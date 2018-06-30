
#include "impl_org_webRtc_VideoTrackSource.h"
#include "impl_org_webRtc_MediaConstraints.h"
#include "impl_org_webRtc_VideoCapturer.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/mediastreaminterface.h"
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


// borrow types from call defintions
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoTrackSource::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoTrackSource::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoTrackSource::NativeType, NativeType);

typedef WrapperImplType::NativeScopedPtr NativeScopedPtr;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaConstraints, UseMediaConstraints);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoCapturer, UseVideoCapturer);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::VideoTrackSource::VideoTrackSource() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoTrackSourcePtr wrapper::org::webRtc::VideoTrackSource::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::VideoTrackSource>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::VideoTrackSource::~VideoTrackSource() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaSourceState wrapper::impl::org::webRtc::VideoTrackSource::get_state() noexcept
{
#pragma ZS_BUILD_NOTE("TODO","(robin)")
  wrapper::org::webRtc::MediaSourceState result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::VideoTrackSource::get_remote() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;
  return native_->remote();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoTrackSourcePtr wrapper::org::webRtc::VideoTrackSource::create(wrapper::org::webRtc::VideoCapturerPtr capturer) noexcept
{
  auto factory = UseWebrtcLib::peerConnectionFactory();
  ZS_ASSERT(factory);
  if (!factory) return WrapperTypePtr();

  auto converted = UseVideoCapturer::toNative(capturer);

  return WrapperImplType::toWrapper(factory->CreateVideoSource(converted.get()));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoTrackSourcePtr wrapper::org::webRtc::VideoTrackSource::create(
  wrapper::org::webRtc::VideoCapturerPtr capturer,
  wrapper::org::webRtc::MediaConstraintsPtr constraints
  ) noexcept
{
  auto factory = UseWebrtcLib::peerConnectionFactory();
  ZS_ASSERT(factory);
  if (!factory) return WrapperTypePtr();

  auto convertedCapture = UseVideoCapturer::toNative(capturer);
  auto convertedConstraints = UseMediaConstraints::toNative(constraints);

  return WrapperImplType::toWrapper(factory->CreateVideoSource(std::move(convertedCapture), convertedConstraints.get()));
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::VideoTrackSource::get_isScreencast() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;
  return native_->is_screencast();
}

//------------------------------------------------------------------------------
Optional< bool > wrapper::impl::org::webRtc::VideoTrackSource::get_needsDenoising() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;
  auto value = native_->needs_denoising();

  if (!value.has_value()) return Optional<bool>();
  return value.value();
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::VideoTrackSourceStatsPtr > wrapper::impl::org::webRtc::VideoTrackSource::get_stats() noexcept
{
#pragma ZS_BUILD_NOTE("TODO","(robin)")
  Optional< wrapper::org::webRtc::VideoTrackSourceStatsPtr > result {};
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = NativeScopedPtr(native);
  return result;
}

//------------------------------------------------------------------------------
NativeScopedPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeScopedPtr();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return NativeScopedPtr();
  return converted->native_;
}
