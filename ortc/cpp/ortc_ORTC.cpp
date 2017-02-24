/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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
#include <ortc/internal/ortc.stats.events.h>
#include <ortc/internal/ortc_RTPMediaEngine.h>

#include <ortc/services/IHelper.h>
#include <ortc/services/ILogger.h>

#include <zsLib/IMessageQueueManager.h>
#include <zsLib/ISettings.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::ILogger, UseServicesLogger);
  ZS_DECLARE_USING_PTR(zsLib, ISettings);

  namespace internal
  {
    ZS_DECLARE_TYPEDEF_PTR(zsLib::IMessageQueueManager, UseMessageQueueManager)

    void initSubsystems();
    void installCertificateSettingsDefaults();
    void installDataChannelSettingsDefaults();
    void installDTMFSenderSettingsDefaults();
    void installDTLSTransportSettingsDefaults();
    void installICEGathererSettingsDefaults();
    void installICETransportSettingsDefaults();
    void installIdentitySettingsDefaults();
    void installMediaDevicesSettingsDefaults();
    void installMediaStreamTrackSettingsDefaults();
    void installRTPListenerSettingsDefaults();
    void installRTPMediaEngineSettingsDefaults();
    void installRTPReceiverSettingsDefaults();
    void installRTPReceiverChannelAudioSettingsDefaults();
    void installRTPReceiverChannelSettingsDefaults();
    void installRTPReceiverChannelVideoSettingsDefaults();
    void installRTPSenderSettingsDefaults();
    void installRTPSenderChannelSettingsDefaults();
    void installRTPSenderChannelAudioSettingsDefaults();
    void installRTPSenderChannelVideoSettingsDefaults();
    void installStatsReportSettingsDefaults();
    void installSCTPTransportSettingsDefaults();
    void installSCTPTransportListenerSettingsDefaults();
    void installSRTPTransportSettingsDefaults();
    void installSRTPSDESTransportSettingsDefaults();

    //-------------------------------------------------------------------------
    static void installAllDefaults()
    {
      installCertificateSettingsDefaults();
      installDataChannelSettingsDefaults();
      installDTMFSenderSettingsDefaults();
      installDTLSTransportSettingsDefaults();
      installICEGathererSettingsDefaults();
      installICETransportSettingsDefaults();
      installIdentitySettingsDefaults();
      installMediaDevicesSettingsDefaults();
      installMediaStreamTrackSettingsDefaults();
      installRTPListenerSettingsDefaults();
      installRTPMediaEngineSettingsDefaults();
      installRTPReceiverSettingsDefaults();
      installRTPReceiverChannelAudioSettingsDefaults();
      installRTPReceiverChannelSettingsDefaults();
      installRTPReceiverChannelVideoSettingsDefaults();
      installRTPSenderSettingsDefaults();
      installRTPSenderChannelSettingsDefaults();
      installRTPSenderChannelAudioSettingsDefaults();
      installRTPSenderChannelVideoSettingsDefaults();
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
    #pragma mark
    #pragma mark IORTCForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    void IORTCForInternal::overrideQueueDelegate(IMessageQueuePtr queue)
    {
      return (ORTC::singleton())->overrideQueueDelegate(queue);
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueDelegate()
    {
      return (ORTC::singleton())->queueDelegate();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueORTC()
    {
      return (ORTC::singleton())->queueORTC();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queuePacket()
    {
      return (ORTC::singleton())->queuePacket();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueBlockingMediaStartStopThread()
    {
      return (ORTC::singleton())->queueBlockingMediaStartStopThread();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueCertificateGeneration()
    {
      return (ORTC::singleton())->queueCertificateGeneration();
    }

    //-------------------------------------------------------------------------
    Optional<Log::Level> IORTCForInternal::webrtcLogLevel()
    {
      return (ORTC::singleton())->webrtcLogLevel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ORTC
    #pragma mark

    //-------------------------------------------------------------------------
    ORTC::ORTC(const make_private &) :
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
    ORTC::~ORTC()
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
    void ORTC::init()
    {
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::convert(IORTCPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ORTC, object);
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::create()
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
    #pragma mark
    #pragma mark ORTC => IORTC
    #pragma mark

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::singleton()
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
    void ORTC::setup(IMessageQueuePtr defaultDelegateMessageQueue)
    {
      {
        AutoRecursiveLock lock(mLock);

        if (defaultDelegateMessageQueue) {
          mDelegateQueue = defaultDelegateMessageQueue;
        }
      }

      UseServicesHelper::setup();
      installAllDefaults();
      ISettings::applyDefaults();
    }

#ifdef WINRT
    //-------------------------------------------------------------------------
    void ORTC::setup(Windows::UI::Core::CoreDispatcher ^dispatcher)
    {
      UseServicesHelper::setup(dispatcher);
      installAllDefaults();
      ISettings::applyDefaults();
    }
#endif //WINRT

    //-------------------------------------------------------------------------
    Milliseconds ORTC::ntpServerTime() const
    {
      AutoRecursiveLock(*this);
      return mNTPServerTime;
    }

    //-------------------------------------------------------------------------
    void ORTC::ntpServerTime(const Milliseconds &value)
    {
      {
        AutoRecursiveLock(*this);
        mNTPServerTime = value;
      }
      auto totalMilliseconds = value.count();
      IRTPMediaEngineForORTC::ntpServerTime(value);
    }

    //-------------------------------------------------------------------------
    void ORTC::defaultWebrtcLogLevel(Log::Level level)
    {
      AutoRecursiveLock(*this);
      mDefaultWebRTCLogLevel = level;
    }

    //-------------------------------------------------------------------------
    void ORTC::webrtcLogLevel(Log::Level level)
    {
      AutoRecursiveLock(*this);
      mWebRTCLogLevel = level;
    }

    //-------------------------------------------------------------------------
    void ORTC::startMediaTracing()
    {
      IRTPMediaEngineForORTC::startMediaTracing();
    }

    //-------------------------------------------------------------------------
    void ORTC::stopMediaTracing()
    {
      IRTPMediaEngineForORTC::stopMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool ORTC::isMediaTracing()
    {
      return IRTPMediaEngineForORTC::isMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool ORTC::saveMediaTrace(String filename)
    {
      return IRTPMediaEngineForORTC::saveMediaTrace(filename);
    }

    //-------------------------------------------------------------------------
    bool ORTC::saveMediaTrace(String host, int port)
    {
      return IRTPMediaEngineForORTC::saveMediaTrace(host, port);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Stack => IORTCForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    void ORTC::overrideQueueDelegate(IMessageQueuePtr queue)
    {
      AutoRecursiveLock lock(*this);
      mDelegateQueue = queue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueDelegate() const
    {
      AutoRecursiveLock lock(*this);
      if (!mDelegateQueue) {
        mDelegateQueue = UseMessageQueueManager::getMessageQueueForGUIThread();
      }
      return mDelegateQueue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueORTC() const
    {
      AutoRecursiveLock lock(*this);
      if (!mORTCQueue) {
        mORTCQueue = UseMessageQueueManager::getThreadPoolQueue(ORTC_QUEUE_MAIN_THREAD_NAME);
      }
      return mORTCQueue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queuePacket() const
    {
      AutoRecursiveLock lock(*this);

      size_t index = mNextPacketQueueThread % ORTC_QUEUE_TOTAL_PACKET_THREADS;

      if (!mPacketQueues[index]) {
        mPacketQueues[index] = UseMessageQueueManager::getMessageQueue((String(ORTC_QUEUE_PACKET_THREAD_NAME) + string(index)).c_str());
      }

      ++mNextPacketQueueThread;
      return mPacketQueues[index];
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueBlockingMediaStartStopThread() const
    {
      AutoRecursiveLock lock(*this);
      if (!mBlockingMediaStartStopThread) {
        mBlockingMediaStartStopThread = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME);
      }
      return mBlockingMediaStartStopThread;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueCertificateGeneration() const
    {
      AutoRecursiveLock lock(*this);
      if (!mCertificateGeneration) {
        mCertificateGeneration = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_CERTIFICATE_GENERATION_NAME);
      }
      return mCertificateGeneration;
    }

    //-------------------------------------------------------------------------
    Optional<Log::Level> ORTC::webrtcLogLevel() const
    {
      AutoRecursiveLock lock(*this);
      if (mWebRTCLogLevel.hasValue()) return mWebRTCLogLevel;
      return mDefaultWebRTCLogLevel;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Stack => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params ORTC::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ORTC");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ORTC::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::ORTC");
      return Log::Params(message, objectEl);
    }

  } // namespace internal

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IORTC
  #pragma mark

  //---------------------------------------------------------------------------
  void IORTC::setup(IMessageQueuePtr defaultDelegateMessageQueue)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->setup(defaultDelegateMessageQueue);
  }

  //---------------------------------------------------------------------------
#ifdef WINRT
  void IORTC::setup(Windows::UI::Core::CoreDispatcher ^dispatcher)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->setup(dispatcher);
  }
#endif //WINRT

  //-------------------------------------------------------------------------
  Milliseconds IORTC::ntpServerTime()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return Milliseconds();
    return singleton->ntpServerTime();
  }

  //-------------------------------------------------------------------------
  void IORTC::ntpServerTime(const Milliseconds &value)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->ntpServerTime(value);
  }

  //-------------------------------------------------------------------------
  void IORTC::setDefaultLogLevel(Log::Level level)
  {
    auto singleton = internal::ORTC::singleton();
    if (singleton) {
      singleton->defaultWebrtcLogLevel(level);
    }

    UseServicesLogger::setLogLevel(level);
  }

  //-------------------------------------------------------------------------
  void IORTC::setLogLevel(const char *componenet, Log::Level level)
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
  void IORTC::setDefaultEventingLevel(Log::Level level)
  {
    UseServicesLogger::setEventingLevel(level);
  }

  //-------------------------------------------------------------------------
  void IORTC::setEventingLevel(const char *componenet, Log::Level level)
  {
    UseServicesLogger::setEventingLevel(componenet, level);
  }

  //-------------------------------------------------------------------------
  void IORTC::startMediaTracing()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->startMediaTracing();
  }

  //-------------------------------------------------------------------------
  void IORTC::stopMediaTracing()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->stopMediaTracing();
  }

  //-------------------------------------------------------------------------
  bool IORTC::isMediaTracing()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->isMediaTracing();
  }

  //-------------------------------------------------------------------------
  bool IORTC::saveMediaTrace(String filename)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->saveMediaTrace(filename);
  }

  //-------------------------------------------------------------------------
  bool IORTC::saveMediaTrace(String host, int port)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->saveMediaTrace(host, port);
  }

}
