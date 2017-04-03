
#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_MediaTrackCapabilities.h"
#include "impl_org_ortc_MediaTrackConstraints.h"
#include "impl_org_ortc_MediaTrackSettings.h"
#include "impl_org_ortc_MediaSource.h"
#include "impl_org_ortc_OverconstrainedError.h"
#include "impl_org_ortc_OverconstrainedErrorEvent.h"
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

namespace wrapper { namespace impl { namespace org { namespace ortc { ZS_DECLARE_SUBSYSTEM(ortc_wrapper); } } } }

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaStreamTrack::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaStreamTrack::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaStreamTrack::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaStreamTrack::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaStreamTrack::MediaStreamTrack()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamTrackPtr wrapper::org::ortc::MediaStreamTrack::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaStreamTrack>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaStreamTrack::~MediaStreamTrack()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamTrackPtr wrapper::impl::org::ortc::MediaStreamTrack::clone()
{
  return toWrapper(native_->clone());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamTrack::stop()
{
  return native_->stop();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackCapabilitiesPtr wrapper::impl::org::ortc::MediaStreamTrack::getCapabilities()
{
  return MediaTrackCapabilities::toWrapper(native_->getCapabilities());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackConstraintsPtr wrapper::impl::org::ortc::MediaStreamTrack::getConstraints()
{
  return MediaTrackConstraints::toWrapper(native_->getConstraints());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackSettingsPtr wrapper::impl::org::ortc::MediaStreamTrack::getSettings()
{
  return MediaTrackSettings::toWrapper(native_->getSettings());
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::ortc::MediaStreamTrack::applyConstraints(wrapper::org::ortc::MediaTrackConstraintsPtr constraints)
{
  ZS_THROW_INVALID_ARGUMENT_IF(!constraints);

  return Helper::toWrapper(native_->applyConstraints(*MediaTrackConstraints::toNative(constraints)));
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::MediaStreamTrack::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamTrackKind wrapper::impl::org::ortc::MediaStreamTrack::get_kind()
{
  return Helper::toWrapper(native_->kind());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaStreamTrack::get_id()
{
  return native_->id();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaStreamTrack::get_deviceId()
{
  return native_->deviceID();
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaStreamTrack::get_label()
{
  return native_->label();
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::MediaStreamTrack::get_enabled()
{
  return native_->enabled();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamTrack::set_enabled(bool value)
{
  native_->enabled(value);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::MediaStreamTrack::get_muted()
{
  return native_->muted();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamTrack::set_muted(bool value)
{
  native_->muted(value);
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::MediaStreamTrack::get_remote()
{
  return native_->remote();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamTrackState wrapper::impl::org::ortc::MediaStreamTrack::get_readyState()
{
  return Helper::toWrapper(native_->readyState());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaSourcePtr wrapper::impl::org::ortc::MediaStreamTrack::get_source()
{
  zsLib::AutoLock lock(lock_);
  if (mediaSource_) return mediaSource_;

  mediaSource_ = MediaSource::createWithTrack(thisWeak_.lock());
  return mediaSource_;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamTrack::wrapper_onObserverCountChanged(size_t count)
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamTrack::onMediaStreamTrackMute(
                                                                        IMediaStreamTrackPtr track,
                                                                        bool isMuted
                                                                        )
{
  if (isMuted) {
    onMute();
  } else {
    onUnmute();
  }
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamTrack::onMediaStreamTrackEnded(IMediaStreamTrackPtr track)
{
  onEnded();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamTrack::onMediaStreamTrackOverConstrained(
                                                                                   IMediaStreamTrackPtr track,
                                                                                   OverconstrainedErrorPtr error
                                                                                   )
{
  onOverConstrained(OverconstrainedErrorEvent::toWrapper(wrapper::impl::org::ortc::OverconstrainedError::toWrapper(error)));
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr track)
{
  if (!track) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = track;
  result->defaultSubscription_ = false;
  result->subscribe();
  return result;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  auto result = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!result) return NativeTypePtr();
  return result->native_;
}

//------------------------------------------------------------------------------
void WrapperImplType::subscribe()
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
