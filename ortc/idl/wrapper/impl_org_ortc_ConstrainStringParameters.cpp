
#include "impl_org_ortc_ConstrainStringParameters.h"
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
wrapper::impl::org::ortc::ConstrainStringParameters::ConstrainStringParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ConstrainStringParametersPtr wrapper::org::ortc::ConstrainStringParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainStringParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainStringParameters::~ConstrainStringParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainStringParameters::wrapper_init_org_ortc_ConstrainStringParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainStringParameters::wrapper_init_org_ortc_ConstrainStringParameters(wrapper::org::ortc::ConstrainStringParametersPtr source)
{
  if (!source) return;
  wrapper::org::ortc::ConstrainStringParametersPtr pThis = thisWeak_.lock();

  auto native = toNative(source);
  if (!native) return;

  wrapper::org::ortc::ConstrainStringParametersPtr wrapper = toWrapper(native);
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainStringParameters::wrapper_init_org_ortc_ConstrainStringParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeConstrainStringParameters>(Json::toNative(json));

  wrapper::org::ortc::ConstrainStringParametersPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::ConstrainStringParametersPtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::ConstrainStringParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("ConstrainStringParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::ConstrainStringParameters::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainStringParametersPtr wrapper::impl::org::ortc::ConstrainStringParameters::toWrapper(NativeConstrainStringParametersPtr native)
{
  if (!native) return ConstrainStringParametersPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainStringParametersPtr wrapper::impl::org::ortc::ConstrainStringParameters::toWrapper(const NativeConstrainStringParameters &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainStringParameters>();
  pThis->thisWeak_ = pThis;

  if (native.mExact.hasValue()) {
    pThis->exact = StringOrStringList::toWrapper(native.mExact.value());
  }
  if (native.mIdeal.hasValue()) {
    pThis->ideal = StringOrStringList::toWrapper(native.mIdeal.value());
  }

  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainStringParameters::NativeConstrainStringParametersPtr wrapper::impl::org::ortc::ConstrainStringParameters::toNative(wrapper::org::ortc::ConstrainStringParametersPtr wrapper)
{
  if (!wrapper) return NativeConstrainStringParametersPtr();

  auto result = make_shared< NativeConstrainStringParameters >();

  if (wrapper->exact.hasValue()) {
    if (wrapper->exact.value()) {
      result->mExact = *StringOrStringList::toNative(wrapper->exact.value());
    }
  }
  if (wrapper->ideal.hasValue()) {
    if (wrapper->ideal.value()) {
      result->mIdeal = *StringOrStringList::toNative(wrapper->ideal.value());
    }
  }

  return result;
}
