
#include "impl_org_webRtc_MillisecondIntervalRange.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/timeutils.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MillisecondIntervalRange::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MillisecondIntervalRange::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MillisecondIntervalRange::NativeType, NativeType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::MillisecondIntervalRange::MillisecondIntervalRange() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MillisecondIntervalRangePtr wrapper::org::webRtc::MillisecondIntervalRange::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::MillisecondIntervalRange>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::MillisecondIntervalRange::~MillisecondIntervalRange() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::MillisecondIntervalRange::wrapper_init_org_webRtc_MillisecondIntervalRange() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::MillisecondIntervalRange::wrapper_init_org_webRtc_MillisecondIntervalRange(wrapper::org::webRtc::MillisecondIntervalRangePtr source) noexcept
{
  if (!source) {
    wrapper_init_org_webRtc_MillisecondIntervalRange();
    return;
  }

  min = source->min;
  max = source->max;
}

//------------------------------------------------------------------------------
static void apply(const NativeType &from, WrapperImplType &to)
{
  to.min = decltype(to.min)(SafeInt<decltype(to.min)::rep>(from.min()));
  to.max = decltype(to.min)(SafeInt<decltype(to.max)::rep>(from.max()));
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
  return make_shared<NativeType>(SafeInt<int>(wrapper->min.count()), SafeInt<int>(wrapper->max.count()));
}
