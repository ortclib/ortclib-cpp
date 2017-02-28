
#include "impl_org_ortc_RTCSrtpSdesCryptoParameters.h"
#include "impl_org_ortc_RTCSrtpSdesKeyParameters.h"
#include "impl_org_ortc_Helper.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::RTCSrtpSdesCryptoParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr wrapper::org::ortc::RTCSrtpSdesCryptoParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::~RTCSrtpSdesCryptoParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters(wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::wrapper_init_org_ortc_RTCSrtpSdesCryptoParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCSrtpSdesCryptoParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSrtpSdesCryptoParameters::hash()
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
  pThis->tag = SafeInt<decltype(pThis->tag)>(native.mTag);
  pThis->cryptoSuite = native.mCryptoSuite;
  pThis->keyParams = make_shared< list< wrapper::org::ortc::RTCSrtpSdesKeyParametersPtr > >();
  for (auto iter = native.mKeyParams.begin(); iter != native.mKeyParams.end(); ++iter) {
    auto wrapper = RTCSrtpSdesKeyParameters::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->keyParams->push_back(wrapper);
  }
  pThis->sessionParams = make_shared< list< String > >(native.mSessionParams);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mTag = SafeInt<decltype(result->mTag)>(wrapper->tag);
  if (wrapper->keyParams) {
    for (auto iter = wrapper->keyParams->begin(); iter != wrapper->keyParams->end(); ++iter) {
      auto native = RTCSrtpSdesKeyParameters::toNative(*iter);
      if (!native) continue;
      result->mKeyParams.push_back(*native);
    }
  }
  if (wrapper->sessionParams) {
    result->mSessionParams = *(wrapper->sessionParams);
  }
  return result;
}
