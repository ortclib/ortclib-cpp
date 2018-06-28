
#include "impl_org_webrtc_AudioTrackSource.h"


#include "impl_org_webrtc_pre_include.h"
#include "api/mediastreaminterface.h"
#include "impl_org_webrtc_post_include.h"

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
wrapper::impl::org::webrtc::AudioTrackSource::AudioTrackSource() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::AudioTrackSourcePtr wrapper::org::webrtc::AudioTrackSource::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::AudioTrackSource>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::AudioTrackSource::~AudioTrackSource()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::AudioTrackSource::wrapper_init_org_webrtc_AudioTrackSource() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaSourceState wrapper::impl::org::webrtc::AudioTrackSource::get_state() noexcept
{
  wrapper::org::webrtc::MediaSourceState result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::AudioTrackSource::get_remote() noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::AudioTrackSource::set_volume(double value) noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::AudioTrackSource::wrapper_onObserverCountChanged(size_t count) noexcept
{
}


