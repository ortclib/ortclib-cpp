
#include "impl_org_ortc_ConstrainDoubleRange.h"
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
wrapper::impl::org::ortc::ConstrainDoubleRange::ConstrainDoubleRange()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ConstrainDoubleRangePtr wrapper::org::ortc::ConstrainDoubleRange::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainDoubleRange>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainDoubleRange::~ConstrainDoubleRange()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainDoubleRange::wrapper_init_org_ortc_ConstrainDoubleRange()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainDoubleRange::wrapper_init_org_ortc_ConstrainDoubleRange(wrapper::org::ortc::ConstrainDoubleRangePtr source)
{
  if (!source) return;

  wrapper::org::ortc::ConstrainDoubleRangePtr pThis = thisWeak_.lock();
  (*pThis) = (*source);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainDoubleRange::wrapper_init_org_ortc_ConstrainDoubleRange(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeConstrainDoubleRange>(Json::toNative(json));

  wrapper::org::ortc::ConstrainDoubleRangePtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::ConstrainDoubleRangePtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::ConstrainDoubleRange::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("ConstrainDoubleRange"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::ConstrainDoubleRange::hash()
{
  return toNative(thisWeak_.lock())->hash();
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainDoubleRangePtr  wrapper::impl::org::ortc::ConstrainDoubleRange::toWrapper(NativeConstrainDoubleRangePtr native)
{
  if (!native) return ConstrainDoubleRangePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainDoubleRangePtr  wrapper::impl::org::ortc::ConstrainDoubleRange::toWrapper(const NativeConstrainDoubleRange &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainDoubleRange>();
  pThis->thisWeak_ = pThis;

  pThis->min = native.mMin;
  pThis->max = native.mMax;
  pThis->exact = native.mExact;
  pThis->ideal = native.mIdeal;

  pThis->wrapper_init_org_ortc_ConstrainDoubleRange();
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainDoubleRange::NativeConstrainDoubleRangePtr wrapper::impl::org::ortc::ConstrainDoubleRange::toNative(wrapper::org::ortc::ConstrainDoubleRangePtr wrapper)
{
  if (!wrapper) return NativeConstrainDoubleRangePtr();

  auto native = make_shared<NativeConstrainDoubleRange>();
  native->mMin = wrapper->min;
  native->mMax = wrapper->max;
  native->mExact = wrapper->exact;
  native->mIdeal = wrapper->ideal;

  return native;
}

