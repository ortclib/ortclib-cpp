
#include "impl_org_ortc_RTCRtpParameters.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_RTCRtpCodecParameters.h"
#include "impl_org_ortc_RTCRtpHeaderExtensionParameters.h"
#include "impl_org_ortc_RTCRtpEncodingParameters.h"
#include "impl_org_ortc_RTCRtcpParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpParameters::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpParameters::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpParameters::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpParameters::RTCRtpParameters()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpParametersPtr wrapper::org::ortc::RTCRtpParameters::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpParameters>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpParameters::~RTCRtpParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpParameters::wrapper_init_org_ortc_RTCRtpParameters()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpParameters::wrapper_init_org_ortc_RTCRtpParameters(wrapper::org::ortc::RTCRtpParametersPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpParameters::wrapper_init_org_ortc_RTCRtpParameters(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::RTCRtpParameters::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("RTCRtpParameters"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpParameters::hash()
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
  pThis->codecs = make_shared< list< wrapper::org::ortc::RTCRtpCodecParametersPtr > >();
  for (auto iter = native.mCodecs.begin(); iter != native.mCodecs.end(); ++iter) {
    auto wrapper = RTCRtpCodecParameters::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->codecs->push_back(wrapper);
  }
  pThis->headerExtensions = make_shared< list< wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr > >();
  for (auto iter = native.mHeaderExtensions.begin(); iter != native.mHeaderExtensions.end(); ++iter) {
    auto wrapper = RTCRtpHeaderExtensionParameters::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->headerExtensions->push_back(wrapper);
  }
  pThis->encodings = make_shared< list< wrapper::org::ortc::RTCRtpEncodingParametersPtr > >();
  for (auto iter = native.mEncodings.begin(); iter != native.mEncodings.end(); ++iter) {
    auto wrapper = RTCRtpEncodingParameters::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->encodings->push_back(wrapper);
  }
  pThis->rtcp = RTCRtcpParameters::toWrapper(native.mRTCP);
  pThis->degredationPreference = Helper::toWrapper(native.mDegredationPreference);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  if (wrapper->codecs) {
    for (auto iter = wrapper->codecs->begin(); iter != wrapper->codecs->end(); ++iter) {
      auto native = RTCRtpCodecParameters::toNative(*iter);
      if (!native) continue;
      result->mCodecs.push_back(*native);
    }
  }
  if (wrapper->headerExtensions) {
    for (auto iter = wrapper->headerExtensions->begin(); iter != wrapper->headerExtensions->end(); ++iter) {
      auto native = RTCRtpHeaderExtensionParameters::toNative(*iter);
      if (!native) continue;
      result->mHeaderExtensions.push_back(*native);
    }
  }
  if (wrapper->encodings) {
    for (auto iter = wrapper->encodings->begin(); iter != wrapper->encodings->end(); ++iter) {
      auto native = RTCRtpEncodingParameters::toNative(*iter);
      if (!native) continue;
      result->mEncodings.push_back(*native);
    }
  }
  if (wrapper->rtcp) {
    auto native = RTCRtcpParameters::toNative(wrapper->rtcp);
    if (native) {
      result->mRTCP = *native;
    }
  }
  result->mDegredationPreference = Helper::toNative(wrapper->degredationPreference);
  return result;
}
