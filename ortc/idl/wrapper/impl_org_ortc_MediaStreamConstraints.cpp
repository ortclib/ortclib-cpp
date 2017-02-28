
#include "impl_org_ortc_MediaStreamConstraints.h"
#include "impl_org_ortc_MediaTrackConstraints.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaStreamConstraints::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaStreamConstraints::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::MediaStreamConstraints::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaStreamConstraints::MediaStreamConstraints()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamConstraintsPtr wrapper::org::ortc::MediaStreamConstraints::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::MediaStreamConstraints>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::MediaStreamConstraints::~MediaStreamConstraints()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamConstraints::wrapper_init_org_ortc_MediaStreamConstraints()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamConstraints::wrapper_init_org_ortc_MediaStreamConstraints(wrapper::org::ortc::MediaStreamConstraintsPtr source)
{
  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(toNative(source));
  if (!wrapper) return;
  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::MediaStreamConstraints::wrapper_init_org_ortc_MediaStreamConstraints(wrapper::org::ortc::JsonPtr json)
{
  if (!json) return;

  WrapperTypePtr pThis = thisWeak_.lock();
  WrapperTypePtr wrapper = toWrapper(make_shared<NativeType>(Json::toNative(json)));
  if (!wrapper) return;

  (*pThis) = (*wrapper);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::JsonPtr wrapper::impl::org::ortc::MediaStreamConstraints::toJson()
{
  return Json::toWrapper(toNative(thisWeak_.lock())->createElement());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::MediaStreamConstraints::hash()
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
  if (native.mAudio) {
    pThis->audio = MediaTrackConstraints::toWrapper(native.mAudio);
  }
  if (native.mVideo) {
    pThis->video = MediaTrackConstraints::toWrapper(native.mVideo);
  }
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto result = make_shared<NativeType>();
  if (wrapper->audio.hasValue()) {
    if (wrapper->audio.value()) {
      result->mAudio = MediaTrackConstraints::toNative(wrapper->audio.value());
    }
  }
  if (wrapper->video.hasValue()) {
    if (wrapper->video.value()) {
      result->mVideo = MediaTrackConstraints::toNative(wrapper->video.value());
    }
  }
  return result;
}
