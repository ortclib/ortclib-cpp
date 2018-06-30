
#include "impl_org_ortc_ConstrainString.h"
#include "impl_org_ortc_StringOrStringList.h"
#include "impl_org_ortc_ConstrainStringParameters.h"
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
wrapper::impl::org::ortc::ConstrainString::ConstrainString() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ConstrainStringPtr wrapper::org::ortc::ConstrainString::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainString>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainString::~ConstrainString() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainString::wrapper_init_org_ortc_ConstrainString() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainString::wrapper_init_org_ortc_ConstrainString(wrapper::org::ortc::ConstrainStringPtr source) noexcept
{
  if (!source) return;

  wrapper::org::ortc::ConstrainStringPtr pThis = thisWeak_.lock();
  (*pThis) = (*source);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainString::wrapper_init_org_ortc_ConstrainString(wrapper::org::ortc::JsonPtr json) noexcept
{
  if (!json) return;

  auto native = make_shared<NativeConstrainString>(Json::toNative(json));
  wrapper::org::ortc::ConstrainStringPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::ConstrainStringPtr pThis = thisWeak_.lock();

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::ConstrainString::toJson() noexcept
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("ConstrainString"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::ConstrainString::hash() noexcept
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainStringPtr wrapper::impl::org::ortc::ConstrainString::toWrapper(NativeConstrainStringPtr native) noexcept
{
  if (!native) return ConstrainStringPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainStringPtr wrapper::impl::org::ortc::ConstrainString::toWrapper(const NativeConstrainString &native) noexcept
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainString>();
  pThis->thisWeak_ = pThis;

  if (native.mValue.hasValue()) {
    pThis->value = StringOrStringList::toWrapper(native.mValue.value());
  }
  if (native.mParameters.hasValue()) {
    pThis->parameters = ConstrainStringParameters::toWrapper(native.mParameters);
  }
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainString::NativeConstrainStringPtr wrapper::impl::org::ortc::ConstrainString::toNative(wrapper::org::ortc::ConstrainStringPtr wrapper) noexcept
{
  if (!wrapper) return NativeConstrainStringPtr();

  auto result = make_shared<NativeConstrainString>();

  if (wrapper->value.hasValue()) {
    if (wrapper->value.value()) {
      result->mValue = *StringOrStringList::toNative(wrapper->value.value());
    }
  }
  if (wrapper->parameters.hasValue()) {
    if (wrapper->parameters.value()) {
      result->mParameters = *ConstrainStringParameters::toNative(wrapper->parameters.value());
    }
  }

  return result;
}

