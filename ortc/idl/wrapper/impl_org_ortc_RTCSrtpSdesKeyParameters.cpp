
#include "impl_org_ortc_RTCSrtpSdesKeyParameters.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_Helper.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::RTCSrtpSdesKeyParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSrtpSdesKeyParametersPtr wrapper::org::ortc::RTCSrtpSdesKeyParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::~RTCSrtpSdesKeyParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::wrapper_init_org_ortc_RTCSrtpSdesKeyParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::wrapper_init_org_ortc_RTCSrtpSdesKeyParameters(wrapper::org::ortc::RTCSrtpSdesKeyParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::wrapper_init_org_ortc_RTCSrtpSdesKeyParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCSrtpSdesKeyParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSrtpSdesKeyParameters::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->keyMethod = native.mKeyMethod;
  pThis->keySalt = native.mKeySalt;
  pThis->lifetime = native.mLifetime;
  pThis->mkiValue = native.mMKIValue;
  pThis->mkiLength = SafeInt<decltype(pThis->mkiLength)>(native.mMKILength);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mKeyMethod = wrapper->keyMethod;
  result->mKeySalt = wrapper->keySalt;
  result->mLifetime = wrapper->lifetime;
  result->mMKIValue = wrapper->mkiValue;
  result->mMKILength = SafeInt<decltype(result->mMKILength)>(wrapper->mkiLength);
  return result;
}
