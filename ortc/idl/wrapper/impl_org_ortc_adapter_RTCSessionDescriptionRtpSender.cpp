
#include "impl_org_ortc_adapter_RTCSessionDescriptionRtpSender.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionRtpSenderDetails.h"
#include "impl_org_ortc_RTCRtpParameters.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::RTCSessionDescriptionRtpSender()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderPtr wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSender::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::~RTCSessionDescriptionRtpSender()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSender()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSender(wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::wrapper_init_org_ortc_adapter_RTCSessionDescriptionRtpSender(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescriptionRtpSender::hash()
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
  pThis->id = native.mID;
  pThis->details = RTCSessionDescriptionRtpSenderDetails::toWrapper(native.mDetails);
  pThis->rtpMediaLineId = native.mRTPMediaLineID;
  pThis->parameters = RTCRtpParameters::toWrapper(native.mParameters);
  pThis->mediaStreamTrackId = native.mMediaStreamTrackID;
  pThis->mediaStreamIds = make_shared< set< String > >(native.mMediaStreamIDs);
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mID = wrapper->id;
  result->mDetails = RTCSessionDescriptionRtpSenderDetails::toNative(wrapper->details);
  result->mRTPMediaLineID = wrapper->rtpMediaLineId;
  result->mParameters = RTCRtpParameters::toNative(wrapper->parameters);
  result->mMediaStreamTrackID = wrapper->mediaStreamTrackId;
  if (wrapper->mediaStreamIds) {
    result->mMediaStreamIDs = *(wrapper->mediaStreamIds);
  }
  return result;
}
