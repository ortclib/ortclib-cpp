
#include "impl_org_ortc_StringOrStringList.h"
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
wrapper::impl::org::ortc::StringOrStringList::StringOrStringList() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::StringOrStringListPtr wrapper::org::ortc::StringOrStringList::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::StringOrStringList>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::StringOrStringList::~StringOrStringList() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::StringOrStringList::wrapper_init_org_ortc_StringOrStringList() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::StringOrStringList::wrapper_init_org_ortc_StringOrStringList(wrapper::org::ortc::StringOrStringListPtr source) noexcept
{
  if (!source) return;
  wrapper::org::ortc::StringOrStringListPtr pThis = thisWeak_.lock();
  (*pThis) = (*source);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::StringOrStringList::wrapper_init_org_ortc_StringOrStringList(wrapper::org::ortc::JsonPtr json) noexcept
{
  if (!json) return;

  auto native = make_shared<NativeStringOrStringList>(Json::toNative(json));

  wrapper::org::ortc::StringOrStringListPtr pThis = thisWeak_.lock();
  wrapper::org::ortc::StringOrStringListPtr wrapper = toWrapper(native);

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::StringOrStringList::toJson() noexcept
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("StringOrStringList"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::StringOrStringList::hash() noexcept
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::StringOrStringListPtr wrapper::impl::org::ortc::StringOrStringList::toWrapper(NativeStringOrStringListPtr native) noexcept
{
  if (!native) return StringOrStringListPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::StringOrStringListPtr wrapper::impl::org::ortc::StringOrStringList::toWrapper(const NativeStringOrStringList &native) noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::StringOrStringList>();
  pThis->thisWeak_ = pThis;

  pThis->value = native.mValue;
  if (native.mValues.hasValue()) {
    pThis->values = make_shared< list<String> >(native.mValues.value());
  }

  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::StringOrStringList::NativeStringOrStringListPtr wrapper::impl::org::ortc::StringOrStringList::toNative(wrapper::org::ortc::StringOrStringListPtr wrapper) noexcept
{
  if (!wrapper) return NativeStringOrStringListPtr();

  auto result = make_shared<NativeStringOrStringList>();

  result->mValue = wrapper->value;
  if (wrapper->values.hasValue()) {
    if (wrapper->values.value()) {
      result->mValues = *(wrapper->values.value());
    }
  }

  return result;
}
