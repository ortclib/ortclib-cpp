
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
wrapper::impl::org::ortc::ConstrainBooleanParameters::ConstrainBooleanParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::ConstrainBooleanParametersPtr wrapper::org::ortc::ConstrainBooleanParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainBooleanParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainBooleanParameters::~ConstrainBooleanParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainBooleanParameters::wrapper_init_org_ortc_ConstrainBooleanParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainBooleanParameters::wrapper_init_org_ortc_ConstrainBooleanParameters(wrapper::org::ortc::ConstrainBooleanParametersPtr source)
{
  if (!source) return;
  wrapper::org::ortc::ConstrainBooleanParametersPtr pThis = thisWeak_.lock();
  (*pThis) = (*source);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::ConstrainBooleanParameters::wrapper_init_org_ortc_ConstrainBooleanParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeConstrainBooleanParameters>(Json::toNative(json));

  wrapper::org::ortc::ConstrainBooleanParametersPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::ConstrainBooleanParametersPtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::ConstrainBooleanParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("ConstrainBooleanParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::ConstrainBooleanParameters::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainBooleanParametersPtr wrapper::impl::org::ortc::ConstrainBooleanParameters::toWrapper(NativeConstrainBooleanParametersPtr native)
{
  if (!native) return ConstrainBooleanParametersPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainBooleanParametersPtr wrapper::impl::org::ortc::ConstrainBooleanParameters::toWrapper(const NativeConstrainBooleanParameters &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::ConstrainBooleanParameters>();
  pThis->thisWeak_ = pThis;

  pThis->exact = native.mExact;
  pThis->ideal = native.mIdeal;

  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::ConstrainBooleanParameters::NativeConstrainBooleanParametersPtr wrapper::impl::org::ortc::ConstrainBooleanParameters::toNative(wrapper::org::ortc::ConstrainBooleanParametersPtr wrapper)
{
  if (!wrapper) return NativeConstrainBooleanParametersPtr();

  auto result = make_shared<NativeConstrainBooleanParameters>();

  result->mExact = wrapper->exact;
  result->mIdeal = wrapper->ideal;

  return result;
}
