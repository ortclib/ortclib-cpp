
#include "impl_org_ortc_adapter_RTCConfiguration.h"
#include "impl_org_ortc_RTCIceGatherOptions.h"
#include "impl_org_ortc_RTCCertificate.h"
#include "impl_org_ortc_Helper.h"

#include <zsLib/SafeInt.h>

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCConfiguration::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCConfiguration::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCConfiguration::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCConfiguration::RTCConfiguration()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCConfigurationPtr wrapper::org::ortc::adapter::RTCConfiguration::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCConfiguration>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCConfiguration::~RTCConfiguration()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCConfiguration::wrapper_init_org_ortc_adapter_RTCConfiguration()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCConfiguration::wrapper_init_org_ortc_adapter_RTCConfiguration(wrapper::org::ortc::adapter::RTCConfigurationPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return RTCConfigurationPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->gatherOptions = RTCIceGatherOptions::toWrapper(native.mGatherOptions);
  pThis->signalingMode = Helper::toWrapper(native.mSignalingMode);
  pThis->negotiateSrtpSdes = native.mNegotiateSRTPSDES;
  pThis->bundlePolicy = Helper::toWrapper(native.mBundlePolicy);
  pThis->rtcpMuxPolicy = Helper::toWrapper(native.mRTCPMuxPolicy);
  pThis->certificates = make_shared< list< wrapper::org::ortc::RTCCertificatePtr> >();
  for (auto iter = native.mCertificates.begin(); iter != native.mCertificates.end(); ++iter) {
    pThis->certificates->push_back(RTCCertificate::toWrapper(*iter));
  }
  pThis->iceCandidatePoolSize = native.mICECandidatePoolSize;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  auto result = make_shared < NativeType >();
  result->mGatherOptions = RTCIceGatherOptions::toNative(wrapper->gatherOptions);
  result->mSignalingMode = Helper::toNative(wrapper->signalingMode);
  result->mNegotiateSRTPSDES = wrapper->negotiateSrtpSdes;
  result->mBundlePolicy = Helper::toNative(wrapper->bundlePolicy);
  result->mRTCPMuxPolicy = Helper::toNative(wrapper->rtcpMuxPolicy);
  if (wrapper->certificates) {
    for (auto iter = wrapper->certificates->begin(); iter != wrapper->certificates->end(); ++iter) {
      result->mCertificates.push_back(RTCCertificate::toNative(*iter));
    }
  }
  result->mICECandidatePoolSize = SafeInt<decltype(result->mICECandidatePoolSize)>(wrapper->iceCandidatePoolSize);
  return result;
}
