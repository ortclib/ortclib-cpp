
#include "impl_org_ortc_MediaTrackConstraints.h"
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
wrapper::impl::org::ortc::MediaTrackConstraints::MediaTrackConstraints()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackConstraintsPtr wrapper::org::ortc::MediaTrackConstraints::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackConstraints>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackConstraints::~MediaTrackConstraints()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::MediaTrackConstraints::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaTrackConstraints::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackConstraints::wrapper_init_org_ortc_MediaTrackConstraints()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackConstraints::wrapper_init_org_ortc_MediaTrackConstraints(wrapper::org::ortc::MediaTrackConstraintsPtr source)
{
  if (!source) return;
  wrapper::org::ortc::MediaTrackConstraintsPtr pThis = thisWeak_.lock();

  auto native = toNative(source);
  if (!native) return;

  wrapper::org::ortc::MediaTrackConstraintsPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackConstraints::wrapper_init_org_ortc_MediaTrackConstraints(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  auto native = make_shared<NativeTrackConstraints>(Json::toNative(json));

  wrapper::org::ortc::MediaTrackConstraintsPtr wrapper = toWrapper(native);
  if (!wrapper) return;

  wrapper::org::ortc::MediaTrackConstraintsPtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackConstraintsPtr wrapper::impl::org::ortc::MediaTrackConstraints::toWrapper(NativeTrackConstraintsPtr native)
{
  if (!native) return MediaTrackConstraintsPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackConstraintsPtr wrapper::impl::org::ortc::MediaTrackConstraints::toWrapper(const NativeTrackConstraints &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackConstraints>();
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

  pThis->advanced = make_shared< list< wrapper::org::ortc::MediaTrackConstraintSetPtr > >();

  for (auto iter = native.mAdvanced.begin(); iter != native.mAdvanced.end(); ++iter)
  {
    auto wrapper = wrapper::impl::org::ortc::MediaTrackConstraintSet::toWrapper(*iter);
    if (!wrapper) continue;
    pThis->advanced->push_back(wrapper);
  }

  return pThis;
}
//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackConstraints::NativeTrackConstraintsPtr wrapper::impl::org::ortc::MediaTrackConstraints::toNative(wrapper::org::ortc::MediaTrackConstraintsPtr wrapper)
{
  if (!wrapper) return NativeTrackConstraintsPtr();

  auto result = make_shared<NativeTrackConstraints>();

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

  if (wrapper->advanced) {
    for (auto iter = wrapper->advanced->begin(); iter != wrapper->advanced->end(); ++iter)
    {
      auto native = wrapper::impl::org::ortc::MediaTrackConstraintSet::toNative(*iter);
      if (!native) continue;
      result->mAdvanced.push_back(native);
    }
  }

  return result;
}
