
#include "impl_org_ortc_RTCRtpUnhandledEvent.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpUnhandledEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpUnhandledEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpUnhandledEvent::RTCRtpUnhandledEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpUnhandledEventPtr wrapper::org::ortc::RTCRtpUnhandledEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpUnhandledEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpUnhandledEvent::~RTCRtpUnhandledEvent()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpUnhandledEvent::wrapper_init_org_ortc_RTCRtpUnhandledEvent()
{
}

//------------------------------------------------------------------------------
uint32_t wrapper::impl::org::ortc::RTCRtpUnhandledEvent::get_ssrc()
{
  return ssrc_;
}

//------------------------------------------------------------------------------
uint8_t wrapper::impl::org::ortc::RTCRtpUnhandledEvent::get_payloadType()
{
  return payloadType_;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpUnhandledEvent::get_muxId()
{
  return mid_;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCRtpUnhandledEvent::get_rid()
{
  return rid_;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(
  SSRCType ssrc,
  PayloadType payloadType,
  const String &mid,
  const String &rid
)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->ssrc_ = ssrc;
  pThis->payloadType_ = payloadType;
  pThis->mid_ = mid;
  pThis->rid_ = rid;
  return pThis;
}
