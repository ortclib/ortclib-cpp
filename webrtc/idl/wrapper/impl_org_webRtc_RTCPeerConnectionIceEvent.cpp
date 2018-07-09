
#include "impl_org_webRtc_RTCPeerConnectionIceEvent.h"
#include "impl_org_webRtc_RTCIceCandidate.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent::RTCPeerConnectionIceEvent() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCPeerConnectionIceEventPtr wrapper::org::webRtc::RTCPeerConnectionIceEvent::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent::~RTCPeerConnectionIceEvent() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceCandidatePtr wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent::get_candidate() noexcept
{
  return candidate_;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent::get_url() noexcept
{
  ZS_ASSERT(candidate_);
  if (!candidate_) return String();
  return candidate_->serverUrl();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(UseIceCandidatePtr value) noexcept
{
  if (!value) return WrapperImplTypePtr();
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->candidate_ = value;
  return result;
}

