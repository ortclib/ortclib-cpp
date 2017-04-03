
#include "impl_org_ortc_adapter_MediaStream.h"
#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_RTCStatsTypeSet.h"
#include "impl_org_ortc_RTCStatsReport.h"

#include "impl_org_ortc_Helper.h"

#include <zsLib/Log.h>

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

namespace wrapper { namespace impl { namespace org { namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortc_adapter_wrapper); } } } } }

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::MediaStream::MediaStream()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::MediaStreamPtr wrapper::org::ortc::adapter::MediaStream::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::MediaStream>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::MediaStream::~MediaStream()
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::adapter::MediaStream::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::wrapper_init_org_ortc_adapter_MediaStream()
{
  native_ = IMediaStream::create(thisWeak_.lock());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::wrapper_init_org_ortc_adapter_MediaStream(wrapper::org::ortc::adapter::MediaStreamPtr source)
{
  if (!source) {
    wrapper_init_org_ortc_adapter_MediaStream();
    return;
  }
  native_ = IMediaStream::create(thisWeak_.lock(), toNative(source));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::wrapper_init_org_ortc_adapter_MediaStream(shared_ptr< list< wrapper::org::ortc::MediaStreamTrackPtr > > tracks)
{
  if (!tracks) {
    wrapper_init_org_ortc_adapter_MediaStream();
    return;
  }

  native_ = IMediaStream::create(thisWeak_.lock(), *Helper::toNative(tracks));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamTrackPtr wrapper::impl::org::ortc::adapter::MediaStream::getTrackById(String id)
{
  if (!native_) return MediaStreamTrackPtr();

  auto result = native_->getTrackByID(id);
  if (!result) return MediaStreamTrackPtr();

  return wrapper::impl::org::ortc::MediaStreamTrack::toWrapper(result);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::addTrack(wrapper::org::ortc::MediaStreamTrackPtr track)
{
  if (!track) return;
  native_->addTrack(wrapper::impl::org::ortc::MediaStreamTrack::toNative(track));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::removeTrack(wrapper::org::ortc::MediaStreamTrackPtr track)
{
  if (!track) return;
  native_->removeTrack(wrapper::impl::org::ortc::MediaStreamTrack::toNative(track));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::MediaStreamPtr wrapper::impl::org::ortc::adapter::MediaStream::clone()
{
  return toWrapper(native_->clone());
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::adapter::MediaStream::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::adapter::MediaStream::get_id()
{
  return native_->id();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::adapter::MediaStream::get_active()
{
  return native_->active();
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::MediaStreamTrackPtr > > wrapper::impl::org::ortc::adapter::MediaStream::get_tracks()
{
  return Helper::toWrapper(native_->getTracks());
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::MediaStreamTrackPtr > > wrapper::impl::org::ortc::adapter::MediaStream::get_audioTracks()
{
  return Helper::toWrapper(native_->getAudioTracks());
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::MediaStreamTrackPtr > > wrapper::impl::org::ortc::adapter::MediaStream::get_videoTracks()
{
  return Helper::toWrapper(native_->getVideoTracks());
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::adapter::MediaStream::get_count()
{
  return native_->size();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::wrapper_onObserverCountChanged(size_t count)
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::onMediaStreamAddTrack(
                                                                           IMediaStreamPtr stream,
                                                                           IMediaStreamTrackPtr track
                                                                           )
{
  onAddTrack(wrapper::impl::org::ortc::MediaStreamTrack::toWrapper(track));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::onMediaStreamRemoveTrack(
                                                                              IMediaStreamPtr stream,
                                                                              IMediaStreamTrackPtr track
                                                                              )
{
  onRemoveTrack(wrapper::impl::org::ortc::MediaStreamTrack::toWrapper(track));
}

//------------------------------------------------------------------------------
::wrapper::impl::org::ortc::adapter::MediaStreamPtr wrapper::impl::org::ortc::adapter::MediaStream::toWrapper(IMediaStreamPtr native)
{
  if (!native) return MediaStreamPtr();

  auto result = make_shared<MediaStream>();
  result->thisWeak_ = result;
  result->defaultSubscription_ = false;
  result->native_ = native;
  result->subscribe();
  return result;
}

//------------------------------------------------------------------------------
::ortc::adapter::IMediaStreamPtr wrapper::impl::org::ortc::adapter::MediaStream::toNative(::wrapper::org::ortc::adapter::MediaStreamPtr wrapper)
{
  if (!wrapper) return IMediaStreamPtr();
  auto result = std::dynamic_pointer_cast<wrapper::impl::org::ortc::adapter::MediaStream>(wrapper);
  if (!result) return IMediaStreamPtr();
  return result->native_;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::MediaStream::subscribe()
{
  if (defaultSubscription_) return;
  if (!native_) return;

  zsLib::AutoLock lock(lock_);
  if (subscriptionCount_ < 1) {
    if (!subscription_) return;
    subscription_->cancel();
    return;
  }
  if (subscription_) return;
  subscription_ = native_->subscribe(thisWeak_.lock());
}
