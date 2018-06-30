
#include "impl_org_webRtc_RTCSessionDescriptionInit.h"

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

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCSessionDescriptionInit::RTCSessionDescriptionInit() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSessionDescriptionInitPtr wrapper::org::webRtc::RTCSessionDescriptionInit::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCSessionDescriptionInit>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCSessionDescriptionInit::~RTCSessionDescriptionInit() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCSessionDescriptionInit::wrapper_init_org_webRtc_RTCSessionDescriptionInit() noexcept
{
}


