

#ifdef WINUWP

#ifdef __cplusplus_winrt
#include <windows.media.core.h>
#endif //__cplusplus_winrt

#ifdef __has_include
#if __has_include(<winrt/Windows.Media.Core.h>)
#include <winrt/Windows.Media.Core.h>
#endif //__has_include(<winrt/Windows.Media.Core.h>)
#endif //__has_include

#else

#ifdef _WIN32
#endif //_WIN32

#endif //WINUWP

#include "impl_org_webRtc_WebRtcLib.h"
#include "impl_org_webRtc_EventQueue.h"
#include "impl_org_webRtc_helpers.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/peerconnectioninterface.h"
#include "api/test/fakeconstraints.h"
#include "common_video/video_common_winuwp.h"
#include "rtc_base/event_tracer.h"
#include "rtc_base/ssladapter.h"
#include "rtc_base/win32socketinit.h"
#include "third_party/winuwp_h264/winuwp_h264_factory.h"
#include "media/engine/webrtcvideocapturerfactory.h"
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

#include <zsLib/eventing/IHelper.h>

namespace wrapper { namespace impl { namespace org { namespace webRtc { ZS_DECLARE_SUBSYSTEM(wrapper_org_webRtc); } } } }


// borrow from previous class definitions
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::WebRtcLib::WrapperImplType, WrapperImplType);

typedef WrapperImplType::PeerConnectionFactoryInterfaceScopedPtr PeerConnectionFactoryInterfaceScopedPtr;
ZS_DECLARE_TYPEDEF_PTR(WrapperImplType::UseVideoDeviceCaptureFacrtory, UseVideoDeviceCaptureFacrtory);

ZS_DECLARE_TYPEDEF_PTR(::cricket::WebRtcVideoDeviceCapturerFactory, UseWebrtcVideoDeviceCaptureFacrtory);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::EventQueue::WrapperImplType, UseEventQueue);


ZS_DECLARE_TYPEDEF_PTR(zsLib::eventing::IHelper, UseHelper);

//------------------------------------------------------------------------------
WrapperImplType::WebRtcLib() noexcept
{
  ZS_GET_LOG_LEVEL(); // force the subsystem to get referenced before any usage happens
}

//------------------------------------------------------------------------------
wrapper::impl::org::webRtc::WebRtcLib::~WebRtcLib()
{
  thisWeak_.reset();
  notifySingletonCleanup();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::create() noexcept
{
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  return result;
}

//------------------------------------------------------------------------------
void wrapper::org::webRtc::WebRtcLib::setup() noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_setup();
}

//------------------------------------------------------------------------------
void wrapper::org::webRtc::WebRtcLib::setup(wrapper::org::webRtc::EventQueuePtr queue) noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_setup(queue);
}

//------------------------------------------------------------------------------
void wrapper::org::webRtc::WebRtcLib::startMediaTracing() noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_startMediaTracing();
}

//------------------------------------------------------------------------------
void wrapper::org::webRtc::WebRtcLib::stopMediaTracing() noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_stopMediaTracing();
}

//------------------------------------------------------------------------------
bool wrapper::org::webRtc::WebRtcLib::isMediaTracing() noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_isMediaTracing();
}

//------------------------------------------------------------------------------
bool wrapper::org::webRtc::WebRtcLib::startMediaTrace(String filename) noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_startMediaTrace(filename);
}

//------------------------------------------------------------------------------
bool wrapper::org::webRtc::WebRtcLib::startMediaTrace(
  String host,
  int port
  ) noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_startMediaTrace(host, port);
}

//------------------------------------------------------------------------------
::zsLib::Milliseconds wrapper::org::webRtc::WebRtcLib::get_ntpServerTime() noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_get_ntpServerTime();
}

//------------------------------------------------------------------------------
void wrapper::org::webRtc::WebRtcLib::set_ntpServerTime(::zsLib::Milliseconds value) noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_set_ntpServerTime(value);
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_setup() noexcept
{
  actual_setup(wrapper::org::webRtc::EventQueuePtr());
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_setup(wrapper::org::webRtc::EventQueuePtr queue) noexcept
{
  // prevent multiple setups being called simulatuously
  if (setupCalledOnce_.test_and_set()) return;

#pragma ZS_BUILD_NOTE("TODO","(robin) prepare webrtc peer connection factory here")

#ifdef WINUWP

  // Setup for WinWUP...

#if defined(__cplusplus_winrt) && defined(CPPWINRT_VERSION)

#else

#ifdef __cplusplus_winrt
  // setup when ONLY WinUWP CX is defined...
#endif // __cplusplus_winrt

#ifdef CPPWINRT_VERSION
  // setup when ONLY CppWinRT is defined...
#endif //CPPWINRT_VERSION

#endif //defined(__cplusplus_winrt) && defined(CPPWINRT_VERSION)

#ifdef __cplusplus_winrt
  // Setup if WinUWP CX is defined...
  {
    auto nativeCx = UseEventQueue::toNative_cx(queue);
    if ((nativeCx) && (!didSetupZsLib_.test_and_set())) {
      UseHelper::setup(nativeCx);
    }
  }
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION
  {
    // Setup if WinUWP CppWinRT is defined...
    auto nativeCppWinrt = UseEventQueue::toNative_winrt(queue);
    if ((nativeCppWinrt) && (!didSetupZsLib_.test_and_set())) {
      UseHelper::setup(nativeCppWinrt);
    }
  }
#endif //CPPWINRT_VERSION

#else

  // setup for non WinWUP targets...

#ifdef _WIN32

  // setup specific to Win32...

#endif //_WIN32

#endif //WINUWP

  if (!didSetupZsLib_.test_and_set()) {
    UseHelper::setup();
  }

#ifdef WINUWP

  // Setup for WinWUP...

#if (defined(__cplusplus_winrt) && defined(CPPWINRT_VERSION)) || defined(__cplusplus_winrt)

  Windows::UI::Core::CoreDispatcher^ dispatcher{};

#if defined(CPPWINRT_VERSION)
  if (!dispatcher) {
    auto nativeCppWinrt = UseEventQueue::toNative_winrt(queue);
    if (nativeCppWinrt) {
      dispatcher = WRAPPER_TO_CX(Windows::UI::Core::CoreDispatcher, nativeCppWinrt);
    }
  }
#endif //defined(CPPWINRT_VERSION)

  if (!dispatcher) {
    dispatcher = UseEventQueue::toNative_cx(queue);
  }

  // on WinUWP a dispatcher is not optional
  ZS_ASSERT(dispatcher);

  ::webrtc::VideoCommonWinUWP::SetCoreDispatcher(dispatcher);

#else

#error cppwinrt requires CX consume runtime support to be enabled (for now)

#endif //(defined(__cplusplus_winrt) && defined(CPPWINRT_VERSION)) || defined(__cplusplus_winrt)

#else //WINUWP

#endif //WINUWP

  rtc::EnsureWinsockInit();
  rtc::InitializeSSL([](void *) { return false; }); // no custom verifier of SSL

  networkThread = rtc::Thread::CreateWithSocketServer();
  networkThread->Start();

  workerThread = rtc::Thread::Create();
  workerThread->Start();

  signalingThread = rtc::Thread::Create();
  signalingThread->Start();


  auto encoderFactory = new ::webrtc::WinUWPH264EncoderFactory();
  auto decoderFactory = new ::webrtc::WinUWPH264DecoderFactory();

  peerConnectionFactory_ = ::webrtc::CreatePeerConnectionFactory(
    networkThread.get(),
    workerThread.get(),
    signalingThread.get(),
    nullptr,
    ::webrtc::CreateBuiltinAudioEncoderFactory(),
    ::webrtc::CreateBuiltinAudioDecoderFactory(),
    encoderFactory,
    decoderFactory
  );

#ifdef WINUWP
  videoDeviceCaptureFactory_ = make_shared<::cricket::WebRtcVideoDeviceCapturerFactory>();
#else
#error PLATFORM REQUIRES FACTORY
#endif //WEBRTC

  rtc::tracing::SetupInternalTracer();

  if (setupComplete_.exchange(true)) {
    ZS_ASSERT_FAIL("already setup webrtc wrapper");
    return;
  }
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_startMediaTracing() noexcept
{
  if (!actual_checkSetup()) return;

  // prevent multiple calls to start or stop simultaneously
  if (isTracingStartOrStopping_.test_and_set()) return;

  if (isTracing_.exchange(false)) {
    isTracingStartOrStopping_.clear();
    return;
  }

  //rtc::tracing::StartInternalCapture();
#pragma ZS_BUILD_NOTE("TODO","(mosa) actual_startMediaTrace no options")

  isTracingStartOrStopping_.clear();
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_stopMediaTracing() noexcept
{
  if (!actual_checkSetup()) return;

  // prevent multiple calls to start or stop simultaneously
  if (isTracingStartOrStopping_.test_and_set()) return;

  if (isTracing_.exchange(false)) {
    isTracingStartOrStopping_.clear();
    return;
  }

  rtc::tracing::StopInternalCapture();

  isTracingStartOrStopping_.clear();
}

//------------------------------------------------------------------------------
bool WrapperImplType::actual_isMediaTracing() noexcept
{
  if (!actual_checkSetup(false)) return false;
  return isTracing_;
}

//------------------------------------------------------------------------------
bool WrapperImplType::actual_startMediaTrace(String filename) noexcept
{
  if (!actual_checkSetup()) return false;

  // prevent multiple calls to start or stop simultaneously
  if (isTracingStartOrStopping_.test_and_set()) return false;

  ZS_ASSERT(!isTracing_);
  if (isTracing_.exchange(true)) {
    isTracingStartOrStopping_.clear();
    return false;
  }

  rtc::tracing::StartInternalCapture(filename.c_str());

  isTracingStartOrStopping_.clear();
  return false;
}

//------------------------------------------------------------------------------
bool WrapperImplType::actual_startMediaTrace(
                                            String host,
                                            int port
                                            ) noexcept
{
  if (!actual_checkSetup()) return false;

  // prevent multiple calls to start or stop simultaneously
  if (isTracingStartOrStopping_.test_and_set()) return false;

  ZS_ASSERT(!isTracing_);
  if (isTracing_.exchange(true)) {
    isTracingStartOrStopping_.clear();
    return false;
  }

#pragma ZS_BUILD_NOTE("TODO","(mosa) actual_startMediaTrace host/port")

  isTracingStartOrStopping_.clear();
  return false;
}


//------------------------------------------------------------------------------
::zsLib::Milliseconds WrapperImplType::actual_get_ntpServerTime() noexcept
{
  if (!actual_checkSetup()) return ::zsLib::Milliseconds();
  zsLib::AutoLock lock(lock_);
  return ntpServerTime_;
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_set_ntpServerTime(::zsLib::Milliseconds value) noexcept
{
  if (!actual_checkSetup()) return;

  {
    zsLib::AutoLock lock(lock_);
    ntpServerTime_ = value;
  }

#pragma ZS_BUILD_NOTE("TODO","(mosa) set the NTP time from the server inside webrtc engine here")

}

//------------------------------------------------------------------------------
bool WrapperImplType::actual_checkSetup(bool assert) noexcept
{
  if ((assert) && ((!setupComplete_) && (!alreadyCleaned_))) {
    ZS_ASSERT_FAIL("Setup was never called on wrapper");
  }
  return setupComplete_ && (!alreadyCleaned_);
}

//------------------------------------------------------------------------------
PeerConnectionFactoryInterfaceScopedPtr WrapperImplType::actual_peerConnectionFactory() noexcept
{
  if (!actual_checkSetup()) return PeerConnectionFactoryInterfaceScopedPtr();

  return peerConnectionFactory_;
}


//------------------------------------------------------------------------------
UseVideoDeviceCaptureFacrtoryPtr WrapperImplType::actual_videoDeviceCaptureFactory() noexcept
{
  if (!actual_checkSetup()) return UseVideoDeviceCaptureFacrtoryPtr();

  return videoDeviceCaptureFactory_;
}


//------------------------------------------------------------------------------
void WrapperImplType::notifySingletonCleanup() noexcept
{
  // prevent clean-up twice
  if (!actual_checkSetup()) return;
  if (alreadyCleaned_.exchange(true)) return;

  // Add what is needed to shutdown webrtc...

#pragma ZS_BUILD_NOTE("TODO","(mosa) shutdown webrtc engine here")

  rtc::tracing::ShutdownInternalTracer();

  // reset the factory (cannot be used anymore)...
  peerConnectionFactory_ = PeerConnectionFactoryInterfaceScopedPtr();
  videoDeviceCaptureFactory_.reset();

#pragma ZS_BUILD_NOTE("TODO","(mosa) shutdown threads need something more?")

  networkThread.reset();
  workerThread.reset();
  signalingThread.reset();

  rtc::CleanupSSL();
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::singleton() noexcept
{
  static zsLib::SingletonLazySharedPtr<WrapperImplType> singleton(WrapperImplType::create());

  auto result = singleton.singleton();

  static zsLib::SingletonManager::Register registerSingleton("org.webrtc.WebRtcLib", result);

  // prevent crashes when accessing singleton during shutdown
  if (!result) {
    class BogusSingleton : public WrapperImplType {

    private:

      void actual_setup() noexcept final {}
      void actual_setup(wrapper::org::webRtc::EventQueuePtr queue) noexcept final {}
      void actual_startMediaTracing() noexcept final {}
      void actual_stopMediaTracing() noexcept final {}
      bool actual_isMediaTracing() noexcept final { return false; }
      bool actual_startMediaTrace(String) noexcept final { return false; }
      bool actual_startMediaTrace(
                                 String,
                                 int
                                 ) noexcept final { return false; }

      ::zsLib::Milliseconds actual_get_ntpServerTime() noexcept final { return zsLib::Milliseconds(); }
      void actual_set_ntpServerTime(::zsLib::Milliseconds) noexcept final {}

      bool actual_checkSetup(bool) noexcept final { return false; }

      //-----------------------------------------------------------------------
      PeerConnectionFactoryInterfaceScopedPtr actual_peerConnectionFactory() noexcept override
      {
        ZS_ASSERT_FAIL("why is the factory needed during shutdown?");
        // no way around this one with a bogus factory...
        return PeerConnectionFactoryInterfaceScopedPtr();
      }

      //-----------------------------------------------------------------------
      UseVideoDeviceCaptureFacrtoryPtr actual_videoDeviceCaptureFactory() noexcept override
      {
        ZS_ASSERT_FAIL("why is the factory needed during shutdown?");
        // no way around this one with a bogus factory...
        return UseVideoDeviceCaptureFacrtoryPtr();
      }

      void notifySingletonCleanup() noexcept final {}
    };
    return make_shared<BogusSingleton>();
  }
  return result;
}

//------------------------------------------------------------------------------
bool WrapperImplType::checkSetup(bool assert) noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_checkSetup(assert);
}


//------------------------------------------------------------------------------
PeerConnectionFactoryInterfaceScopedPtr WrapperImplType::peerConnectionFactory() noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_peerConnectionFactory();
}

//------------------------------------------------------------------------------
UseVideoDeviceCaptureFacrtoryPtr WrapperImplType::videoDeviceCaptureFactory() noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_videoDeviceCaptureFactory();
}
