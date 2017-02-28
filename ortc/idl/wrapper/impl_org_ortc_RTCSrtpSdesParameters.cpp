
#include "impl_org_ortc_RTCSrtpSdesParameters.h"
#include "impl_org_ortc_RTCSrtpSdesCryptoParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCSrtpSdesParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesParameters::RTCSrtpSdesParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCSrtpSdesParametersPtr wrapper::org::ortc::RTCSrtpSdesParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCSrtpSdesParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCSrtpSdesParameters::~RTCSrtpSdesParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesParameters::wrapper_init_org_ortc_RTCSrtpSdesParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesParameters::wrapper_init_org_ortc_RTCSrtpSdesParameters(wrapper::org::ortc::RTCSrtpSdesParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCSrtpSdesParameters::wrapper_init_org_ortc_RTCSrtpSdesParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCSrtpSdesParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCSrtpSdesParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCSrtpSdesParameters::hash()
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
  pThis->cryptoParams = make_shared< list< wrapper::org::ortc::RTCSrtpSdesCryptoParametersPtr > >();
  for (auto iter = native.mCryptoParams.begin(); iter != native.mCryptoParams.end(); ++iter) {
    auto wrapper = RTCSrtpSdesCryptoParameters::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->cryptoParams->push_back(wrapper);
  }
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  if (wrapper->cryptoParams) {
    for (auto iter = wrapper->cryptoParams->begin(); iter != wrapper->cryptoParams->end(); ++iter) {
      auto native = RTCSrtpSdesCryptoParameters::toNative(*iter);
      if (!native) continue;
      result->mCryptoParams.push_back(*native);
    }
  }
  return result;
}
