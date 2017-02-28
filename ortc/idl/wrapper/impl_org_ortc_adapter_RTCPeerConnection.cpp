
#include "impl_org_ortc_adapter_RTCPeerConnection.h"
#include "impl_org_ortc_adapter_RTCConfiguration.h"
#include "impl_org_ortc_adapter_RTCSessionDescription.h"
#include "impl_org_ortc_adapter_RTCIceCandidate.h"
#include "impl_org_ortc_adapter_RTCCapabilityOptions.h"
#include "impl_org_ortc_adapter_RTCOfferOptions.h"
#include "impl_org_ortc_adapter_RTCAnswerOptions.h"
#include "impl_org_ortc_adapter_RTCMediaStreamTrackConfiguration.h"
#include "impl_org_ortc_adapter_RTCPeerConnectionIceEvent.h"
#include "impl_org_ortc_adapter_RTCPeerConnectionIceErrorEvent.h"
#include "impl_org_ortc_adapter_MediaStream.h"
#include "impl_org_ortc_adapter_RTCTrackEvent.h"
#include "impl_org_ortc_RTCDataChannelEvent.h"
#include "impl_org_ortc_RTCRtpSender.h"
#include "impl_org_ortc_RTCRtpReceiver.h"
#include "impl_org_ortc_RTCIceServer.h"
#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_RTCDataChannel.h"
#include "impl_org_ortc_RTCDataChannelParameters.h"
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

namespace wrapper { namespace impl { namespace org { namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortc_adapter_wrapper); } } } } }

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnection::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnection::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnection::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::adapter::RTCPeerConnection::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCPeerConnection::RTCPeerConnection()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCPeerConnectionPtr wrapper::org::ortc::adapter::RTCPeerConnection::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::adapter::RTCPeerConnection>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::adapter::RTCPeerConnection::~RTCPeerConnection()
{
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
{
  return Helper::getStats(native_, statTypes);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCPeerConnection::wrapper_init_org_ortc_adapter_RTCPeerConnection()
{
  native_ = NativeType::create(thisWeak_.lock());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCPeerConnection::wrapper_init_org_ortc_adapter_RTCPeerConnection(wrapper::org::ortc::adapter::RTCConfigurationPtr configuration)
{
  auto native = RTCConfiguration::toNative(configuration);
  ZS_THROW_INVALID_ARGUMENT_IF(!native);
  native_ = NativeType::create(thisWeak_.lock(), *native);
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::createOffer()
{
  return toWrapper(native_->createOffer());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::createOffer(wrapper::org::ortc::adapter::RTCOfferOptionsPtr options)
{
  Optional< ::ortc::adapter::IPeerConnectionTypes::OfferOptions > native;
  auto convert = RTCOfferOptions::toNative(options);
  if (convert) {
    native = *convert;
  }

  return toWrapper(native_->createOffer(native));
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::createAnswer()
{
  return toWrapper(native_->createAnswer());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::createAnswer(wrapper::org::ortc::adapter::RTCAnswerOptionsPtr options)
{
  Optional< ::ortc::adapter::IPeerConnectionTypes::AnswerOptions > native;
  auto convert = RTCAnswerOptions::toNative(options);
  if (convert) {
    native = *convert;
  }

  return toWrapper(native_->createAnswer(native));
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::createCapabilities()
{
  return toWrapper(native_->createCapabilities());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::adapter::RTCSessionDescriptionPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::createCapabilities(wrapper::org::ortc::adapter::RTCCapabilityOptionsPtr options)
{
  Optional< ::ortc::adapter::IPeerConnectionTypes::CapabilityOptions > native;
  auto convert = RTCCapabilityOptions::toNative(options);
  if (convert) {
    native = *convert;
  }

  return toWrapper(native_->createCapabilities(native));
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::setLocalDescription(wrapper::org::ortc::adapter::RTCSessionDescriptionPtr description)
{
  return Helper::toWrapper(native_->setLocalDescription(RTCSessionDescription::toNative(description)));
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::setRemoteDescription(wrapper::org::ortc::adapter::RTCSessionDescriptionPtr description)
{
  return Helper::toWrapper(native_->setRemoteDescription(RTCSessionDescription::toNative(description)));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCPeerConnection::addIceCandidate(wrapper::org::ortc::adapter::RTCIceCandidatePtr candidate)
{
  auto native = RTCIceCandidate::toNative(candidate);
  ZS_THROW_INVALID_ARGUMENT_IF(!native);
  native_->addICECandidate(*native);
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::RTCIceServerPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::getDefaultIceServers()
{
  auto resultList = make_shared< list< wrapper::org::ortc::RTCIceServerPtr > >();
  auto nativeList = native_->getDefaultIceServers();
  if (nativeList) {
    for (auto iter = nativeList->begin(); iter != nativeList->end(); ++iter) {
      auto wrapper = RTCIceServer::toWrapper(*iter);
      if (!wrapper) continue;
      resultList->push_back(wrapper);
    }
  }
  return resultList;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCPeerConnection::close()
{
  native_->close();
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::RTCRtpSenderPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::getSenders()
{
  auto resultList = make_shared< list< wrapper::org::ortc::RTCRtpSenderPtr > >();
  auto nativeList = native_->getSenders();
  if (nativeList) {
    for (auto iter = nativeList->begin(); iter != nativeList->end(); ++iter) {
      auto wrapper = RTCRtpSender::toWrapper(*iter);
      if (!wrapper) continue;
      resultList->push_back(wrapper);
    }
  }
  return resultList;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::ortc::RTCRtpReceiverPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::getReceivers()
{
  auto resultList = make_shared< list< wrapper::org::ortc::RTCRtpReceiverPtr > >();
  auto nativeList = native_->getReceivers();
  if (nativeList) {
    for (auto iter = nativeList->begin(); iter != nativeList->end(); ++iter) {
      auto wrapper = RTCRtpReceiver::toWrapper(*iter);
      if (!wrapper) continue;
      resultList->push_back(wrapper);
    }
  }
  return resultList;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::addTrack(wrapper::org::ortc::MediaStreamTrackPtr track)
{
  return toWrapper(native_->addTrack(MediaStreamTrack::toNative(track)));
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::addTrack(
  wrapper::org::ortc::MediaStreamTrackPtr track,
  wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr config
  )
{
  auto nativeTrack = MediaStreamTrack::toNative(track);
  auto nativeConfig = RTCMediaStreamTrackConfiguration::toNative(config);
  ZS_THROW_INVALID_ARGUMENT_IF(!nativeConfig);

  return toWrapper(native_->addTrack(nativeTrack, *nativeConfig));
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCRtpSenderPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::addTrack(
  wrapper::org::ortc::MediaStreamTrackPtr track,
  shared_ptr< list< wrapper::org::ortc::adapter::MediaStreamPtr > > mediaStreams,
  wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr config
  )
{
  auto nativeTrack = MediaStreamTrack::toNative(track);
  list<::ortc::adapter::IMediaStreamPtr> nativeList;
  if (mediaStreams) {
    for (auto iter = mediaStreams->begin(); iter != mediaStreams->end(); ++iter) {
      auto native = MediaStream::toNative(*iter);
      if (!native) continue;
      nativeList.push_back(native);
    }
  }
  auto nativeConfig = RTCMediaStreamTrackConfiguration::toNative(config);
  ZS_THROW_INVALID_ARGUMENT_IF(!nativeConfig);

  return toWrapper(native_->addTrack(nativeTrack, nativeList, *nativeConfig));
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCPeerConnection::removeTrack(wrapper::org::ortc::RTCRtpSenderPtr sender)
{
  native_->removeTrack(RTCRtpSender::toNative(sender));
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCDataChannelPtr > > wrapper::impl::org::ortc::adapter::RTCPeerConnection::createDataChannel(wrapper::org::ortc::RTCDataChannelParametersPtr parameters)
{
  auto native = RTCDataChannelParameters::toNative(parameters);
  ZS_THROW_INVALID_ARGUMENT_IF(!native);
  return toWrapper(native_->createDataChannel(*native));
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionPtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_localDescription()
{
  return RTCSessionDescription::toWrapper(native_->localDescription());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionPtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_currentDescription()
{
  return RTCSessionDescription::toWrapper(native_->currentDescription());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionPtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_pendingDescription()
{
  return RTCSessionDescription::toWrapper(native_->pendingDescription());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionPtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_remoteDescription()
{
  return RTCSessionDescription::toWrapper(native_->remoteDescription());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionPtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_currentRemoteDescription()
{
  return RTCSessionDescription::toWrapper(native_->currentRemoteDescription());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSessionDescriptionPtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_pendingRemoteDescription()
{
  return RTCSessionDescription::toWrapper(native_->pendingRemoteDescription());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCSignalingState wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_signalingState()
{
  return Helper::toWrapper(native_->signalingState());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCIceGathererState wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_iceGatheringState()
{
  return Helper::toWrapper(native_->iceGatheringState());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCIceConnectionState wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_iceConnectionState()
{
  return Helper::toPeerConnectionWrapper(native_->iceConnectionState());
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCPeerConnectionState wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_connectionState()
{
  return Helper::toWrapper(native_->connectionState());
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_canTrickleCandidates()
{
  return native_->canTrickleCandidates();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::adapter::RTCConfigurationPtr wrapper::impl::org::ortc::adapter::RTCPeerConnection::get_getConfiguration()
{
  return RTCConfiguration::toWrapper(native_->getConfiguration());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCPeerConnection::set_getConfiguration(wrapper::org::ortc::adapter::RTCConfigurationPtr value)
{
  auto native = RTCConfiguration::toNative(value);
  ZS_THROW_INVALID_ARGUMENT_IF(!native);
  native_->setConfiguration(*native);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::adapter::RTCPeerConnection::wrapper_onObserverCountChanged(size_t count)
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionNegotiationNeeded(IPeerConnectionPtr connection)
{
  onNegotiationNeeded();
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionIceCandidate(
  IPeerConnectionPtr connection,
  ICECandidatePtr candidate,
  const char *url
)
{
  onIceCandidate(RTCPeerConnectionIceEvent::toWrapper(candidate, String(url)));
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionIceCandidateError(
  IPeerConnectionPtr connection,
  ICECandidateErrorEventPtr error
)
{
  onIceCandidateError(RTCPeerConnectionIceErrorEvent::toWrapper(error));
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionSignalingStateChange(
  IPeerConnectionPtr connection,
  SignalingStates state
)
{
  onSignalingStateChange();
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionICEGatheringStateChange(
  IPeerConnectionPtr connection,
  ICEGatheringStates state
)
{
  onIceGatheringStateChange();
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionICEConnectionStateChange(
  IPeerConnectionPtr connection,
  ICEConnectionStates state
)
{
  onIceConnectionStateChange();
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionConnectionStateChange(
  IPeerConnectionPtr connection,
  PeerConnectionStates state
)
{
  onConnectionStateChange();
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionTrack(
  IPeerConnectionPtr connection,
  MediaStreamTrackEventPtr event
)
{
  onTrack(RTCTrackEvent::toWrapper(event));
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionTrackGone(
  IPeerConnectionPtr connection,
  MediaStreamTrackEventPtr event
)
{
  onTrackGone(RTCTrackEvent::toWrapper(event));
}

//------------------------------------------------------------------------------
void WrapperImplType::onPeerConnectionDataChannel(
  IPeerConnectionPtr connection,
  IDataChannelPtr dataChannel
)
{
  onDataChannel(RTCDataChannelEvent::toWrapper(dataChannel));
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

//------------------------------------------------------------------------------
WrapperImplType::WrapperPromiseWithSessionDescriptionPtr WrapperImplType::toWrapper(NativePromiseWithSessionDescriptionPtr promise)
{
  if (!promise) return WrapperPromiseWithSessionDescriptionPtr();

  auto result = WrapperPromiseWithSessionDescription::create(Helper::getGuiQueue());
  promise->thenClosure([result, promise] {
    if (promise->isRejected()) {
      Helper::reject(promise, result);
      return;
    }
    result->resolve(RTCSessionDescription::toWrapper(promise->value()));
  });
  promise->background();
  return result;
}

//------------------------------------------------------------------------------
WrapperImplType::WrapperPromiseWithSenderPtr WrapperImplType::toWrapper(NativePromiseWithSenderPtr promise)
{
  if (!promise) return WrapperPromiseWithSenderPtr();

  auto result = WrapperPromiseWithSender::create(Helper::getGuiQueue());
  promise->thenClosure([result, promise] {
    if (promise->isRejected()) {
      Helper::reject(promise, result);
      return;
    }
    result->resolve(RTCRtpSender::toWrapper(promise->value()));
  });
  promise->background();
  return result;
}

//------------------------------------------------------------------------------
WrapperImplType::WrapperPromiseWithDataChannelPtr WrapperImplType::toWrapper(NativePromiseWithDataChannelPtr promise)
{
  if (!promise) return WrapperPromiseWithDataChannelPtr();

  auto result = WrapperPromiseWithDataChannel::create(Helper::getGuiQueue());
  promise->thenClosure([result, promise] {
    if (promise->isRejected()) {
      Helper::reject(promise, result);
      return;
    }
    result->resolve(RTCDataChannel::toWrapper(promise->value()));
  });
  promise->background();
  return result;
}
