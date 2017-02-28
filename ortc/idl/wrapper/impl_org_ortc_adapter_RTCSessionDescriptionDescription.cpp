
#include "impl_org_ortc_adapter_RTCSessionDescriptionDescription.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionDescriptionDetails.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionTransport.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionRtpMediaLine.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionSctpMediaLine.h"
#include "impl_org_ortc_adapter_RTCSessionDescriptionRtpSender.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::RTCSessionDescriptionDescription()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr wrapper::org::ortc::adapter::RTCSessionDescriptionDescription::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::~RTCSessionDescriptionDescription()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescription()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescription(wrapper::org::ortc::adapter::RTCSessionDescriptionDescriptionPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::wrapper_init_org_ortc_adapter_RTCSessionDescriptionDescription(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCSessionDescriptionDescription::hash()
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
  pThis->details = RTCSessionDescriptionDescriptionDetails::toWrapper(native.mDetails);
  pThis->transports = make_shared< list< wrapper::org::ortc::adapter::RTCSessionDescriptionTransportPtr > >();
  for (auto iter = native.mTransports.begin(); iter != native.mTransports.end(); ++iter) {
    auto wrapper = RTCSessionDescriptionTransport::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->transports->push_back(wrapper);
  }
  pThis->rtpMediaLines = make_shared< list< wrapper::org::ortc::adapter::RTCSessionDescriptionRtpMediaLinePtr > >();
  for (auto iter = native.mRTPMediaLines.begin(); iter != native.mRTPMediaLines.end(); ++iter) {
    auto wrapper = RTCSessionDescriptionRtpMediaLine::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->rtpMediaLines->push_back(wrapper);
  }
  pThis->sctpMediaLines = make_shared< list< wrapper::org::ortc::adapter::RTCSessionDescriptionSctpMediaLinePtr > >();
  for (auto iter = native.mSCTPMediaLines.begin(); iter != native.mSCTPMediaLines.end(); ++iter) {
    auto wrapper = RTCSessionDescriptionSctpMediaLine::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->sctpMediaLines->push_back(wrapper);
  }
  pThis->rtpSenders = make_shared< list< wrapper::org::ortc::adapter::RTCSessionDescriptionRtpSenderPtr > >();
  for (auto iter = native.mRTPSenders.begin(); iter != native.mRTPSenders.end(); ++iter) {
    auto wrapper = RTCSessionDescriptionRtpSender::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->rtpSenders->push_back(wrapper);
  }

  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mDetails = RTCSessionDescriptionDescriptionDetails::toNative(wrapper->details);
  if (wrapper->transports) {
    for (auto iter = wrapper->transports->begin(); iter != wrapper->transports->end(); ++iter) {
      auto native = RTCSessionDescriptionTransport::toNative(*iter);
      if (!native) continue;
      result->mTransports.push_back(native);
    }
  }
  if (wrapper->rtpMediaLines) {
    for (auto iter = wrapper->rtpMediaLines->begin(); iter != wrapper->rtpMediaLines->end(); ++iter) {
      auto native = RTCSessionDescriptionRtpMediaLine::toNative(*iter);
      if (!native) continue;
      result->mRTPMediaLines.push_back(native);
    }
  }
  if (wrapper->sctpMediaLines) {
    for (auto iter = wrapper->sctpMediaLines->begin(); iter != wrapper->sctpMediaLines->end(); ++iter) {
      auto native = RTCSessionDescriptionSctpMediaLine::toNative(*iter);
      if (!native) continue;
      result->mSCTPMediaLines.push_back(native);
    }
  }
  if (wrapper->rtpSenders) {
    for (auto iter = wrapper->rtpSenders->begin(); iter != wrapper->rtpSenders->end(); ++iter) {
      auto native = RTCSessionDescriptionRtpSender::toNative(*iter);
      if (!native) continue;
      result->mRTPSenders.push_back(native);
    }
  }
  return result;
}
