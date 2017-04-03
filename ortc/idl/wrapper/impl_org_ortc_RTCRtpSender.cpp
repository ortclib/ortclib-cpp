
#include "impl_org_ortc_RTCRtpSender.h"
#include "impl_org_ortc_RTCRtpTransport.h"
#include "impl_org_ortc_RTCRtcpTransport.h"
#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_RTCRtpParameters.h"
#include "impl_org_ortc_RTCRtpCapabilities.h"
#include "impl_org_ortc_RTCSsrcConflictEvent.h"
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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpSender::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpSender::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpSender::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpSender::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpSender::RTCRtpSender()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpSenderPtr wrapper::org::ortc::RTCRtpSender::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpSender>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpSender::~RTCRtpSender()
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCRtpSender::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpSender::wrapper_init_org_ortc_RTCRtpSender(
  wrapper::org::ortc::MediaStreamTrackPtr track,
  wrapper::org::ortc::RTCRtpTransportPtr transport
  )
{
  native_ = NativeType::create(thisWeak_.lock(), wrapper::impl::org::ortc::MediaStreamTrack::toNative(track), wrapper::impl::org::ortc::RTCRtpTransport::toNative(transport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpSender::wrapper_init_org_ortc_RTCRtpSender(
  wrapper::org::ortc::MediaStreamTrackPtr track,
  wrapper::org::ortc::RTCRtpTransportPtr transport,
  wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
  )
{
  native_ = NativeType::create(thisWeak_.lock(), wrapper::impl::org::ortc::MediaStreamTrack::toNative(track), wrapper::impl::org::ortc::RTCRtpTransport::toNative(transport), wrapper::impl::org::ortc::RTCRtcpTransport::toNative(rtcpTransport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpSender::setTransport(wrapper::org::ortc::RTCRtpTransportPtr transport)
{
  native_->setTransport(RTCRtpTransport::toNative(transport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpSender::setTransport(
  wrapper::org::ortc::RTCRtpTransportPtr transport,
  wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
  )
{
  native_->setTransport(RTCRtpTransport::toNative(transport), RTCRtcpTransport::toNative(rtcpTransport));
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::ortc::RTCRtpSender::setTrack(wrapper::org::ortc::MediaStreamTrackPtr track)
{
  return Helper::toWrapper(native_->setTrack(MediaStreamTrack::toNative(track)));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCapabilitiesPtr wrapper::org::ortc::RTCRtpSender::getCapabilities(Optional< wrapper::org::ortc::MediaStreamTrackKind > kind)
{
  Optional<::ortc::IMediaStreamTrackTypes::Kinds> native;
  if (kind.hasValue()) {
    native = wrapper::impl::org::ortc::Helper::toNative(kind.value());
  }
  return wrapper::impl::org::ortc::RTCRtpCapabilities::toWrapper(NativeType::getCapabilities(native));
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::ortc::RTCRtpSender::send(wrapper::org::ortc::RTCRtpParametersPtr parameters)
{
  auto native = RTCRtpParameters::toNative(parameters);
  ZS_THROW_INVALID_ARGUMENT_IF(!native);
  return Helper::toWrapper(native_->send(*native));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpSender::stop()
{
  native_->stop();
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCRtpSender::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamTrackPtr wrapper::impl::org::ortc::RTCRtpSender::get_track()
{
  return MediaStreamTrack::toWrapper(native_->track());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpTransportPtr wrapper::impl::org::ortc::RTCRtpSender::get_transport()
{
  return RTCRtpTransport::toWrapper(native_->transport());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtcpTransportPtr wrapper::impl::org::ortc::RTCRtpSender::get_rtcpTransport()
{
  return RTCRtcpTransport::toWrapper(native_->rtcpTransport());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpSender::wrapper_onObserverCountChanged(size_t count)
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onRTPSenderSSRCConflict(
  IRTPSenderPtr sender,
  SSRCType ssrc
)
{
  onSsrcConflict(RTCSsrcConflictEvent::toWrapper(ssrc));
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
