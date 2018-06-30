
#include "impl_org_ortc_ConstrainLongRange.h"
#include "impl_org_ortc_Json.h"

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

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLongRange::ConstrainLongRange() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ConstrainLongRangePtr wrapper::org::ortc::ConstrainLongRange::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainLongRange>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLongRange::~ConstrainLongRange() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainLongRange::wrapper_init_org_ortc_ConstrainLongRange() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainLongRange::wrapper_init_org_ortc_ConstrainLongRange(wrapper::org::ortc::ConstrainLongRangePtr source) noexcept
{
  if (!source) return;

  wrapper::org::ortc::ConstrainLongRangePtr pThis = thisWeak_.lock();
  (*pThis) = (*source);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainLongRange::wrapper_init_org_ortc_ConstrainLongRange(wrapper::org::ortc::JsonPtr json) noexcept
{
  if (!json) return;

  auto native = make_shared<NativeConstrainLongRange>(Json::toNative(json));

  wrapper::org::ortc::ConstrainLongRangePtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::ConstrainLongRangePtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::ConstrainLongRange::toJson() noexcept
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("ConstrainLongRange"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::ConstrainLongRange::hash() noexcept
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLongRangePtr  wrapper::impl::org::ortc::ConstrainLongRange::toWrapper(NativeConstrainLongRangePtr native) noexcept
{
  if (!native) return ConstrainLongRangePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLongRangePtr  wrapper::impl::org::ortc::ConstrainLongRange::toWrapper(const NativeConstrainLongRange &native) noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainLongRange>();
  pThis->thisWeak_ = pThis;

  pThis->min = native.mMin;
  pThis->max = native.mMax;
  pThis->exact = native.mExact;
  pThis->ideal = native.mIdeal;

  pThis->wrapper_init_org_ortc_ConstrainLongRange();
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLongRange::NativeConstrainLongRangePtr wrapper::impl::org::ortc::ConstrainLongRange::toNative(wrapper::org::ortc::ConstrainLongRangePtr wrapper) noexcept
{
  if (!wrapper) return NativeConstrainLongRangePtr();

  auto native = make_shared<NativeConstrainLongRange>();
  native->mMin = wrapper->min;
  native->mMax = wrapper->max;
  native->mExact = wrapper->exact;
  native->mIdeal = wrapper->ideal;

  return native;
}

