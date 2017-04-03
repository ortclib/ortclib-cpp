
#include "impl_org_ortc_ConstrainLong.h"
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
wrapper::impl::org::ortc::ConstrainLong::ConstrainLong()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ConstrainLongPtr wrapper::org::ortc::ConstrainLong::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainLong>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLong::~ConstrainLong()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainLong::wrapper_init_org_ortc_ConstrainLong()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainLong::wrapper_init_org_ortc_ConstrainLong(wrapper::org::ortc::ConstrainLongPtr source)
{
  if (!source) return;

  wrapper::org::ortc::ConstrainLongPtr pThis = thisWeak_.lock();
  (*pThis) = (*source);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainLong::wrapper_init_org_ortc_ConstrainLong(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeConstrainLong>(Json::toNative(json));
  wrapper::org::ortc::ConstrainLongPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::ConstrainLongPtr pThis = thisWeak_.lock();

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::ConstrainLong::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("ConstrainLong"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::ConstrainLong::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLongPtr wrapper::impl::org::ortc::ConstrainLong::toWrapper(NativeConstrainLongPtr native)
{
  if (!native) return ConstrainLongPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLongPtr wrapper::impl::org::ortc::ConstrainLong::toWrapper(const NativeConstrainLong &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainLong>();
  pThis->thisWeak_ = pThis;

  pThis->value = native.mValue;
  if (native.mRange.hasValue()) {
    pThis->range = ConstrainLongRange::toWrapper(native.mRange.value());
  }

  pThis->wrapper_init_org_ortc_ConstrainLong();
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainLong::NativeConstrainLongPtr wrapper::impl::org::ortc::ConstrainLong::toNative(wrapper::org::ortc::ConstrainLongPtr wrapper)
{
  if (!wrapper) return NativeConstrainLongPtr();

  auto result = make_shared<NativeConstrainLong>();
  result->mValue = wrapper->value;
  if (wrapper->range.hasValue()) {
    auto native = ConstrainLongRange::toNative(wrapper->range.value());
    if (native) {
      result->mRange = *native;
    }
  }
  return result;
}
