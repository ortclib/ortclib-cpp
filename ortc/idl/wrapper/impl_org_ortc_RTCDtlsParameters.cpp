
#include "impl_org_ortc_RTCDtlsParameters.h"
#include "impl_org_ortc_RTCDtlsFingerprint.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtlsParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtlsParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtlsParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtlsParameters::RTCDtlsParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtlsParametersPtr wrapper::org::ortc::RTCDtlsParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDtlsParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtlsParameters::~RTCDtlsParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsParameters::wrapper_init_org_ortc_RTCDtlsParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsParameters::wrapper_init_org_ortc_RTCDtlsParameters(wrapper::org::ortc::RTCDtlsParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtlsParameters::wrapper_init_org_ortc_RTCDtlsParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCDtlsParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCDtlsParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDtlsParameters::hash()
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
  pThis->role = Helper::toWrapper(native.mRole);
  pThis->fingerprints = make_shared< list< wrapper::org::ortc::RTCDtlsFingerprintPtr > >();
  for (auto iter = native.mFingerprints.begin(); iter != native.mFingerprints.end(); ++iter) {
    auto wrapper = RTCDtlsFingerprint::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->fingerprints->push_back(wrapper);
  }
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mRole = Helper::toNative(wrapper->role);
  if (wrapper->fingerprints) {
    for (auto iter = wrapper->fingerprints->begin(); iter != wrapper->fingerprints->end(); ++iter) {
      auto native = RTCDtlsFingerprint::toNative(*iter);
      if (!native) continue;
      result->mFingerprints.push_back(*native);
    }
  }
  return result;
}
