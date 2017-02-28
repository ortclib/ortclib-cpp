
#include "impl_org_ortc_LongRange.h"
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
wrapper::impl::org::ortc::LongRange::LongRange()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::LongRangePtr wrapper::org::ortc::LongRange::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::LongRange>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::LongRange::~LongRange()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::LongRange::wrapper_init_org_ortc_LongRange()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::LongRange::wrapper_init_org_ortc_LongRange(long value)
{
  min = max = value;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::LongRange::wrapper_init_org_ortc_LongRange(
  long min,
  long max
  )
{
  this->min = min;
  this->max = max;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::LongRange::wrapper_init_org_ortc_LongRange(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeCapabilityLong>(Json::toNative(json));

  wrapper::org::ortc::LongRangePtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::LongRangePtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::LongRange::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("LongRange"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::LongRange::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::LongRangePtr wrapper::impl::org::ortc::LongRange::toWrapper(NativeCapabilityLongPtr native)
{
  if (!native) return LongRangePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::LongRangePtr wrapper::impl::org::ortc::LongRange::toWrapper(const NativeCapabilityLong &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::LongRange>();
  pThis->thisWeak_ = pThis;

  pThis->min = native.mMin;
  pThis->max = native.mMax;

  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::LongRange::NativeCapabilityLongPtr wrapper::impl::org::ortc::LongRange::toNative(wrapper::org::ortc::LongRangePtr wrapper)
{
  if (!wrapper) return NativeCapabilityLongPtr();

  auto result = make_shared<NativeCapabilityLong>();

  result->mMin = wrapper->min;
  result->mMax = wrapper->max;

  return result;
}
