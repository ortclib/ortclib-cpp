
#include "impl_org_ortc_RTCRtpReceiver.h"
#include "impl_org_ortc_RTCRtpTransport.h"
#include "impl_org_ortc_RTCRtcpTransport.h"
#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_RTCRtpContributingSource.h"
#include "impl_org_ortc_RTCRtpCapabilities.h"
#include "impl_org_ortc_RTCRtpParameters.h"

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpReceiver::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpReceiver::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpReceiver::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtpReceiver::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpReceiver::RTCRtpReceiver()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpReceiverPtr wrapper::org::ortc::RTCRtpReceiver::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtpReceiver>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtpReceiver::~RTCRtpReceiver()
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::RTCRtpReceiver::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpReceiver::wrapper_init_org_ortc_RTCRtpReceiver(
  wrapper::org::ortc::MediaStreamTrackKind kind,
  wrapper::org::ortc::RTCRtpTransportPtr transport
  )
{
  native_ = IRTPReceiver::create(thisWeak_.lock(), Helper::toNative(kind), RTCRtpTransport::toNative(transport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpReceiver::wrapper_init_org_ortc_RTCRtpReceiver(
  wrapper::org::ortc::MediaStreamTrackKind kind,
  wrapper::org::ortc::RTCRtpTransportPtr transport,
  wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
  )
{
  native_ = IRTPReceiver::create(thisWeak_.lock(), Helper::toNative(kind), RTCRtpTransport::toNative(transport), RTCRtcpTransport::toNative(rtcpTransport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpReceiver::setTransport(wrapper::org::ortc::RTCRtpTransportPtr transport)
{
  native_->setTransport(RTCRtpTransport::toNative(transport));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpReceiver::setTransport(
  wrapper::org::ortc::RTCRtpTransportPtr transport,
  wrapper::org::ortc::RTCRtcpTransportPtr rtcpTransport
  )
{
  native_->setTransport(RTCRtpTransport::toNative(transport), RTCRtcpTransport::toNative(rtcpTransport));
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpCapabilitiesPtr wrapper::org::ortc::RTCRtpReceiver::getCapabilities(Optional< wrapper::org::ortc::MediaStreamTrackKind > kind)
{
  return wrapper::impl::org::ortc::RTCRtpCapabilities::toWrapper(NativeType::getCapabilities(kind.hasValue() ? Optional<::ortc::IMediaStreamTrackTypes::Kinds>(wrapper::impl::org::ortc::Helper::toNative(kind.value())) : Optional<::ortc::IMediaStreamTrackTypes::Kinds>()));
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::ortc::RTCRtpReceiver::receive(wrapper::org::ortc::RTCRtpParametersPtr parameters)
{
  auto native = RTCRtpParameters::toNative(parameters);
  if (!native) return Promise::createRejected(Helper::getGuiQueue());
  return Helper::toWrapper(native_->receive(*native));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpReceiver::stop()
{
  native_->stop();
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::RTCRtpContributingSourcePtr > > wrapper::impl::org::ortc::RTCRtpReceiver::getContributingSources()
{
  auto result = make_shared< list< wrapper::org::ortc::RTCRtpContributingSourcePtr > >();
  auto sources = native_->getContributingSources();
  for (auto iter = sources.begin(); iter != sources.end(); ++iter) {
    auto wrapper = RTCRtpContributingSource::toWrapper(*iter);
    if (!wrapper) continue;
    result->push_back(wrapper);
  }
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCRtpReceiver::requestSendCsrc(uint32_t csrc)
{
  native_->requestSendCSRC(csrc);
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCRtpReceiver::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::MediaStreamTrackPtr wrapper::impl::org::ortc::RTCRtpReceiver::get_track()
{
  return MediaStreamTrack::toWrapper(native_->track());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpTransportPtr wrapper::impl::org::ortc::RTCRtpReceiver::get_transport()
{
  return RTCRtpTransport::toWrapper(native_->transport());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtcpTransportPtr wrapper::impl::org::ortc::RTCRtpReceiver::get_rtcpTransport()
{
  return RTCRtcpTransport::toWrapper(native_->rtcpTransport());
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

