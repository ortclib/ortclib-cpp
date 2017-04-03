
#include "impl_org_ortc_MediaTrackConstraintSet.h"
#include "impl_org_ortc_ConstrainLong.h"
#include "impl_org_ortc_ConstrainDouble.h"
#include "impl_org_ortc_ConstrainString.h"
#include "impl_org_ortc_ConstrainBoolean.h"
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
wrapper::impl::org::ortc::MediaTrackConstraintSet::MediaTrackConstraintSet()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackConstraintSetPtr wrapper::org::ortc::MediaTrackConstraintSet::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackConstraintSet>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackConstraintSet::~MediaTrackConstraintSet()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackConstraintSet::wrapper_init_org_ortc_MediaTrackConstraintSet()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackConstraintSet::wrapper_init_org_ortc_MediaTrackConstraintSet(wrapper::org::ortc::MediaTrackConstraintSetPtr source)
{
  if (!source) return;
  wrapper::org::ortc::MediaTrackConstraintSetPtr pThis = thisWeak_.lock();

  auto native = toNative(source);
  if (!native) return;

  wrapper::org::ortc::MediaTrackConstraintSetPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackConstraintSet::wrapper_init_org_ortc_MediaTrackConstraintSet(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeConstraintSet>(Json::toNative(json));

  wrapper::org::ortc::MediaTrackConstraintSetPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::MediaTrackConstraintSetPtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::MediaTrackConstraintSet::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaTrackConstraintSet::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackConstraintSetPtr wrapper::impl::org::ortc::MediaTrackConstraintSet::toWrapper(NativeConstraintSetPtr native)
{
  if (!native) return MediaTrackConstraintSetPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackConstraintSetPtr wrapper::impl::org::ortc::MediaTrackConstraintSet::toWrapper(const NativeConstraintSet &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackConstraintSet>();
  pThis->thisWeak_ = pThis;

  pThis->width = ConstrainLong::toWrapper(native.mWidth);
  pThis->height = ConstrainLong::toWrapper(native.mHeight);
  pThis->aspectRatio = ConstrainDouble::toWrapper(native.mAspectRatio);
  pThis->frameRate = ConstrainDouble::toWrapper(native.mFrameRate);
  pThis->facingMode = ConstrainString::toWrapper(native.mFacingMode);
  pThis->volume = ConstrainDouble::toWrapper(native.mVolume);
  pThis->sampleRate = ConstrainLong::toWrapper(native.mSampleRate);
  pThis->sampleSize = ConstrainLong::toWrapper(native.mSampleSize);
  pThis->echoCancellation = ConstrainBoolean::toWrapper(native.mEchoCancellation);
  pThis->latency = ConstrainDouble::toWrapper(native.mLatency);
  pThis->channelCount = ConstrainLong::toWrapper(native.mChannelCount);
  pThis->deviceId = ConstrainString::toWrapper(native.mDeviceID);
  pThis->groupId = ConstrainString::toWrapper(native.mGroupID);

  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackConstraintSet::NativeConstraintSetPtr wrapper::impl::org::ortc::MediaTrackConstraintSet::toNative(wrapper::org::ortc::MediaTrackConstraintSetPtr wrapper)
{
  if (!wrapper) return NativeConstraintSetPtr();
  
  auto result = make_shared<NativeConstraintSet>();

  if (wrapper->width) {
    result->mWidth = *ConstrainLong::toNative(wrapper->width);
  }
  if (wrapper->height) {
    result->mHeight = *ConstrainLong::toNative(wrapper->height);
  }
  if (wrapper->aspectRatio) {
    result->mAspectRatio = *ConstrainDouble::toNative(wrapper->aspectRatio);
  }
  if (wrapper->frameRate) {
    result->mFrameRate = *ConstrainDouble::toNative(wrapper->frameRate);
  }
  if (wrapper->facingMode) {
    result->mFacingMode = *ConstrainString::toNative(wrapper->facingMode);
  }
  if (wrapper->volume) {
    result->mVolume = *ConstrainDouble::toNative(wrapper->volume);
  }
  if (wrapper->sampleRate) {
    result->mSampleRate = *ConstrainLong::toNative(wrapper->sampleRate);
  }
  if (wrapper->sampleSize) {
    result->mSampleSize = *ConstrainLong::toNative(wrapper->sampleSize);
  }
  if (wrapper->echoCancellation) {
    result->mEchoCancellation = *ConstrainBoolean::toNative(wrapper->echoCancellation);
  }
  if (wrapper->latency) {
    result->mLatency = *ConstrainDouble::toNative(wrapper->latency);
  }
  if (wrapper->channelCount) {
    result->mChannelCount = *ConstrainLong::toNative(wrapper->channelCount);
  }
  if (wrapper->deviceId) {
    result->mDeviceID = *ConstrainString::toNative(wrapper->deviceId);
  }
  if (wrapper->groupId) {
    result->mGroupID = *ConstrainString::toNative(wrapper->groupId);
  }

  return result;
}
