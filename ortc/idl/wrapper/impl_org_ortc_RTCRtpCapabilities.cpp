
#include "impl_org_ortc_RTCRtpCapabilities.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_RTCRtpCodecCapability.h"
#include "impl_org_ortc_RTCRtpHeaderExtension.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCapabilities::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCapabilities::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpCapabilities::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCapabilities::RTCRtpCapabilities()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCapabilitiesPtr wrapper::org::ortc::RTCRtpCapabilities::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpCapabilities>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpCapabilities::~RTCRtpCapabilities()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCapabilities::wrapper_init_org_ortc_RTCRtpCapabilities()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCapabilities::wrapper_init_org_ortc_RTCRtpCapabilities(wrapper::org::ortc::RTCRtpCapabilitiesPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpCapabilities::wrapper_init_org_ortc_RTCRtpCapabilities(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpCapabilities::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpCapabilities::hash()
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
  pThis->codecs = make_shared< list< wrapper::org::ortc::RTCRtpCodecCapabilityPtr > >();
  for (auto iter = native.mCodecs.begin(); iter != native.mCodecs.end(); ++iter) {
    auto wrapper = RTCRtpCodecCapability::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->codecs->push_back(wrapper);
  }
  pThis->headerExtensions = make_shared< list< wrapper::org::ortc::RTCRtpHeaderExtensionPtr > >();
  for (auto iter = native.mHeaderExtensions.begin(); iter != native.mHeaderExtensions.end(); ++iter) {
    auto wrapper = RTCRtpHeaderExtension::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->headerExtensions->push_back(wrapper);
  }
  pThis->fecMechanisms = make_shared< list<String> >(native.mFECMechanisms);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  if (wrapper->codecs) {
    for (auto iter = wrapper->codecs->begin(); iter != wrapper->codecs->end(); ++iter) {
      auto native = RTCRtpCodecCapability::toNative(*iter);
      if (!native) continue;
      result->mCodecs.push_back(*native);
    }
  }
  if (wrapper->headerExtensions) {
    for (auto iter = wrapper->headerExtensions->begin(); iter != wrapper->headerExtensions->end(); ++iter) {
      auto native = RTCRtpHeaderExtension::toNative(*iter);
      if (!native) continue;
      result->mHeaderExtensions.push_back(*native);
    }
  }
  if (wrapper->fecMechanisms) {
    result->mFECMechanisms = *(wrapper->fecMechanisms);
  }
  return result;
}
