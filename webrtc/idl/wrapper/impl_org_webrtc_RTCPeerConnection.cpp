
#include "impl_org_webrtc_RTCPeerConnection.h"
#include "impl_org_webrtc_helpers.h"
#include "impl_org_webrtc_RTCConfiguration.h"
#include "impl_org_webrtc_RTCError.h"
#include "impl_org_webrtc_WebrtcLib.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/peerconnectionproxy.h"
#include "api/rtcerror.h"
#include "pc/peerconnection.h"
#include "impl_org_webrtc_post_include.h"

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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCPeerConnection::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCPeerConnection::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCPeerConnection::NativeType, NativeType);

typedef wrapper::impl::org::webrtc::RTCPeerConnection::NativeScopedPtr NativeScopedPtr;

typedef wrapper::impl::org::webrtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::WebRtcLib, UseWebrtcLib);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCConfiguration, UseConfiguration);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCError, UseError);

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
  auto converted = dynamic_cast<::webrtc::PeerConnection *>(native);
  if (!converted) return nullptr;

  return WRAPPER_DEPROXIFY_CLASS(::webrtc, PeerConnection, converted);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCPeerConnection::RTCPeerConnection() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCPeerConnectionPtr wrapper::org::webrtc::RTCPeerConnection::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::RTCPeerConnection>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::RTCPeerConnection::~RTCPeerConnection() noexcept
{
  thisWeak_.reset();
  teardownObserver();
  mapperSingleton().remove(native_.get());
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCStatsReportPtr > > wrapper::impl::org::webrtc::RTCPeerConnection::getStats(wrapper::org::webrtc::RTCStatsTypeSetPtr statTypes) noexcept(false)
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCStatsReportPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCCertificatePtr > > wrapper::org::webrtc::RTCPeerConnection::generateCertificate(wrapper::org::webrtc::RTCKeyParamsPtr keygenAlgorithm) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCCertificatePtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCPeerConnection::wrapper_init_org_webrtc_RTCPeerConnection(wrapper::org::webrtc::RTCConfigurationPtr config) noexcept
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
shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webrtc::RTCPeerConnection::createOffer(wrapper::org::webrtc::RTCOfferOptionsPtr options) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webrtc::RTCPeerConnection::createOffer(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webrtc::RTCPeerConnection::createAnswer(wrapper::org::webrtc::RTCAnswerOptionsPtr options) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webrtc::RTCPeerConnection::createAnswer(wrapper::org::webrtc::MediaConstraintsPtr constraints) noexcept
{
  shared_ptr< PromiseWithHolderPtr< wrapper::org::webrtc::RTCSessionDescriptionPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webrtc::RTCPeerConnection::setLocalDescription(wrapper::org::webrtc::RTCSessionDescriptionPtr constraints) noexcept
{
  PromisePtr result {};
  return result;
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webrtc::RTCPeerConnection::setRemoteDescription(wrapper::org::webrtc::RTCSessionDescriptionPtr constraints) noexcept
{
  PromisePtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCConfigurationPtr wrapper::impl::org::webrtc::RTCPeerConnection::getConfiguration() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return UseConfigurationPtr();

  auto nativeResult = native_->GetConfiguration();
  return UseConfiguration::toWrapper(nativeResult);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCPeerConnection::setConfiguration(wrapper::org::webrtc::RTCConfigurationPtr config) noexcept(false)
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
PromisePtr wrapper::impl::org::webrtc::RTCPeerConnection::addIceCandidate(wrapper::org::webrtc::RTCIceCandidatePtr candidate) noexcept
{
  PromisePtr result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCPeerConnection::removeIceCandidates(shared_ptr< list< wrapper::org::webrtc::RTCIceCandidatePtr > > candidates) noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCPeerConnection::setBitrate(wrapper::org::webrtc::RTCBitrateParametersPtr params) noexcept(false)
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCPeerConnection::close() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;
  native_->Close();
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCRtpSenderPtr wrapper::impl::org::webrtc::RTCPeerConnection::addTrack(wrapper::org::webrtc::MediaStreamTrackPtr track) noexcept(false)
{
  wrapper::org::webrtc::RTCRtpSenderPtr result {};
  return result;
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webrtc::RTCPeerConnection::removeTrack(wrapper::org::webrtc::RTCRtpSenderPtr sender) noexcept
{
  bool result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCRtpTransceiverPtr wrapper::impl::org::webrtc::RTCPeerConnection::addTransceiver(wrapper::org::webrtc::MediaStreamTrackPtr track) noexcept(false)
{
  wrapper::org::webrtc::RTCRtpTransceiverPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCRtpTransceiverPtr wrapper::impl::org::webrtc::RTCPeerConnection::addTransceiver(String kind) noexcept(false)
{
  wrapper::org::webrtc::RTCRtpTransceiverPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCRtpSenderPtr wrapper::impl::org::webrtc::RTCPeerConnection::addSender(
  String kind,
  String trackId
  ) noexcept
{
  wrapper::org::webrtc::RTCRtpSenderPtr result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webrtc::RTCRtpSenderPtr > > wrapper::impl::org::webrtc::RTCPeerConnection::getSenders() noexcept
{
  shared_ptr< list< wrapper::org::webrtc::RTCRtpSenderPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webrtc::RTCRtpReceiverPtr > > wrapper::impl::org::webrtc::RTCPeerConnection::getReceivers() noexcept
{
  shared_ptr< list< wrapper::org::webrtc::RTCRtpReceiverPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webrtc::RTCRtpTransceiverPtr > > wrapper::impl::org::webrtc::RTCPeerConnection::getTransceivers() noexcept
{
  shared_ptr< list< wrapper::org::webrtc::RTCRtpTransceiverPtr > > result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCDataChannelPtr wrapper::impl::org::webrtc::RTCPeerConnection::createDataChannel(
  String label,
  wrapper::org::webrtc::RTCDataChannelInitPtr init
  ) noexcept
{
  wrapper::org::webrtc::RTCDataChannelPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSignalingState wrapper::impl::org::webrtc::RTCPeerConnection::get_signalingState() noexcept
{
  wrapper::org::webrtc::RTCSignalingState result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCIceGatheringState wrapper::impl::org::webrtc::RTCPeerConnection::get_iceGatheringState() noexcept
{
  wrapper::org::webrtc::RTCIceGatheringState result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCIceConnectionState wrapper::impl::org::webrtc::RTCPeerConnection::get_iceConnectionState() noexcept
{
  wrapper::org::webrtc::RTCIceConnectionState result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCPeerConnectionState wrapper::impl::org::webrtc::RTCPeerConnection::get_connectionState_NotAvailable() noexcept
{
  wrapper::org::webrtc::RTCPeerConnectionState result {};
  return result;
}

//------------------------------------------------------------------------------
Optional< bool > wrapper::impl::org::webrtc::RTCPeerConnection::get_canTrickleIceCandidates_NotAvailable() noexcept
{
  Optional< bool > result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSessionDescriptionPtr wrapper::impl::org::webrtc::RTCPeerConnection::get_localDescription() noexcept
{
  wrapper::org::webrtc::RTCSessionDescriptionPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSessionDescriptionPtr wrapper::impl::org::webrtc::RTCPeerConnection::get_currentLocalDescription() noexcept
{
  wrapper::org::webrtc::RTCSessionDescriptionPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSessionDescriptionPtr wrapper::impl::org::webrtc::RTCPeerConnection::get_pendingLocalDescription() noexcept
{
  wrapper::org::webrtc::RTCSessionDescriptionPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSessionDescriptionPtr wrapper::impl::org::webrtc::RTCPeerConnection::get_remoteDescription() noexcept
{
  wrapper::org::webrtc::RTCSessionDescriptionPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSessionDescriptionPtr wrapper::impl::org::webrtc::RTCPeerConnection::get_currentRemoteDescription() noexcept
{
  wrapper::org::webrtc::RTCSessionDescriptionPtr result {};
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::RTCSessionDescriptionPtr wrapper::impl::org::webrtc::RTCPeerConnection::get_pendingRemoteDescription() noexcept
{
  wrapper::org::webrtc::RTCSessionDescriptionPtr result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCPeerConnection::wrapper_onObserverCountChanged(size_t count) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifySignalingChange(NativeType::SignalingState new_state) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyDataChannel(rtc::scoped_refptr<::webrtc::DataChannelInterface> data_channel) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyRenegotiationNeeded() noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyIceConnectionChange(NativeType::PeerConnectionInterface::IceConnectionState new_state) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyIceGatheringChange(NativeType::PeerConnectionInterface::IceGatheringState new_state) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyIceCandidate(const ::webrtc::IceCandidateInterface* candidate) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyIceConnectionReceivingChange(bool receiving) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyAddTrack(
                                     rtc::scoped_refptr<::webrtc::RtpReceiverInterface> receiver,
                                     const std::vector<rtc::scoped_refptr<::webrtc::MediaStreamInterface>>& streams
                                     ) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyTrack(rtc::scoped_refptr<::webrtc::RtpTransceiverInterface> transceiver) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::notifyRemoveTrack(rtc::scoped_refptr<::webrtc::RtpReceiverInterface> receiver) noexcept
{
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver()
{
  if (!native_) return;
  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock());
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
NativeScopedPtr WrapperImplType::toNative(WrapperTypePtr wrapper) noexcept
{
  if (!wrapper) return rtc::scoped_refptr<NativeType>();
  auto converted = ZS_DYNAMIC_PTR_CAST(WrapperImplType, wrapper);
  if (!converted) return rtc::scoped_refptr<NativeType>();
  return converted->native_;
}
