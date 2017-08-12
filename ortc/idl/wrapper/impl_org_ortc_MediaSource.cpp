
#include "impl_org_ortc_MediaSource.h"
#include "impl_org_ortc_MediaStreamTrack.h"

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
wrapper::impl::org::ortc::MediaSource::MediaSource()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr wrapper::org::ortc::MediaSource::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaSource>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaSource::~MediaSource()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaSource::wrapper_init_org_ortc_MediaSource()
{
}

//------------------------------------------------------------------------------
AnyPtr wrapper::impl::org::ortc::MediaSource::get_source()
{
  zsLib::AutoLock lock(lock_);
  return source_;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaSource::set_source(AnyPtr value)
{
  zsLib::AutoLock lock(lock_);
  source_ = value;
}

//------------------------------------------------------------------------------
AnyPtr wrapper::impl::org::ortc::MediaSource::get_track()
{
  auto holder = make_shared < AnyHolder< ::ortc::IMediaStreamTrackPtr > > ();
  holder->value_ = MediaStreamTrack::toNative(track_);
  return holder;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaSourcePtr wrapper::impl::org::ortc::MediaSource::createWithTrack(MediaStreamTrackPtr track)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaSource>();
  pThis->thisWeak_ = pThis;
  pThis->track_ = track;
  pThis->wrapper_init_org_ortc_MediaSource();
  return pThis;
}
