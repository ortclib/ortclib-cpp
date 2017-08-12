
#include "impl_org_ortc_ConstrainDouble.h"
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
wrapper::impl::org::ortc::ConstrainDouble::ConstrainDouble()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ConstrainDoublePtr wrapper::org::ortc::ConstrainDouble::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainDouble>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainDouble::~ConstrainDouble()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainDouble::wrapper_init_org_ortc_ConstrainDouble()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainDouble::wrapper_init_org_ortc_ConstrainDouble(wrapper::org::ortc::ConstrainDoublePtr source)
{
  if (!source) return;

  wrapper::org::ortc::ConstrainDoublePtr pThis = thisWeak_.lock();
  (*pThis) = (*source);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainDouble::wrapper_init_org_ortc_ConstrainDouble(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeConstrainDouble>(Json::toNative(json));
  wrapper::org::ortc::ConstrainDoublePtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::ConstrainDoublePtr pThis = thisWeak_.lock();

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::ConstrainDouble::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("ConstrainDouble"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::ConstrainDouble::hash()
{
  return toNative(thisWeak_.lock())->hash();
}


//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainDoublePtr wrapper::impl::org::ortc::ConstrainDouble::toWrapper(NativeConstrainDoublePtr native)
{
  if (!native) return ConstrainDoublePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainDoublePtr wrapper::impl::org::ortc::ConstrainDouble::toWrapper(const NativeConstrainDouble &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainDouble>();
  pThis->thisWeak_ = pThis;

  pThis->value = native.mValue;
  if (native.mRange.hasValue()) {
    pThis->range = ConstrainDoubleRange::toWrapper(native.mRange.value());
  }

  pThis->wrapper_init_org_ortc_ConstrainDouble();
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainDouble::NativeConstrainDoublePtr wrapper::impl::org::ortc::ConstrainDouble::toNative(wrapper::org::ortc::ConstrainDoublePtr wrapper)
{
  if (!wrapper) return NativeConstrainDoublePtr();

  auto result = make_shared<NativeConstrainDouble>();
  result->mValue = wrapper->value;
  if (wrapper->range.hasValue()) {
    auto native = ConstrainDoubleRange::toNative(wrapper->range.value());
    if (native) {
      result->mRange = *native;
    }
  }
  return result;
}

