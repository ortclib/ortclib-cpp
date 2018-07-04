
#include "impl_org_webRtc_RTCPeerConnection.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCConfiguration.h"
#include "impl_org_webRtc_RTCCertificate.h"
#include "impl_org_webRtc_RTCOfferOptions.h"
#include "impl_org_webRtc_RTCAnswerOptions.h"
#include "impl_org_webRtc_RTCSessionDescription.h"
#include "impl_org_webRtc_RTCDataChannelEvent.h"
#include "impl_org_webRtc_RTCDataChannel.h"
#include "impl_org_webRtc_RTCDataChannelInit.h"
#include "impl_org_webRtc_RTCIceCandidate.h"
#include "impl_org_webRtc_RTCPeerConnectionIceEvent.h"
#include "impl_org_webRtc_RTCTrackEvent.h"
#include "impl_org_webRtc_MediaStreamTrack.h"
#include "impl_org_webRtc_MediaConstraints.h"
#include "impl_org_webRtc_RTCRtpSender.h"
#include "impl_org_webRtc_RTCRtpReceiver.h"
#include "impl_org_webRtc_RTCRtpTransceiver.h"
#include "impl_org_webRtc_RTCRtpTransceiverInit.h"
#include "impl_org_webRtc_RTCSessionDescription.h"
#include "impl_org_webRtc_RTCError.h"
#include "impl_org_webRtc_RTCBitrateParameters.h"
#include "impl_org_webRtc_RTCKeyParams.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectionproxy.h"
#include "api/rtcerror.h"
#include "pc/peerconnection.h"
#include "rtc_base/refcountedobject.h"
#include "rtc_base/rtccertificategenerator.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCCertificate, UseCertificate);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError, UseError);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCOfferOptions, UseOfferOptions);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCAnswerOptions, UseAnswerOptions);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCSessionDescription, UseSessionDescription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannel, UseDataChannel);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannelInit, UseDataChannelInit);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannelEvent, UseDataChannelEvent);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCIceCandidate, UseIceCandidate);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCPeerConnectionIceEvent, UseIceCandidateEvent);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSender, UseRtpSender);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver, UseRtpReceiver);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiver, UseRtpTransceiver);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpTransceiverInit, UseRtpTransceiverInit);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCTrackEvent, UseTrackEvent);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaStreamTrack, UseMediaStreamTrack);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaConstraints, UseMediaConstraints);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCBitrateParameters, UseBitrateParameters);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCKeyParams, UseKeyParams);


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

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {
        namespace peer_connection {
          typedef PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > SessionDescriptionPromiseType;
          ZS_DECLARE_PTR(SessionDescriptionPromiseType);

          class SessionDescriptionObserver : public ::rtc::RefCountedObject<::webrtc::CreateSessionDescriptionObserver> {
          public:
            typedef rtc::scoped_refptr<SessionDescriptionObserver> ObserverScopedPtr;

            void OnSuccess(::webrtc::SessionDescriptionInterface* desc) override
            {
              auto wrapper = UseSessionDescription::toWrapper(desc);
              promise_->resolve(wrapper);
            }

            void OnFailure(const std::string& error) override
            {
              UseError::rejectPromise(promise_, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER, error.c_str()));
            }

            static ObserverScopedPtr create(SessionDescriptionPromiseTypePtr promise) noexcept {
              ObserverScopedPtr result(new SessionDescriptionObserver());
              result->promise_ = promise;
              return result;
            }

            SessionDescriptionPromiseTypePtr promise_;
          };
        } // namespace peer_connection
      } // namespace webRtc
    } // namespace org
  } // namespace impl
} // namespace wrapper

using namespace wrapper::impl::org::webRtc::peer_connection;


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
  typedef PromiseWithHolderPtr< wrapper::org::webRtc::RTCCertificatePtr > CertificatePromiseType;
  ZS_DECLARE_PTR(CertificatePromiseType);

  class Observer : public ::rtc::RefCountedObject<::rtc::RTCCertificateGeneratorCallback> {
  public:
    typedef rtc::scoped_refptr<Observer> ObserverScopedPtr;
    typedef rtc::scoped_refptr<::rtc::RTCCertificate> CertificateScopedPtr;

    void OnSuccess(const CertificateScopedPtr& certificate) override
    {
      auto wrapper = UseCertificate::toWrapper(certificate);
      promise_->resolve(wrapper);
    }

    void OnFailure()
    {
      UseError::rejectPromise(promise_, ::webrtc::RTCError(::webrtc::RTCErrorType::UNSUPPORTED_PARAMETER));
    }
    
    static ObserverScopedPtr create(
      CertificatePromiseTypePtr promise,
      const std::shared_ptr<rtc::RTCCertificateGenerator> &gen
    ) noexcept
    {
      ObserverScopedPtr result(new Observer());
      result->promise_ = promise;
      result->gen_ = std::move(gen);
      return result;
    }

    CertificatePromiseTypePtr promise_;
    std::shared_ptr<rtc::RTCCertificateGenerator> gen_;
  };


  auto result = CertificatePromiseType::create(UseWebrtcLib::delegateQueue());

  auto factory = UseWebrtcLib::peerConnectionFactory();
  if (!factory) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_STATE));
    return result;
  }

  auto realFactory = dynamic_cast<::webrtc::PeerConnection *>(factory.get());
  ZS_ASSERT(realFactory);

  auto nativeValue = UseKeyParams::toNative(keygenAlgorithm);
  if (!nativeValue) nativeValue = UseKeyParams::toNative(UseKeyParams::createDefault());
  ZS_ASSERT(nativeValue);
  if (!nativeValue) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_STATE));
    return result;
  }

  auto gen = std::make_shared<rtc::RTCCertificateGenerator>(realFactory->signaling_thread(), realFactory->network_thread());

  auto observer = Observer::create(result, std::move(gen));

  gen->GenerateCertificateAsync(*nativeValue, ::rtc::Optional<uint64_t>(), observer);

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
  ZS_ASSERT(native_);
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::createOffer(wrapper::org::webRtc::RTCOfferOptionsPtr options) noexcept
{
  auto result = SessionDescriptionPromiseType::create(UseWebrtcLib::delegateQueue());

  ZS_ASSERT(native_);
  if (!native_) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_STATE));
    return result;
  }

  auto nativeOptions = UseOfferOptions::toNative(options);
  if (!nativeOptions) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER));
    return result;
  }

  native_->CreateOffer(SessionDescriptionObserver::create(result), *nativeOptions);
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::createOffer(wrapper::org::webRtc::MediaConstraintsPtr constraints) noexcept
{
  auto result = SessionDescriptionPromiseType::create(UseWebrtcLib::delegateQueue());

  ZS_ASSERT(native_);
  if (!native_) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_STATE));
    return result;
  }

  auto nativeConstraints = UseMediaConstraints::toNative(constraints);
  if (!nativeConstraints) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER));
    return result;
  }

  native_->CreateOffer(SessionDescriptionObserver::create(result), nativeConstraints.get());
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::createAnswer(wrapper::org::webRtc::RTCAnswerOptionsPtr options) noexcept
{
  auto result = SessionDescriptionPromiseType::create(UseWebrtcLib::delegateQueue());

  ZS_ASSERT(native_);
  if (!native_) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_STATE));
    return result;
  }

  auto nativeOptions = UseAnswerOptions::toNative(options);
  if (!nativeOptions) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER));
    return result;
  }

  native_->CreateAnswer(SessionDescriptionObserver::create(result), *nativeOptions);
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCSessionDescriptionPtr > > wrapper::impl::org::webRtc::RTCPeerConnection::createAnswer(wrapper::org::webRtc::MediaConstraintsPtr constraints) noexcept
{
  auto result = SessionDescriptionPromiseType::create(UseWebrtcLib::delegateQueue());

  ZS_ASSERT(native_);
  if (!native_) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_STATE));
    return result;
  }

  auto nativeConstraints = UseMediaConstraints::toNative(constraints);
  if (!nativeConstraints) {
    UseError::rejectPromise(result, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER));
    return result;
  }

  native_->CreateAnswer(SessionDescriptionObserver::create(result), nativeConstraints.get());
  return result;
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCPeerConnection::setLocalDescription(wrapper::org::webRtc::RTCSessionDescriptionPtr description) noexcept
{
  class Observer : public ::rtc::RefCountedObject<::webrtc::SetSessionDescriptionObserver> {
  public:
    typedef rtc::scoped_refptr<Observer> ObserverScopedPtr;

    void OnSuccess() override
    {
      promise_->resolve();
    }

    void OnFailure(const std::string& error) override
    {
      UseError::rejectPromise(promise_, ::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER, error.c_str()));
    }

    static ObserverScopedPtr create(PromisePtr promise) noexcept {
      ObserverScopedPtr result(new Observer());
      result->promise_ = promise;
      return result;
    }

    PromisePtr promise_;
  };

  ZS_ASSERT(native_);
  if (!native_) return UseError::toPromise(::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_STATE));

  auto nativeDescription = UseSessionDescription::toNative(description);
  if (!nativeDescription) return UseError::toPromise(::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER));

  auto promise = Promise::create(UseWebrtcLib::delegateQueue());
  native_->SetLocalDescription(Observer::create(promise), nativeDescription.release());
  return promise;
}

//------------------------------------------------------------------------------
PromisePtr wrapper::impl::org::webRtc::RTCPeerConnection::setRemoteDescription(wrapper::org::webRtc::RTCSessionDescriptionPtr description) noexcept
{
  class Observer : public ::rtc::RefCountedObject<::webrtc::SetRemoteDescriptionObserverInterface> {
  public:
    typedef rtc::scoped_refptr<Observer> ObserverScopedPtr;

    void OnSetRemoteDescriptionComplete(::webrtc::RTCError error) override
    {
      if (error.ok()) {
        promise_->resolve();
        return;
      }
      UseError::rejectPromise(promise_, error);
    }

    static ObserverScopedPtr create(PromisePtr promise) noexcept {
      ObserverScopedPtr result(new Observer());
      result->promise_ = promise;
      return result;
    }

    PromisePtr promise_;
  };

  ZS_ASSERT(native_);
  if (!native_) return UseError::toPromise(::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_STATE));

  auto nativeDescription = UseSessionDescription::toNative(description);
  if (!nativeDescription) return UseError::toPromise(::webrtc::RTCError(::webrtc::RTCErrorType::INVALID_PARAMETER));

  auto promise = Promise::create(UseWebrtcLib::delegateQueue());
  native_->SetRemoteDescription(std::move(nativeDescription), Observer::create(promise));
  return promise;
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
  ZS_ASSERT(native_);
  if (!native_) return zsLib::Promise::createRejected(UseWebrtcLib::delegateQueue());

  auto nativeCandidate = UseIceCandidate::toNative(candidate);
  if (!nativeCandidate) return zsLib::Promise::createRejected(UseWebrtcLib::delegateQueue());

  auto result = native_->AddIceCandidate(nativeCandidate.get());
  if (!result) return zsLib::Promise::createRejected(UseWebrtcLib::delegateQueue());
  return zsLib::Promise::createResolved(UseWebrtcLib::delegateQueue());
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCPeerConnection::removeIceCandidates(shared_ptr< list< wrapper::org::webRtc::RTCIceCandidatePtr > > candidates) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return;

  if (!candidates) return;

  std::vector<cricket::Candidate> nativeCandidates;

  for (auto iter = candidates->begin(); iter != candidates->end(); ++iter) {
    auto native = UseIceCandidate::toNative(*iter);
    if (!native) continue;
    nativeCandidates.push_back(native->candidate());
  }

  native_->RemoveIceCandidates(nativeCandidates);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webRtc::RTCPeerConnection::setBitrate(wrapper::org::webRtc::RTCBitrateParametersPtr params) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return;
  auto nativeParams = UseBitrateParameters::toNative(params);

  auto result = native_->SetBitrate(*nativeParams);
  if (!result.ok()) throw UseError::toWrapper(result);
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
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCRtpSenderPtr();

  auto nativeTrack = UseMediaStreamTrack::toNative(track);
  if (!nativeTrack) return wrapper::org::webRtc::RTCRtpSenderPtr();

  std::vector<::webrtc::MediaStreamInterface*> ignored;
  return UseRtpSender::toWrapper(native_->AddTrack(nativeTrack, ignored));
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::webRtc::RTCPeerConnection::removeTrack(wrapper::org::webRtc::RTCRtpSenderPtr sender) noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return false;

  auto nativeSender = UseRtpSender::toNative(sender);
  if (!nativeSender) return false;

  return native_->RemoveTrack(nativeSender);
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverPtr wrapper::impl::org::webRtc::RTCPeerConnection::addTransceiver(wrapper::org::webRtc::MediaStreamTrackPtr track) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return UseRtpTransceiverPtr();

  auto nativeTrack = UseMediaStreamTrack::toNative(track);
  if (!nativeTrack) {
    ::webrtc::RTCError error(::webrtc::RTCErrorType::INVALID_PARAMETER);
    throw UseError::toWrapper(error);
  }

  auto result = native_->AddTransceiver(nativeTrack);
  if (!result.ok()) throw UseError::toWrapper(result.error());
  return UseRtpTransceiver::toWrapper(result.value());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverPtr wrapper::impl::org::webRtc::RTCPeerConnection::addTransceiver(
  wrapper::org::webRtc::MediaStreamTrackPtr track,
  wrapper::org::webRtc::RTCRtpTransceiverInitPtr init
  ) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return UseRtpTransceiverPtr();

  auto nativeTrack = UseMediaStreamTrack::toNative(track);
  auto nativeInit = UseRtpTransceiverInit::toNative(init);
  if ((!nativeTrack) ||
      (!nativeInit)) {
    ::webrtc::RTCError error(::webrtc::RTCErrorType::INVALID_PARAMETER);
    throw UseError::toWrapper(error);
  }

  auto result = native_->AddTransceiver(nativeTrack, *nativeInit);
  if (!result.ok()) throw UseError::toWrapper(result.error());
  return UseRtpTransceiver::toWrapper(result.value());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpTransceiverPtr wrapper::impl::org::webRtc::RTCPeerConnection::addTransceiver(String kind) noexcept(false)
{
  ZS_ASSERT(native_);
  if (!native_) return UseRtpTransceiverPtr();

  ::cricket::MediaType type {};
  try {
    type = UseEnum::toNativeMediaType(kind);
  } catch (const ::zsLib::Exceptions::InvalidArgument &) {
    ::webrtc::RTCError error(::webrtc::RTCErrorType::INVALID_PARAMETER);
    throw UseError::toWrapper(error);
  }

  auto result = native_->AddTransceiver(type);
  if (!result.ok()) throw UseError::toWrapper(result.error());
  return UseRtpTransceiver::toWrapper(result.value());
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
void WrapperImplType::setupObserver() noexcept
{
  if (!native_) return;
  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock(), UseWebrtcLib::delegateQueue());
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver() noexcept
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
