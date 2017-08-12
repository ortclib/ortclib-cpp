
#include "impl_org_ortc_ConstrainBoolean.h"
#include "impl_org_ortc_ConstrainBooleanParameters.h"
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
wrapper::impl::org::ortc::ConstrainBoolean::ConstrainBoolean()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ConstrainBooleanPtr wrapper::org::ortc::ConstrainBoolean::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainBoolean>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainBoolean::~ConstrainBoolean()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainBoolean::wrapper_init_org_ortc_ConstrainBoolean()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainBoolean::wrapper_init_org_ortc_ConstrainBoolean(wrapper::org::ortc::ConstrainBooleanPtr source)
{
  if (!source) return;

  wrapper::org::ortc::ConstrainBooleanPtr pThis = thisWeak_.lock();
  (*pThis) = (*source);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainBoolean::wrapper_init_org_ortc_ConstrainBoolean(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeConstrainBoolean>(Json::toNative(json));

  wrapper::org::ortc::ConstrainBooleanPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::ConstrainBooleanPtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::ConstrainBoolean::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("ConstrainBoolean"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::ConstrainBoolean::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainBooleanPtr wrapper::impl::org::ortc::ConstrainBoolean::toWrapper(NativeConstrainBooleanPtr native)
{
  if (!native) return ConstrainBooleanPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainBooleanPtr wrapper::impl::org::ortc::ConstrainBoolean::toWrapper(const NativeConstrainBoolean &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainBoolean>();
  pThis->thisWeak_ = pThis;

  pThis->value = native.mValue;
  if (native.mParameters.hasValue()) {
    pThis->parameters = ConstrainBooleanParameters::toWrapper(native.mParameters.value());
  }

  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainBoolean::NativeConstrainBooleanPtr wrapper::impl::org::ortc::ConstrainBoolean::toNative(wrapper::org::ortc::ConstrainBooleanPtr wrapper)
{
  auto result = make_shared<NativeConstrainBoolean>();

  result->mValue = wrapper->value;
  if (wrapper->parameters.hasValue()) {
    if (wrapper->parameters.value()) {
      result->mParameters = *ConstrainBooleanParameters::toNative(wrapper->parameters.value());
    }
  }

  return result;
}
