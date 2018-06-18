/*

 Copyright (c) 2014, Hookflash Inc.
 Copyright (c) 2017, Optical Tone Ltd.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/ortc.events.jman.h>
#include <ortc/internal/ortc.stats.events.h>
#include <ortc/internal/ortc.stats.events.jman.h>
#include <ortc/internal/ortc_MediaEngine.h>

#include <ortc/services/IHelper.h>
#include <ortc/services/ILogger.h>

#include <zsLib/IMessageQueueManager.h>
#include <zsLib/ISettings.h>
#include <zsLib/Log.h>
#include <zsLib/Socket.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::ILogger, UseServicesLogger);
  ZS_DECLARE_USING_PTR(zsLib, ISettings);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(ORTCSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(zsLib::IMessageQueueManager, UseMessageQueueManager);

    void initSubsystems() noexcept;
    void installORTCSettingsDefaults() noexcept;
    void installCertificateSettingsDefaults() noexcept;
    void installDataChannelSettingsDefaults() noexcept;
    void installDTMFSenderSettingsDefaults() noexcept;
    void installDTLSTransportSettingsDefaults() noexcept;
    void installICEGathererSettingsDefaults() noexcept;
    void installICETransportSettingsDefaults() noexcept;
    void installIdentitySettingsDefaults() noexcept;
    void installMediaDevicesSettingsDefaults() noexcept;
    void installMediaStreamTrackSettingsDefaults() noexcept;
    void installRTPListenerSettingsDefaults() noexcept;
    void installMediaChannelTraceHelperDefaults() noexcept;
    void installMediaDeviceCaptureAudioSettingsDefaults() noexcept;
    void installMediaDeviceCaptureVideoSettingsDefaults() noexcept;
    void installRTPDecoderAudioSettingsDefaults() noexcept;
    void installRTPDecoderVideoSettingsDefaults() noexcept;
    void installRTPEncoderAudioSettingsDefaults() noexcept;
    void installRTPEncoderVideoSettingsDefaults() noexcept;
    void installMediaEngineSettingsDefaults() noexcept;
    void installRTPReceiverSettingsDefaults() noexcept;
    void installRTPReceiverChannelSettingsDefaults() noexcept;
    void installRTPSenderSettingsDefaults() noexcept;
    void installRTPSenderChannelSettingsDefaults() noexcept;
    void installStatsReportSettingsDefaults() noexcept;
    void installSCTPTransportSettingsDefaults() noexcept;
    void installSCTPTransportListenerSettingsDefaults() noexcept;
    void installSRTPTransportSettingsDefaults() noexcept;
    void installSRTPSDESTransportSettingsDefaults() noexcept;

    //-------------------------------------------------------------------------
    static void installAllDefaults()noexcept
    {
      installORTCSettingsDefaults();
      installCertificateSettingsDefaults();
      installDataChannelSettingsDefaults();
      installDTMFSenderSettingsDefaults();
      installDTLSTransportSettingsDefaults();
      installICEGathererSettingsDefaults();
      installICETransportSettingsDefaults();
      installIdentitySettingsDefaults();
      installMediaDevicesSettingsDefaults();
      installMediaChannelTraceHelperDefaults();
      installMediaStreamTrackSettingsDefaults();
      installRTPListenerSettingsDefaults();
      installMediaDeviceCaptureAudioSettingsDefaults();
      installMediaDeviceCaptureVideoSettingsDefaults();
      installRTPDecoderAudioSettingsDefaults();
      installRTPDecoderVideoSettingsDefaults();
      installRTPEncoderAudioSettingsDefaults();
      installRTPEncoderVideoSettingsDefaults();
      installMediaEngineSettingsDefaults();
      installRTPReceiverSettingsDefaults();
      installRTPReceiverChannelSettingsDefaults();
      installRTPSenderSettingsDefaults();
      installRTPSenderChannelSettingsDefaults();
      installStatsReportSettingsDefaults();
      installSCTPTransportSettingsDefaults();
      installSCTPTransportListenerSettingsDefaults();
      installSRTPTransportSettingsDefaults();
      installSRTPSDESTransportSettingsDefaults();
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ORTCSettingsDefaults
    //

    class ORTCSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~ORTCSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static ORTCSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<ORTCSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static ORTCSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<ORTCSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
        ISettings::setString(ORTC_SETTING_ORTC_QUEUE_MAIN_THREAD_NAME, zsLib::toString(zsLib::ThreadPriority_NormalPriority));
        ISettings::setString(ORTC_SETTING_ORTC_QUEUE_PIPELINE_THREAD_NAME, zsLib::toString(zsLib::ThreadPriority_HighPriority));
        ISettings::setString(ORTC_SETTING_ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME, "normal");
        ISettings::setString(ORTC_SETTING_ORTC_QUEUE_CERTIFICATE_GENERATION_NAME, "low");

        for (size_t index = 0; index < ORTC_QUEUE_TOTAL_MEDIA_DEVICE_THREADS; ++index) {
          ISettings::setString((String(ORTC_SETTING_ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME) + string(index)).c_str(), "higest");
        }
        for (size_t index = 0; index < ORTC_QUEUE_TOTAL_RTP_THREADS; ++index) {
          ISettings::setString((String(ORTC_SETTING_ORTC_QUEUE_RTP_THREAD_NAME) + string(index)).c_str(), "higest");
        }
        ISettings::setString(ZSLIB_SETTING_SOCKET_MONITOR_THREAD_PRIORITY, zsLib::toString(zsLib::ThreadPriority_HighPriority));
      }
      
    };

    //-------------------------------------------------------------------------
    void installORTCSettingsDefaults() noexcept
    {
      ORTCSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IORTCForInternal
    //

    //-------------------------------------------------------------------------
    void IORTCForInternal::overrideQueueDelegate(IMessageQueuePtr queue) noexcept
    {
      return (ORTC::singleton())->overrideQueueDelegate(queue);
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueDelegate() noexcept
    {
      return (ORTC::singleton())->queueDelegate();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueORTC() noexcept
    {
      return (ORTC::singleton())->queueORTC();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueORTCPipeline() noexcept
    {
      return (ORTC::singleton())->queueORTCPipeline();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueBlockingMediaStartStopThread() noexcept
    {
      return (ORTC::singleton())->queueBlockingMediaStartStopThread();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueMediaDevices() noexcept
    {
      return (ORTC::singleton())->queueMediaDevices();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueRTP() noexcept
    {
      return (ORTC::singleton())->queueRTP();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueCertificateGeneration() noexcept
    {
      return (ORTC::singleton())->queueCertificateGeneration();
    }

    //-------------------------------------------------------------------------
    Optional<Log::Level> IORTCForInternal::webrtcLogLevel() noexcept
    {
      return (ORTC::singleton())->webrtcLogLevel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ORTC
    //

    //-------------------------------------------------------------------------
    ORTC::ORTC(const make_private &) noexcept :
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      ZS_EVENTING_EXCLUSIVE(OrtcLib);
      ZS_EVENTING_REGISTER(OrtcLib);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLibStatsReport);
      ZS_EVENTING_REGISTER(OrtcLibStatsReport);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLib);
      ZS_EVENTING_0(x, i, Detail, OrtcCreate, ol, Ortc, Start);
      ZS_EVENTING_EXCLUSIVE(x);

      initSubsystems();
      ZS_LOG_DETAIL(log("created"));
    }

    //-------------------------------------------------------------------------
    ORTC::~ORTC() noexcept
    {
      mThisWeak.reset();
      ZS_LOG_DETAIL(log("destroyed"));

      ZS_EVENTING_EXCLUSIVE(OrtcLib);
      ZS_EVENTING_0(x, i, Detail, OrtcDestroy, ol, Ortc, Stop);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLibStatsReport);
      ZS_EVENTING_1(x, i, Detail, OrtcStatsReportCommand, ols, Stats, Start, string, command_name, "stop");
      ZS_EVENTING_ASSIGN_VALUE(OrtcStatsReportCommand, 106);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLib);
      ZS_EVENTING_UNREGISTER(OrtcLib);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLibStatsReport);
      ZS_EVENTING_UNREGISTER(OrtcLibStatsReport);
      ZS_EVENTING_EXCLUSIVE(x);
    }

    //-------------------------------------------------------------------------
    void ORTC::init() noexcept
    {
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::convert(IORTCPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(ORTC, object);
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::create() noexcept
    {
      ORTCPtr pThis(make_shared<ORTC>(make_private{}));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ORTC => IORTC
    //

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::singleton() noexcept
    {
      AutoRecursiveLock lock(*UseServicesHelper::getGlobalLock());
      static SingletonLazySharedPtr<ORTC> singleton(ORTC::create());
      ORTCPtr result = singleton.singleton();
      if (!result) {
        ZS_LOG_WARNING(Detail, slog("singleton gone"));
      }
      return result;
    }

    //-------------------------------------------------------------------------
    void ORTC::setup(IMessageQueuePtr defaultDelegateMessageQueue) noexcept
    {
      {
        AutoRecursiveLock lock(mLock);

        if (defaultDelegateMessageQueue) {
          mDelegateQueue = defaultDelegateMessageQueue;
        }
      }

      UseServicesHelper::setup();
      internalSetup();
    }

#ifdef WINUWP
    //-------------------------------------------------------------------------
    void ORTC::setup(Windows::UI::Core::CoreDispatcher ^dispatcher)
    {
      UseServicesHelper::setup(dispatcher);
      internalSetup();
    }
#endif //WINUWP

    //-------------------------------------------------------------------------
    Milliseconds ORTC::ntpServerTime() const noexcept
    {
      AutoRecursiveLock(*this);
      return mNTPServerTime;
    }

    //-------------------------------------------------------------------------
    void ORTC::ntpServerTime(const Milliseconds &value) noexcept
    {
      {
        AutoRecursiveLock(*this);
        mNTPServerTime = value;
      }
      IMediaEngineForORTC::ntpServerTime(value);
    }

    //-------------------------------------------------------------------------
    void ORTC::defaultWebrtcLogLevel(Log::Level level) noexcept
    {
      AutoRecursiveLock(*this);
      mDefaultWebRTCLogLevel = level;
    }

    //-------------------------------------------------------------------------
    void ORTC::webrtcLogLevel(Log::Level level) noexcept
    {
      AutoRecursiveLock(*this);
      mWebRTCLogLevel = level;
    }

    //-------------------------------------------------------------------------
    void ORTC::startMediaTracing() noexcept
    {
      IMediaEngineForORTC::startMediaTracing();
    }

    //-------------------------------------------------------------------------
    void ORTC::stopMediaTracing() noexcept
    {
      IMediaEngineForORTC::stopMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool ORTC::isMediaTracing() noexcept
    {
      return IMediaEngineForORTC::isMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool ORTC::saveMediaTrace(String filename) noexcept
    {
      return IMediaEngineForORTC::saveMediaTrace(filename);
    }

    //-------------------------------------------------------------------------
    bool ORTC::saveMediaTrace(String host, int port) noexcept
    {
      return IMediaEngineForORTC::saveMediaTrace(host, port);
    }

    //-------------------------------------------------------------------------
    bool ORTC::isMRPInstalled() noexcept
    {
#pragma ZS_BUILD_NOTE("TODO","Implment MRP installed")
//      return IRTPMediaEngineForORTC::isMRPInstalled();
      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Stack => IORTCForInternal
    //

    //-------------------------------------------------------------------------
    void ORTC::overrideQueueDelegate(IMessageQueuePtr queue) noexcept
    {
      AutoRecursiveLock lock(*this);
      mDelegateQueue = queue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueDelegate() const noexcept
    {
      AutoRecursiveLock lock(*this);
      if (!mDelegateQueue) {
        mDelegateQueue = UseMessageQueueManager::getMessageQueueForGUIThread();
      }
      return mDelegateQueue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueORTC() const noexcept
    {
      AutoRecursiveLock lock(*this);
      class Once {
      public:
        Once() {
          zsLib::IMessageQueueManager::registerMessageQueueThreadPriority(ORTC_QUEUE_MAIN_THREAD_NAME, zsLib::threadPriorityFromString(ISettings::getString(ORTC_SETTING_ORTC_QUEUE_MAIN_THREAD_NAME)));
        }
      };
      static Once once;
      return UseMessageQueueManager::getThreadPoolQueue(ORTC_QUEUE_MAIN_THREAD_NAME);
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueORTCPipeline() const noexcept
    {
      AutoRecursiveLock lock(*this);
      class Once {
      public:
        Once() {
          zsLib::IMessageQueueManager::registerMessageQueueThreadPriority(ORTC_QUEUE_PIPELINE_THREAD_NAME, zsLib::threadPriorityFromString(ISettings::getString(ORTC_SETTING_ORTC_QUEUE_PIPELINE_THREAD_NAME)));
        }
      };
      static Once once;
      return UseMessageQueueManager::getThreadPoolQueue(ORTC_QUEUE_PIPELINE_THREAD_NAME);
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueBlockingMediaStartStopThread() const noexcept
    {
      AutoRecursiveLock lock(*this);
      if (!mBlockingMediaStartStopThread) {
        mBlockingMediaStartStopThread = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME);
      }
      return mBlockingMediaStartStopThread;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueMediaDevices() const noexcept
    {
      AutoRecursiveLock lock(*this);

      size_t index = mNextMediaQueueThread % ORTC_QUEUE_TOTAL_MEDIA_DEVICE_THREADS;

      if (!mMediaDeviceQueues[index]) {
        mMediaDeviceQueues[index] = UseMessageQueueManager::getMessageQueue((String(ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME) + string(index)).c_str());
      }

      ++mNextMediaQueueThread;
      return mMediaDeviceQueues[index];
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueRTP() const noexcept
    {
      AutoRecursiveLock lock(*this);

      size_t index = mNextRTPQueueThread % ORTC_QUEUE_TOTAL_RTP_THREADS;

      if (!mRTPQueues[index]) {
        mRTPQueues[index] = UseMessageQueueManager::getMessageQueue((String(ORTC_QUEUE_RTP_THREAD_NAME) + string(index)).c_str());
      }

      ++mNextRTPQueueThread;
      return mRTPQueues[index];
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueCertificateGeneration() const noexcept
    {
      AutoRecursiveLock lock(*this);
      if (!mCertificateGeneration) {
        mCertificateGeneration = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_CERTIFICATE_GENERATION_NAME);
      }
      return mCertificateGeneration;
    }

    //-------------------------------------------------------------------------
    Optional<Log::Level> ORTC::webrtcLogLevel() const noexcept
    {
      AutoRecursiveLock lock(*this);
      if (mWebRTCLogLevel.hasValue()) return mWebRTCLogLevel;
      return mDefaultWebRTCLogLevel;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Stack => (internal)
    //

    //-------------------------------------------------------------------------
    Log::Params ORTC::log(const char *message) const noexcept
    {
      ElementPtr objectEl = Element::create("ortc::ORTC");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ORTC::slog(const char *message) noexcept
    {
      ElementPtr objectEl = Element::create("ortc::ORTC");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    void ORTC::internalSetup() noexcept
    {
      installAllDefaults();
      ISettings::applyDefaults();

      UseMessageQueueManager::registerMessageQueueThreadPriority(ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME, zsLib::threadPriorityFromString(ISettings::getString(ORTC_SETTING_ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME)));
      UseMessageQueueManager::registerMessageQueueThreadPriority(ORTC_QUEUE_CERTIFICATE_GENERATION_NAME, zsLib::threadPriorityFromString(ISettings::getString(ORTC_SETTING_ORTC_QUEUE_CERTIFICATE_GENERATION_NAME)));

      for (size_t index = 0; index < ORTC_QUEUE_TOTAL_MEDIA_DEVICE_THREADS; ++index) {
        UseMessageQueueManager::registerMessageQueueThreadPriority(
          (String(ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME) + string(index)).c_str(),
          zsLib::threadPriorityFromString(ISettings::getString((String(ORTC_SETTING_ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME) + string(index)).c_str()))
        );
      }
      for (size_t index = 0; index < ORTC_QUEUE_TOTAL_RTP_THREADS; ++index) {
        UseMessageQueueManager::registerMessageQueueThreadPriority(
          (String(ORTC_QUEUE_RTP_THREAD_NAME) + string(index)).c_str(),
          zsLib::threadPriorityFromString(ISettings::getString((String(ORTC_SETTING_ORTC_QUEUE_RTP_THREAD_NAME) + string(index)).c_str()))
        );
      }
    }

  } // namespace internal

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IORTC
  //

  //---------------------------------------------------------------------------
  void IORTC::setup(IMessageQueuePtr defaultDelegateMessageQueue) noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->setup(defaultDelegateMessageQueue);
  }

  //---------------------------------------------------------------------------
#ifdef WINUWP
  void IORTC::setup(Windows::UI::Core::CoreDispatcher ^dispatcher) noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->setup(dispatcher);
  }
#endif //WINUWP

  //-------------------------------------------------------------------------
  Milliseconds IORTC::ntpServerTime() noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return Milliseconds();
    return singleton->ntpServerTime();
  }

  //-------------------------------------------------------------------------
  void IORTC::ntpServerTime(const Milliseconds &value) noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->ntpServerTime(value);
  }

  //-------------------------------------------------------------------------
  void IORTC::setDefaultLogLevel(Log::Level level) noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (singleton) {
      singleton->defaultWebrtcLogLevel(level);
    }

    UseServicesLogger::setLogLevel(level);
  }

  //-------------------------------------------------------------------------
  void IORTC::setLogLevel(const char *componenet, Log::Level level) noexcept
  {
    String str(componenet);

    if (0 == str.compare("ortclib_webrtc")) {
      auto singleton = internal::ORTC::singleton();
      if (singleton) {
        singleton->webrtcLogLevel(level);
      }
    }

    UseServicesLogger::setLogLevel(componenet, level);
  }

  //-------------------------------------------------------------------------
  void IORTC::setDefaultEventingLevel(Log::Level level) noexcept
  {
    UseServicesLogger::setEventingLevel(level);
  }

  //-------------------------------------------------------------------------
  void IORTC::setEventingLevel(const char *componenet, Log::Level level) noexcept
  {
    UseServicesLogger::setEventingLevel(componenet, level);
  }

  //-------------------------------------------------------------------------
  void IORTC::startMediaTracing() noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->startMediaTracing();
  }

  //-------------------------------------------------------------------------
  void IORTC::stopMediaTracing() noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->stopMediaTracing();
  }

  //-------------------------------------------------------------------------
  bool IORTC::isMediaTracing() noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->isMediaTracing();
  }

  //-------------------------------------------------------------------------
  bool IORTC::saveMediaTrace(String filename) noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->saveMediaTrace(filename);
  }

  //-------------------------------------------------------------------------
  bool IORTC::saveMediaTrace(String host, int port) noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->saveMediaTrace(host, port);
  }

  //-------------------------------------------------------------------------
  bool IORTC::isMRPInstalled() noexcept
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->isMRPInstalled();
  }

}
