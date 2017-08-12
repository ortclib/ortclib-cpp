
#include "impl_org_ortc_adapter_RTCSessionDescriptionTransportParameters.h"
#include "impl_org_ortc_RTCIceParameters.h"
#include "impl_org_ortc_RTCDtlsParameters.h"
#include "impl_org_ortc_RTCSrtpSdesParameters.h"
#include "impl_org_ortc_RTCIceCandidate.h"
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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::RTCSessionDescriptionTransportParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionTransportParametersPtr wrapper::org::ortc::adapter::RTCSessionDescriptionTransportParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::~RTCSessionDescriptionTransportParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters(wrapper::org::ortc::adapter::RTCSessionDescriptionTransportParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::wrapper_init_org_ortc_adapter_RTCSessionDescriptionTransportParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCSessionDescriptionTransportParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescriptionTransportParameters::hash()
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
  pThis->iceParameters = RTCIceParameters::toWrapper(native.mICEParameters);
  pThis->dtlsParameters = RTCDtlsParameters::toWrapper(native.mDTLSParameters);
  pThis->srtpSdesParameters = RTCSrtpSdesParameters::toWrapper(native.mSRTPSDESParameters);
  pThis->iceCandidates = make_shared< list< wrapper::org::ortc::RTCIceCandidatePtr > >();
  for (auto iter = native.mICECandidates.begin(); iter != native.mICECandidates.end(); ++iter) {
    auto wrapper = wrapper::impl::org::ortc::RTCIceCandidate::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->iceCandidates->push_back(wrapper);
  }
  pThis->endOfCandidates = native.mEndOfCandidates;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mICEParameters = RTCIceParameters::toNative(wrapper->iceParameters);
  result->mDTLSParameters = RTCDtlsParameters::toNative(wrapper->dtlsParameters);
  result->mSRTPSDESParameters = RTCSrtpSdesParameters::toNative(wrapper->srtpSdesParameters);
  if (wrapper->iceCandidates) {
    for (auto iter = wrapper->iceCandidates->begin(); iter != wrapper->iceCandidates->end(); ++iter) {
      auto native = wrapper::impl::org::ortc::RTCIceCandidate::toNative(*iter);
      if (!native) continue;
      result->mICECandidates.push_back(native);
    }
  }
  result->mEndOfCandidates = wrapper->endOfCandidates;
  return result;
}
