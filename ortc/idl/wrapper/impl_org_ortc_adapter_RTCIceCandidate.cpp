
#include "impl_org_ortc_adapter_RTCIceCandidate.h"
#include "impl_org_ortc_Json.h"
#include "impl_org_ortc_Helper.h"
#include "impl_org_ortc_RTCIceCandidate.h"
#include "impl_org_ortc_RTCIceCandidateComplete.h"

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


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCIceCandidate::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCIceCandidate::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCIceCandidate::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCIceCandidate::RTCIceCandidate()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCIceCandidatePtr wrapper::org::ortc::adapter::RTCIceCandidate::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCIceCandidate>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCIceCandidate::~RTCIceCandidate()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCIceCandidate::wrapper_init_org_ortc_adapter_RTCIceCandidate()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCIceCandidate::wrapper_init_org_ortc_adapter_RTCIceCandidate(wrapper::org::ortc::adapter::RTCIceCandidatePtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCIceCandidate::wrapper_init_org_ortc_adapter_RTCIceCandidate(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr wrapper = toWrapper(NativeType::createFromJSON(Json::toNative(json)));  
  WrapperTypePtr pThis = thisWeak_.lock();
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCIceCandidatePtr wrapper::org::ortc::adapter::RTCIceCandidate::fromSdpStringWithMLineIndex(
  String sdp,
  uint64_t mlineIndex
  )
{
  auto native = NativeType::createFromSDP(sdp);
  native->mMLineIndex = SafeInt<decltype(native->mMLineIndex)::UseType>(mlineIndex);

  return WrapperImplType::toWrapper(native);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCIceCandidatePtr wrapper::org::ortc::adapter::RTCIceCandidate::fromSdpStringWithMid(
  String sdp,
  String mid
  )
{
  auto native = NativeType::createFromSDP(sdp);
  native->mMid = mid;

  return WrapperImplType::toWrapper(native);
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCIceCandidate::toSdp()
{
  return toNative(thisWeak_.lock())->toSDP();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCIceCandidate::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->toJSON());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCIceCandidate::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCIceCandidate::get_sdpMid()
{
  return mid;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCIceCandidate::set_sdpMid(String value)
{
  mid = value;
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
  pThis->mid = native.mMid;
  Helper::optionalSafeIntConvert(native.mMLineIndex, pThis->sdpMLineIndex);
  if (native.mCandidate) {
    {
      auto nativeCandidate = std::dynamic_pointer_cast<::ortc::IICETypes::Candidate>(native.mCandidate);
      if (nativeCandidate) {
        pThis->candidate = impl::org::ortc::RTCIceCandidate::toWrapper(nativeCandidate);
      }
    }
    {
      auto nativeCandidate = std::dynamic_pointer_cast<::ortc::IICETypes::CandidateComplete>(native.mCandidate);
      if (nativeCandidate) {
        pThis->candidate = impl::org::ortc::RTCIceCandidateComplete::toWrapper(nativeCandidate);
      }
    }
  }
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mMid = wrapper->mid;
  Helper::optionalSafeIntConvert(wrapper->sdpMLineIndex, result->mMLineIndex);
  if (wrapper->candidate) {
    {
      auto nativeCandidate = impl::org::ortc::RTCIceCandidate::toNative(wrapper->candidate);
      if (nativeCandidate) {
        result->mCandidate = nativeCandidate;
      }
    }
    {
      auto nativeCandidate = impl::org::ortc::RTCIceCandidateComplete::toNative(wrapper->candidate);
      if (nativeCandidate) {
        result->mCandidate = nativeCandidate;
      }
    }
  }
  return result;
}
