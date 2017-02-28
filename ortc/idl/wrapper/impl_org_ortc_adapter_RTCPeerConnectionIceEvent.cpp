
#include "impl_org_ortc_adapter_RTCPeerConnectionIceEvent.h"
#include "impl_org_ortc_adapter_RTCIceCandidate.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceEvent::RTCPeerConnectionIceEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCPeerConnectionIceEventPtr wrapper::org::ortc::adapter::RTCPeerConnectionIceEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceEvent::~RTCPeerConnectionIceEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCIceCandidatePtr wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceEvent::get_candidate()
{
  return candidate_;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::RTCPeerConnectionIceEvent::get_uri()
{
  return uri_;
}


//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(
  ::ortc::adapter::IPeerConnectionTypes::ICECandidatePtr candidate,
  const String &uri
)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->candidate_ = RTCIceCandidate::toWrapper(candidate);
  pThis->uri_ = uri;
  return pThis;
}
