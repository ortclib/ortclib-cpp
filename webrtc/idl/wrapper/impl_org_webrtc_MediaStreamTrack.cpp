// Generated by zsLibEventingTool

#include "impl_org_webrtc_MediaStreamTrack.h"

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
wrapper::impl::org::webrtc::MediaStreamTrack::MediaStreamTrack() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackPtr wrapper::org::webrtc::MediaStreamTrack::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::MediaStreamTrack>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::MediaStreamTrack::~MediaStreamTrack()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::wrapper_init_org_webrtc_MediaStreamTrack() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackPtr wrapper::org::webrtc::MediaStreamTrack::createAudioSource(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept
{
  wrapper::org::webrtc::MediaStreamTrackPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackPtr wrapper::org::webrtc::MediaStreamTrack::createVideoSource(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept
{
  wrapper::org::webrtc::MediaStreamTrackPtr result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webrtc::MediaStreamTrack::get_kind() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
String wrapper::impl::org::webrtc::MediaStreamTrack::get_id() noexcept
{
  String result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::MediaStreamTrack::get_enabled() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::set_enabled(bool value) noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaStreamTrackState wrapper::impl::org::webrtc::MediaStreamTrack::get_state() noexcept
{
  wrapper::org::webrtc::MediaStreamTrackState result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaSourcePtr wrapper::impl::org::webrtc::MediaStreamTrack::get_source() noexcept
{
  wrapper::org::webrtc::MediaSourcePtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaElementPtr wrapper::impl::org::webrtc::MediaStreamTrack::get_element() noexcept
{
  wrapper::org::webrtc::MediaElementPtr result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::set_element(wrapper::org::webrtc::MediaElementPtr value) noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaStreamTrack::wrapper_onObserverCountChanged(size_t count) noexcept
{
}

