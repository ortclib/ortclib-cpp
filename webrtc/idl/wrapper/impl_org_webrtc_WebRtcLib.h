
#pragma once

#include "types.h"

#include "generated/org_webrtc_WebRtcLib.h"

#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "media/base/videocapturerfactory.h"
#include "impl_org_webrtc_post_include.h"

#include <zsLib/Singleton.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct WebRtcLib : public wrapper::org::webrtc::WebRtcLib,
                           public zsLib::ISingletonManagerDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::WebRtcLib, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::WebRtcLib, WrapperImplType);

          typedef rtc::scoped_refptr<::webrtc::PeerConnectionFactoryInterface> PeerConnectionFactoryInterfaceScopedPtr;

          ZS_DECLARE_TYPEDEF_PTR(::cricket::VideoDeviceCapturerFactory, UseVideoDeviceCaptureFacrtory);

          WebRtcLib() noexcept;
          WebRtcLib(const WebRtcLib &) = delete;
          virtual ~WebRtcLib() noexcept;

          WebRtcLibWeakPtr thisWeak_;
          std::atomic_flag setupCalledOnce_{};
          std::atomic_bool setupComplete_ {};
          std::atomic_bool alreadyCleaned_{};
          std::atomic_flag didSetupZsLib_ {};
          std::atomic_flag isTracingStartOrStopping_ {};
          std::atomic_bool isTracing_ {};
          zsLib::Lock lock_;
          PeerConnectionFactoryInterfaceScopedPtr peerConnectionFactory_;
          UseVideoDeviceCaptureFacrtoryPtr videoDeviceCaptureFactory_;
          ::zsLib::Milliseconds ntpServerTime_;

          std::unique_ptr<rtc::Thread> networkThread;
          std::unique_ptr<rtc::Thread> workerThread;
          std::unique_ptr<rtc::Thread> signalingThread;

          // constructor
          static WrapperImplTypePtr create() noexcept;

          // overrides of base class
          virtual void actual_setup() noexcept;
          virtual void actual_setup(wrapper::org::webrtc::EventQueuePtr queue) noexcept;
          virtual void actual_startMediaTracing() noexcept;
          virtual void actual_stopMediaTracing() noexcept;
          virtual bool actual_isMediaTracing() noexcept;
          virtual bool actual_startMediaTrace(String filename) noexcept;
          virtual bool actual_startMediaTrace(
                                             String host,
                                             int port
                                             ) noexcept;

          virtual ::zsLib::Milliseconds actual_get_ntpServerTime() noexcept;
          virtual void actual_set_ntpServerTime(::zsLib::Milliseconds value) noexcept;

          // addition methods needed
          virtual bool actual_checkSetup(bool assert = true) noexcept;
          virtual PeerConnectionFactoryInterfaceScopedPtr actual_peerConnectionFactory() noexcept;
          virtual UseVideoDeviceCaptureFacrtoryPtr actual_videoDeviceCaptureFactory() noexcept;

          //-------------------------------------------------------------------
          //
          // zsLib::ISingletonManagerDelegate
          // 
          void notifySingletonCleanup() noexcept override;

          // public methods
          static WebRtcLibPtr singleton() noexcept;

          // addition methods needed
          static PeerConnectionFactoryInterfaceScopedPtr peerConnectionFactory() noexcept;
          static UseVideoDeviceCaptureFacrtoryPtr videoDeviceCaptureFactory() noexcept;
          static bool checkSetup(bool assert = true) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

