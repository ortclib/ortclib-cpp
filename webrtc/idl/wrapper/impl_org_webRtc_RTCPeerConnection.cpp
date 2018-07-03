
#include "impl_org_webRtc_RTCPeerConnection.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCConfiguration.h"
#include "impl_org_webRtc_RTCDataChannelEvent.h"
#include "impl_org_webRtc_RTCDataChannel.h"
#include "impl_org_webRtc_RTCDataChannelInit.h"
#include "impl_org_webRtc_RTCIceCandidate.h"
#include "impl_org_webRtc_RTCPeerConnectionIceEvent.h"
#include "impl_org_webRtc_RTCTrackEvent.h"
#include "impl_org_webRtc_RTCRtpSender.h"
#include "impl_org_webRtc_RTCRtpReceiver.h"
#include "impl_org_webRtc_RTCRtpTransceiver.h"
#include "impl_org_webRtc_RTCSessionDescription.h"
#include "impl_org_webRtc_RTCError.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectionproxy.h"
#include "api/rtcerror.h"
#include "pc/peerconnection.h"
#include "impl_org_webRtc_post_include.h"

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

// borrow definitions from class
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCPeerConnection::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCPeerConnection::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCPeerConnection::NativeType, NativeType);

typedef wrapper::impl::org::webRtc::RTCPeerConnection::NativeTypeScopedPtr NativeTypeScopedPtr;

typedef wrapper::impl::org::webRtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCConfiguration, UseConfiguration);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError, UseError);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannel, UseDataChannel);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannelInit, UseDataChannelInit);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannelEvent, UseDataChannelEvent);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidate, UseIceCandidate);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent, UseIceCandidateEvent);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSender, UseRtpSender);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver, UseRtpReceiver);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiver, UseRtpTransceiver);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCTrackEvent, UseTrackEvent);

//------------------------------------------------------------------------------
static UseWrapperMapper &mapperSingleton()
{
  static UseWrapperMapper singleton;
  return singleton;
}

//------------------------------------------------------------------------------
static ::webrtc::PeerConnectionInterface *unproxyNative(NativeType *native)
{
  if (!native) return nullptr;
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::PeerConnection, ::webrtc::PeerConnection, native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCPeerConnection::RTCPeerConnection() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCPeerConnectionPtr wrapper::org::webRtc::RTCPeerConnection::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCPeerConnection>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCPeerConnection::~RTCPeerConnection() noexcept
{
  thisWeak_.reset();
  teardownObserver();
  mapperSingleton().remove(native_.get());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCStatsReportPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::getStats(wrapper::org::webRtc::RTCStatsTypeSetPtr statTypes) noexcept(false)
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCStatsReportPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCCertificatePtr > > wrapper::org::webRtc::RTCPeerConnection::generateCertificate(wrapper::org::webRtc::RTCKeyParamsPtr keygenAlgorithm) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCCertificatePtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCPeerConnection::wrapper_init_org_webRtc_RTCPeerConnection(wrapper::org::webRtc::RTCConfigurationPtr config) noexcept
{
  auto factory = UseWebrtcLib::peerConnectionFactory();
  if (!factory) return;

  ZS_ASSERT(config);

  setupObserver();
  ZS_ASSERT(observer_);

  auto nativeConfig = UseConfiguration::toNative(config);

  native_ = factory->CreatePeerConnection(*nativeConfig, nullptr, nullptr, observer_.get());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::createOffer(wrapper::org::webRtc::RTCOfferOptionsPtr options) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::createOffer(wrapper::org::webRtc::MediaConstraintsPtr constraints) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::createAnswer(wrapper::org::webRtc::RTCAnswerOptionsPtr options) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::createAnswer(wrapper::org::webRtc::MediaConstraintsPtr constraints) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCPeerConnection::setLocalDescription(wrapper::org::webRtc::RTCSessionDescriptionPtr constraints) noexcept
{
  PromisePtr result {};
  return result;
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCPeerConnection::setRemoteDescription(wrapper::org::webRtc::RTCSessionDescriptionPtr constraints) noexcept
{
  PromisePtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCConfigurationPtr wrapper::impl::org::webRtc::RTCPeerConnection::getConfiguration() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return UseConfigurationPtr();

  auto nativeResult = native_->GetConfiguration();
  return UseConfiguration::toWrapper(nativeResult);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCPeerConnection::setConfiguration(wrapper::org::webRtc::RTCConfigurationPtr config) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return;

  auto nativeConfig = UseConfiguration::toNative(config);
  ZS_ASSERT(nativeConfig);
  if (!nativeConfig) return;

  ::webrtc::RTCError error;
  if (!native_->SetConfiguration(*nativeConfig, &error)) throw UseError::toWrapper(error);
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCPeerConnection::addIceCandidate(wrapper::org::webRtc::RTCIceCandidatePtr candidate) noexcept
{
  PromisePtr result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCPeerConnection::removeIceCandidates(shared_ptr< list< wrapper::org::webRtc::RTCIceCandidatePtr > > candidates) noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCPeerConnection::setBitrate(wrapper::org::webRtc::RTCBitrateParametersPtr params) noexcept(false)
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCPeerConnection::close() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;
  native_->Close();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSenderPtr wrapper::impl::org::webRtc::RTCPeerConnection::addTrack(wrapper::org::webRtc::MediaStreamTrackPtr track) noexcept(false)
{
  wrapper::org::webRtc::RTCRtpSenderPtr result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCPeerConnection::removeTrack(wrapper::org::webRtc::RTCRtpSenderPtr sender) noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverPtr wrapper::impl::org::webRtc::RTCPeerConnection::addTransceiver(wrapper::org::webRtc::MediaStreamTrackPtr track) noexcept(false)
{
  wrapper::org::webRtc::RTCRtpTransceiverPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverPtr wrapper::impl::org::webRtc::RTCPeerConnection::addTransceiver(String kind) noexcept(false)
{
  wrapper::org::webRtc::RTCRtpTransceiverPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpSenderPtr wrapper::impl::org::webRtc::RTCPeerConnection::addSender(
  String kind,
  String trackId
  ) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return UseRtpSenderPtr();

  return UseRtpSender::toWrapper(native_->CreateSender(kind, trackId));
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::RTCRtpSenderPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::getSenders() noexcept
{
  auto result = make_shared< list< wrapper::org::webRtc::RTCRtpSenderPtr > >();
  ZS_ASSERT(native_);
  if (!native_) return result;

  auto values = native_->GetSenders();
  for (auto iter = values.begin(); iter != values.end(); ++iter) {
    auto wrapper = UseRtpSender::toWrapper(*iter);
    if (!wrapper) continue;
    result->push_back(wrapper);
  }
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::RTCRtpReceiverPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::getReceivers() noexcept
{
  auto result = make_shared< list< wrapper::org::webRtc::RTCRtpReceiverPtr > >();
  ZS_ASSERT(native_);
  if (!native_) return result;

  auto values = native_->GetReceivers();
  for (auto iter = values.begin(); iter != values.end(); ++iter) {
    auto wrapper = UseRtpReceiver::toWrapper(*iter);
    if (!wrapper) continue;
    result->push_back(wrapper);
  }
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::RTCRtpTransceiverPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::getTransceivers() noexcept
{
  auto result = make_shared< list< wrapper::org::webRtc::RTCRtpTransceiverPtr > >();
  ZS_ASSERT(native_);
  if (!native_) return result;

  auto values = native_->GetTransceivers();
  for (auto iter = values.begin(); iter != values.end(); ++iter) {
    auto wrapper = UseRtpTransceiver::toWrapper(*iter);
    if (!wrapper) continue;
    result->push_back(wrapper);
  }
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCDataChannelPtr wrapper::impl::org::webRtc::RTCPeerConnection::createDataChannel(
  String label,
  wrapper::org::webRtc::RTCDataChannelInitPtr init
  ) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCDataChannelPtr();

  auto nativeInit = UseDataChannelInit::toNative(init);
  if (!nativeInit) return wrapper::org::webRtc::RTCDataChannelPtr();

  return UseDataChannel::toWrapper(native_->CreateDataChannel(label, nativeInit.get()));
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSignalingState wrapper::impl::org::webRtc::RTCPeerConnection::get_signalingState() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCSignalingState::RTCSignalingState_closed;
  return UseEnum::toWrapper(native_->signaling_state());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceGatheringState wrapper::impl::org::webRtc::RTCPeerConnection::get_iceGatheringState() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCIceGatheringState::RTCIceGatheringState_complete;
  return UseEnum::toWrapper(native_->ice_gathering_state());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCIceConnectionState wrapper::impl::org::webRtc::RTCPeerConnection::get_iceConnectionState() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCIceConnectionState::RTCIceConnectionState_closed;
  return UseEnum::toWrapper(native_->ice_connection_state());
}

//------------------------------------------------------------------------------
Optional< wrapper::org::webRtc::RTCPeerConnectionState > wrapper::impl::org::webRtc::RTCPeerConnection::get_connectionState_NotAvailable() noexcept
{
#pragma ZS_BUILD_NOTE("LATER","Does not appear to be any way to ask peer connection for connection state")
  Optional< wrapper::org::webRtc::RTCPeerConnectionState > result {};
  return result;
}

//------------------------------------------------------------------------------
Optional< bool > wrapper::impl::org::webRtc::RTCPeerConnection::get_canTrickleIceCandidates_NotAvailable() noexcept
{
#pragma ZS_BUILD_NOTE("LATER","Does not appear to be any way to check if peer connection is allowed to trickle or not")
  return Optional< bool >();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSessionDescriptionPtr wrapper::impl::org::webRtc::RTCPeerConnection::get_localDescription() noexcept
{
  auto result = get_pendingLocalDescription();
  if (result) return result;
  return get_currentLocalDescription();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSessionDescriptionPtr wrapper::impl::org::webRtc::RTCPeerConnection::get_currentLocalDescription() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return UseSessionDescriptionPtr();
  return UseSessionDescription::toWrapper(native_->current_local_description());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSessionDescriptionPtr wrapper::impl::org::webRtc::RTCPeerConnection::get_pendingLocalDescription() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return UseSessionDescriptionPtr();
  return UseSessionDescription::toWrapper(native_->pending_local_description());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSessionDescriptionPtr wrapper::impl::org::webRtc::RTCPeerConnection::get_remoteDescription() noexcept
{
  auto result = get_pendingRemoteDescription();
  if (result) return result;
  return get_currentRemoteDescription();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSessionDescriptionPtr wrapper::impl::org::webRtc::RTCPeerConnection::get_currentRemoteDescription() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return UseSessionDescriptionPtr();
  return UseSessionDescription::toWrapper(native_->current_remote_description());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCSessionDescriptionPtr wrapper::impl::org::webRtc::RTCPeerConnection::get_pendingRemoteDescription() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return UseSessionDescriptionPtr();
  return UseSessionDescription::toWrapper(native_->pending_remote_description());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCPeerConnection::wrapper_onObserverCountChanged(ZS_MAYBE_USED() size_t count) noexcept
{
  ZS_MAYBE_USED(count);
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverSignalingChange(ZS_MAYBE_USED() NativeType::SignalingState new_state) noexcept
{
  ZS_MAYBE_USED(new_state);
  onSignalingStateChange();
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverDataChannel(rtc::scoped_refptr<::webrtc::DataChannelInterface> data_channel) noexcept
{
  auto event = RTCDataChannelEvent::toWrapper(UseDataChannel::toWrapper(data_channel));
  if (!event) return;
  onDataChannel(event);
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverRenegotiationNeeded() noexcept
{
  onNegotiationNeeded();
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverIceConnectionChange(ZS_MAYBE_USED() NativeType::PeerConnectionInterface::IceConnectionState new_state) noexcept
{
  ZS_MAYBE_USED(new_state);
  onIceConnectionStateChange();
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverIceGatheringChange(ZS_MAYBE_USED() NativeType::PeerConnectionInterface::IceGatheringState new_state) noexcept
{
  ZS_MAYBE_USED(new_state);
  onIceGatheringStateChange();
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverIceCandidate(UseIceCandidatePtr candidate) noexcept
{
  auto event = UseIceCandidateEvent::toWrapper(candidate);
  if (!event) return;
  onIceCandidate(event);
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverIceCandidatesRemoved(shared_ptr< list< UseIceCandidatePtr > > candidates) noexcept
{
  if (!candidates) return;
  for (auto iter = candidates->begin(); iter != candidates->end(); ++iter) {
    auto &candidate = (*iter);
    auto event = UseIceCandidateEvent::toWrapper(candidate);
    if (!event) continue;
    onIceCandidateRemoved(event);
  }
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverIceConnectionReceivingChange(ZS_MAYBE_USED() bool receiving) noexcept
{
  ZS_MAYBE_USED(receiving);
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverAddTrack(rtc::scoped_refptr<::webrtc::RtpReceiverInterface> receiver) noexcept
{
  auto event = UseTrackEvent::toWrapper(UseRtpReceiver::toWrapper(receiver));
  if (event) return;
  onTrack(event);
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverTrack(rtc::scoped_refptr<::webrtc::RtpTransceiverInterface> transceiver) noexcept
{
  auto event = UseTrackEvent::toWrapper(UseRtpTransceiver::toWrapper(transceiver));
  if (event) return;
  onTrack(event);
}

//------------------------------------------------------------------------------
void WrapperImplType::onWebrtcObserverRemoveTrack(rtc::scoped_refptr<::webrtc::RtpReceiverInterface> receiver) noexcept
{
  auto event = UseTrackEvent::toWrapper(UseRtpReceiver::toWrapper(receiver));
  if (event) return;
  onRemoveTrack(event);
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver()
{
  if (!native_) return;
  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock(), UseWebrtcLib::delegateQueue());
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver()
{
  if (!observer_) return;
  if (!native_) return;

#pragma ZS_BUILD_NOTE("LATER","no apparent way to remove obvserver from peer connection object")
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  NativeType *original = unproxyNative(native);
  if (!original) return WrapperImplTypePtr();

  // search for original non-proxied pointer in map
  auto wrapper = mapperSingleton().getExistingOrCreateNew(original, [native]() {
    auto result = make_shared<WrapperImplType>();
    result->thisWeak_ = result;
    result->native_ = rtc::scoped_refptr<NativeType>(native); // only use proxy and never original pointer
    result->setupObserver();
    return result;
  });
  return wrapper;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypeScopedPtr native) noexcept
{
  return toWrapper(native.get());
}

//------------------------------------------------------------------------------
NativeTypeScopedPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return rtc::scoped_refptr<NativeType>();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return rtc::scoped_refptr<NativeType>();
  return converted->native_;
}
