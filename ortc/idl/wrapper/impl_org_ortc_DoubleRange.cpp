
#include "impl_org_ortc_DoubleRange.h"
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
wrapper::impl::org::ortc::DoubleRange::DoubleRange()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::DoubleRangePtr wrapper::org::ortc::DoubleRange::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::DoubleRange>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::DoubleRange::~DoubleRange()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::DoubleRange::wrapper_init_org_ortc_DoubleRange()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::DoubleRange::wrapper_init_org_ortc_DoubleRange(double value)
{
  min = max = value;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::DoubleRange::wrapper_init_org_ortc_DoubleRange(
  double min,
  double max
  )
{
  this->min = min;
  this->max = max;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::DoubleRange::wrapper_init_org_ortc_DoubleRange(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeCapabilityDouble>(Json::toNative(json));

  wrapper::org::ortc::DoubleRangePtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::DoubleRangePtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::DoubleRange::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("DoubleRange"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::DoubleRange::hash()
{
  return toNative(thisWeak_.lock())->hash();
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::DoubleRangePtr wrapper::impl::org::ortc::DoubleRange::toWrapper(NativeCapabilityDoublePtr native)
{
  if (!native) return DoubleRangePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::DoubleRangePtr wrapper::impl::org::ortc::DoubleRange::toWrapper(const NativeCapabilityDouble &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::DoubleRange>();
  pThis->thisWeak_ = pThis;

  pThis->min = native.mMin;
  pThis->max = native.mMax;

  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::DoubleRange::NativeCapabilityDoublePtr wrapper::impl::org::ortc::DoubleRange::toNative(wrapper::org::ortc::DoubleRangePtr wrapper)
{
  if (!wrapper) return NativeCapabilityDoublePtr();

  auto result = make_shared<NativeCapabilityDouble>();

  result->mMin = wrapper->min;
  result->mMax = wrapper->max;

  return result;
}

