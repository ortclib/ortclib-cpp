
#include "impl_org_webRtc_VideoFormat.h"

#include "impl_org_webRtc_pre_include.h"
#include "media/base/videocommon.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoFormat::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::VideoFormat::VideoFormat() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::VideoFormatPtr wrapper::org::webRtc::VideoFormat::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::VideoFormat>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::VideoFormat::~VideoFormat() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::VideoFormat::IsPixelRateLess(wrapper::org::webRtc::VideoFormatPtr other) noexcept
{
  if (!other) return false;

  ZS_ASSERT(native_);
  if (!native_) return false;

  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, other);
  ZS_ASSERT(converted);
  if (!converted) return false;

  return native_->IsPixelRateLess(*(converted->native_));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::VideoFormat::wrapper_init_org_webRtc_VideoFormat() noexcept
{
  native_ = make_shared<NativeType>();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::VideoFormat::wrapper_init_org_webRtc_VideoFormat(wrapper::org::webRtc::VideoFormatPtr source) noexcept
{
  if (!source) {
    wrapper_init_org_webRtc_VideoFormat();
    return;
  }

  native_ = toNative(source);
}

//------------------------------------------------------------------------------
int wrapper::impl::org::webRtc::VideoFormat::get_width() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return int{};
  return SafeInt<int>(native_->width);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::VideoFormat::set_width(int value) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;
  native_->width = SafeInt<decltype(native_->width)>(value);
}

//------------------------------------------------------------------------------
int wrapper::impl::org::webRtc::VideoFormat::get_height() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return int{};
  return SafeInt<int>(native_->height);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::VideoFormat::set_height(int value) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;
  native_->height = SafeInt<decltype(native_->height)>(value);
}

//------------------------------------------------------------------------------
::zsLib::Nanoseconds wrapper::impl::org::webRtc::VideoFormat::get_interval() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return ::zsLib::Nanoseconds{};
  return ::zsLib::Nanoseconds{ native_->interval = SafeInt<::zsLib::Nanoseconds::rep>(native_->interval) };
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::VideoFormat::set_interval(::zsLib::Nanoseconds value) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;
  native_->interval = SafeInt<decltype(native_->interval)>(value.count());
}

//------------------------------------------------------------------------------
uint32_t wrapper::impl::org::webRtc::VideoFormat::get_fourcc() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return uint32_t{};

  return SafeInt<uint32_t>(native_->fourcc);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::VideoFormat::set_fourcc(uint32_t value) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;
  native_->fourcc = value;
}

//------------------------------------------------------------------------------
int wrapper::impl::org::webRtc::VideoFormat::get_framerate() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return int{};

  return native_->framerate();
}

//------------------------------------------------------------------------------
float wrapper::impl::org::webRtc::VideoFormat::get_framerateFloat() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return float{};

  return NativeType::IntervalToFpsFloat(native_->interval);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::VideoFormat::get_isSize0x0() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;

  return native_->IsSize0x0();
}


//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native) noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = make_shared<NativeType>(native);
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return NativeTypePtr();

  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  ZS_ASSERT(converted);
  if (!converted) return NativeTypePtr();
  ZS_ASSERT(converted->native_);
  return make_shared<NativeType>(*converted->native_);
}
