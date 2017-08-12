
#include "impl_org_ortc_adapter_RTCTrackEvent.h"
#include "impl_org_ortc_adapter_MediaStream.h"
#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_RTCRtpReceiver.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCTrackEvent::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCTrackEvent::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCTrackEvent::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCTrackEvent::RTCTrackEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCTrackEventPtr wrapper::org::ortc::adapter::RTCTrackEvent::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCTrackEvent>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCTrackEvent::~RTCTrackEvent()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpReceiverPtr wrapper::impl::org::ortc::adapter::RTCTrackEvent::get_receiver()
{
  return wrapper::impl::org::ortc::RTCRtpReceiver::toWrapper(native_->mReceiver);
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamTrackPtr wrapper::impl::org::ortc::adapter::RTCTrackEvent::get_track()
{
  return wrapper::impl::org::ortc::MediaStreamTrack::toWrapper(native_->mTrack);
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::adapter::MediaStreamPtr > > wrapper::impl::org::ortc::adapter::RTCTrackEvent::get_mediaStreams()
{
  auto result = make_shared< list< wrapper::org::ortc::adapter::MediaStreamPtr > >();
  for (auto iter = native_->mMediaStreams.begin(); iter != native_->mMediaStreams.end(); ++iter) {
    auto wrapper = MediaStream::toWrapper(*iter);
    if (!wrapper) continue;
    result->push_back(wrapper);
  }
  return result;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();
  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!impl) return NativeTypePtr();

  return impl->native_;
}

