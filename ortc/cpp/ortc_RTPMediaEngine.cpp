/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/ortc_RTPMediaEngine.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc_Tracing.h>
#include <ortc/internal/platform.h>

#include <ortc/IStatsReport.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Singleton.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>
#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>

#include <webrtc/base/timeutils.h>
#include <webrtc/voice_engine/include/voe_codec.h>
#include <webrtc/voice_engine/include/voe_rtp_rtcp.h>
#include <webrtc/voice_engine/include/voe_network.h>
#include <webrtc/voice_engine/include/voe_hardware.h>
#include <webrtc/system_wrappers/include/cpu_info.h>
#include <webrtc/webrtc/base/scoped_ptr.h>
#include <webrtc/webrtc/voice_engine/include/voe_audio_processing.h>
#ifdef WINRT
#include <third_party/h264_winrt/h264_winrt_factory.h>
#endif

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtpmediaengine) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings);
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper);
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP);

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPMediaEngineRegistration);
    ZS_DECLARE_CLASS_PTR(RTPMediaEngineSingleton);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    // foreward declaration
    void webrtcTrace(Log::Severity severity, Log::Level level, const char *message);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngineRegistration
    #pragma mark

    class RTPMediaEngineRegistration : public IRTPMediaEngineRegistration
    {
    protected:
      struct make_private {};
    public:
      //-----------------------------------------------------------------------
      RTPMediaEngineRegistration(const make_private &)
      {}

      //-----------------------------------------------------------------------
      ~RTPMediaEngineRegistration()
      {
        mEngine->shutdown();
        mEngine.reset();
      }

      //-----------------------------------------------------------------------
      static RTPMediaEngineRegistrationPtr create()
      {
        RTPMediaEngineRegistrationPtr pThis(make_shared<RTPMediaEngineRegistration>(make_private{}));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void init()
      {
        mEngine = IRTPMediaEngineFactory::singleton().create(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      PromiseWithRTPMediaEnginePtr notify()
      {
        auto promise = PromiseWithRTPMediaEngine::create(IORTCForInternal::queueORTC());
        promise->setReferenceHolder(mThisWeak.lock());
        mEngine->notify(promise);
        return promise;
      }

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IRTPMediaEngineRegistration
      #pragma mark

      virtual RTPMediaEnginePtr getRTPEngine() const {return mEngine;}

    protected:
      RTPMediaEngineRegistrationWeakPtr mThisWeak;
      RTPMediaEnginePtr mEngine;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngineSingleton
    #pragma mark

    class RTPMediaEngineSingleton : public SharedRecursiveLock,
                                    public ISingletonManagerDelegate
    {
    protected:
      struct make_private {};

    public:
      //-----------------------------------------------------------------------
      RTPMediaEngineSingleton(const make_private &) :
        SharedRecursiveLock(SharedRecursiveLock::create())
      {
      }

      //-----------------------------------------------------------------------
      static RTPMediaEngineSingletonPtr create()
      {
        RTPMediaEngineSingletonPtr pThis(make_shared<RTPMediaEngineSingleton>(make_private{}));
        return pThis;
      }

      //-----------------------------------------------------------------------
      static RTPMediaEngineSingletonPtr singleton()
      {
        AutoRecursiveLock lock(*UseServicesHelper::getGlobalLock());
        static SingletonLazySharedPtr<RTPMediaEngineSingleton> singleton(create());
        RTPMediaEngineSingletonPtr result = singleton.singleton();

        static zsLib::SingletonManager::Register registerSingleton("openpeer::ortc::RTPMediaEngineSingleton", result);

        if (!result) {
          ZS_LOG_WARNING(Detail, slog("singleton gone"))
        }

        return result;
      }

      //-----------------------------------------------------------------------
      static RTPMediaEnginePtr getEngineIfAlive()
      {
        auto pThis(singleton());
        if (!pThis) return RTPMediaEnginePtr();

        AutoRecursiveLock lock(*pThis);
        auto result = pThis->mEngineRegistration.lock();
        if (!result) return RTPMediaEnginePtr();

        return result->getRTPEngine();
      }

      //-----------------------------------------------------------------------
      Log::Params log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::RTPMediaEngineSingleton");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      static Log::Params slog(const char *message)
      {
        return Log::Params(message, "ortc::RTPMediaEngineSingleton");
      }

      //-----------------------------------------------------------------------
      Log::Params debug(const char *message) const
      {
        return Log::Params(message, toDebug());
      }

      //-----------------------------------------------------------------------
      virtual ElementPtr toDebug() const
      {
        AutoRecursiveLock lock(*this);
        ElementPtr resultEl = Element::create("ortc::RTPMediaEngineSingleton");

        UseServicesHelper::debugAppend(resultEl, "id", mID);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      RTPMediaEngineRegistrationPtr getEngineRegistration()
      {
        AutoRecursiveLock lock(*this);
        auto result = mEngineRegistration.lock();
        if (!result) {
          result = RTPMediaEngineRegistration::create();
          mEngineRegistration = result;
        }
        return result;
      }

    protected:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ISingletonManagerDelegate
      #pragma mark

      virtual void notifySingletonCleanup()
      {
        AutoRecursiveLock lock(*this);
        mEngineRegistration.reset();
      }

    protected:
      AutoPUID mID;

      RTPMediaEngineRegistrationWeakPtr mEngineRegistration;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForORTC
    #pragma mark

    //-------------------------------------------------------------------------
    void IRTPMediaEngineForORTC::setLogLevel(Log::Level level)
    {
      RTPMediaEngine::setLogLevel(level);
    }

    //-------------------------------------------------------------------------
    void IRTPMediaEngineForORTC::ntpServerTime(const Milliseconds &value)
    {
      RTPMediaEngine::ntpServerTime(value);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IRTPMediaEngineForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPReceiverChannelMediaBase
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPMediaEngineForRTPReceiverChannelMediaBase::toDebug(ForRTPReceiverChannelMediaBasePtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEnginePtr IRTPMediaEngineForRTPReceiverChannelMediaBase::create()
    {
      return RTPMediaEngine::createEnginePromise();
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineDeviceResourcePtr IRTPMediaEngineForRTPReceiverChannelMediaBase::getDeviceResource(const char *deviceID)
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineDeviceResource::createRejected(IORTCForInternal::queueORTC());
      return singleton->getEngineRegistration()->getRTPEngine()->getDeviceResource(deviceID);
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineChannelResourcePtr IRTPMediaEngineForRTPReceiverChannelMediaBase::setupChannel(
                                                                                                            UseReceiverChannelMediaBasePtr channel,
                                                                                                            TransportPtr transport,
                                                                                                            MediaStreamTrackPtr track,
                                                                                                            ParametersPtr parameters,
                                                                                                            RTPPacketPtr packet
                                                                                                            )
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineChannelResource::createRejected(IORTCForInternal::queueORTC());
      return singleton->getEngineRegistration()->getRTPEngine()->setupChannel(
                                                                              channel,
                                                                              transport,
                                                                              track,
                                                                              parameters,
                                                                              packet
                                                                              );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForRTPSenderChannelMediaBase
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPMediaEngineForRTPSenderChannelMediaBase::toDebug(ForRTPSenderChannelMediaBasePtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEnginePtr IRTPMediaEngineForRTPSenderChannelMediaBase::create()
    {
      return RTPMediaEngine::createEnginePromise();
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineDeviceResourcePtr IRTPMediaEngineForRTPSenderChannelMediaBase::getDeviceResource(const char *deviceID)
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineDeviceResource::createRejected(IORTCForInternal::queueORTC());
      return singleton->getEngineRegistration()->getRTPEngine()->getDeviceResource(deviceID);
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineChannelResourcePtr IRTPMediaEngineForRTPSenderChannelMediaBase::setupChannel(
                                                                                                          UseSenderChannelMediaBasePtr channel,
                                                                                                          TransportPtr transport,
                                                                                                          MediaStreamTrackPtr track,
                                                                                                          ParametersPtr parameters
                                                                                                          )
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineChannelResource::createRejected(IORTCForInternal::queueORTC());
      return singleton->getEngineRegistration()->getRTPEngine()->setupChannel(
                                                                              channel,
                                                                              transport,
                                                                              track,
                                                                              parameters
                                                                              );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPMediaEngine::toString(States state)
    {
      switch (state) {
        case State_Pending:       return "pending";
        case State_Ready:         return "ready";
        case State_ShuttingDown:  return "shutting down";
        case State_Shutdown:      return "shutdown";
      }
      return "UNDEFINED";
    }
    
    //-------------------------------------------------------------------------
    RTPMediaEngine::RTPMediaEngine(
                                   const make_private &,
                                   IMessageQueuePtr queue,
                                   IRTPMediaEngineRegistrationPtr registration
                                   ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mRegistration(registration),
      mTraceCallback(new WebRtcTraceCallback()),
      mLogSink(new WebRtcLogSink())
    {
      EventWriteOrtcRtpMediaEngineCreate(__func__, mID);
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::init()
    {
      webrtc::Trace::CreateTrace();
      webrtc::Trace::SetTraceCallback(mTraceCallback.get());

      auto level = IORTCForInternal::webrtcLogLevel();
      if (level.hasValue()) {
        internalSetLogLevel(level);
      }

      AutoRecursiveLock lock(*this);

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::~RTPMediaEngine()
    {
      if (isNoop()) return;

      webrtc::Trace::SetTraceCallback(nullptr);
      webrtc::Trace::ReturnTrace();

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
      EventWriteOrtcRtpMediaEngineDestroy(__func__, mID);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForRTPReceiverChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForRTPReceiverChannelMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForRTPReceiverChannelAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForRTPReceiverChannelVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForRTPSenderChannelMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForRTPSenderChannelAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForRTPSenderChannelVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }

    RTPMediaEnginePtr RTPMediaEngine::convert(ForDeviceResourcePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => RTPMediaEngineSingleton/RTPMediaEngineRegistration
    #pragma mark
    
    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEnginePtr RTPMediaEngine::createEnginePromise()
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) {
        return PromiseWithRTPMediaEngine::createRejected(IORTCForInternal::queueORTC());
      }
      return singleton->getEngineRegistration()->notify();
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr RTPMediaEngine::create(IRTPMediaEngineRegistrationPtr registration)
    {
      RTPMediaEnginePtr pThis(make_shared<RTPMediaEngine>(make_private {}, IORTCForInternal::queueBlockingMediaStartStopThread(), registration));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::notify(PromiseWithRTPMediaEnginePtr promise)
    {
      IRTPMediaEngineRegistrationPtr registration;

      {
        AutoRecursiveLock lock(*this);
        if (!isReady()) {
          mPendingReady.push_back(promise);
          return;
        }
        registration = mRegistration.lock();
      }

      if (registration) {
        promise->resolve(registration->getRTPEngine());
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::shutdown()
    {
      AutoRecursiveLock lock(*this);

      // WARNING: Do NOT call cancel directly as this object must only be
      // shutdown on the object's media queue.
      setState(State_ShuttingDown);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForORTC
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::setLogLevel(Log::Level level)
    {
      auto engine = RTPMediaEngineSingleton::getEngineIfAlive();
      if (engine) {
        engine->internalSetLogLevel(level);
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::ntpServerTime(const Milliseconds &value)
    {
      rtc::SyncWithNtp(value.count());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannelMediaBase
    #pragma mark

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineDeviceResourcePtr RTPMediaEngine::getDeviceResource(const char *deviceID)
    {
      DeviceResourcePtr resource = DeviceResource::create(mRegistration.lock(), deviceID);

      {
        AutoRecursiveLock lock(*this);
        mExampleDeviceResources[resource->getID()] = resource;
        mExamplePendingDeviceResources.push_back(resource);
      }

      // invoke "step" mechanism asynchronously to do something with this resource...
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      return resource->createPromise<IRTPMediaEngineDeviceResource>();
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineChannelResourcePtr RTPMediaEngine::setupChannel(
                                                                             UseReceiverChannelMediaBasePtr channel,
                                                                             TransportPtr transport,
                                                                             MediaStreamTrackPtr track,
                                                                             ParametersPtr parameters,
                                                                             RTPPacketPtr packet
                                                                             )
    {
       PromiseWithRTPMediaEngineChannelResourcePtr promise;

      {
        AutoRecursiveLock lock(*this);
        if (ZS_DYNAMIC_PTR_CAST(IRTPReceiverChannelAudioForRTPMediaEngine, channel)) {
          AudioReceiverChannelResourcePtr resource = AudioReceiverChannelResource::create(
                                                                                          mRegistration.lock(),
                                                                                          transport,
                                                                                          track,
                                                                                          parameters,
                                                                                          packet
                                                                                          );
          promise = resource->createPromise<IRTPMediaEngineChannelResource>();
          mChannelResources[resource->getID()] = resource;
          mPendingSetupChannelResources.push_back(resource);
        } else if (ZS_DYNAMIC_PTR_CAST(IRTPReceiverChannelVideoForRTPMediaEngine, channel)) {
          VideoReceiverChannelResourcePtr resource = VideoReceiverChannelResource::create(
                                                                                          mRegistration.lock(),
                                                                                          transport,
                                                                                          track,
                                                                                          parameters,
                                                                                          packet
                                                                                          );
          promise = resource->createPromise<IRTPMediaEngineChannelResource>();
          mChannelResources[resource->getID()] = resource;
          mPendingSetupChannelResources.push_back(resource);
        }
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannelAudio
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPReceiverChannelVideo
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannel
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannelMediaBase
    #pragma mark

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineChannelResourcePtr RTPMediaEngine::setupChannel(
                                                                             UseSenderChannelMediaBasePtr channel,
                                                                             TransportPtr transport,
                                                                             MediaStreamTrackPtr track,
                                                                             ParametersPtr parameters
                                                                             )
    {
      PromiseWithRTPMediaEngineChannelResourcePtr promise;

      {
        AutoRecursiveLock lock(*this);
        if (ZS_DYNAMIC_PTR_CAST(IRTPSenderChannelAudioForRTPMediaEngine, channel)) {
          ChannelResourcePtr resource = AudioSenderChannelResource::create(
                                                                           mRegistration.lock(),
                                                                           transport,
                                                                           track,
                                                                           parameters
                                                                           );
          promise = resource->createPromise<IRTPMediaEngineChannelResource>();
          mChannelResources[resource->getID()] = resource;
          mPendingSetupChannelResources.push_back(resource);
        } else if (ZS_DYNAMIC_PTR_CAST(IRTPSenderChannelVideoForRTPMediaEngine, channel)) {
          ChannelResourcePtr resource = VideoSenderChannelResource::create(
                                                                           mRegistration.lock(),
                                                                           transport,
                                                                           track,
                                                                           parameters
                                                                           );
          promise = resource->createPromise<IRTPMediaEngineChannelResource>();
          mChannelResources[resource->getID()] = resource;
          mPendingSetupChannelResources.push_back(resource);
        }
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannelAudio
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForRTPSenderChannelVideo
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForDeviceResource
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::notifyResourceGone(IRTPMediaEngineDeviceResource &inResource)
    {
      DeviceResource &resource = dynamic_cast<DeviceResource &>(inResource);

      PUID resourceID = resource.getID();

      AutoRecursiveLock lock(*this);

      auto found = mExampleDeviceResources.find(resourceID);
      if (found != mExampleDeviceResources.end()) {
        mExampleDeviceResources.erase(found);
      }

      // invoke "step" mechanism again
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::notifyResourceGone(IChannelResourceForRTPMediaEngine &inResource)
    {
      PUID resourceID = inResource.getID();

      AutoRecursiveLock lock(*this);

      auto found = mChannelResources.find(resourceID);
      if (found != mChannelResources.end()) {
        mChannelResources.erase(found);
      }

      // invoke "step" mechanism again
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    webrtc::VoiceEngine *RTPMediaEngine::getVoiceEngine()
    {
      return mVoiceEngine.get();
    }

    //-------------------------------------------------------------------------
    rtc::scoped_refptr<webrtc::AudioState> RTPMediaEngine::getAudioState()
    {
      return mAudioState;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::onTimer(TimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
      // NOTE: ADD IF NEEDED...
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => (friend ChannelResource)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::shutdownChannelResource(ChannelResourcePtr channelResource)
    {
      {
        AutoRecursiveLock lock(*this);
        mPendingCloseChannelResources.push_back(channelResource);
      }
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPMediaEngine::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPMediaEngine");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPMediaEngine::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPMediaEngine::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPMediaEngine");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto registration = mRegistration.lock();
      UseServicesHelper::debugAppend(resultEl, "registration", (bool)registration);

      UseServicesHelper::debugAppend(resultEl, "pending ready", mPendingReady.size());

      UseServicesHelper::debugAppend(resultEl, "device resources", mExampleDeviceResources.size());
      UseServicesHelper::debugAppend(resultEl, "pending device resources", mExamplePendingDeviceResources.size());

      UseServicesHelper::debugAppend(resultEl, "channel resources", mChannelResources.size());
      UseServicesHelper::debugAppend(resultEl, "pending setup channel resources", mPendingSetupChannelResources.size());
      UseServicesHelper::debugAppend(resultEl, "pending close channel resources", mPendingCloseChannelResources.size());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::internalSetLogLevel(Log::Level level)
    {
      webrtc::TraceLevel traceLevel {webrtc::kTraceAll};
      rtc::LoggingSeverity rtcLevel {rtc::LS_INFO};

      switch (level)
      {
        case Log::Level::None:    rtcLevel = rtc::LS_NONE; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceNone); break;
        case Log::Level::Basic:   rtcLevel = rtc::LS_ERROR; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceCritical | webrtc::kTraceError); break;
        case Log::Level::Detail:  rtcLevel = rtc::LS_WARNING; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceWarning | webrtc::kTraceError | webrtc::kTraceCritical); break;
        case Log::Level::Debug:   rtcLevel = rtc::LS_INFO; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceDefault); break;
        case Log::Level::Trace:   rtcLevel = rtc::LS_VERBOSE; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceDefault | webrtc::kTraceModuleCall | webrtc::kTraceTimer | webrtc::kTraceDebug); break;
        case Log::Level::Insane:  rtcLevel = rtc::LS_SENSITIVE; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceAll); break;
      }

      rtc::LogMessage::RemoveLogToStream(mLogSink.get());
      rtc::LogMessage::AddLogToStream(mLogSink.get(), rtcLevel);

      webrtc::Trace::set_level_filter(traceLevel);
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::isReady() const
    {
      return State_Ready == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        stepCancel();
        return;
      }

      // ... other steps here ...
      if (!stepSetup()) goto not_ready;
      if (!stepExampleSetupDeviceResources()) goto not_ready;
      if (!stepSetupChannels()) goto not_ready;
      if (!stepCloseChannels()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
        setState(State_Ready);
      }
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepSetup()
    {
      if (isReady()) {
        ZS_LOG_TRACE(log("already setup"))
        return true;
      }

      mVoiceEngine = rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter>(webrtc::VoiceEngine::Create());

      webrtc::VoEBase::GetInterface(mVoiceEngine.get())->Init();

      webrtc::AudioState::Config audioStateConfig;
      audioStateConfig.voice_engine = mVoiceEngine.get();
      mAudioState = rtc::scoped_refptr<webrtc::AudioState>(webrtc::AudioState::Create(audioStateConfig));

      webrtc::EcModes ecMode = webrtc::kEcConference;
      webrtc::AecmModes aecmMode = webrtc::kAecmSpeakerphone;
      webrtc::AgcModes agcMode = webrtc::kAgcAdaptiveAnalog;
      webrtc::NsModes nsMode = webrtc::kNsHighSuppression;
#if defined(WINRT)
      ecMode = webrtc::kEcAecm;
#endif

      if (webrtc::VoEHardware::GetInterface(mVoiceEngine.get())->BuiltInAECIsAvailable())
        webrtc::VoEHardware::GetInterface(mVoiceEngine.get())->EnableBuiltInAEC(true);
      webrtc::VoEAudioProcessing::GetInterface(mVoiceEngine.get())->SetEcStatus(true, ecMode);
#if !defined(WINRT)
      webrtc::VoEAudioProcessing::GetInterface(mVoiceEngine.get())->SetEcMetricsStatus(true);
#endif
      if (ecMode == webrtc::kEcAecm)
        webrtc::VoEAudioProcessing::GetInterface(mVoiceEngine.get())->SetAecmMode(aecmMode);

      if (webrtc::VoEHardware::GetInterface(mVoiceEngine.get())->BuiltInAGCIsAvailable())
        webrtc::VoEHardware::GetInterface(mVoiceEngine.get())->EnableBuiltInAGC(true);
      webrtc::VoEAudioProcessing::GetInterface(mVoiceEngine.get())->SetAgcStatus(true, agcMode);

      if (webrtc::VoEHardware::GetInterface(mVoiceEngine.get())->BuiltInNSIsAvailable())
        webrtc::VoEHardware::GetInterface(mVoiceEngine.get())->EnableBuiltInNS(true);
      webrtc::VoEAudioProcessing::GetInterface(mVoiceEngine.get())->SetNsStatus(true, nsMode);

      webrtc::VoEAudioProcessing::GetInterface(mVoiceEngine.get())->EnableHighPassFilter(true);

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepExampleSetupDeviceResources()
    {
      while (mExamplePendingDeviceResources.size() > 0) {
        auto deviceResource = mExamplePendingDeviceResources.front().lock();

        if (deviceResource) {
          // Only remember WEAK pointer to device so it's possible the example
          // device resource was already destroyed. Thus only setup the device
          // if the object is still alive.
          deviceResource->notifyPromisesResolve();
        }

        mExamplePendingDeviceResources.pop_front();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepSetupChannels()
    {
      while (mPendingSetupChannelResources.size() > 0) {
        auto channelResource = mPendingSetupChannelResources.front();

        channelResource->stepSetup();

        mPendingSetupChannelResources.pop_front();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepCloseChannels()
    {
      while (mPendingCloseChannelResources.size() > 0) {
        auto channelResource = mPendingCloseChannelResources.front();

        channelResource->stepShutdown();
        
        mPendingCloseChannelResources.pop_front();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_ShuttingDown);

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      {
        for (auto iter = mPendingSetupChannelResources.begin(); iter != mPendingSetupChannelResources.end(); ++iter)
        {
          auto channelResource = (*iter);
          channelResource->notifyPromisesReject();
        }
        mPendingSetupChannelResources.clear();
      }

      if (mGracefulShutdownReference) {
        // perform any graceful asynchronous shutdown processes needed and
        // re-attempt shutdown again later if needed.

#define TODO_IMPLEMENT_MEDIA_GRACEFUL_SHUTDOWN 1
#define TODO_IMPLEMENT_MEDIA_GRACEFUL_SHUTDOWN 2
//        return;
      }

      //.......................................................................
      // final cleanup (hard shutdown)

      setState(State_Shutdown);

#define TODO_IMPLEMENT_MEDIA_HARD_SHUTDOWN 1
#define TODO_IMPLEMENT_MEDIA_HARD_SHUTDOWN 2

      // resolve any outstanding promises
      {
        auto registration = mRegistration.lock();
        while (mPendingReady.size() > 0)
        {
          auto &front = mPendingReady.front();
          if (registration) {
            front->resolve(registration);
          } else {
            front->reject();
          }
          mPendingReady.pop_front();
        }
      }

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::stepCancel()
    {
      {
        for (auto iter = mPendingCloseChannelResources.begin(); iter != mPendingCloseChannelResources.end(); ++iter)
        {
          auto channelResource = (*iter);
          channelResource->stepShutdown();
        }
        mPendingCloseChannelResources.clear();
      }

      {
        for (auto iter = mChannelResources.begin(); iter != mChannelResources.end(); ++iter)
        {
          auto channelResource = (*iter).second.lock();
          channelResource->stepShutdown();
        }

        mChannelResources.clear();
      }
      cancel();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

      if (isReady()) {
        auto registration = mRegistration.lock();

        while (mPendingReady.size() > 0)
        {
          auto &front = mPendingReady.front();
          if (registration) {
            front->resolve(registration->getRTPEngine());
          } else {
            front->reject();
          }
          mPendingReady.pop_front();
        }
      }

//      RTPMediaEnginePtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPMediaEngineStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != mLastError) {
        ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
        return;
      }

      mLastError = errorCode;
      mLastErrorReason = reason;

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::WebRtcTraceCallback
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::WebRtcTraceCallback::Print(webrtc::TraceLevel trace, const char* message, int length)
    {
      static const size_t stripLength = 34;

      Log::Severity severity {Log::Severity::Informational};
      Log::Level level {Log::Level::Basic};

      switch (trace)
      {
        case webrtc::kTraceNone:        level = Log::Level::None; break;
        case webrtc::kTraceStateInfo:   level = Log::Level::Debug; break;
        case webrtc::kTraceCritical:    severity = Log::Severity::Fatal; level = Log::Level::Basic; break;
        case webrtc::kTraceError:       severity = Log::Severity::Error; level = Log::Level::Basic; break;
        case webrtc::kTraceWarning:     severity = Log::Severity::Warning; level = Log::Level::Detail; break;
        case webrtc::kTraceApiCall:     level = Log::Level::Debug; break;
        case webrtc::kTraceDefault:     level = Log::Level::Debug; break;
        case webrtc::kTraceModuleCall:  level = Log::Level::Trace; break;
        case webrtc::kTraceMemory:      level = Log::Level::Trace; break;
        case webrtc::kTraceTimer:       level = Log::Level::Trace; break;
        case webrtc::kTraceStream:      level = Log::Level::Insane; break;
        case webrtc::kTraceDebug:       level = Log::Level::Insane; break;
        case webrtc::kTraceInfo:        level = Log::Level::Insane; break;
        case webrtc::kTraceTerseInfo:   level = Log::Level::Insane; break;
        case webrtc::kTraceAll:         level = Log::Level::Insane; break;
      }

      // Skip past boilerplate prefix text
      if (length < stripLength) {
        std::string msg(message, length);
        webrtcTrace(Log::Severity::Error, Log::Level::Basic, (std::string("Malformed webrtc log message: ") + msg).c_str());
      } else {
        std::string msg(message + (stripLength-1), length - stripLength);
        webrtcTrace(severity, level, msg.c_str());
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::WebRtcTraceCallback
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::WebRtcLogSink::OnLogMessage(const std::string& message)
    {
      if (message.length() < 1) return;

      bool hasEOL = ('\n' == message[message.length() - 1]);
     
      std::string msg(message.c_str(), message.length() - (hasEOL ? 1 : 0));

      webrtcTrace(Log::Severity::Informational, Log::Level::Basic, msg.c_str());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::BaseResource
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::BaseResource::BaseResource(
                                               const make_private &,
                                               IRTPMediaEngineRegistrationPtr registration,
                                               RTPMediaEnginePtr engine
                                               ) :
      SharedRecursiveLock(engine ? SharedRecursiveLock(engine->getSharedLock()) : SharedRecursiveLock::create()),
      MessageQueueAssociator(engine ? engine->getAssociatedMessageQueue() : IORTCForInternal::queueBlockingMediaStartStopThread()),
      mMediaEngine(engine),
      mRegistration(registration)
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::BaseResource::~BaseResource()
    {
      mThisWeak.reset();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::BaseResource::notifyPromisesResolve()
    {
      {
        AutoRecursiveLock lock(*this);
        mNotifiedReady = true;
      }
      internalFixState();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::BaseResource::notifyPromisesReject()
    {
      {
        AutoRecursiveLock lock(*this);
        mNotifiedRejected = true;
      }
      internalFixState();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::BaseResource => (internal)
    #pragma mark


    //-------------------------------------------------------------------------
    IMessageQueuePtr RTPMediaEngine::BaseResource::delegateQueue()
    {
      return IORTCForInternal::queueORTC();
    }

    //-------------------------------------------------------------------------
    PromisePtr RTPMediaEngine::BaseResource::internalSetupPromise(PromisePtr promise)
    {
      auto lifetime = make_shared<LifetimeHolder>(mThisWeak.lock());
      promise->setReferenceHolder(lifetime);

      {
        AutoRecursiveLock lock(*this);
        mPendingPromises.push_back(promise);
      }
      internalFixState();
      return promise;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::BaseResource::internalFixState()
    {
      PendingPromiseList promises;

      {
        AutoRecursiveLock lock(*this);

        {
          if (mNotifiedRejected) goto prepare_reject_all;
          if (mNotifiedReady) goto prepare_resolve_all;
          return;
        }

      prepare_resolve_all:
        {
          promises = mPendingPromises;
          mPendingPromises.clear();
          goto resolve_all;
        }

      prepare_reject_all:
        {
          promises = mPendingPromises;
          mPendingPromises.clear();
          goto reject_all;
        }
      }

    resolve_all:
      {
        for (auto iter = promises.begin(); iter != promises.end(); ++iter) {
          auto promise = (*iter).lock();
          if (!promise) continue;
          promise->resolve(mThisWeak.lock());
        }
        return;
      }

    reject_all:
      {
        for (auto iter = promises.begin(); iter != promises.end(); ++iter) {
          auto promise = (*iter).lock();
          if (!promise) continue;
          promise->reject();
        }
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::DeviceResource
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::DeviceResource::DeviceResource(
                                                   const make_private &priv,
                                                   IRTPMediaEngineRegistrationPtr registration,
                                                   const char *deviceID
                                                   ) :
      BaseResource(priv, registration, registration ? registration->getRTPEngine() : RTPMediaEnginePtr()),
      mDeviceID(deviceID)
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::DeviceResource::~DeviceResource()
    {
      mThisWeak.reset();  // shared pointer to self is no longer valid

      // inform the rtp media engine of this resource no longer being in use
      UseEnginePtr engine = getEngine<UseEngine>();
      if (engine) {
        engine->notifyResourceGone(*this);
      }
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::DeviceResourcePtr RTPMediaEngine::DeviceResource::create(
                                                                             IRTPMediaEngineRegistrationPtr registration,
                                                                             const char *deviceID
                                                                             )
    {
      auto pThis = make_shared<DeviceResource>(make_private{}, registration, deviceID);
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::init()
    {
      DeviceResourcePtr pThis = getThis<DeviceResource>();  // example of how to get pThis from base class
#define EXAMPLE_OF_HOW_TO_DO_CUSTOM_RESOURC_SETUP_STEP 1
#define EXAMPLE_OF_HOW_TO_DO_CUSTOM_RESOURC_SETUP_STEP 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::DeviceResource => IRTPMediaEngineDeviceResource
    #pragma mark

    //-------------------------------------------------------------------------
    String RTPMediaEngine::DeviceResource::getDeviceID() const
    {
#define EXAMPLE_OF_HOW_TO_EXPOSE_API_TO_CONSUMER_OF_RESOURCE 1
#define EXAMPLE_OF_HOW_TO_EXPOSE_API_TO_CONSUMER_OF_RESOURCE 2
      return mDeviceID;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::ChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::ChannelResource::ChannelResource(
                                                     const make_private &priv,
                                                     IRTPMediaEngineRegistrationPtr registration
                                                     ) : 
      BaseResource(priv, registration, registration ? registration->getRTPEngine() : RTPMediaEnginePtr()),
      mHandlePacketQueue(IORTCForInternal::queuePacket()),
      mClock(webrtc::Clock::GetRealTimeClock()),
      mRemb(mClock)
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::ChannelResource::~ChannelResource()
    {
      mThisWeak.reset();
      UseEnginePtr engine = getEngine<UseEngine>();
      if (engine) {
        engine->notifyResourceGone(*this);
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::ChannelResource::lifetimeHolderGone()
    {
      auto shutdownPromise = shutdown();
      if (shutdownPromise) {
        // don't really care about result
        shutdownPromise->then(IPromiseDelegateProxy::createNoop(IORTCForInternal::queueBlockingMediaStartStopThread()));
        shutdownPromise->background();
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::ChannelResource => IRTPMediaEngineChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    PromisePtr RTPMediaEngine::ChannelResource::shutdown()
    {
      PromisePtr promise = getShutdownPromise();
      if (isShutdown()) return promise;
      if (mShuttingDown) return promise;

      mShuttingDown = true;

      auto outer = mMediaEngine.lock();
      if (outer) {
        outer->shutdownChannelResource(ZS_DYNAMIC_PTR_CAST(ChannelResource, mThisWeak.lock()));
      } else {
        stepShutdown();
      }
      return promise;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::ChannelResource::notifyTransportState(ISecureTransportTypes::States state)
    {
      auto pThis = ZS_DYNAMIC_PTR_CAST(ChannelResource, mThisWeak.lock());
      IRTPMediaEngineChannelResourceAsyncDelegateProxy::create(pThis)->onSecureTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::ChannelResource::notifyUpdate(ParametersPtr param)
    {
      auto pThis = ZS_DYNAMIC_PTR_CAST(ChannelResource, mThisWeak.lock());
      IRTPMediaEngineChannelResourceAsyncDelegateProxy::create(pThis)->onUpdate(param);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::ChannelResource::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
    {
      auto pThis = ZS_DYNAMIC_PTR_CAST(ChannelResource, mThisWeak.lock());
      IRTPMediaEngineChannelResourceAsyncDelegateProxy::create(pThis)->onProvideStats(promise, stats);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::ChannelResource => (internal friend derived)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::ChannelResource::notifyPromisesShutdown()
    {
      AutoRecursiveLock lock(*this);

      mShutdown = true;
      mShuttingDown = false;

      for (auto iter = mShutdownPromises.begin(); iter != mShutdownPromises.end(); ++iter)
      {
        auto promise = (*iter);
        promise->resolve();
      }
      mShutdownPromises.clear();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::ChannelResource => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    PromisePtr RTPMediaEngine::ChannelResource::getShutdownPromise()
    {
      if (isShutdown()) {
        return Promise::createResolved(delegateQueue());
      }
      PromisePtr promise = Promise::create(delegateQueue());
      mShutdownPromises.push_back(promise);
      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::AudioReceiverChannelResource::AudioReceiverChannelResource(
                                                                               const make_private &priv,
                                                                               IRTPMediaEngineRegistrationPtr registration,
                                                                               TransportPtr transport,
                                                                               MediaStreamTrackPtr track,
                                                                               ParametersPtr parameters,
                                                                               RTPPacketPtr packet
                                                                               ) :
      ChannelResource(priv, registration),
      mTransport(transport),
      mTrack(track),
      mParameters(parameters),
      mInitPacket(packet)
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::AudioReceiverChannelResource::~AudioReceiverChannelResource()
    {
      mThisWeak.reset();  // shared pointer to self is no longer valid
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::AudioReceiverChannelResourcePtr RTPMediaEngine::AudioReceiverChannelResource::create(
                                                                                                         IRTPMediaEngineRegistrationPtr registration,
                                                                                                         TransportPtr transport,
                                                                                                         MediaStreamTrackPtr track,
                                                                                                         ParametersPtr parameters,
                                                                                                         RTPPacketPtr packet  
                                                                                                         )
    {
      auto pThis = make_shared<AudioReceiverChannelResource>(
                                                             make_private{},
                                                             registration,
                                                             transport,
                                                             track,
                                                             parameters,
                                                             packet
                                                             );
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::init()
    {
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => IRTPMediaEngineChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => IRTPMediaEngineChannelResourceAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::onSecureTransportState(ISecureTransport::States state)
    {
      AutoRecursiveLock lock(*this);

      if (state == mTransportState) return;

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      ISecureTransport::States previousState = mTransportState;
      mTransportState = state;
      if (mParameters && (mParameters->mEncodings.size() == 0 ||
        (mParameters->mEncodings.size() > 0 && mParameters->mEncodings.begin()->mActive))) {
        if (state == ISecureTransport::State_Connected) {
          webrtc::VoEBase::GetInterface(voiceEngine)->StartReceive(mChannel);
          webrtc::VoEBase::GetInterface(voiceEngine)->StartPlayout(mChannel);
        } else if (previousState == ISecureTransport::State_Connected) {
          webrtc::VoEBase::GetInterface(voiceEngine)->StopPlayout(mChannel);
          webrtc::VoEBase::GetInterface(voiceEngine)->StopReceive(mChannel);
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::onUpdate(ParametersPtr params)
    {
      AutoRecursiveLock lock(*this);

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      ParametersPtr previousParams = mParameters;
      mParameters = params;
      bool previousActive = false;
      bool currentActive = false;
      if (previousParams->mEncodings.size() == 0 ||
        (previousParams->mEncodings.size() > 0 && previousParams->mEncodings.begin()->mActive)) {
        previousActive = true;
      }
      if (params->mEncodings.size() == 0 ||
        (params->mEncodings.size() > 0 && params->mEncodings.begin()->mActive)) {
        currentActive = true;
      }
      if (mTransportState == ISecureTransport::State_Connected) {
        if (!previousActive && currentActive) {
          webrtc::VoEBase::GetInterface(voiceEngine)->StartReceive(mChannel);
          webrtc::VoEBase::GetInterface(voiceEngine)->StartPlayout(mChannel);
        } else if (previousActive && !currentActive) {
          webrtc::VoEBase::GetInterface(voiceEngine)->StopPlayout(mChannel);
          webrtc::VoEBase::GetInterface(voiceEngine)->StopReceive(mChannel);
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::onProvideStats(PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      AutoRecursiveLock lock(*this);

      UseStatsReport::StatMap reportStats;

      webrtc::AudioReceiveStream::Stats receiveStreamStats = mReceiveStream->GetStats();

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_InboundRTP)) {

        auto report = make_shared<IStatsReport::InboundRTPStreamStats>();

        report->mID = string(receiveStreamStats.remote_ssrc) + "_recv";

        report->mSSRC = receiveStreamStats.remote_ssrc;
        report->mIsRemote = true;
        report->mMediaType = "audio";
        report->mMediaTrackID = mTrack->id();
        report->mCodecID = mCodecPayloadName;
        report->mPacketsReceived = receiveStreamStats.packets_rcvd;
        report->mBytesReceived = receiveStreamStats.bytes_rcvd;
        report->mPacketsLost = receiveStreamStats.packets_lost;
        report->mJitter = receiveStreamStats.jitter_ms;
        report->mFractionLost = receiveStreamStats.fraction_lost;
        report->mEndToEndDelay = Milliseconds(receiveStreamStats.end_to_end_delayMs);

        reportStats[report->mID] = report;
      }

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_Codec)) {

        auto report = make_shared<IStatsReport::Codec>();

        report->mID = string(receiveStreamStats.remote_ssrc) + "_recv_codec";

        report->mPayloadType = mCodecPayloadType;
        report->mCodec = mCodecPayloadName;
        report->mClockRate = 0;
        report->mChannels = 0;

        reportStats[report->mID] = report;
      }

      promise->resolve(UseStatsReport::create(reportStats));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => IRTPMediaEngineAudioReceiverChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::AudioReceiverChannelResource::handlePacket(const RTPPacket &packet)
    {
      IRTPMediaEngineHandlePacketAsyncDelegateProxy::createUsingQueue(mHandlePacketQueue, getThis<AudioReceiverChannelResource>())->onHandleRTPPacket(packet.timestamp(), packet.buffer());
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::AudioReceiverChannelResource::handlePacket(const RTCPPacket &packet)
    {
      IRTPMediaEngineHandlePacketAsyncDelegateProxy::createUsingQueue(mHandlePacketQueue, getThis<AudioReceiverChannelResource>())->onHandleRTCPPacket(packet.buffer());
      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => IRTPMediaEngineHandlePacketAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::onHandleRTPPacket(DWORD timestamp, SecureByteBlockPtr buffer)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);

      if (mDenyNonLockedAccess) return;

      webrtc::PacketTime time(timestamp, 0);

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      webrtc::VoENetwork::GetInterface(voiceEngine)->ReceivedRTPPacket(getChannel(), buffer->BytePtr(), buffer->SizeInBytes(), time);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::onHandleRTCPPacket(SecureByteBlockPtr buffer)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);
      
      if (mDenyNonLockedAccess) return;

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      webrtc::VoENetwork::GetInterface(voiceEngine)->ReceivedRTCPPacket(getChannel(), buffer->BytePtr(), buffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => webrtc::BitrateObserver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs)
    {
      AutoRecursiveLock lock(*this);

      mCurrentTargetBitrate = targetBitrateBps;

      uint32_t allocatedBitrateBps = mBitrateAllocator->OnNetworkChanged(
                                                                         targetBitrateBps,
                                                                         fractionLoss,
                                                                         rttMs
                                                                         );

      int padUpToBitrateBps = 0;
      uint32_t pacerBitrateBps = std::max(targetBitrateBps, allocatedBitrateBps);

      if (mCongestionController)
        mCongestionController->UpdatePacerBitrate(
                                                  targetBitrateBps / 1000,
                                                  webrtc::PacedSender::kDefaultPaceMultiplier * pacerBitrateBps / 1000,
                                                  padUpToBitrateBps / 1000
                                                  );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => friend RTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::stepSetup()
    {
      AutoRecursiveLock lock(*this);

      auto engine = mMediaEngine.lock();
      if (!engine) {
        notifyPromisesReject();
        return;
      }

      auto voiceEngine = engine->getVoiceEngine();
      if (NULL == voiceEngine) {
        notifyPromisesReject();
        return;
      }

      auto audioState = engine->getAudioState();

      mModuleProcessThread = webrtc::ProcessThread::Create("AudioReceiverChannelResourceModuleProcessThread");
      mPacerThread = webrtc::ProcessThread::Create("AudioReceiverChannelResourcePacerThread");

      mBitrateAllocator = rtc::scoped_ptr<webrtc::BitrateAllocator>(new webrtc::BitrateAllocator());
      mCallStats = rtc::scoped_ptr<webrtc::CallStats>(new webrtc::CallStats(mClock));
      mCongestionController =
        rtc::scoped_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mClock,
                                                                                       this,
                                                                                       &mRemb
                                                                                       ));

      mCallStats->RegisterStatsObserver(mCongestionController.get());

      mChannel = webrtc::VoEBase::GetInterface(voiceEngine)->CreateChannel();

      bool audioCodecSet = false;
      webrtc::CodecInst codec;
      for (auto codecIter = mParameters->mCodecs.begin(); codecIter != mParameters->mCodecs.end(); codecIter++) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::getCodecKind(supportedCodec) == IRTPTypes::CodecKind_Audio && audioCodecSet)
          continue;
        codec = getAudioCodec(voiceEngine, codecIter->mName);
        codec.pltype = codecIter->mPayloadType;
        if (codecIter->mPTime != Milliseconds::zero())
          codec.pacsize = (codec.plfreq / 1000) * codecIter->mPTime.count();
        if (codecIter->mNumChannels.hasValue())
          codec.channels = codecIter->mNumChannels;
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_Opus:
            codec.rate = 48000;
            webrtc::VoECodec::GetInterface(voiceEngine)->SetRecPayloadType(mChannel, codec);
            goto set_rtcp_feedback;
          case IRTPTypes::SupportedCodec_Isac:
          case IRTPTypes::SupportedCodec_G722:
          case IRTPTypes::SupportedCodec_ILBC:
          case IRTPTypes::SupportedCodec_PCMU:
          case IRTPTypes::SupportedCodec_PCMA:
            webrtc::VoECodec::GetInterface(voiceEngine)->SetRecPayloadType(mChannel, codec);
            goto set_rtcp_feedback;
          case IRTPTypes::SupportedCodec_RED:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetREDStatus(mChannel, true, codecIter->mPayloadType);
            break;
        }
        continue;

      set_rtcp_feedback:
        for (auto rtcpFeedbackIter = codecIter->mRTCPFeedback.begin(); rtcpFeedbackIter != codecIter->mRTCPFeedback.end(); rtcpFeedbackIter++) {
          IRTPTypes::KnownFeedbackTypes feedbackType = IRTPTypes::toKnownFeedbackType(rtcpFeedbackIter->mType);
          IRTPTypes::KnownFeedbackParameters feedbackParameter = IRTPTypes::toKnownFeedbackParameter(rtcpFeedbackIter->mParameter);
          if (IRTPTypes::KnownFeedbackType_NACK == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetNACKStatus(mChannel, true, 250);
          }
        }
        audioCodecSet = true;
      }

      mCodecPayloadName = codec.plname;
      mCodecPayloadType = codec.pltype;

      webrtc::AudioReceiveStream::Config config;
      config.voe_channel_id = mChannel;

      for (auto encodingParamIter = mParameters->mEncodings.begin(); encodingParamIter != mParameters->mEncodings.end(); encodingParamIter++) {

        IRTPTypes::PayloadType codecPayloadType {};
        if (encodingParamIter->mCodecPayloadType.hasValue())
          codecPayloadType = encodingParamIter->mCodecPayloadType;
        else
          codecPayloadType = codec.pltype;

        if (codecPayloadType == codec.pltype) {
          uint32_t ssrc = 0;
          if (encodingParamIter->mSSRC.hasValue())
            ssrc = encodingParamIter->mSSRC;
          if (encodingParamIter->mFEC.hasValue()) {
            IRTPTypes::FECParameters fec = encodingParamIter->mFEC;
            if (fec.mSSRC.hasValue())
              ssrc = fec.mSSRC;
          }
          config.rtp.remote_ssrc = ssrc;
        }
      }
      if (config.rtp.remote_ssrc == 0) {
        config.rtp.remote_ssrc = mInitPacket->ssrc();
        mInitPacket.reset();
      }

      for (auto headerExtensionIter = mParameters->mHeaderExtensions.begin(); headerExtensionIter != mParameters->mHeaderExtensions.end(); headerExtensionIter++) {
        IRTPTypes::HeaderExtensionURIs headerExtensionURI = IRTPTypes::toHeaderExtensionURI(headerExtensionIter->mURI);
        switch (headerExtensionURI) {
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_ClienttoMixerAudioLevelIndication:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetReceiveAudioLevelIndicationStatus(mChannel, true, headerExtensionIter->mID);
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_AbsoluteSendTime:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetReceiveAbsoluteSenderTimeStatus(mChannel, true, headerExtensionIter->mID);
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          default:
            break;
        }
      }

      uint32_t localSSRC = mParameters->mRTCP.mSSRC;
      if (localSSRC == 0)
        localSSRC = 1;
      webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetLocalSSRC(mChannel, localSSRC);
      config.rtp.local_ssrc = localSSRC;
      config.receive_transport = mTransport.get();
      config.rtcp_send_transport = mTransport.get();

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());
      mModuleProcessThread->RegisterModule(mCongestionController.get());
      mPacerThread->RegisterModule(mCongestionController->pacer());
      mPacerThread->RegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mPacerThread->Start();

      mReceiveStream = rtc::scoped_ptr<webrtc::AudioReceiveStream>(
        new webrtc::internal::AudioReceiveStream(
                                                 mCongestionController.get(),
                                                 config,
                                                 audioState
                                                 ));

      webrtc::VoENetwork::GetInterface(voiceEngine)->RegisterExternalTransport(mChannel, *mTransport);

      mTrack->start();

      if (mTransportState == ISecureTransport::State_Connected) {
        webrtc::VoEBase::GetInterface(voiceEngine)->StartReceive(mChannel);
        webrtc::VoEBase::GetInterface(voiceEngine)->StartPlayout(mChannel);
      }

      notifyPromisesResolve();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::stepShutdown()
    {
      mDenyNonLockedAccess = true;

      // rare race condition that can happen so
      while (mAccessFromNonLockedMethods > 0)
      {
        // NOTE: very temporary lock so should clear itself out fast
        std::this_thread::yield();
      }

      AutoRecursiveLock lock(*this);

      auto outer = mMediaEngine.lock();

      if (outer) {
        auto voiceEngine = outer->getVoiceEngine();
        if (voiceEngine) {
          if (mTransportState == ISecureTransport::State_Connected) {
            webrtc::VoEBase::GetInterface(voiceEngine)->StopPlayout(mChannel);
            webrtc::VoEBase::GetInterface(voiceEngine)->StopReceive(mChannel);
          }
          webrtc::VoENetwork::GetInterface(voiceEngine)->DeRegisterExternalTransport(mChannel);
        }
      }

#define FIX_ME_WARNING_NO_TRACK_IS_NOT_STOPPED_JUST_BECAUSE_A_RECEIVER_CHANNEL_IS_DONE 1
#define FIX_ME_WARNING_NO_TRACK_IS_NOT_STOPPED_JUST_BECAUSE_A_RECEIVER_CHANNEL_IS_DONE 2

      if (mTrack)
        mTrack->stop();

      mPacerThread->Stop();
      mPacerThread->DeRegisterModule(mCongestionController->pacer());
      mPacerThread->DeRegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mModuleProcessThread->DeRegisterModule(mCongestionController.get());
      mModuleProcessThread->DeRegisterModule(mCallStats.get());
      mModuleProcessThread->Stop();

      mCallStats->DeregisterStatsObserver(mCongestionController.get());

      mReceiveStream.reset();
      mCongestionController.reset();
      mCallStats.reset();
      mBitrateAllocator.reset();
      mModuleProcessThread.reset();
      mPacerThread.reset();

      notifyPromisesShutdown();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    int RTPMediaEngine::AudioReceiverChannelResource::getChannel() const
    {
      return mChannel;
    }

    //-------------------------------------------------------------------------
    webrtc::CodecInst RTPMediaEngine::AudioReceiverChannelResource::getAudioCodec(
                                                                                  webrtc::VoiceEngine *voiceEngine,
                                                                                  String payloadName
                                                                                  )
    {
      webrtc::CodecInst codec;
      int numOfCodecs = webrtc::VoECodec::GetInterface(voiceEngine)->NumOfCodecs();
      for (int i = 0; i < numOfCodecs; ++i) {
        webrtc::CodecInst currentCodec;
        webrtc::VoECodec::GetInterface(voiceEngine)->GetCodec(i, currentCodec);
        if (0 == String(currentCodec.plname).compareNoCase(payloadName)) {
          codec = currentCodec;
          break;
        }
      }

      return codec;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::AudioSenderChannelResource::AudioSenderChannelResource(
                                                                           const make_private &priv,
                                                                           IRTPMediaEngineRegistrationPtr registration,
                                                                           TransportPtr transport,
                                                                           MediaStreamTrackPtr track,
                                                                           ParametersPtr parameters
                                                                           ) :
      ChannelResource(priv, registration),
      mTransport(transport),
      mTrack(track),
      mParameters(parameters)
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::AudioSenderChannelResource::~AudioSenderChannelResource()
    {
      mThisWeak.reset();  // shared pointer to self is no longer valid
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::AudioSenderChannelResourcePtr RTPMediaEngine::AudioSenderChannelResource::create(
                                                                                                     IRTPMediaEngineRegistrationPtr registration,
                                                                                                     TransportPtr transport,
                                                                                                     MediaStreamTrackPtr track,
                                                                                                     ParametersPtr parameters
                                                                                                     )
    {
      auto pThis = make_shared<AudioSenderChannelResource>(
                                                           make_private{},
                                                           registration,
                                                           transport,
                                                           track,
                                                           parameters
                                                           );
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::init()
    {
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => IRTPMediaEngineChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => IRTPMediaEngineChannelResourceAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::onSecureTransportState(ISecureTransport::States state)
    {
      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      ISecureTransport::States previousState = mTransportState;
      mTransportState = state;
      if (mParameters && (mParameters->mEncodings.size() == 0 ||
        (mParameters->mEncodings.size() > 0 && mParameters->mEncodings.begin()->mActive))) {
        if (state == ISecureTransport::State_Connected) {
          webrtc::VoEBase::GetInterface(voiceEngine)->StartSend(mChannel);
        } else if (previousState == ISecureTransport::State_Connected) {
          webrtc::VoEBase::GetInterface(voiceEngine)->StopSend(mChannel);
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::onUpdate(ParametersPtr params)
    {
      AutoRecursiveLock lock(*this);

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      ParametersPtr previousParams = mParameters;
      mParameters = params;
      bool previousActive = false;
      bool currentActive = false;
      if (previousParams->mEncodings.size() == 0 ||
        (previousParams->mEncodings.size() > 0 && previousParams->mEncodings.begin()->mActive)) {
        previousActive = true;
      }
      if (params->mEncodings.size() == 0 ||
        (params->mEncodings.size() > 0 && params->mEncodings.begin()->mActive)) {
        currentActive = true;
      }
      if (mTransportState == ISecureTransport::State_Connected) {
        if (!previousActive && currentActive) {
          webrtc::VoEBase::GetInterface(voiceEngine)->StartSend(mChannel);
        } else if (previousActive && !currentActive) {
          webrtc::VoEBase::GetInterface(voiceEngine)->StopSend(mChannel);
        }
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::onProvideStats(PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      AutoRecursiveLock lock(*this);

      UseStatsReport::StatMap reportStats;

      webrtc::AudioSendStream::Stats sendStreamStats = mSendStream->GetStats();

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_OutboundRTP)) {

        auto report = make_shared<IStatsReport::OutboundRTPStreamStats>();

        report->mID = string(sendStreamStats.local_ssrc) + "_send";

        report->mSSRC = sendStreamStats.local_ssrc;
        report->mIsRemote = false;
        report->mMediaType = "audio";
        report->mMediaTrackID = mTrack->id();
        report->mCodecID = mCodecPayloadName;
        report->mPacketsSent = sendStreamStats.packets_sent;
        report->mBytesSent = sendStreamStats.bytes_sent;
        report->mTargetBitrate = (DOUBLE)mCurrentTargetBitrate;
        report->mRoundTripTime = (DOUBLE)mCallStats->rtcp_rtt_stats()->LastProcessedRtt();

        reportStats[report->mID] = report;
      }

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_Codec)) {
        auto report = make_shared<IStatsReport::Codec>();

        report->mID = string(sendStreamStats.local_ssrc) + "_send_codec";

        report->mPayloadType = mCodecPayloadType;
        report->mCodec = mCodecPayloadName;
        report->mClockRate = 0;
        report->mChannels = 0;

        reportStats[report->mID] = report;
      }

      promise->resolve(UseStatsReport::create(reportStats));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => IRTPMediaEngineAudioSenderChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::AudioSenderChannelResource::handlePacket(const RTCPPacket &packet)
    {
      IRTPMediaEngineHandlePacketAsyncDelegateProxy::createUsingQueue(mHandlePacketQueue, getThis<AudioSenderChannelResource>())->onHandleRTCPPacket(packet.buffer());
      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => IRTPMediaEngineHandlePacketAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::onHandleRTCPPacket(SecureByteBlockPtr buffer)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);

      if (mDenyNonLockedAccess) return;

      auto stream = mSendStream.get();
      if (NULL == stream) return;

      bool result = stream->DeliverRtcp(buffer->BytePtr(), buffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => webrtc::BitrateObserver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs)
    {
      AutoRecursiveLock lock(*this);

      mCurrentTargetBitrate = targetBitrateBps;

      uint32_t allocatedBitrateBps = mBitrateAllocator->OnNetworkChanged(
                                                                         targetBitrateBps,
                                                                         fractionLoss,
                                                                         rttMs
                                                                         );

      int padUpToBitrateBps = 0;
      uint32_t pacerBitrateBps = std::max(targetBitrateBps, allocatedBitrateBps);

      if (mCongestionController)
        mCongestionController->UpdatePacerBitrate(
                                                  targetBitrateBps / 1000,
                                                  webrtc::PacedSender::kDefaultPaceMultiplier * pacerBitrateBps / 1000,
                                                  padUpToBitrateBps / 1000
                                                  );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => friend RTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::stepSetup()
    {
      AutoRecursiveLock lock(*this);

      auto engine = mMediaEngine.lock();
      if (!engine) {
        notifyPromisesReject();
        return;
      }

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) {
        notifyPromisesReject();
        return;
      }

      auto audioState = engine->getAudioState();

      mModuleProcessThread = webrtc::ProcessThread::Create("AudioSenderChannelResourceModuleProcessThread");
      mPacerThread = webrtc::ProcessThread::Create("AudioSenderChannelResourcePacerThread");

      mBitrateAllocator = rtc::scoped_ptr<webrtc::BitrateAllocator>(new webrtc::BitrateAllocator());
      mCallStats = rtc::scoped_ptr<webrtc::CallStats>(new webrtc::CallStats(mClock));
      mCongestionController =
        rtc::scoped_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mClock,
                                                                                       this,
                                                                                       &mRemb
                                                                                       ));

      mCallStats->RegisterStatsObserver(mCongestionController.get());

      mChannel = webrtc::VoEBase::GetInterface(voiceEngine)->CreateChannel();

      bool audioCodecSet = false;
      webrtc::CodecInst codec;
      for (auto codecIter = mParameters->mCodecs.begin(); codecIter != mParameters->mCodecs.end(); codecIter++) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::getCodecKind(supportedCodec) == IRTPTypes::CodecKind_Audio && audioCodecSet)
          continue;
        codec = getAudioCodec(voiceEngine, codecIter->mName);
        codec.pltype = codecIter->mPayloadType;
        if (codecIter->mPTime != Milliseconds::zero())
          codec.pacsize = (codec.plfreq / 1000) * codecIter->mPTime.count();
        if (codecIter->mNumChannels.hasValue())
          codec.channels = codecIter->mNumChannels;
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_Opus:
          {
            codec.rate = 32000 * codec.channels;
            webrtc::VoECodec::GetInterface(voiceEngine)->SetSendCodec(mChannel, codec);
            auto parameters = IRTPTypes::OpusCodecParameters::convert(codecIter->mParameters);
            if (parameters->mUseInbandFEC.hasValue())
              webrtc::VoECodec::GetInterface(voiceEngine)->SetFECStatus(mChannel, parameters->mUseInbandFEC);
            if (parameters->mUseDTX.hasValue())
              webrtc::VoECodec::GetInterface(voiceEngine)->SetOpusDtx(mChannel, parameters->mUseDTX);
            webrtc::VoECodec::GetInterface(voiceEngine)->SetOpusMaxPlaybackRate(mChannel, 48000);
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_Isac:
          case IRTPTypes::SupportedCodec_G722:
          case IRTPTypes::SupportedCodec_ILBC:
          case IRTPTypes::SupportedCodec_PCMU:
          case IRTPTypes::SupportedCodec_PCMA:
            webrtc::VoECodec::GetInterface(voiceEngine)->SetSendCodec(mChannel, codec);
            goto set_rtcp_feedback;
          case IRTPTypes::SupportedCodec_RED:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetREDStatus(mChannel, true, codecIter->mPayloadType);
            break;
        }
        continue;

      set_rtcp_feedback:
        for (auto rtcpFeedbackIter = codecIter->mRTCPFeedback.begin(); rtcpFeedbackIter != codecIter->mRTCPFeedback.end(); rtcpFeedbackIter++) {
          IRTPTypes::KnownFeedbackTypes feedbackType = IRTPTypes::toKnownFeedbackType(rtcpFeedbackIter->mType);
          IRTPTypes::KnownFeedbackParameters feedbackParameter = IRTPTypes::toKnownFeedbackParameter(rtcpFeedbackIter->mParameter);
          if (IRTPTypes::KnownFeedbackType_NACK == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetNACKStatus(mChannel, true, 250);
          }
        }
        audioCodecSet = true;
      }

      mCodecPayloadName = codec.plname;
      mCodecPayloadType = codec.pltype;

      webrtc::AudioSendStream::Config config(mTransport.get());
      config.voe_channel_id = mChannel;
      
      for (auto encodingParamIter = mParameters->mEncodings.begin(); encodingParamIter != mParameters->mEncodings.end(); encodingParamIter++) {

        IRTPTypes::PayloadType codecPayloadType {};
        if (encodingParamIter->mCodecPayloadType.hasValue())
          codecPayloadType = encodingParamIter->mCodecPayloadType;
        else
          codecPayloadType = codec.pltype;

        if (codecPayloadType == codec.pltype) {
          uint32_t ssrc = 0;
          if (encodingParamIter->mSSRC.hasValue()) {
            ssrc = encodingParamIter->mSSRC;
          }
          if (encodingParamIter->mFEC.hasValue()) {
            IRTPTypes::FECParameters fec = encodingParamIter->mFEC;
            if (fec.mSSRC.hasValue()) {
              ssrc = fec.mSSRC;
            }
          }
          webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetLocalSSRC(mChannel, ssrc);
          config.rtp.ssrc = ssrc;
        }
      }
      if (config.rtp.ssrc == 0) {
        uint32_t ssrc = SafeInt<uint32>(openpeer::services::IHelper::random(1, 0xFFFFFFFF));
        webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetLocalSSRC(mChannel, ssrc);
        config.rtp.ssrc = ssrc;
      }

      for (auto headerExtensionIter = mParameters->mHeaderExtensions.begin(); headerExtensionIter != mParameters->mHeaderExtensions.end(); headerExtensionIter++) {
        IRTPTypes::HeaderExtensionURIs headerExtensionURI = IRTPTypes::toHeaderExtensionURI(headerExtensionIter->mURI);
        switch (headerExtensionURI) {
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_ClienttoMixerAudioLevelIndication:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetSendAudioLevelIndicationStatus(mChannel, true, headerExtensionIter->mID);
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_AbsoluteSendTime:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetSendAbsoluteSenderTimeStatus(mChannel, true, headerExtensionIter->mID);
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          default:
            break;
        }
      }

      webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetRTCPStatus(mChannel, true);
      webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetRTCP_CNAME(mChannel, mParameters->mRTCP.mCName);

      mCongestionController->SetBweBitrates(10000, 40000, 100000);

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());
      mModuleProcessThread->RegisterModule(mCongestionController.get());
      mPacerThread->RegisterModule(mCongestionController->pacer());
      mPacerThread->RegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mPacerThread->Start();

      mSendStream = rtc::scoped_ptr<webrtc::AudioSendStream>(
        new webrtc::internal::AudioSendStream(
                                              config,
                                              audioState,
                                              mCongestionController.get()
                                              ));

      webrtc::VoENetwork::GetInterface(voiceEngine)->RegisterExternalTransport(mChannel, *mTransport);

      mTrack->start();

      if (mTransportState == ISecureTransport::State_Connected)
        webrtc::VoEBase::GetInterface(voiceEngine)->StartSend(mChannel);

      notifyPromisesResolve();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::stepShutdown()
    {
      mDenyNonLockedAccess = true;

      // rare race condition that can happen so
      while (mAccessFromNonLockedMethods > 0)
      {
        // NOTE: very temporary lock so should clear itself out fast
        std::this_thread::yield();
      }

      AutoRecursiveLock lock(*this);

      auto engine = mMediaEngine.lock();
      if (engine) {
        auto voiceEngine = engine->getVoiceEngine();
        if (voiceEngine) {
          if (mTransportState == ISecureTransport::State_Connected)
            webrtc::VoEBase::GetInterface(voiceEngine)->StopSend(mChannel);
          webrtc::VoENetwork::GetInterface(voiceEngine)->DeRegisterExternalTransport(mChannel);
        }
      }

#define FIX_ME_WARNING_NO_TRACK_IS_NOT_STOPPED_JUST_BECAUSE_A_RECEIVER_CHANNEL_IS_DONE 1
#define FIX_ME_WARNING_NO_TRACK_IS_NOT_STOPPED_JUST_BECAUSE_A_RECEIVER_CHANNEL_IS_DONE 2

      if (mTrack)
        mTrack->stop();

      mPacerThread->Stop();
      mPacerThread->DeRegisterModule(mCongestionController->pacer());
      mPacerThread->DeRegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mModuleProcessThread->DeRegisterModule(mCongestionController.get());
      mModuleProcessThread->DeRegisterModule(mCallStats.get());
      mModuleProcessThread->Stop();

      mCallStats->DeregisterStatsObserver(mCongestionController.get());

      mSendStream.reset();
      mCongestionController.reset();
      mCallStats.reset();
      mBitrateAllocator.reset();
      mModuleProcessThread.reset();
      mPacerThread.reset();

      notifyPromisesShutdown();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    webrtc::CodecInst RTPMediaEngine::AudioSenderChannelResource::getAudioCodec(
                                                                                webrtc::VoiceEngine *voiceEngine,
                                                                                String payloadName
                                                                                )
    {
      webrtc::CodecInst codec;
      int numOfCodecs = webrtc::VoECodec::GetInterface(voiceEngine)->NumOfCodecs();
      for (int i = 0; i < numOfCodecs; ++i) {
        webrtc::CodecInst currentCodec;
        webrtc::VoECodec::GetInterface(voiceEngine)->GetCodec(i, currentCodec);
        if (0 == String(currentCodec.plname).compareNoCase(payloadName)) {
          codec = currentCodec;
          break;
        }
      }
      return codec;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource::ReceiverVideoRenderer
    #pragma mark

    //---------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::ReceiverVideoRenderer::setMediaStreamTrack(UseMediaStreamTrackPtr videoTrack)
    {
      mVideoTrack = videoTrack;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::ReceiverVideoRenderer::RenderFrame(const webrtc::VideoFrame& video_frame, int time_to_render_ms)
    {
      mVideoTrack->renderVideoFrame(video_frame);
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::VideoReceiverChannelResource::ReceiverVideoRenderer::IsTextureSupported() const
    {
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::VideoReceiverChannelResource::VideoReceiverChannelResource(
                                                                               const make_private &priv,
                                                                               IRTPMediaEngineRegistrationPtr registration,
                                                                               TransportPtr transport,
                                                                               MediaStreamTrackPtr track,
                                                                               ParametersPtr parameters,
                                                                               RTPPacketPtr packet
                                                                               ) :
      ChannelResource(priv, registration),
      mTransport(transport),
      mTrack(track),
      mParameters(parameters),
      mInitPacket(packet)
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::VideoReceiverChannelResource::~VideoReceiverChannelResource()
    {
      mThisWeak.reset();  // shared pointer to self is no longer valid
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::VideoReceiverChannelResourcePtr RTPMediaEngine::VideoReceiverChannelResource::create(
                                                                                                         IRTPMediaEngineRegistrationPtr registration,
                                                                                                         TransportPtr transport,
                                                                                                         MediaStreamTrackPtr track,
                                                                                                         ParametersPtr parameters,
                                                                                                         RTPPacketPtr packet
                                                                                                         )
    {
      auto pThis = make_shared<VideoReceiverChannelResource>(
                                                             make_private{},
                                                             registration,
                                                             transport,
                                                             track,
                                                             parameters,
                                                             packet
                                                             );
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::init()
    {
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource => IRTPMediaEngineChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource => IRTPMediaEngineChannelResourceAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::onSecureTransportState(ISecureTransport::States state)
    {
      AutoRecursiveLock lock(*this);

      if (state == mTransportState) return;

      ISecureTransport::States previousState = mTransportState;
      mTransportState = state;

      if (!mReceiveStream) return;

      if (mParameters && (mParameters->mEncodings.size() == 0 ||
        (mParameters->mEncodings.size() > 0 && mParameters->mEncodings.begin()->mActive))) {
        if (state == ISecureTransport::State_Connected)
          mReceiveStream->Start();
        else if (previousState == ISecureTransport::State_Connected)
          mReceiveStream->Stop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::onUpdate(ParametersPtr params)
    {
      AutoRecursiveLock lock(*this);

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      ParametersPtr previousParams = mParameters;
      mParameters = params;

      if (!mReceiveStream) return;

      bool previousActive = false;
      bool currentActive = false;
      if (previousParams->mEncodings.size() == 0 ||
        (previousParams->mEncodings.size() > 0 && previousParams->mEncodings.begin()->mActive)) {
        previousActive = true;
      }
      if (params->mEncodings.size() == 0 ||
        (params->mEncodings.size() > 0 && params->mEncodings.begin()->mActive)) {
        currentActive = true;
      }
      if (mTransportState == ISecureTransport::State_Connected) {
        if (!previousActive && currentActive)
          mReceiveStream->Start();
        else if (previousActive && !currentActive)
          mReceiveStream->Stop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::onProvideStats(PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      AutoRecursiveLock lock(*this);

      UseStatsReport::StatMap reportStats;

      webrtc::VideoReceiveStream::Stats receiveStreamStats = mReceiveStream->GetStats();

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_InboundRTP)) {

        auto report = make_shared<IStatsReport::InboundRTPStreamStats>();

        report->mID = string(receiveStreamStats.ssrc) + "_recv";

        report->mSSRC = receiveStreamStats.ssrc;
        report->mIsRemote = true;
        report->mMediaType = "video";
        report->mMediaTrackID = mTrack->id();
        report->mCodecID = mCodecPayloadName;
        report->mFIRCount = receiveStreamStats.rtcp_packet_type_counts.fir_packets;
        report->mPLICount = receiveStreamStats.rtcp_packet_type_counts.pli_packets;
        report->mNACKCount = receiveStreamStats.rtcp_packet_type_counts.nack_packets;
        report->mPacketsReceived = receiveStreamStats.rtp_stats.transmitted.packets;
        report->mBytesReceived = receiveStreamStats.rtp_stats.transmitted.header_bytes +
          receiveStreamStats.rtp_stats.transmitted.payload_bytes +
          receiveStreamStats.rtp_stats.transmitted.padding_bytes;
        report->mPacketsLost = receiveStreamStats.rtp_stats.retransmitted.packets;
        report->mJitter = receiveStreamStats.rtcp_stats.jitter;
        report->mFractionLost = receiveStreamStats.rtcp_stats.fraction_lost;
        report->mEndToEndDelay = Milliseconds(receiveStreamStats.current_endtoend_delay_ms);

        reportStats[report->mID] = report;
      }

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_Codec)) {

        auto report = make_shared<IStatsReport::Codec>();

        report->mID = string(receiveStreamStats.ssrc) + "_recv_codec";

        report->mPayloadType = mCodecPayloadType;
        report->mCodec = mCodecPayloadName;
        report->mClockRate = 0;
        report->mChannels = 0;

        reportStats[report->mID] = report;
      }

      promise->resolve(UseStatsReport::create(reportStats));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource => IRTPMediaEngineVideoReceiverChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::VideoReceiverChannelResource::handlePacket(const RTPPacket &packet)
    {
      IRTPMediaEngineHandlePacketAsyncDelegateProxy::createUsingQueue(mHandlePacketQueue, getThis<VideoReceiverChannelResource>())->onHandleRTPPacket(packet.timestamp(), packet.buffer());
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::VideoReceiverChannelResource::handlePacket(const RTCPPacket &packet)
    {
      IRTPMediaEngineHandlePacketAsyncDelegateProxy::createUsingQueue(mHandlePacketQueue, getThis<VideoReceiverChannelResource>())->onHandleRTCPPacket(packet.buffer());
      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource => IRTPMediaEngineHandlePacketAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::onHandleRTPPacket(DWORD timestamp, SecureByteBlockPtr buffer)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);

      if (mDenyNonLockedAccess) return;

      auto stream = mReceiveStream.get();
      if (NULL == stream) return;

      webrtc::PacketTime time(timestamp, 0);
      bool result = stream->DeliverRtp(buffer->BytePtr(), buffer->SizeInBytes(), time);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::onHandleRTCPPacket(SecureByteBlockPtr buffer)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);

      if (mDenyNonLockedAccess) return;

      auto stream = mReceiveStream.get();
      if (NULL == stream) return;

      bool result = stream->DeliverRtcp(buffer->BytePtr(), buffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource => webrtc::BitrateObserver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs)
    {
      AutoRecursiveLock lock(*this);

      mCurrentTargetBitrate = targetBitrateBps;

      uint32_t allocatedBitrateBps = mBitrateAllocator->OnNetworkChanged(
                                                                         targetBitrateBps,
                                                                         fractionLoss,
                                                                         rttMs
                                                                         );

      int padUpToBitrateBps = 0;
      uint32_t pacerBitrateBps = std::max(targetBitrateBps, allocatedBitrateBps);

      if (mCongestionController)
        mCongestionController->UpdatePacerBitrate(
                                                  targetBitrateBps / 1000,
                                                  webrtc::PacedSender::kDefaultPaceMultiplier * pacerBitrateBps / 1000,
                                                  padUpToBitrateBps / 1000
                                                  );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource => friend RTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::stepSetup()
    {
      AutoRecursiveLock lock(*this);

      mModuleProcessThread = webrtc::ProcessThread::Create("VideoReceiverChannelResourceModuleProcessThread");
      mPacerThread = webrtc::ProcessThread::Create("VideoReceiverChannelResourcePacerThread");

      mReceiverVideoRenderer.setMediaStreamTrack(mTrack);

      mBitrateAllocator = rtc::scoped_ptr<webrtc::BitrateAllocator>(new webrtc::BitrateAllocator());
      mCallStats = rtc::scoped_ptr<webrtc::CallStats>(new webrtc::CallStats(mClock));
      mCongestionController =
        rtc::scoped_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mClock,
                                                                                       this,
                                                                                       &mRemb
                                                                                       ));

      mCallStats->RegisterStatsObserver(mCongestionController.get());

      int numCpuCores = webrtc::CpuInfo::DetectNumberOfCores();

      webrtc::Transport* transport = mTransport.get();
      webrtc::VideoReceiveStream::Config config(transport);
      webrtc::VideoReceiveStream::Decoder decoder;
      std::vector<IRTPTypes::PayloadType> rtxPayloadTypes;

      bool videoCodecSet = false;
      for (auto codecIter = mParameters->mCodecs.begin(); codecIter != mParameters->mCodecs.end(); ++codecIter) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::getCodecKind(supportedCodec) == IRTPTypes::CodecKind_Video && videoCodecSet)
          continue;
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_VP8:
          {
            webrtc::VideoDecoder* videoDecoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::kVp8);
            decoder.decoder = videoDecoder;
            decoder.payload_name = IRTPTypes::toString(supportedCodec);
            decoder.payload_type = codecIter->mPayloadType;
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_VP9:
          {
            webrtc::VideoDecoder* videoDecoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::kVp9);
            decoder.decoder = videoDecoder;
            decoder.payload_name = IRTPTypes::toString(supportedCodec);
            decoder.payload_type = codecIter->mPayloadType;
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_H264:
          {
#ifndef WINRT
            webrtc::VideoDecoder* videoDecoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::kH264);
#else
            std::unique_ptr<cricket::WebRtcVideoDecoderFactory> decoderFactory = std::make_unique<webrtc::H264WinRTDecoderFactory>();
            webrtc::VideoDecoder* videoDecoder = decoderFactory->CreateVideoDecoder(webrtc::VideoCodecType::kVideoCodecH264);
#endif
            decoder.decoder = videoDecoder;
            decoder.payload_name = IRTPTypes::toString(supportedCodec);
            decoder.payload_type = codecIter->mPayloadType;
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_RTX:
          {
            rtxPayloadTypes.push_back(codecIter->mPayloadType);
            break;
          }
          case IRTPTypes::SupportedCodec_RED:
          {
            config.rtp.fec.red_payload_type = codecIter->mPayloadType;
            break;
          }
          case IRTPTypes::SupportedCodec_ULPFEC:
          {
            config.rtp.fec.ulpfec_payload_type = codecIter->mPayloadType;
            break;
          }
          case IRTPTypes::SupportedCodec_FlexFEC:
          {
            break;
          }
        }
        continue;

      set_rtcp_feedback:
        for (auto rtcpFeedbackIter = codecIter->mRTCPFeedback.begin(); rtcpFeedbackIter != codecIter->mRTCPFeedback.end(); rtcpFeedbackIter++) {
          IRTPTypes::KnownFeedbackTypes feedbackType = IRTPTypes::toKnownFeedbackType(rtcpFeedbackIter->mType);
          IRTPTypes::KnownFeedbackParameters feedbackParameter = IRTPTypes::toKnownFeedbackParameter(rtcpFeedbackIter->mParameter);
          if (IRTPTypes::KnownFeedbackType_NACK == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            config.rtp.nack.rtp_history_ms = 1000;
          } else if (IRTPTypes::KnownFeedbackType_REMB == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            config.rtp.remb = true;
          }
        }
        videoCodecSet = true;
      }

      mCodecPayloadName = decoder.payload_name;
      mCodecPayloadType = decoder.payload_type;

      for (auto encodingParamIter = mParameters->mEncodings.begin(); encodingParamIter != mParameters->mEncodings.end(); encodingParamIter++) {

        IRTPTypes::PayloadType codecPayloadType {};
        if (encodingParamIter->mCodecPayloadType.hasValue())
          codecPayloadType = encodingParamIter->mCodecPayloadType;
        else
          codecPayloadType = decoder.payload_type;

        if (codecPayloadType == decoder.payload_type) {
          uint32_t ssrc = 0;
          if (encodingParamIter->mSSRC.hasValue())
            ssrc = encodingParamIter->mSSRC;
          if (encodingParamIter->mFEC.hasValue()) {
            IRTPTypes::FECParameters fec = encodingParamIter->mFEC;
            if (fec.mSSRC.hasValue()) {
              ssrc = encodingParamIter->mSSRC;
            }
          }
          config.rtp.remote_ssrc = ssrc;
          if (encodingParamIter->mRTX.hasValue()) {
            IRTPTypes::RTXParameters rtxEncodingParam = encodingParamIter->mRTX;
            webrtc::VideoReceiveStream::Config::Rtp::Rtx rtx;
            rtx.payload_type = rtxPayloadTypes.front();
            if (rtxEncodingParam.mSSRC.hasValue())
              rtx.ssrc = rtxEncodingParam.mSSRC;
            config.rtp.rtx[codecPayloadType] = rtx;
          }
        }
      }
      if (config.rtp.remote_ssrc == 0) {
        config.rtp.remote_ssrc = mInitPacket->ssrc();
        mInitPacket.reset();
      }
      uint32_t localSSRC = mParameters->mRTCP.mSSRC;
      if (localSSRC == 0)
        localSSRC = 1;
      config.rtp.local_ssrc = localSSRC;

      for (auto headerExtensionIter = mParameters->mHeaderExtensions.begin(); headerExtensionIter != mParameters->mHeaderExtensions.end(); headerExtensionIter++) {
        IRTPTypes::HeaderExtensionURIs headerExtensionURI = IRTPTypes::toHeaderExtensionURI(headerExtensionIter->mURI);
        switch (headerExtensionURI) {
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_TransmissionTimeOffsets:
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_AbsoluteSendTime:
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_3gpp_VideoOrientation:
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_TransportSequenceNumber:
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          default:
            break;
        }
      }

      if (mParameters->mRTCP.mReducedSize)
        config.rtp.rtcp_mode = webrtc::RtcpMode::kReducedSize;
      config.decoders.push_back(decoder);
      config.renderer = &mReceiverVideoRenderer;

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());
      mModuleProcessThread->RegisterModule(mCongestionController.get());
      mPacerThread->RegisterModule(mCongestionController->pacer());
      mPacerThread->RegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mPacerThread->Start();

      mReceiveStream = rtc::scoped_ptr<webrtc::VideoReceiveStream>(
        new webrtc::internal::VideoReceiveStream(
                                                 numCpuCores,
                                                 mCongestionController.get(),
                                                 config,
                                                 NULL,
                                                 mModuleProcessThread.get(),
                                                 mCallStats.get(),
                                                 &mRemb
                                                 ));

      if (mTransportState == ISecureTransport::State_Connected)
        mReceiveStream->Start();

      notifyPromisesResolve();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::stepShutdown()
    {
      mDenyNonLockedAccess = true;

      // rare race condition that can happen so
      while (mAccessFromNonLockedMethods > 0)
      {
        // NOTE: very temporary lock so should clear itself out fast
        std::this_thread::yield();
      }

      AutoRecursiveLock lock(*this);

      if (mReceiveStream && mTransportState == ISecureTransport::State_Connected)
        mReceiveStream->Stop();

      mPacerThread->Stop();
      mPacerThread->DeRegisterModule(mCongestionController->pacer());
      mPacerThread->DeRegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mModuleProcessThread->DeRegisterModule(mCongestionController.get());
      mModuleProcessThread->DeRegisterModule(mCallStats.get());
      mModuleProcessThread->Stop();

      mCallStats->DeregisterStatsObserver(mCongestionController.get());

      mReceiveStream.reset();
      mCongestionController.reset();
      mCallStats.reset();
      mBitrateAllocator.reset();
      mModuleProcessThread.reset();
      mPacerThread.reset();

      notifyPromisesShutdown();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::VideoSenderChannelResource::VideoSenderChannelResource(
                                                                           const make_private &priv,
                                                                           IRTPMediaEngineRegistrationPtr registration,
                                                                           TransportPtr transport,
                                                                           MediaStreamTrackPtr track,
                                                                           ParametersPtr parameters
                                                                           ) :
      ChannelResource(priv, registration),
      mTransport(transport),
      mTrack(track),
      mParameters(parameters)
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::VideoSenderChannelResource::~VideoSenderChannelResource()
    {
      mThisWeak.reset();  // shared pointer to self is no longer valid
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::VideoSenderChannelResourcePtr RTPMediaEngine::VideoSenderChannelResource::create(
                                                                                                     IRTPMediaEngineRegistrationPtr registration,
                                                                                                     TransportPtr transport,
                                                                                                     MediaStreamTrackPtr track,
                                                                                                     ParametersPtr parameters
                                                                                                     )
    {
      auto pThis = make_shared<VideoSenderChannelResource>(
                                                           make_private{},
                                                           registration,
                                                           transport,
                                                           track,
                                                           parameters
                                                           );
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::init()
    {
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource => IRTPMediaEngineChannelResource
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource => IRTPMediaEngineChannelResourceAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::onSecureTransportState(ISecureTransport::States state)
    {
      AutoRecursiveLock lock(*this);

      if (state == mTransportState) return;

      ISecureTransport::States previousState = mTransportState;
      mTransportState = state;
      
      if (!mSendStream) return;

      if (mParameters && (mParameters->mEncodings.size() == 0 ||
        (mParameters->mEncodings.size() > 0 && mParameters->mEncodings.begin()->mActive))) {
        if (state == ISecureTransport::State_Connected)
          mSendStream->Start();
        else if (previousState == ISecureTransport::State_Connected)
          mSendStream->Stop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::onUpdate(ParametersPtr params)
    {
      AutoRecursiveLock lock(*this);

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      ParametersPtr previousParams = mParameters;
      mParameters = params;

      if (!mSendStream) return;

      bool previousActive = false;
      bool currentActive = false;
      if (previousParams->mEncodings.size() == 0 ||
        (previousParams->mEncodings.size() > 0 && previousParams->mEncodings.begin()->mActive)) {
        previousActive = true;
      }
      if (params->mEncodings.size() == 0 ||
        (params->mEncodings.size() > 0 && params->mEncodings.begin()->mActive)) {
        currentActive = true;
      }
      if (mTransportState == ISecureTransport::State_Connected) {
        if (!previousActive && currentActive)
          mSendStream->Start();
        else if (previousActive && !currentActive)
          mSendStream->Stop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::onProvideStats(PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      AutoRecursiveLock lock(*this);

      UseStatsReport::StatMap reportStats;

      webrtc::VideoSendStream::Stats sendStreamStats = mSendStream->GetStats();

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_OutboundRTP)) {

        for (auto statsIter = sendStreamStats.substreams.begin(); statsIter != sendStreamStats.substreams.end(); statsIter++) {

          auto report = make_shared<IStatsReport::OutboundRTPStreamStats>();

          report->mID = string((*statsIter).first) + "_send";

          report->mSSRC = (*statsIter).first;
          report->mIsRemote = false;
          report->mMediaType = "video";
          report->mMediaTrackID = mTrack->id();
          report->mCodecID = mCodecPayloadName;
          report->mFIRCount = (*statsIter).second.rtcp_packet_type_counts.fir_packets;
          report->mPLICount = (*statsIter).second.rtcp_packet_type_counts.pli_packets;
          report->mNACKCount = (*statsIter).second.rtcp_packet_type_counts.nack_packets;
          report->mPacketsSent = (*statsIter).second.rtp_stats.transmitted.packets;
          report->mBytesSent = (*statsIter).second.rtp_stats.transmitted.header_bytes +
            (*statsIter).second.rtp_stats.transmitted.payload_bytes +
            (*statsIter).second.rtp_stats.transmitted.padding_bytes;
          report->mTargetBitrate = (DOUBLE)mCurrentTargetBitrate;
          report->mRoundTripTime = (DOUBLE)mCallStats->rtcp_rtt_stats()->LastProcessedRtt();

          reportStats[report->mID] = report;
        }
      }

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_Codec)) {
        auto report = make_shared<IStatsReport::Codec>();

        report->mID = string((*sendStreamStats.substreams.begin()).first) + "_send_codec";

        report->mPayloadType = mCodecPayloadType;
        report->mCodec = mCodecPayloadName;
        report->mClockRate = 0;
        report->mChannels = 0;

        reportStats[report->mID] = report;
      }

      promise->resolve(UseStatsReport::create(reportStats));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource => IRTPMediaEngineHandlePacketAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::VideoSenderChannelResource::handlePacket(const RTCPPacket &packet)
    {
      IRTPMediaEngineHandlePacketAsyncDelegateProxy::createUsingQueue(mHandlePacketQueue, getThis<VideoSenderChannelResource>())->onHandleRTCPPacket(packet.buffer());
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::sendVideoFrame(const webrtc::VideoFrame& videoFrame)
    {
      auto frameCopy = make_shared<webrtc::VideoFrame>();
      frameCopy->ShallowCopy(videoFrame);

      IRTPMediaEngineHandlePacketAsyncDelegateProxy::createUsingQueue(mHandlePacketQueue, getThis<VideoSenderChannelResource>())->onSendVideoFrame(frameCopy);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource => IRTPMediaEngineHandlePacketAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::onHandleRTCPPacket(SecureByteBlockPtr buffer)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);

      if (mDenyNonLockedAccess) return;

      auto stream = mSendStream.get();
      if (NULL == stream) return;

      bool result = stream->DeliverRtcp(buffer->BytePtr(), buffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::onSendVideoFrame(VideoFramePtr videoFrame)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);

      if (mDenyNonLockedAccess) return;

      auto stream = mSendStream.get();
      if (NULL == stream) return;

      stream->Input()->IncomingCapturedFrame(*videoFrame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource => webrtc::BitrateObserver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs)
    {
      AutoRecursiveLock lock(*this);

      mCurrentTargetBitrate = targetBitrateBps;

      uint32_t allocatedBitrateBps = mBitrateAllocator->OnNetworkChanged(
                                                                         targetBitrateBps,
                                                                         fractionLoss,
                                                                         rttMs
                                                                         );

      int padUpToBitrateBps = 0;
      if (mSendStream)
        padUpToBitrateBps = static_cast<webrtc::internal::VideoSendStream *>(mSendStream.get())->GetPaddingNeededBps();
      uint32_t pacerBitrateBps = std::max(targetBitrateBps, allocatedBitrateBps);

      if (mCongestionController)
        mCongestionController->UpdatePacerBitrate(
                                                  targetBitrateBps / 1000,
                                                  webrtc::PacedSender::kDefaultPaceMultiplier * pacerBitrateBps / 1000,
                                                  padUpToBitrateBps / 1000
                                                  );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource => friend RTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::stepSetup()
    {
      AutoRecursiveLock lock(*this);

      mModuleProcessThread = webrtc::ProcessThread::Create("VideoSenderChannelResourceModuleProcessThread");
      mPacerThread = webrtc::ProcessThread::Create("VideoSenderChannelResourcePacerThread");

      mBitrateAllocator = rtc::scoped_ptr<webrtc::BitrateAllocator>(new webrtc::BitrateAllocator());
      mCallStats = rtc::scoped_ptr<webrtc::CallStats>(new webrtc::CallStats(mClock));
      mCongestionController =
        rtc::scoped_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mClock,
                                                                                       this,
                                                                                       &mRemb
                                                                                       ));

      mCallStats->RegisterStatsObserver(mCongestionController.get());

      int numCpuCores = webrtc::CpuInfo::DetectNumberOfCores();

      size_t sourceWidth = 640;
      size_t sourceHeight = 480;
      int sourceMaxFramerate = 15;
      IMediaStreamTrack::SettingsPtr trackSettings = mTrack->getSettings();
      if (trackSettings->mWidth.hasValue())
        sourceWidth = trackSettings->mWidth.value();
      if (trackSettings->mHeight.hasValue())
        sourceHeight = trackSettings->mHeight.value();
      if (trackSettings->mFrameRate.hasValue())
        sourceMaxFramerate = trackSettings->mFrameRate.value();

      webrtc::VideoSendStream::Config config(mTransport.get());
      webrtc::VideoEncoderConfig encoderConfig;
      std::map<uint32_t, webrtc::RtpState> suspendedSSRCs;

      encoderConfig.min_transmit_bitrate_bps = 0;
      encoderConfig.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;

      bool videoCodecSet = false;
      for (auto codecIter = mParameters->mCodecs.begin(); codecIter != mParameters->mCodecs.end(); codecIter++) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::getCodecKind(supportedCodec) == IRTPTypes::CodecKind_Video && videoCodecSet)
          continue;
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_VP8:
          {
            webrtc::VideoEncoder* videoEncoder = webrtc::VideoEncoder::Create(webrtc::VideoEncoder::kVp8);
            config.encoder_settings.encoder = videoEncoder;
            config.encoder_settings.payload_name = IRTPTypes::toString(supportedCodec);
            config.encoder_settings.payload_type = codecIter->mPayloadType;
            mVideoEncoderSettings.mVp8 = webrtc::VideoEncoder::GetDefaultVp8Settings();
            mVideoEncoderSettings.mVp8.automaticResizeOn = true;
            mVideoEncoderSettings.mVp8.denoisingOn = true;
            mVideoEncoderSettings.mVp8.frameDroppingOn = true;
            encoderConfig.encoder_specific_settings = &mVideoEncoderSettings.mVp8;
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_VP9:
          {
            webrtc::VideoEncoder* videoEncoder = webrtc::VideoEncoder::Create(webrtc::VideoEncoder::kVp9);
            config.encoder_settings.encoder = videoEncoder;
            config.encoder_settings.payload_name = IRTPTypes::toString(supportedCodec);
            config.encoder_settings.payload_type = codecIter->mPayloadType;
            mVideoEncoderSettings.mVp9 = webrtc::VideoEncoder::GetDefaultVp9Settings();
            mVideoEncoderSettings.mVp9.frameDroppingOn = true;
            encoderConfig.encoder_specific_settings = &mVideoEncoderSettings.mVp9;
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_H264:
          {
#ifndef WINRT
            webrtc::VideoEncoder* videoEncoder = webrtc::VideoEncoder::Create(webrtc::VideoEncoder::kH264);
#else
            std::unique_ptr<cricket::WebRtcVideoEncoderFactory> encoderFactory = std::make_unique<webrtc::H264WinRTEncoderFactory>();
            webrtc::VideoEncoder* videoEncoder = encoderFactory->CreateVideoEncoder(webrtc::VideoCodecType::kVideoCodecH264);
#endif
            config.encoder_settings.encoder = videoEncoder;
            config.encoder_settings.payload_name = IRTPTypes::toString(supportedCodec);
            config.encoder_settings.payload_type = codecIter->mPayloadType;
            mVideoEncoderSettings.mH264 = webrtc::VideoEncoder::GetDefaultH264Settings();
            mVideoEncoderSettings.mH264.frameDroppingOn = true;
            encoderConfig.encoder_specific_settings = &mVideoEncoderSettings.mH264;
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_RTX:
          {
            config.rtp.rtx.payload_type = codecIter->mPayloadType;
            break;
          }
          case IRTPTypes::SupportedCodec_RED:
          {
            config.rtp.fec.red_payload_type = codecIter->mPayloadType;
            break;
          }
          case IRTPTypes::SupportedCodec_ULPFEC:
          {
            config.rtp.fec.ulpfec_payload_type = codecIter->mPayloadType;
            break;
          }
          case IRTPTypes::SupportedCodec_FlexFEC:
          {
            break;
          }
        }
        continue;

      set_rtcp_feedback:
        for (auto rtcpFeedbackIter = codecIter->mRTCPFeedback.begin(); rtcpFeedbackIter != codecIter->mRTCPFeedback.end(); rtcpFeedbackIter++) {
          IRTPTypes::KnownFeedbackTypes feedbackType = IRTPTypes::toKnownFeedbackType(rtcpFeedbackIter->mType);
          IRTPTypes::KnownFeedbackParameters feedbackParameter = IRTPTypes::toKnownFeedbackParameter(rtcpFeedbackIter->mParameter);
          if (IRTPTypes::KnownFeedbackType_NACK == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            config.rtp.nack.rtp_history_ms = 1000;
          }
        }
        videoCodecSet = true;
      }

      mCodecPayloadName = config.encoder_settings.payload_name;
      mCodecPayloadType = config.encoder_settings.payload_type;

      int totalMinBitrate = 0;
      int totalMaxBitrate = 0;
      int totalTargetBitrate = 0;
      for (auto encodingParamIter = mParameters->mEncodings.begin(); encodingParamIter != mParameters->mEncodings.end(); encodingParamIter++) {

        IRTPTypes::PayloadType codecPayloadType {};
        if (encodingParamIter->mCodecPayloadType.hasValue())
          codecPayloadType = encodingParamIter->mCodecPayloadType;
        else
          codecPayloadType = config.encoder_settings.payload_type;

        if (codecPayloadType == config.encoder_settings.payload_type) {
          uint32_t ssrc = 0;
          if (encodingParamIter->mSSRC.hasValue())
            ssrc = encodingParamIter->mSSRC;
          if (encodingParamIter->mFEC.hasValue()) {
            IRTPTypes::FECParameters fec = encodingParamIter->mFEC;
            if (fec.mSSRC.hasValue()) {
              ssrc = encodingParamIter->mSSRC;
            }
          }
          if (ssrc == 0)
            ssrc = SafeInt<uint32>(openpeer::services::IHelper::random(1, 0xFFFFFFFF));
          config.rtp.ssrcs.push_back(ssrc);
          if (encodingParamIter->mRTX.hasValue()) {
            IRTPTypes::RTXParameters rtx = encodingParamIter->mRTX;
            if (rtx.mSSRC.hasValue())
              config.rtp.rtx.ssrcs.push_back(rtx.mSSRC);
          }
          webrtc::VideoStream stream;
          stream.width = encodingParamIter->mResolutionScale.hasValue() ? sourceWidth / encodingParamIter->mResolutionScale : sourceWidth;
          stream.height = encodingParamIter->mResolutionScale.hasValue() ? sourceHeight / encodingParamIter->mResolutionScale : sourceHeight;
          stream.max_framerate = encodingParamIter->mFramerateScale.hasValue() ? sourceMaxFramerate / encodingParamIter->mResolutionScale : sourceMaxFramerate;
          stream.min_bitrate_bps = 30000;
          stream.max_bitrate_bps = encodingParamIter->mMaxBitrate.hasValue() ? encodingParamIter->mMaxBitrate : 2000000;
          stream.target_bitrate_bps = stream.max_bitrate_bps / 2;
          stream.max_qp = 56;
          encoderConfig.streams.push_back(stream);
          totalMinBitrate += stream.min_bitrate_bps;
          totalMaxBitrate += stream.max_bitrate_bps;
          totalTargetBitrate += stream.target_bitrate_bps;
        }
      }
      if (encoderConfig.streams.size() == 0) {
        config.rtp.ssrcs.push_back(SafeInt<uint32>(openpeer::services::IHelper::random(1, 0xFFFFFFFF)));
        webrtc::VideoStream stream;
        stream.width = sourceWidth;
        stream.height = sourceHeight;
        stream.max_framerate = sourceMaxFramerate;
        stream.min_bitrate_bps = 30000;
        stream.max_bitrate_bps = 2000000;
        stream.target_bitrate_bps = stream.max_bitrate_bps / 2;
        stream.max_qp = 56;
        encoderConfig.streams.push_back(stream);
        totalMinBitrate = stream.min_bitrate_bps;
        totalMaxBitrate = stream.max_bitrate_bps;
        totalTargetBitrate = stream.target_bitrate_bps;
      }
      if (encoderConfig.streams.size() > 1)
        mVideoEncoderSettings.mVp8.automaticResizeOn = false;

      for (auto headerExtensionIter = mParameters->mHeaderExtensions.begin(); headerExtensionIter != mParameters->mHeaderExtensions.end(); headerExtensionIter++) {
        IRTPTypes::HeaderExtensionURIs headerExtensionURI = IRTPTypes::toHeaderExtensionURI(headerExtensionIter->mURI);
        switch (headerExtensionURI) {
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_TransmissionTimeOffsets:
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_AbsoluteSendTime:
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_3gpp_VideoOrientation:
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_TransportSequenceNumber:
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          default:
            break;
        }
      }

      config.rtp.c_name = mParameters->mRTCP.mCName;

      mCongestionController->SetBweBitrates(totalMinBitrate, totalTargetBitrate, totalMaxBitrate);

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());
      mModuleProcessThread->RegisterModule(mCongestionController.get());
      mPacerThread->RegisterModule(mCongestionController->pacer());
      mPacerThread->RegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mPacerThread->Start();

      mSendStream = rtc::scoped_ptr<webrtc::VideoSendStream>(
        new webrtc::internal::VideoSendStream(
                                              numCpuCores,
                                              mModuleProcessThread.get(),
                                              mCallStats.get(),
                                              mCongestionController.get(),
                                              &mRemb,
                                              mBitrateAllocator.get(),
                                              config,
                                              encoderConfig,
                                              suspendedSSRCs
                                              ));

      if (mTransportState == ISecureTransport::State_Connected)
        mSendStream->Start();

      notifyPromisesResolve();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::stepShutdown()
    {
      mDenyNonLockedAccess = true;

      // rare race condition that can happen so
      while (mAccessFromNonLockedMethods > 0)
      {
        // NOTE: very temporary lock so should clear itself out fast
        std::this_thread::yield();
      }

      AutoRecursiveLock lock(*this);

      if (mSendStream && mTransportState == ISecureTransport::State_Connected)
        mSendStream->Stop();

      mPacerThread->Stop();
      mPacerThread->DeRegisterModule(mCongestionController->pacer());
      mPacerThread->DeRegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mModuleProcessThread->DeRegisterModule(mCongestionController.get());
      mModuleProcessThread->DeRegisterModule(mCallStats.get());
      mModuleProcessThread->Stop();

      mCallStats->DeregisterStatsObserver(mCongestionController.get());

      mSendStream.reset();
      mCongestionController.reset();
      mCallStats.reset();
      mBitrateAllocator.reset();
      mModuleProcessThread.reset();
      mPacerThread.reset();

      notifyPromisesShutdown();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPMediaEngineFactory &IRTPMediaEngineFactory::singleton()
    {
      return RTPMediaEngineFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPMediaEnginePtr IRTPMediaEngineFactory::create(IRTPMediaEngineRegistrationPtr registration)
    {
      if (this) {}
      return internal::RTPMediaEngine::create(registration);
    }

  } // internal namespace
}
