// Generated by zsLibEventingTool

#include "impl_org_webrtc_RTCPeerConnection.h"

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
wrapper::impl::org::webrtc::RTCPeerConnection::~RTCPeerConnection()
{
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
  wrapper::org::webrtc::RTCConfigurationPtr result {};
  return result;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::RTCPeerConnection::setConfiguration(wrapper::org::webrtc::RTCConfigurationPtr config) noexcept(false)
{
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

