

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

#include "impl_org_webrtc_WebRtcLib.h"
#include "impl_org_webrtc_EventQueue.h"

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

// borrow from previous class definitions
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::WebRtcLib::WrapperType, WrapperType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::WebRtcLib::WrapperImplType, WrapperImplType);

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::EventQueue::WrapperImplType, UseEventQueue);

typedef wrapper::impl::org::webrtc::WebRtcLib::PeerConnectionFactoryInterfaceScopedPtr PeerConnectionFactoryInterfaceScopedPtr;

ZS_DECLARE_TYPEDEF_PTR(zsLib::eventing::IHelper, UseHelper);

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::WebRtcLib::~WebRtcLib()
{
  thisWeak_.reset();
  notifySingletonCleanup();
}

//------------------------------------------------------------------------------
void wrapper::org::webrtc::WebRtcLib::setup() noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_setup();
}

//------------------------------------------------------------------------------
void wrapper::org::webrtc::WebRtcLib::setup(wrapper::org::webrtc::EventQueuePtr queue) noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_setup(queue);
}

//------------------------------------------------------------------------------
void wrapper::org::webrtc::WebRtcLib::startMediaTracing() noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_startMediaTracing();
}

//------------------------------------------------------------------------------
void wrapper::org::webrtc::WebRtcLib::stopMediaTracing() noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_stopMediaTracing();
}

//------------------------------------------------------------------------------
bool wrapper::org::webrtc::WebRtcLib::isMediaTracing() noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_isMediaTracing();
}

//------------------------------------------------------------------------------
bool wrapper::org::webrtc::WebRtcLib::saveMediaTrace(String filename) noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_saveMediaTrace(filename);
}

//------------------------------------------------------------------------------
bool wrapper::org::webrtc::WebRtcLib::saveMediaTrace(
  String host,
  int port
  ) noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_saveMediaTrace(host, port);
}

//------------------------------------------------------------------------------
::zsLib::Milliseconds wrapper::org::webrtc::WebRtcLib::get_ntpServerTime() noexcept
{
  auto singleton = WrapperImplType::singleton();
  return singleton->actual_get_ntpServerTime();
}

//------------------------------------------------------------------------------
void wrapper::org::webrtc::WebRtcLib::set_ntpServerTime(::zsLib::Milliseconds value) noexcept
{
  auto singleton = WrapperImplType::singleton();
  singleton->actual_set_ntpServerTime(value);
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_setup() noexcept
{
  actual_setup(wrapper::org::webrtc::EventQueuePtr());
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_setup(wrapper::org::webrtc::EventQueuePtr queue) noexcept
{
  if (alreadySetup_.exchange(true)) {
    ZS_ASSERT_FAIL("already setup webrtc wrapper");
    return;
  }

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
  auto nativeCx = UseEventQueue::toNative_cx(queue);
  if ((nativeCx) && (!didSetupZsLib_.test_and_set())) {
    UseHelper::setup(nativeCx);
  }
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION
  // Setup if WinUWP CppWinRT is defined...
  auto nativeCppWinrt = UseEventQueue::toNative_winrt(queue);
  if ((nativeCppWinrt) && (!didSetupZsLib_.test_and_set())) {
    UseHelper::setup(nativeCppWinrt);
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
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_startMediaTracing() noexcept
{
  if (!actual_checkSetup()) return;

#pragma ZS_BUILD_NOTE("TODO","(mosa) actual_startMediaTracing")
}

//------------------------------------------------------------------------------
void WrapperImplType::actual_stopMediaTracing() noexcept
{
  if (!actual_checkSetup()) return;

#pragma ZS_BUILD_NOTE("TODO","(mosa) actual_stopMediaTracing")
}

//------------------------------------------------------------------------------
bool WrapperImplType::actual_isMediaTracing() noexcept
{
  if (!actual_checkSetup(false)) return false;

#pragma ZS_BUILD_NOTE("TODO","(mosa) actual_isMediaTracing")
  return false;
}

//------------------------------------------------------------------------------
bool WrapperImplType::actual_saveMediaTrace(String filename) noexcept
{
  if (!actual_checkSetup()) return false;

#pragma ZS_BUILD_NOTE("TODO","(mosa) actual_saveMediaTrace")
  return false;
}

//------------------------------------------------------------------------------
bool WrapperImplType::actual_saveMediaTrace(
                                            String host,
                                            int port
                                            ) noexcept
{
  if (!actual_checkSetup()) return false;

#pragma ZS_BUILD_NOTE("TODO","(mosa) actual_saveMediaTrace")
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
  zsLib::AutoLock lock(lock_);
  ntpServerTime_ = value;

#pragma ZS_BUILD_NOTE("TODO","(mosa) set the NTP time from the server inside webrtc engine here")

}

//------------------------------------------------------------------------------
bool WrapperImplType::actual_checkSetup(bool assert) noexcept
{
  if ((assert) && (!alreadySetup_)) {
    ZS_ASSERT_FAIL("Setup was never called on wrapper");
  }
  return alreadySetup_;
}
//------------------------------------------------------------------------------
PeerConnectionFactoryInterfaceScopedPtr WrapperImplType::actual_peerConnectionFactory() noexcept
{
  if (!actual_checkSetup()) return PeerConnectionFactoryInterfaceScopedPtr();

  zsLib::AutoLock lock(lock_);
  return peerConnectionFactory_;
}


//------------------------------------------------------------------------------
void WrapperImplType::notifySingletonCleanup() noexcept
{

  // prevent clean-up twice
  if (alreadyCleaned_.test_and_set()) return;
  if (!actual_checkSetup()) return;

#pragma ZS_BUILD_NOTE("TODO","This is the better spot to clean up webrtc then the destructor")

  // Add what is needed to shutdown webrtc...

  // reset the factory (cannot be used anymore)...
  peerConnectionFactory_ = PeerConnectionFactoryInterfaceScopedPtr();
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
      void actual_setup(wrapper::org::webrtc::EventQueuePtr queue) noexcept final {}
      void actual_startMediaTracing() noexcept final {}
      void actual_stopMediaTracing() noexcept final {}
      bool actual_isMediaTracing() noexcept final { return false; }
      bool actual_saveMediaTrace(String) noexcept final { return false; }
      bool actual_saveMediaTrace(
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
