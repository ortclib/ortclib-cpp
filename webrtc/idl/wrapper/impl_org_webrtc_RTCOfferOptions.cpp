// Generated by zsLibEventingTool

#include "impl_org_webrtc_RTCOfferOptions.h"

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
wrapper::impl::org::webrtc::RTCOfferOptions::RTCOfferOptions() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCOfferOptionsPtr wrapper::org::webrtc::RTCOfferOptions::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCOfferOptions>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCOfferOptions::~RTCOfferOptions()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCOfferOptions::wrapper_init_org_webrtc_RTCOfferOptions() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCOfferOptions::wrapper_init_org_webrtc_RTCOfferOptions(wrapper::org::webrtc::RTCOfferOptionsPtr source) noexcept
{
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCOfferOptions::get_iceRestart() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCOfferOptions::set_iceRestart(bool value) noexcept
{
}

//------------------------------------------------------------------------------
Optional< bool > wrapper::impl::org::webrtc::RTCOfferOptions::get_offerToReceiveVideo() noexcept
{
  Optional< bool > result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCOfferOptions::set_offerToReceiveVideo(Optional< bool > value) noexcept
{
}

//------------------------------------------------------------------------------
Optional< bool > wrapper::impl::org::webrtc::RTCOfferOptions::get_offerToReceiveAudio() noexcept
{
  Optional< bool > result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCOfferOptions::set_offerToReceiveAudio(Optional< bool > value) noexcept
{
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCOfferOptions::get_useRtpMux() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCOfferOptions::set_useRtpMux(bool value) noexcept
{
}

