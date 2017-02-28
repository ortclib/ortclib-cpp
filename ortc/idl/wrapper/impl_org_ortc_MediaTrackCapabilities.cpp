
#include "impl_org_ortc_MediaTrackCapabilities.h"
#include "impl_org_ortc_LongRange.h"
#include "impl_org_ortc_DoubleRange.h"
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
wrapper::impl::org::ortc::MediaTrackCapabilities::MediaTrackCapabilities()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackCapabilitiesPtr wrapper::org::ortc::MediaTrackCapabilities::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackCapabilities>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackCapabilities::~MediaTrackCapabilities()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackCapabilities::wrapper_init_org_ortc_MediaTrackCapabilities()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackCapabilities::wrapper_init_org_ortc_MediaTrackCapabilities(wrapper::org::ortc::MediaTrackCapabilitiesPtr source)
{
  wrapper::org::ortc::MediaTrackCapabilitiesPtr pThis = thisWeak_.lock();
  wrapper::org::ortc::MediaTrackCapabilitiesPtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackCapabilities::wrapper_init_org_ortc_MediaTrackCapabilities(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  wrapper::org::ortc::MediaTrackCapabilitiesPtr wrapper = toWrapper(make_shared<NativeCapabilities>(Json::toNative(json)));
  if (!wrapper) return;

  wrapper::org::ortc::MediaTrackCapabilitiesPtr pThis = thisWeak_.lock();
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::MediaTrackCapabilities::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaTrackCapabilities::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackCapabilitiesPtr wrapper::impl::org::ortc::MediaTrackCapabilities::toWrapper(NativeCapabilitiesPtr native)
{
  if (!native) return MediaTrackCapabilitiesPtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackCapabilitiesPtr wrapper::impl::org::ortc::MediaTrackCapabilities::toWrapper(const NativeCapabilities &native)
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackCapabilities>();
  pThis->thisWeak_ = pThis;

  if (native.mWidth.hasValue()) {
    pThis->width = LongRange::toWrapper(native.mWidth.value());
  }
  if (native.mHeight.hasValue()) {
    pThis->height = LongRange::toWrapper(native.mHeight.value());
  }
  if (native.mAspectRatio.hasValue()) {
    pThis->aspectRatio = DoubleRange::toWrapper(native.mAspectRatio.value());
  }
  if (native.mFrameRate.hasValue()) {
    pThis->frameRate = DoubleRange::toWrapper(native.mFrameRate.value());
  }
  if (native.mFacingMode.hasValue()) {
    pThis->facingMode = make_shared<  list< String > >();
    for (auto iter = native.mFacingMode.value().begin(); iter != native.mFacingMode.value().end(); ++iter)
    {
      pThis->facingMode->push_back(*iter);
    }
  }
  if (native.mVolume.hasValue()) {
    pThis->volume = DoubleRange::toWrapper(native.mVolume.value());
  }
  if (native.mSampleRate.hasValue()) {
    pThis->sampleRate = LongRange::toWrapper(native.mSampleRate.value());
  }
  if (native.mSampleSize.hasValue()) {
    pThis->sampleSize = LongRange::toWrapper(native.mSampleSize.value());
  }
  if (native.mEchoCancellation.hasValue()) {
    pThis->echoCancellation = make_shared< list< bool > >();
    for (auto iter = native.mEchoCancellation.value().begin(); iter != native.mEchoCancellation.value().end(); ++iter)
    {
      pThis->echoCancellation->push_back(*iter);
    }
  }
  if (native.mLatency.hasValue()) {
    pThis->latency = DoubleRange::toWrapper(native.mLatency.value());
  }
  if (native.mChannelCount.hasValue()) {
    pThis->channelCount = LongRange::toWrapper(native.mChannelCount.value());
  }
  pThis->deviceId = native.mDeviceID;
  pThis->groupId = native.mGroupID;
  
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackCapabilities::NativeCapabilitiesPtr wrapper::impl::org::ortc::MediaTrackCapabilities::toNative(wrapper::org::ortc::MediaTrackCapabilitiesPtr wrapper)
{
  if (!wrapper) return NativeCapabilitiesPtr();

  auto native = make_shared< NativeCapabilities >();

  if (wrapper->width.hasValue()) {
    if (wrapper->width.value()) {
      native->mWidth = *LongRange::toNative(wrapper->width);
    }
  }
  if (wrapper->height.hasValue()) {
    if (wrapper->height.value()) {
      native->mHeight = *LongRange::toNative(wrapper->height);
    }
  }
  if (wrapper->aspectRatio.hasValue()) {
    if (wrapper->aspectRatio.value()) {
      native->mAspectRatio = *DoubleRange::toNative(wrapper->aspectRatio);
    }
  }
  if (wrapper->frameRate.hasValue()) {
    if (wrapper->frameRate.value()) {
      native->mFrameRate = *DoubleRange::toNative(wrapper->frameRate);
    }
  }
  if (wrapper->facingMode) {
    auto value = make_shared<NativeCapabilityString>();
    for (auto iter = wrapper->facingMode->begin(); iter != wrapper->facingMode->end(); ++iter) {
      value->insert(*iter);
    }
    native->mFacingMode = *value;
  }
  if (wrapper->volume.hasValue()) {
    if (wrapper->volume.value()) {
      native->mVolume = *DoubleRange::toNative(wrapper->volume);
    }
  }
  if (wrapper->sampleRate.hasValue()) {
    if (wrapper->sampleRate.value()) {
      native->mSampleRate = *LongRange::toNative(wrapper->sampleRate);
    }
  }
  if (wrapper->sampleSize.hasValue()) {
    if (wrapper->sampleSize.value()) {
      native->mSampleSize = *LongRange::toNative(wrapper->sampleSize);
    }
  }
  if (wrapper->echoCancellation) {
    auto value = make_shared<NativeCapabilityBoolean>();
    for (auto iter = wrapper->echoCancellation->begin(); iter != wrapper->echoCancellation->end(); ++iter) {
      value->insert(*iter);
    }
    native->mEchoCancellation = *value;
  }
  if (wrapper->latency.hasValue()) {
    if (wrapper->latency.value()) {
      native->mLatency = *DoubleRange::toNative(wrapper->latency);
    }
  }
  if (wrapper->channelCount.hasValue()) {
    if (wrapper->channelCount.value()) {
      native->mChannelCount = *LongRange::toNative(wrapper->channelCount);
    }
  }

  native->mDeviceID = wrapper->deviceId;
  native->mGroupID = wrapper->groupId;

  return native;
}
