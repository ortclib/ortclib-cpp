
#include "impl_org_webRtc_RTCRtpReceiver.h"
#include "impl_org_webRtc_RTCRtpReceiveParameters.h"
#include "impl_org_webRtc_RTCRtpCapabilities.h"
#include "impl_org_webRtc_MediaStreamTrack.h"
#include "impl_org_webRtc_helpers.h"
#include "impl_org_webRtc_enums.h"
#include "impl_org_webRtc_RTCRtpContributingSource.h"
#include "impl_org_webRtc_RTCRtpSynchronizationSource.h"
#include "impl_org_webRtc_WebrtcLib.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/rtpreceiverinterface.h"
#include "pc/rtpreceiver.h"
#include "pc/peerconnectionfactory.h"
#include "media/base/mediaengine.h"
#include "ortc/rtpparametersconversion.h"
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
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiver::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::NativeType, NativeType);

typedef WrapperImplType::NativeTypeScopedPtr NativeTypeScopedPtr;

typedef wrapper::impl::org::webRtc::WrapperMapper<NativeType, WrapperImplType> UseWrapperMapper;

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib, UseWebrtcLib);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::IEnum, UseEnum);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpReceiveParameters, UseRtpReceiveParameters);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpCapabilities, UseRtpCapabilities);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaStreamTrack, UseMediaStreamTrack);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpContributingSource, UseRtpContributingSource);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCRtpSynchronizationSource, UseRtpSynchronizationSource);


//-----------------------------------------------------------------------------
// NOTE: borrowed from webrtc/ortc/ortcfactory.cc
namespace {

  // Asserts that all of the built-in capabilities can be converted to
  // RtpCapabilities. If they can't, something's wrong (for example, maybe a new
  // feedback mechanism is supported, but an enum value wasn't added to
  // rtpparameters.h).
  template <typename C>
  ::webrtc::RtpCapabilities ToRtpCapabilitiesWithAsserts(
    const std::vector<C>& cricket_codecs,
    const cricket::RtpHeaderExtensions& cricket_extensions) {
    ::webrtc::RtpCapabilities capabilities =
      ::webrtc::ToRtpCapabilities(cricket_codecs, cricket_extensions);
    RTC_DCHECK_EQ(capabilities.codecs.size(), cricket_codecs.size());
    for (size_t i = 0; i < capabilities.codecs.size(); ++i) {
      RTC_DCHECK_EQ(capabilities.codecs[i].rtcp_feedback.size(),
        cricket_codecs[i].feedback_params.params().size());
    }
    RTC_DCHECK_EQ(capabilities.header_extensions.size(),
      cricket_extensions.size());
    return capabilities;
  }

}  // namespace

//------------------------------------------------------------------------------
static UseWrapperMapper &mapperSingleton()
{
  static UseWrapperMapper singleton;
  return singleton;
}

//------------------------------------------------------------------------------
static NativeType *unproxyAudioNative(NativeType *native)
{
  if (!native) return nullptr;
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::RtpReceiver, ::webrtc::AudioRtpReceiver,  native);
}

//------------------------------------------------------------------------------
static NativeType *unproxyVideoNative(NativeType *native)
{
  if (!native) return nullptr;
  return WRAPPER_DEPROXIFY_CLASS(::webrtc::RtpReceiver, ::webrtc::VideoRtpReceiver, native);
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpReceiver::RTCRtpReceiver() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpReceiverPtr wrapper::org::webRtc::RTCRtpReceiver::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webRtc::RTCRtpReceiver>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::RTCRtpReceiver::~RTCRtpReceiver() noexcept
{
  thisWeak_.reset();
  teardownObserver();
  mapperSingleton().remove(native_.get());
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpCapabilitiesPtr wrapper::org::webRtc::RTCRtpReceiver::getCapabilities(String kindStr) noexcept
{
  auto realFactory = UseWebrtcLib::realPeerConnectionFactory();
  ZS_ASSERT(realFactory);
  if (!realFactory) return wrapper::org::webRtc::RTCRtpCapabilitiesPtr();

  auto channelManager = realFactory->channel_manager();

  try {
    auto kind = UseEnum::toNativeMediaType(kindStr);

    switch (kind) {
    case cricket::MEDIA_TYPE_AUDIO: {
      cricket::AudioCodecs cricket_codecs;
      cricket::RtpHeaderExtensions cricket_extensions;
      channelManager->GetSupportedAudioReceiveCodecs(&cricket_codecs);
      channelManager->GetSupportedAudioRtpHeaderExtensions(
        &cricket_extensions);
      return UseRtpCapabilities::toWrapper(ToRtpCapabilitiesWithAsserts(cricket_codecs, cricket_extensions));
    }
    case cricket::MEDIA_TYPE_VIDEO: {
      cricket::VideoCodecs cricket_codecs;
      cricket::RtpHeaderExtensions cricket_extensions;
      channelManager->GetSupportedVideoCodecs(&cricket_codecs);
      channelManager->GetSupportedVideoRtpHeaderExtensions(
        &cricket_extensions);
      return UseRtpCapabilities::toWrapper(ToRtpCapabilitiesWithAsserts(cricket_codecs, cricket_extensions));
    }
    case cricket::MEDIA_TYPE_DATA:
      return UseRtpCapabilities::toWrapper(::webrtc::RtpCapabilities());
    }
  } catch (const ::zsLib::Exceptions::InvalidArgument &) {
  }

  return wrapper::org::webRtc::RTCRtpCapabilitiesPtr();
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::RTCRtpReceiveParametersPtr wrapper::impl::org::webRtc::RTCRtpReceiver::getParameters() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::RTCRtpReceiveParametersPtr();

  return UseRtpReceiveParameters::toWrapper(native_->GetParameters());
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::RTCRtpContributingSourcePtr > > wrapper::impl::org::webRtc::RTCRtpReceiver::getContributingSources() noexcept
{
  typedef shared_ptr< list< wrapper::org::webRtc::RTCRtpContributingSourcePtr > > ReturnType;
  ZS_ASSERT(native_);
  if (!native_) return ReturnType();

  auto result = make_shared< ReturnType::element_type >();

  auto sources = native_->GetSources();
  for (auto iter = sources.begin(); iter != sources.end(); ++iter) {
    auto wrapper = UseRtpContributingSource::toWrapper(*iter);
    if (!wrapper) continue;
    result->push_back(wrapper);
  }
  return result;
}

//------------------------------------------------------------------------------
shared_ptr< list< wrapper::org::webRtc::RTCRtpSynchronizationSourcePtr > > wrapper::impl::org::webRtc::RTCRtpReceiver::getSynchronizationSources() noexcept
{
  typedef shared_ptr< list< wrapper::org::webRtc::RTCRtpSynchronizationSourcePtr > > ReturnType;
  ZS_ASSERT(native_);
  if (!native_) return ReturnType();

  auto result = make_shared< ReturnType::element_type >();

  auto sources = native_->GetSources();
  for (auto iter = sources.begin(); iter != sources.end(); ++iter) {
    auto wrapper = UseRtpSynchronizationSource::toWrapper(*iter);
    if (!wrapper) continue;
    result->push_back(wrapper);
  }
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaStreamTrackPtr wrapper::impl::org::webRtc::RTCRtpReceiver::get_track() noexcept
{
  ZS_ASSERT(native_);
  if (!native_) return wrapper::org::webRtc::MediaStreamTrackPtr();

  return UseMediaStreamTrack::toWrapper(native_->track());
}

//------------------------------------------------------------------------------
void WrapperImplType::setupObserver() noexcept
{
//  if (!native_) return;
//  observer_ = std::make_unique<WebrtcObserver>(thisWeak_.lock(), UseWebrtcLib::delegateQueue());
}

//------------------------------------------------------------------------------
void WrapperImplType::teardownObserver() noexcept
{
//  if (!observer_) return;
//  if (!native_) return;
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeType *native) noexcept
{
  if (!native) return WrapperImplTypePtr();

  NativeType *original = unproxyAudioNative(native);
  original = unproxyVideoNative(original);

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

