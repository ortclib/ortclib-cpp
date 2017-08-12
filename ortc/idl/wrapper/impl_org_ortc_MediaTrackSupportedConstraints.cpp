
#include "impl_org_ortc_MediaTrackSupportedConstraints.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaTrackSupportedConstraints::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaTrackSupportedConstraints::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaTrackSupportedConstraints::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackSupportedConstraints::MediaTrackSupportedConstraints()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaTrackSupportedConstraintsPtr wrapper::org::ortc::MediaTrackSupportedConstraints::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaTrackSupportedConstraints>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaTrackSupportedConstraints::~MediaTrackSupportedConstraints()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackSupportedConstraints::wrapper_init_org_ortc_MediaTrackSupportedConstraints()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackSupportedConstraints::wrapper_init_org_ortc_MediaTrackSupportedConstraints(wrapper::org::ortc::MediaTrackSupportedConstraintsPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaTrackSupportedConstraints::wrapper_init_org_ortc_MediaTrackSupportedConstraints(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::MediaTrackSupportedConstraints::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement("MediaTrackSupportedConstraints"));
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaTrackSupportedConstraints::hash()
{
  return toNative(thisWeak_.lock())->hash();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  return toWrapper(*native);
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(const NativeType &native)
{
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->width = native.mWidth;
  pThis->height = native.mHeight;
  pThis->aspectRatio = native.mAspectRatio;
  pThis->frameRate = native.mFrameRate;
  pThis->facingMode = native.mFacingMode;
  pThis->volume = native.mVolume;
  pThis->sampleRate = native.mSampleRate;
  pThis->sampleSize = native.mSampleSize;
  pThis->echoCancellation = native.mEchoCancellation;
  pThis->latency = native.mLatency;
  pThis->channelCount = native.mChannelCount;
  pThis->deviceId = native.mDeviceID;
  pThis->groupId = native.mGroupID;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  result->mWidth = wrapper->width;
  result->mHeight = wrapper->height;
  result->mAspectRatio = wrapper->aspectRatio;
  result->mFrameRate = wrapper->frameRate;
  result->mFacingMode = wrapper->facingMode;
  result->mVolume = wrapper->volume;
  result->mSampleRate = wrapper->sampleRate;
  result->mSampleSize = wrapper->sampleSize;
  result->mEchoCancellation = wrapper->echoCancellation;
  result->mLatency = wrapper->latency;
  result->mChannelCount = wrapper->channelCount;
  result->mDeviceID = wrapper->deviceId;
  result->mGroupID = wrapper->groupId;
  return result;
}

