
#include "impl_org_ortc_MediaTrackSettings.h"
#include "impl_org_ortc_Json.h"

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
wrapper::impl::org::ortc::MediaTrackSettings::MediaTrackSettings()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackSettingsPtr wrapper::org::ortc::MediaTrackSettings::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackSettings>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackSettings::~MediaTrackSettings()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackSettings::wrapper_init_org_ortc_MediaTrackSettings()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackSettings::wrapper_init_org_ortc_MediaTrackSettings(wrapper::org::ortc::MediaTrackSettingsPtr source)
{
  if (!source) return;

  wrapper::org::ortc::MediaTrackSettingsPtr wrapper = source;
  wrapper::org::ortc::MediaTrackSettingsPtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackSettings::wrapper_init_org_ortc_MediaTrackSettings(wrapper::org::ortc::JsonPtr json)
{
  auto rootEl = Json::toNative(json);
  if (!rootEl) return;

  auto native = Settings::create(rootEl);
  if (!native) return;

  wrapper::org::ortc::MediaTrackSettingsPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::MediaTrackSettingsPtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::MediaTrackSettings::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaTrackSettings::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackSettingsPtr wrapper::impl::org::ortc::MediaTrackSettings::toWrapper(SettingsPtr native)
{
  if (!native) return MediaTrackSettingsPtr();

  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackSettings>();
  pThis->thisWeak_ = pThis;

  pThis->width = native->mWidth;
  pThis->height = native->mHeight;
  pThis->aspectRatio = native->mAspectRatio;
  pThis->frameRate = native->mFrameRate;
  pThis->facingMode = native->mFacingMode;
  pThis->volume = native->mVolume;
  pThis->sampleRate = native->mSampleRate;
  pThis->sampleSize = native->mSampleSize;
  pThis->echoCancellation = native->mEchoCancellation;
  pThis->latency = native->mLatency;
  pThis->channelCount = native->mChannelCount;
  pThis->deviceId = native->mDeviceID;
  pThis->groupId = native->mGroupID;

  pThis->wrapper_init_org_ortc_MediaTrackSettings();
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackSettings::SettingsPtr wrapper::impl::org::ortc::MediaTrackSettings::toNative(MediaTrackSettingsPtr wrapper)
{
  if (!wrapper) return SettingsPtr();

  auto native = Settings::create();

  native->mWidth = wrapper->width;
  native->mHeight = wrapper->height;
  native->mAspectRatio = wrapper->aspectRatio;
  native->mFrameRate = wrapper->frameRate;
  native->mFacingMode = wrapper->facingMode;
  native->mVolume = wrapper->volume;
  native->mSampleRate = wrapper->sampleRate;
  native->mSampleSize = wrapper->sampleSize;
  native->mEchoCancellation = wrapper->echoCancellation;
  native->mLatency = wrapper->latency;
  native->mChannelCount = wrapper->channelCount;
  native->mDeviceID = wrapper->deviceId;
  native->mGroupID = wrapper->groupId;

  return native;
}
