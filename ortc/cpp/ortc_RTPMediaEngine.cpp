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
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IStatsReport.h>

#include <ortc/IHelper.h>
#include <ortc/services/IHTTP.h>

#include <zsLib/ISettings.h>
#include <zsLib/Singleton.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>
#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>

#include <limits>
#include <float.h>
#include <math.h>

#include <webrtc/base/timeutils.h>
#include <webrtc/base/event_tracer.h>
#include <webrtc/call/rtc_event_log.h>
#include <webrtc/voice_engine/include/voe_codec.h>
#include <webrtc/voice_engine/include/voe_rtp_rtcp.h>
#include <webrtc/voice_engine/include/voe_network.h>
#include <webrtc/voice_engine/include/voe_hardware.h>
#include <webrtc/system_wrappers/include/cpu_info.h>
#include <webrtc/voice_engine/include/voe_audio_processing.h>
#include <webrtc/modules/video_capture/video_capture_factory.h>
#include <webrtc/modules/audio_coding/codecs/builtin_audio_decoder_factory.h>
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
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPMediaEngineSettingsDefaults);
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
    #pragma mark RTPMediaEngineSettingsDefaults
    #pragma mark

    class RTPMediaEngineSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~RTPMediaEngineSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static RTPMediaEngineSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<RTPMediaEngineSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static RTPMediaEngineSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<RTPMediaEngineSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installRTPMediaEngineSettingsDefaults()
    {
      RTPMediaEngineSettingsDefaults::singleton();
    }

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
        AutoRecursiveLock lock(*IHelper::getGlobalLock());
        static SingletonLazySharedPtr<RTPMediaEngineSingleton> singleton(create());
        RTPMediaEngineSingletonPtr result = singleton.singleton();

        static zsLib::SingletonManager::Register registerSingleton("org.ortc.RTPMediaEngineSingleton", result);

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
        IHelper::debugAppend(objectEl, "id", mID);
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

        IHelper::debugAppend(resultEl, "id", mID);

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
    void IRTPMediaEngineForORTC::startMediaTracing()
    {
      RTPMediaEngine::startMediaTracing();
    }

    //-------------------------------------------------------------------------
    void IRTPMediaEngineForORTC::stopMediaTracing()
    {
      RTPMediaEngine::stopMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool IRTPMediaEngineForORTC::isMediaTracing()
    {
      return RTPMediaEngine::isMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool IRTPMediaEngineForORTC::saveMediaTrace(String filename)
    {
      return RTPMediaEngine::saveMediaTrace(filename);
    }

    //-------------------------------------------------------------------------
    bool IRTPMediaEngineForORTC::saveMediaTrace(String host, int port)
    {
      return RTPMediaEngine::saveMediaTrace(host, port);
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
    PromiseWithRTPMediaEngineChannelResourcePtr IRTPMediaEngineForRTPReceiverChannelMediaBase::setupChannel(
                                                                                                            UseReceiverChannelMediaBasePtr channel,
                                                                                                            TransportPtr transport,
                                                                                                            UseMediaStreamTrackPtr track,
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
    PromiseWithRTPMediaEngineChannelResourcePtr IRTPMediaEngineForRTPSenderChannelMediaBase::setupChannel(
                                                                                                          UseSenderChannelMediaBasePtr channel,
                                                                                                          TransportPtr transport,
                                                                                                          UseMediaStreamTrackPtr track,
                                                                                                          ParametersPtr parameters,
                                                                                                          IDTMFSenderDelegatePtr dtmfDelegate
                                                                                                          )
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineChannelResource::createRejected(IORTCForInternal::queueORTC());
      return singleton->getEngineRegistration()->getRTPEngine()->setupChannel(
                                                                              channel,
                                                                              transport,
                                                                              track,
                                                                              parameters,
                                                                              dtmfDelegate
                                                                              );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPMediaEngineForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPMediaEngineForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPMediaEngine, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEnginePtr IRTPMediaEngineForMediaStreamTrack::create()
    {
      return RTPMediaEngine::createEnginePromise();
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineDeviceResourcePtr IRTPMediaEngineForMediaStreamTrack::setupDevice(UseMediaStreamTrackPtr track)
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineDeviceResource::createRejected(IORTCForInternal::queueORTC());
      return singleton->getEngineRegistration()->getRTPEngine()->setupDevice(track);
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
      ZS_EVENTING_1(x, i, Detail, RtpMediaEngineCreate, ol, RtpMediaEngine, Start, puid, id, mID);
      ZS_LOG_DETAIL(debug("created"));
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

#if defined(WINRT)
      webrtc::SetupEventTracer(&GetCategoryGroupEnabled, &AddTraceEvent);
#endif

      AutoRecursiveLock lock(*this);

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::~RTPMediaEngine()
    {
      if (isNoop()) return;

      webrtc::Trace::SetTraceCallback(nullptr);
      webrtc::Trace::ReturnTrace();

      ZS_LOG_DETAIL(log("destroyed"));
      mThisWeak.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, RtpMediaEngineDestroy, ol, RtpMediaEngine, Stop, puid, id, mID);
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

    RTPMediaEnginePtr RTPMediaEngine::convert(ForChannelResourcePtr object)
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
    void RTPMediaEngine::startMediaTracing()
    {
      auto engine = RTPMediaEngineSingleton::getEngineIfAlive();
      if (engine) {
        engine->internalStartMediaTracing();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::stopMediaTracing()
    {
      auto engine = RTPMediaEngineSingleton::getEngineIfAlive();
      if (engine) {
        engine->internalStopMediaTracing();
      }
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::isMediaTracing()
    {
      auto engine = RTPMediaEngineSingleton::getEngineIfAlive();
      if (engine) {
        return engine->internalIsMediaTracing();
      }
      return false;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::saveMediaTrace(String filename)
    {
      auto engine = RTPMediaEngineSingleton::getEngineIfAlive();
      if (engine) {
        return engine->internalSaveMediaTrace(filename);
      }
      return false;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::saveMediaTrace(String host, int port)
    {
      auto engine = RTPMediaEngineSingleton::getEngineIfAlive();
      if (engine) {
        return engine->internalSaveMediaTrace(host, port);
      }
      return false;
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
    PromiseWithRTPMediaEngineChannelResourcePtr RTPMediaEngine::setupChannel(
                                                                             UseReceiverChannelMediaBasePtr channel,
                                                                             TransportPtr transport,
                                                                             UseMediaStreamTrackPtr track,
                                                                             ParametersPtr parameters,
                                                                             RTPPacketPtr packet
                                                                             )
    {
      // WARNING - DO NOT ENTER A LOCK!

      auto setup = make_shared<IRTPMediaEngineAsyncDelegate::SetupReceiverChannel>();
      setup->mRegistration = mRegistration.lock();
      setup->mPromise = PromiseWithRTPMediaEngineChannelResource::create(IORTCForInternal::queueORTC());
      setup->mChannel = channel;
      setup->mTransport = transport;
      setup->mTrack = track;
      setup->mParameters = parameters;
      setup->mPacket = packet;

      IRTPMediaEngineAsyncDelegateProxy::create(mThisWeak.lock())->onSetupReceiverChannel(setup);

      return setup->mPromise;
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
                                                                             UseMediaStreamTrackPtr track,
                                                                             ParametersPtr parameters,
                                                                             IDTMFSenderDelegatePtr dtmfDelegate
                                                                             )
    {
      // WARNING - DO NOT ENTER A LOCK!

      auto setup = make_shared<IRTPMediaEngineAsyncDelegate::SetupSenderChannel>();
      setup->mRegistration = mRegistration.lock();
      setup->mPromise = PromiseWithRTPMediaEngineChannelResource::create(IORTCForInternal::queueORTC());
      setup->mChannel = channel;
      setup->mTransport = transport;
      setup->mTrack = track;
      setup->mParameters = parameters;
      setup->mDTMFDelegate = dtmfDelegate;

      IRTPMediaEngineAsyncDelegateProxy::create(mThisWeak.lock())->onSetupSenderChannel(setup);

      return setup->mPromise;
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
    #pragma mark RTPMediaEngine => IRTPMediaEngineForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineDeviceResourcePtr RTPMediaEngine::setupDevice(UseMediaStreamTrackPtr track)
    {
      // WARNING - DO NOT ENTER A LOCK!

      auto setup = make_shared<IRTPMediaEngineAsyncDelegate::SetupDevice>();
      setup->mRegistration = mRegistration.lock();
      setup->mPromise = PromiseWithRTPMediaEngineDeviceResource::create(IORTCForInternal::queueORTC());
      setup->mTrack = track;

      IRTPMediaEngineAsyncDelegateProxy::create(mThisWeak.lock())->onSetupDevice(setup);

      return setup->mPromise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => IRTPMediaEngineForDeviceResource
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::notifyResourceGone(IDeviceResourceForRTPMediaEngine &inResource)
    {
      PUID resourceID = inResource.getID();

      AutoRecursiveLock lock(*this);

      auto found = mDeviceResources.find(resourceID);
      if (found != mDeviceResources.end()) {
        mDeviceResources.erase(found);
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
    rtc::scoped_refptr<webrtc::AudioDecoderFactory> RTPMediaEngine::getAudioDecoderFactory()
    {
      return mAudioDecoderFactory;
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

      // Do not call step() method within a lock!
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
    void RTPMediaEngine::onTimer(ITimerPtr timer)
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
    void RTPMediaEngine::onSetupSenderChannel(SetupSenderChannelPtr setup)
    {
      {
        AutoRecursiveLock lock(*this);

        if (ZS_DYNAMIC_PTR_CAST(IRTPSenderChannelAudioForRTPMediaEngine, setup->mChannel)) {
          ChannelResourcePtr resource = AudioSenderChannelResource::create(
                                                                           setup->mRegistration,
                                                                           setup->mTransport,
                                                                           setup->mTrack,
                                                                           setup->mParameters,
                                                                           setup->mDTMFDelegate
                                                                           );
          resource->registerPromise(setup->mPromise);
          mChannelResources[resource->getID()] = resource;
          mPendingSetupChannelResources.push_back(resource);
        } else if (ZS_DYNAMIC_PTR_CAST(IRTPSenderChannelVideoForRTPMediaEngine, setup->mChannel)) {
          ChannelResourcePtr resource = VideoSenderChannelResource::create(
                                                                           setup->mRegistration,
                                                                           setup->mTransport,
                                                                           setup->mTrack,
                                                                           setup->mParameters
                                                                           );
          resource->registerPromise(setup->mPromise);
          mChannelResources[resource->getID()] = resource;
          mPendingSetupChannelResources.push_back(resource);
        }
      }

      // Do not call step() method within a lock!
      step();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::onSetupReceiverChannel(SetupReceiverChannelPtr setup)
    {
      {
        AutoRecursiveLock lock(*this);

        if (ZS_DYNAMIC_PTR_CAST(IRTPReceiverChannelAudioForRTPMediaEngine, setup->mChannel)) {
          AudioReceiverChannelResourcePtr resource = AudioReceiverChannelResource::create(
                                                                                          setup->mRegistration,
                                                                                          setup->mTransport,
                                                                                          setup->mTrack,
                                                                                          setup->mParameters,
                                                                                          setup->mPacket
                                                                                          );
          resource->registerPromise(setup->mPromise);
          mChannelResources[resource->getID()] = resource;
          mPendingSetupChannelResources.push_back(resource);
        } else if (ZS_DYNAMIC_PTR_CAST(IRTPReceiverChannelVideoForRTPMediaEngine, setup->mChannel)) {
          VideoReceiverChannelResourcePtr resource = VideoReceiverChannelResource::create(
                                                                                          setup->mRegistration,
                                                                                          setup->mTransport,
                                                                                          setup->mTrack,
                                                                                          setup->mParameters,
                                                                                          setup->mPacket
                                                                                          );
          resource->registerPromise(setup->mPromise);
          mChannelResources[resource->getID()] = resource;
          mPendingSetupChannelResources.push_back(resource);
        }
      }

      // Do not call step() method within a lock!
      step();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::onSetupDevice(SetupDevicePtr setup)
    {
      {
        AutoRecursiveLock lock(*this);

        DeviceResourcePtr resource = DeviceResource::create(
                                                            setup->mRegistration,
                                                            setup->mTrack
                                                            );

        resource->registerPromise(setup->mPromise);

        mDeviceResources[resource->getID()] = resource;
        mPendingSetupDeviceResources.push_back(resource);
      }

      // Do not call step() method within a lock!
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => (friend DeviceResource)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::shutdownDeviceResource(DeviceResourcePtr deviceResource)
    {
      AutoRecursiveLock lock(*this);

      mPendingCloseDeviceResources.push_back(deviceResource);

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine => webrtc::SetupEventTracer
    #pragma mark

#if defined(WINRT)
    const unsigned char *RTPMediaEngine::GetCategoryGroupEnabled(const char *categoryGroup)
    {
      return reinterpret_cast<const unsigned char*>("webrtc");
    }

    void __cdecl RTPMediaEngine::AddTraceEvent(
                                               char phase,
                                               const unsigned char *categoryGroupEnabled,
                                               const char *name,
                                               uint64_t id,
                                               int numArgs,
                                               const char **argNames,
                                               const unsigned char *argTypes,
                                               const uint64_t *argValues,
                                               unsigned char flags
                                               )
    {
      auto engine = RTPMediaEngineSingleton::getEngineIfAlive();
      if (engine) {
        engine->internalAddTraceEvent(phase, categoryGroupEnabled, name, id,
          numArgs, argNames, argTypes, argValues, flags);
      }
    }
#endif

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
      AutoRecursiveLock lock(*this);

      mPendingCloseChannelResources.push_back(channelResource);

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
      IHelper::debugAppend(objectEl, "id", mID);
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

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto registration = mRegistration.lock();
      IHelper::debugAppend(resultEl, "registration", (bool)registration);

      IHelper::debugAppend(resultEl, "pending ready", mPendingReady.size());

      IHelper::debugAppend(resultEl, "device resources", mDeviceResources.size());
      IHelper::debugAppend(resultEl, "pending setup device resources", mPendingSetupDeviceResources.size());
      IHelper::debugAppend(resultEl, "pending close device resources", mPendingCloseDeviceResources.size());

      IHelper::debugAppend(resultEl, "channel resources", mChannelResources.size());
      IHelper::debugAppend(resultEl, "pending setup channel resources", mPendingSetupChannelResources.size());
      IHelper::debugAppend(resultEl, "pending close channel resources", mPendingCloseChannelResources.size());

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
    void RTPMediaEngine::internalStartMediaTracing()
    {
      mTraceLog.EnableTraceInternalStorage();
      mTraceLog.StartTracing();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::internalStopMediaTracing()
    {
      mTraceLog.StopTracing();
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::internalIsMediaTracing()
    {
      return mTraceLog.IsTracing();
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::internalSaveMediaTrace(String filename)
    {
      return mTraceLog.Save(filename);
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::internalSaveMediaTrace(String host, int port)
    {
      return mTraceLog.Save(host, port);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::internalAddTraceEvent(
                                               char phase,
                                               const unsigned char *categoryGroupEnabled,
                                               const char *name,
                                               uint64_t id,
                                               int numArgs,
                                               const char **argNames,
                                               const unsigned char *argTypes,
                                               const uint64_t *argValues,
                                               unsigned char flags
                                               )
    {
      mTraceLog.Add(phase, categoryGroupEnabled, name, id,
        numArgs, argNames, argTypes, argValues, flags);
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

      bool shuttingDown;
      bool shutdown;
      {
        AutoRecursiveLock lock(*this);

        shuttingDown = isShuttingDown();
        shutdown = isShutdown();
      }

      if (shuttingDown || shutdown) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
          stepCancel();
        return;
      }

      // ... other steps here ...
      if (!stepSetup()) goto not_ready;
      if (!stepSetupDevices()) goto not_ready;
      if (!stepSetupChannels()) goto not_ready;
      if (!stepCloseDevices()) goto not_ready;
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
        AutoRecursiveLock lock(*this);
        setState(State_Ready);
      }
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepSetup()
    {
      AutoRecursiveLock lock(*this);

      if (isReady()) {
        ZS_LOG_TRACE(log("already setup"))
        return true;
      }

      mVoiceEngine = std::unique_ptr<webrtc::VoiceEngine, VoiceEngineDeleter>(webrtc::VoiceEngine::Create());

      mAudioDecoderFactory = webrtc::CreateBuiltinAudioDecoderFactory();

      webrtc::VoEBase::GetInterface(mVoiceEngine.get())->Init(nullptr, nullptr, mAudioDecoderFactory);

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
    bool RTPMediaEngine::stepSetupDevices()
    {
      DeviceResourceList pendingSetupDeviceResources;
      {
        AutoRecursiveLock lock(*this);
        pendingSetupDeviceResources = mPendingSetupDeviceResources;
        mPendingSetupDeviceResources.clear();
      }

      while (pendingSetupDeviceResources.size() > 0) {
        auto deviceResource = pendingSetupDeviceResources.front();

        deviceResource->stepSetup();

        pendingSetupDeviceResources.pop_front();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepCloseDevices()
    {
      DeviceResourceList pendingCloseDeviceResources;
      {
        AutoRecursiveLock lock(*this);
        pendingCloseDeviceResources = mPendingCloseDeviceResources;
        mPendingSetupDeviceResources.clear();
      }

      while (pendingCloseDeviceResources.size() > 0) {
        auto deviceResource = pendingCloseDeviceResources.front();

        deviceResource->stepShutdown();

        pendingCloseDeviceResources.pop_front();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepSetupChannels()
    {
      ChannelResourceList pendingSetupChannelResources;
      {
        AutoRecursiveLock lock(*this);
        pendingSetupChannelResources = mPendingSetupChannelResources;
        mPendingSetupChannelResources.clear();
      }

      while (pendingSetupChannelResources.size() > 0) {
        auto channelResource = pendingSetupChannelResources.front();

        channelResource->stepSetup();

        pendingSetupChannelResources.pop_front();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepCloseChannels()
    {
      ChannelResourceList pendingCloseChannelResources;
      {
        AutoRecursiveLock lock(*this);
        pendingCloseChannelResources = mPendingCloseChannelResources;
        mPendingCloseChannelResources.clear();
      }

      while (pendingCloseChannelResources.size() > 0) {
        auto channelResource = pendingCloseChannelResources.front();

        channelResource->stepShutdown();
        
        pendingCloseChannelResources.pop_front();
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
      ChannelResourceList pendingCloseChannelResources;
      ChannelResourceWeakMap channelResources;
      {
        AutoRecursiveLock lock(*this);
        pendingCloseChannelResources = mPendingCloseChannelResources;
        mPendingCloseChannelResources.clear();
        channelResources = mChannelResources;
        mChannelResources.clear();
      }

      while (pendingCloseChannelResources.size() > 0) {
        auto channelResource = pendingCloseChannelResources.front();

        channelResource->stepShutdown();

        pendingCloseChannelResources.pop_front();
      }

      for (auto iter = channelResources.begin(); iter != channelResources.end(); ++iter)
      {
        auto channelResource = (*iter).second.lock();
        if (channelResource)
          channelResource->stepShutdown();
      }

      {
        AutoRecursiveLock lock(*this);
        cancel();
      }
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
                                                   UseMediaStreamTrackPtr track
                                                   ) :
      BaseResource(priv, registration, registration ? registration->getRTPEngine() : RTPMediaEnginePtr()),
      mTrack(track)
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
                                                                             UseMediaStreamTrackPtr track
                                                                             )
    {
      auto pThis = make_shared<DeviceResource>(
                                               make_private{},
                                               registration,
                                               track
                                               );
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::lifetimeHolderGone()
    {
      auto shutdownPromise = shutdown();
      if (shutdownPromise) {
        // don't really care about result
        shutdownPromise->then(IPromiseDelegateProxy::createNoop(IORTCForInternal::queueBlockingMediaStartStopThread()));
        shutdownPromise->background();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::init()
    {
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
      return mDeviceID;
    }

    //-------------------------------------------------------------------------
    PromisePtr RTPMediaEngine::DeviceResource::shutdown()
    {
      PromisePtr promise;
      {
        AutoRecursiveLock lock(*this);

        promise = getShutdownPromise();
        if (isShutdown()) return promise;
        if (isShuttingDown()) return promise;

        mShuttingDown = true;
      }

      auto outer = mMediaEngine.lock();
      if (outer) {
        outer->shutdownDeviceResource(ZS_DYNAMIC_PTR_CAST(DeviceResource, mThisWeak.lock()));
      } else {
        stepShutdown();
      }
      return promise;
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::stop()
    {
      rtc::scoped_refptr<webrtc::VideoCaptureModule> videoCaptureModule;
      {
        AutoRecursiveLock lock(*this);

        videoCaptureModule = mVideoCaptureModule;
      }

      if (videoCaptureModule) {
        videoCaptureModule->StopCapture();
        videoCaptureModule->DeRegisterCaptureDataCallback();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::updateConstraints(PromisePtr promise, TrackConstraintsPtr constraints)
    {
      auto pThis = ZS_DYNAMIC_PTR_CAST(DeviceResource, mThisWeak.lock());
      IRTPMediaEngineDeviceResourceAsyncDelegateProxy::create(pThis)->onUpdateConstraints(promise, constraints);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats)
    {
      auto pThis = ZS_DYNAMIC_PTR_CAST(DeviceResource, mThisWeak.lock());
      IRTPMediaEngineDeviceResourceAsyncDelegateProxy::create(pThis)->onProvideStats(promise, stats);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::setVideoRenderCallback(IMediaStreamTrackRenderCallbackPtr callback)
    {
      auto track = mTrack.lock();
      if (!track) return;
      auto kind = track->kind();

      AutoRecursiveLock lock(*this);

      if (kind == Kinds::Kind_Video) {
        mVideoRenderCallbackReferenceHolder = callback;
        mVideoRendererCallback = dynamic_cast<IMediaStreamTrackRenderCallback*>(callback.get());
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::renderVideoFrame(VideoFramePtr videoFrame)
    {
      ++mFramesReceived;

      auto track = mTrack.lock();
      if (!track) return;

      auto settings = track->getSettings();
      if (!settings) return;

      if (!settings->mWidth.hasValue() || (settings->mWidth != videoFrame->width()))
        settings->mWidth = videoFrame->width();
      if (!settings->mHeight.hasValue() || (settings->mHeight != videoFrame->height()))
        settings->mHeight = videoFrame->height();

      AutoRecursiveLock lock(*this);

      if (mVideoRendererCallback) {
        mVideoRendererCallback->RenderFrame(1, *videoFrame);
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::setFrameCount()
    {
      AutoRecursiveLock lock(*this);

      if (mFramesSent > 5) {
        mAverageFramesSent += mFramesSent;
      }
      if (mFramesReceived > 5) {
        mAverageFramesReceived += mFramesReceived;
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::DeviceResource => IRTPMediaEngineDeviceResourceAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::onUpdateConstraints(PromisePtr promise, TrackConstraintsPtr constraints)
    {
      auto track = mTrack.lock();
      if (!track) {
        notifyPromisesReject();
        return;
      }

      auto settings = track->getSettings();
      if (!settings) {
        notifyPromisesReject();
        return;
      }

      auto kind = track->kind();
      auto remote = track->remote();

      AutoRecursiveLock lock(*this);

      if (kind == Kinds::Kind_Audio && remote) {

        mDeviceID = constraints->mAdvanced.front()->mDeviceID.mValue.value().mValue.value();

        auto engine = mMediaEngine.lock();
        if (!engine) return;

        auto voiceEngine = engine->getVoiceEngine();
        if (!voiceEngine) return;

        int audioDeviceIndex = getAudioDeviceIndex(voiceEngine, mDeviceID);

        if (webrtc::VoEHardware::GetInterface(voiceEngine)->SetPlayoutDevice(audioDeviceIndex) == -1) {
          return;
        }

        settings->mDeviceID = mDeviceID;
      }

      promise->resolve();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::onProvideStats(PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      auto track = mTrack.lock();
      if (!track) {
        notifyPromisesReject();
        return;
      }

      auto settings = track->getSettings();
      if (!settings) {
        notifyPromisesReject();
        return;
      }

      auto remote = track->remote();
      auto id = track->id();

      AutoRecursiveLock lock(*this);

      UseStatsReport::StatMap reportStats;

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_Track)) {

        auto report = make_shared<IStatsReport::MediaStreamTrackStats>();

        decltype(report->mFramesPerSecond) framesPerSecond{};

        if (!remote)
          framesPerSecond = mAverageFramesSent.value<decltype(framesPerSecond)>();
        else
          framesPerSecond = mAverageFramesReceived.value<decltype(framesPerSecond)>();

        report->mID = id;
        report->mTrackID = id;
        report->mRemoteSource = remote;
        report->mFrameWidth = settings->mWidth.hasValue() ? settings->mWidth.value() : 0;
        report->mFrameHeight = settings->mHeight.hasValue() ? settings->mHeight.value() : 0;
        report->mFramesPerSecond = framesPerSecond;
        report->mFramesSent = mFramesSent;
        report->mFramesReceived = mFramesReceived;
        report->mAudioLevel = 0.0;

        reportStats[report->mID] = report;
      }

      promise->resolve(UseStatsReport::create(reportStats));
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::onCapturedVideoFrame(VideoFramePtr frame)
    {
      {
        AutoRecursiveLock lock(*this);

        if (mVideoRendererCallback) mVideoRendererCallback->RenderFrame(1, *frame);
      }

      auto track = mTrack.lock();
      if (!track) return;

      track->sendCapturedVideoFrame(frame);
      ++mFramesSent;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::DeviceResource => friend RTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::stepSetup()
    {
      auto track = mTrack.lock();
      if (!track) {
        notifyPromisesReject();
        return;
      }

      auto settings = track->getSettings();
      if (!settings) {
        notifyPromisesReject();
        return;
      }

      auto kind = track->kind();
      auto remote = track->remote();

      auto constraints = track->getConstraints();
      auto pThis = ZS_DYNAMIC_PTR_CAST(DeviceResource, mThisWeak.lock());

      if (kind == Kinds::Kind_Video && !remote) {

        if (!constraints) {
          notifyPromisesReject();
          return;
        }

        String deviceID;
        rtc::scoped_refptr<webrtc::VideoCaptureModule> videoCaptureModule;
        VideoCaptureTransportPtr transport;

        deviceID = constraints->mAdvanced.front()->mDeviceID.mValue.value().mValue.value();
        videoCaptureModule = webrtc::VideoCaptureFactory::Create(0, deviceID.c_str());
        if (!videoCaptureModule) {
          notifyPromisesReject();
          return;
        }

        transport = VideoCaptureTransport::create(pThis);
        videoCaptureModule->RegisterCaptureDataCallback(*transport);

        webrtc::VideoCaptureModule::DeviceInfo* info = webrtc::VideoCaptureFactory::CreateDeviceInfo(0);
        if (!info) {
          notifyPromisesReject();
          return;
        }

        std::list<VideoCaptureCapabilityWithDistance> capabilityCandidates;
        int32_t numCapabilities = info->NumberOfCapabilities(deviceID.c_str());
        for (INT i = 0; i < numCapabilities; ++i) {
          webrtc::VideoCaptureCapability capability;
          if (info->GetCapability(deviceID.c_str(), i, capability) == -1)
            continue;
          ConstrainLongRange widthRange;
          ConstrainLongRange heightRange;
          if (constraints->mWidth.mValue.hasValue())
            widthRange.mIdeal = constraints->mWidth.mValue.value();
          if (constraints->mHeight.mValue.hasValue())
            heightRange.mIdeal = constraints->mHeight.mValue.value();
          if (constraints->mWidth.mRange.hasValue())
            widthRange = constraints->mWidth.mRange;
          if (constraints->mHeight.mRange.hasValue())
            heightRange = constraints->mHeight.mRange;
          FLOAT sizeDistance = calculateSizeDistance(widthRange, heightRange, capability);
          if (sizeDistance == FLT_MAX)
            continue;
          ConstrainDoubleRange frameRateRange;
          if (constraints->mFrameRate.mValue.hasValue())
            frameRateRange.mIdeal = constraints->mFrameRate.mValue.value();
          if (constraints->mFrameRate.mRange.hasValue())
            frameRateRange = constraints->mFrameRate.mRange;
          FLOAT frameRateDistance = calculateFrameRateDistance(frameRateRange, capability);
          if (frameRateDistance == FLT_MAX)
            continue;
          ConstrainDoubleRange aspectRatioRange;
          if (constraints->mAspectRatio.mValue.hasValue())
            aspectRatioRange.mIdeal = constraints->mAspectRatio.mValue.value();
          if (constraints->mAspectRatio.mRange.hasValue())
            aspectRatioRange = constraints->mAspectRatio.mRange;
          FLOAT aspectRatioDistance = calculateAspectRatioDistance(aspectRatioRange, capability);
          if (aspectRatioDistance == FLT_MAX)
            continue;
          FLOAT formatDistance = calculateFormatDistance(capability);
          FLOAT totalDistance = 200.0F * sizeDistance + 20.0F * frameRateDistance + 10.0F * aspectRatioDistance + formatDistance;
          VideoCaptureCapabilityWithDistance capabilityWithDistance;
          capabilityWithDistance.mCapability = capability;
          capabilityWithDistance.mDistance = totalDistance;
          capabilityCandidates.push_back(capabilityWithDistance);
        }
        delete info;

        std::list<VideoCaptureCapabilityWithDistance> advancedCapabilityCandidates;
        std::list<ConstraintSetPtr>::iterator constraintsIter = constraints->mAdvanced.begin();
        while (constraintsIter != constraints->mAdvanced.end()) {
          std::list<VideoCaptureCapabilityWithDistance>::iterator capabilityCandidatesIter = capabilityCandidates.begin();
          while (capabilityCandidatesIter != capabilityCandidates.end()) {
            ConstrainLongRange widthRange;
            ConstrainLongRange heightRange;
            if ((*constraintsIter)->mWidth.mValue.hasValue())
              widthRange.mExact = (*constraintsIter)->mWidth.mValue.value();
            if ((*constraintsIter)->mHeight.mValue.hasValue())
              heightRange.mExact = (*constraintsIter)->mHeight.mValue.value();
            if ((*constraintsIter)->mWidth.mRange.hasValue())
              widthRange = (*constraintsIter)->mWidth.mRange;
            if ((*constraintsIter)->mHeight.mRange.hasValue())
              heightRange = (*constraintsIter)->mHeight.mRange;
            FLOAT sizeDistance = calculateSizeDistance(widthRange, heightRange, capabilityCandidatesIter->mCapability);
            if (sizeDistance == FLT_MAX) {
              capabilityCandidatesIter++;
              continue;
            }
            ConstrainDoubleRange frameRateRange;
            if ((*constraintsIter)->mFrameRate.mValue.hasValue())
              frameRateRange.mExact = (*constraintsIter)->mFrameRate.mValue.value();
            if ((*constraintsIter)->mFrameRate.mRange.hasValue())
              frameRateRange = (*constraintsIter)->mFrameRate.mRange;
            FLOAT frameRateDistance = calculateFrameRateDistance(frameRateRange, capabilityCandidatesIter->mCapability);
            if (frameRateDistance == FLT_MAX) {
              capabilityCandidatesIter++;
              continue;
            }
            ConstrainDoubleRange aspectRatioRange;
            if ((*constraintsIter)->mAspectRatio.mValue.hasValue())
              aspectRatioRange.mExact = (*constraintsIter)->mAspectRatio.mValue.value();
            if ((*constraintsIter)->mAspectRatio.mRange.hasValue())
              aspectRatioRange = (*constraintsIter)->mAspectRatio.mRange;
            FLOAT aspectRatioDistance = calculateAspectRatioDistance(aspectRatioRange, capabilityCandidatesIter->mCapability);
            if (aspectRatioDistance == FLT_MAX) {
              capabilityCandidatesIter++;
              continue;
            }
            FLOAT formatDistance = calculateFormatDistance(capabilityCandidatesIter->mCapability);
            FLOAT totalDistance = 200.0F * sizeDistance + 20.0F * frameRateDistance + 10.0F * aspectRatioDistance + formatDistance;
            VideoCaptureCapabilityWithDistance capabilityWithDistance;
            capabilityWithDistance.mCapability = capabilityCandidatesIter->mCapability;
            capabilityWithDistance.mDistance = totalDistance;
            advancedCapabilityCandidates.push_back(capabilityWithDistance);
            capabilityCandidatesIter++;
          }
          if (advancedCapabilityCandidates.size() > 0)
            break;
          constraintsIter++;
        }

        FLOAT bestDistance = FLT_MAX;
        webrtc::VideoCaptureCapability bestCapability;
        if (advancedCapabilityCandidates.size() > 0) {
          std::list<VideoCaptureCapabilityWithDistance>::iterator advancedCapabilityCandidatesIter = advancedCapabilityCandidates.begin();
          while (advancedCapabilityCandidatesIter != advancedCapabilityCandidates.end()) {
            if (advancedCapabilityCandidatesIter->mDistance <= bestDistance) {
              bestDistance = advancedCapabilityCandidatesIter->mDistance;
              bestCapability = advancedCapabilityCandidatesIter->mCapability;
            }
            advancedCapabilityCandidatesIter++;
          }
        } else if (advancedCapabilityCandidates.size() > 0) {
          std::list<VideoCaptureCapabilityWithDistance>::iterator capabilityCandidatesIter = capabilityCandidates.begin();
          while (capabilityCandidatesIter != capabilityCandidates.end()) {
            if (capabilityCandidatesIter->mDistance <= bestDistance) {
              bestDistance = capabilityCandidatesIter->mDistance;
              bestCapability = capabilityCandidatesIter->mCapability;
            }
            capabilityCandidatesIter++;
          }
        }

        if (bestDistance == FLT_MAX) {
          notifyPromisesReject();
          return;
        }

        settings->mWidth = bestCapability.width;
        settings->mHeight = bestCapability.height;
        settings->mFrameRate = bestCapability.maxFPS;
        settings->mDeviceID = deviceID;

        if (videoCaptureModule->StartCapture(bestCapability) != 0) {
          videoCaptureModule->DeRegisterCaptureDataCallback();
          notifyPromisesReject();
          return;
        }

        {
          AutoRecursiveLock lock(*this);
          mDeviceID = deviceID;
          mVideoCaptureModule = videoCaptureModule;
          mTransport = transport;
        }
      } else if (kind == Kinds::Kind_Audio && !remote) {

        if (!constraints) {
          notifyPromisesReject();
          return;
        }

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

        String deviceID;

        deviceID = constraints->mAdvanced.front()->mDeviceID.mValue.value().mValue.value();

        int audioDeviceIndex = getAudioDeviceIndex(voiceEngine, mDeviceID);

        if (webrtc::VoEHardware::GetInterface(voiceEngine)->SetRecordingDevice(audioDeviceIndex) == -1) {
          notifyPromisesReject();
          return;
        }

        settings->mDeviceID = deviceID;

        {
          AutoRecursiveLock lock(*this);
          mDeviceID = deviceID;
        }
      }

      notifyPromisesResolve();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::stepShutdown()
    {
      mDenyNonLockedAccess = true;

      // rare race condition that can happen so
      while (mAccessFromNonLockedMethods > 0)
      {
        // NOTE: very temporary lock so should clear itself out fast
        std::this_thread::yield();
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::DeviceResource => (internal friend derived)
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::notifyPromisesShutdown()
    {
      AutoRecursiveLock lock(*this);

      mShutdown = true;
      mShuttingDown = false;

      for (auto iter = mShutdownPromises.begin(); iter != mShutdownPromises.end(); ++iter) {
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
    #pragma mark RTPMediaEngine::DeviceResource => friend VideoCaptureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::OnIncomingCapturedFrame(const int32_t id, const webrtc::VideoFrame& videoFrame)
    {
      auto newFrame = make_shared<VideoFrame>();
      newFrame->ShallowCopy(videoFrame);

      auto pThis = ZS_DYNAMIC_PTR_CAST(DeviceResource, mThisWeak.lock());

      IRTPMediaEngineDeviceResourceAsyncDelegateProxy::create(pThis)->onCapturedVideoFrame(newFrame);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::OnCaptureDelayChanged(const int32_t id, const int32_t delay)
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::DeviceResource::VideoCaptureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::DeviceResource::VideoCaptureTransport::VideoCaptureTransport(
                                                                                 const make_private &,
                                                                                 DeviceResourcePtr outer
                                                                                 ) :
      mOuter(outer)
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::DeviceResource::VideoCaptureTransport::~VideoCaptureTransport()
    {
      mThisWeak.reset();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::VideoCaptureTransport::init()
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::DeviceResource::VideoCaptureTransportPtr RTPMediaEngine::DeviceResource::VideoCaptureTransport::create(DeviceResourcePtr outer)
    {
      VideoCaptureTransportPtr pThis(make_shared<VideoCaptureTransport>(make_private{}, outer));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::DeviceResource::Transport => webrtc::VideoCaptureDataCallback
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::VideoCaptureTransport::OnIncomingCapturedFrame(const int32_t id, const webrtc::VideoFrame& videoFrame)
    {
      auto outer = mOuter.lock();
      if (!outer) return;
      return outer->OnIncomingCapturedFrame(id, videoFrame);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::DeviceResource::VideoCaptureTransport::OnCaptureDelayChanged(const int32_t id, const int32_t delay)
    {
      auto outer = mOuter.lock();
      if (!outer) return;
      return outer->OnCaptureDelayChanged(id, delay);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::DeviceResource => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    PromisePtr RTPMediaEngine::DeviceResource::getShutdownPromise()
    {
      if (isShutdown()) {
        return Promise::createResolved(delegateQueue());
      }
      PromisePtr promise = Promise::create(delegateQueue());
      mShutdownPromises.push_back(promise);
      return promise;
    }

    int RTPMediaEngine::DeviceResource::getAudioDeviceIndex(webrtc::VoiceEngine *voiceEngine, String deviceID)
    {
      int devices;
      if (webrtc::VoEHardware::GetInterface(voiceEngine)->GetNumOfRecordingDevices(devices) != 0) {
        return -1;
      }
      int index = -1;
      char deviceName[128];
      char deviceUniqueId[128];
      for (int i = 0; i < devices; i++) {
        webrtc::VoEHardware::GetInterface(voiceEngine)->GetRecordingDeviceName(i, deviceName, deviceUniqueId);
        if (0 == String(deviceUniqueId).compareNoCase(deviceID)) {
          index = i;
          break;
        }
      }
      return index;
    }

    //-------------------------------------------------------------------------
    FLOAT RTPMediaEngine::DeviceResource::calculateSizeDistance(
                                                                ConstrainLongRange width,
                                                                ConstrainLongRange height,
                                                                webrtc::VideoCaptureCapability capability
                                                                )
    {
      if (width.mMin.hasValue() && width.mMin.value() > capability.width)
        return FLT_MAX;
      if (width.mMax.hasValue() && width.mMax.value() < capability.width)
        return FLT_MAX;
      if (width.mExact.hasValue() && width.mExact.value() != capability.width)
        return FLT_MAX;
      FLOAT withDistance = 0.0F;
      if (width.mIdeal.hasValue() && width.mIdeal.value() != capability.width) {
        withDistance = (FLOAT)(abs((int)(capability.width - width.mIdeal.value()))) / (FLOAT)width.mIdeal.value();
      }

      if (height.mMin.hasValue() && height.mMin.value() > capability.height)
        return FLT_MAX;
      if (height.mMax.hasValue() && height.mMax.value() < capability.height)
        return FLT_MAX;
      if (height.mExact.hasValue() && height.mExact.value() != capability.height)
        return FLT_MAX;
      FLOAT heightDistance = 0.0F;
      if (height.mIdeal.hasValue() && height.mIdeal.value() != capability.height) {
        heightDistance = (FLOAT)(abs((int)(capability.height - height.mIdeal.value()))) / (FLOAT)height.mIdeal.value();
      }


      return withDistance + heightDistance;
    }

    //-------------------------------------------------------------------------
    FLOAT RTPMediaEngine::DeviceResource::calculateFrameRateDistance(
                                                                     ConstrainDoubleRange frameRate,
                                                                     webrtc::VideoCaptureCapability capability
                                                                     )
    {
      if (frameRate.mMin.hasValue() && frameRate.mMin.value() > (zsLib::DOUBLE)capability.maxFPS)
        return FLT_MAX;
      if (frameRate.mMax.hasValue() && frameRate.mMax.value() < (zsLib::DOUBLE)capability.maxFPS)
        return FLT_MAX;
      if (frameRate.mExact.hasValue() && fabs(frameRate.mExact.value() - (zsLib::DOUBLE)capability.maxFPS) > 0.01F)
        return FLT_MAX;
      FLOAT frameRateDistance = 0.0F;
      if (frameRate.mIdeal.hasValue() && fabs(frameRate.mExact.value() - (zsLib::DOUBLE)capability.maxFPS) > 0.01F) {
        frameRateDistance = (FLOAT)(abs((int)(capability.maxFPS - frameRate.mIdeal.value()))) / (FLOAT)frameRate.mIdeal.value();
      }

      return frameRateDistance;
    }

    //-------------------------------------------------------------------------
    FLOAT RTPMediaEngine::DeviceResource::calculateAspectRatioDistance(
                                                                       ConstrainDoubleRange aspectRatio,
                                                                       webrtc::VideoCaptureCapability capability
                                                                       )
    {
      zsLib::DOUBLE capabilityAspectRatio = (zsLib::DOUBLE)capability.width / (zsLib::DOUBLE)capability.height;

      if (aspectRatio.mMin.hasValue() && aspectRatio.mMin.value() > capabilityAspectRatio)
        return FLT_MAX;
      if (aspectRatio.mMax.hasValue() && aspectRatio.mMax.value() < capabilityAspectRatio)
        return FLT_MAX;
      if (aspectRatio.mExact.hasValue() && fabs(aspectRatio.mExact.value() - capabilityAspectRatio) > 0.001F)
        return FLT_MAX;
      FLOAT aspectRatioDistance = 0.0F;
      if (aspectRatio.mIdeal.hasValue() && fabs(aspectRatio.mIdeal.value() - capabilityAspectRatio) > 0.001F) {
        aspectRatioDistance = (FLOAT)((capabilityAspectRatio - aspectRatio.mIdeal.value()) / aspectRatio.mIdeal.value());
      }

      return aspectRatioDistance;
    }

    //-------------------------------------------------------------------------
    FLOAT RTPMediaEngine::DeviceResource::calculateFormatDistance(webrtc::VideoCaptureCapability capability)
    {
      switch (capability.rawType) {
      case webrtc::kVideoI420:
      case webrtc::kVideoYV12:
      case webrtc::kVideoYUY2:
      case webrtc::kVideoUYVY:
      case webrtc::kVideoIYUV:
      case webrtc::kVideoARGB:
      case webrtc::kVideoRGB24:
      case webrtc::kVideoRGB565:
      case webrtc::kVideoARGB4444:
      case webrtc::kVideoARGB1555:
      case webrtc::kVideoNV12:
      case webrtc::kVideoNV21:
      case webrtc::kVideoBGRA:
        return 0.0F;
      case webrtc::kVideoMJPEG:
        return 1.0F;
      default:
        return FLT_MAX;
      }
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
      mRemb(mClock),
      mEventLog(new webrtc::RtcEventLogNullImpl())
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
      PromisePtr promise;
      {
        AutoRecursiveLock lock(*this);

        promise = getShutdownPromise();
        if (isShutdown()) return promise;
        if (mShuttingDown) return promise;

        mShuttingDown = true;
      }

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

      for (auto iter = mShutdownPromises.begin(); iter != mShutdownPromises.end(); ++iter) {
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
                                                                               UseMediaStreamTrackPtr track,
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
                                                                                                         UseMediaStreamTrackPtr track,
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
      if (state == mTransportState) return;

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      ParametersPtr parameters;
      IRTPTypes::EncodingParametersList encodings;
      ISecureTransport::States previousState;
      {
        AutoRecursiveLock lock(*this);

        parameters = mParameters;
        if (parameters)
          encodings = mParameters->mEncodings;
        previousState = mTransportState;
        mTransportState = state;
      }

      if (parameters && (encodings.size() == 0 ||
        encodings.size() > 0 && encodings.begin()->mActive)) {
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
      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      bool previousActive = false;
      bool currentActive = false;
      {
        AutoRecursiveLock lock(*this);

        ParametersPtr previousParams = mParameters;
        mParameters = params;
        if (previousParams->mEncodings.size() == 0 ||
          (previousParams->mEncodings.size() > 0 && previousParams->mEncodings.begin()->mActive)) {
          previousActive = true;
        }
        if (params->mEncodings.size() == 0 ||
          (params->mEncodings.size() > 0 && params->mEncodings.begin()->mActive)) {
          currentActive = true;
        }
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
      auto track = mTrack.lock();
      if (!track) {
        notifyPromisesReject();
        return;
      }

      AutoRecursiveLock lock(*this);

      if (!mReceiveStream) {
        notifyPromisesReject();
        return;
      }

      UseStatsReport::StatMap reportStats;

      webrtc::AudioReceiveStream::Stats receiveStreamStats = mReceiveStream->GetStats();

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_InboundRTP)) {

        auto report = make_shared<IStatsReport::InboundRTPStreamStats>();

        report->mID = string(receiveStreamStats.remote_ssrc) + "_recv";

        report->mSSRC = receiveStreamStats.remote_ssrc;
        report->mIsRemote = true;
        report->mMediaType = "audio";
        report->mMediaTrackID = track->id();
        report->mCodecID = mCodecPayloadName;
        report->mPacketsReceived = receiveStreamStats.packets_rcvd;
        report->mBytesReceived = receiveStreamStats.bytes_rcvd;
        report->mPacketsLost = receiveStreamStats.packets_lost;
        report->mJitter = receiveStreamStats.jitter_ms;
        report->mFractionLost = receiveStreamStats.fraction_lost;
#ifdef WINRT
        report->mEndToEndDelay = Milliseconds(receiveStreamStats.end_to_end_delay_ms);
#endif
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
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => webrtc::CongestionController::Observer
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs)
    {
      mBitrateAllocator->OnNetworkChanged(
                                          targetBitrateBps,
                                          fractionLoss,
                                          rttMs
                                          );

      AutoRecursiveLock lock(*this);

      mCurrentTargetBitrate = targetBitrateBps;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioReceiverChannelResource => webrtc::BitrateAllocator::LimitObserver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioReceiverChannelResource::OnAllocationLimitsChanged(uint32_t min_send_bitrate_bps, uint32_t max_padding_bitrate_bps)
    {
      AutoRecursiveLock lock(*this);

      mCongestionController->SetAllocatedSendBitrateLimits(min_send_bitrate_bps, max_padding_bitrate_bps);
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

      auto audioDecoderFactory = engine->getAudioDecoderFactory();
      auto audioState = engine->getAudioState();

      ParametersPtr parameters;
      webrtc::AudioReceiveStream::Config config;

      std::unique_ptr<webrtc::ProcessThread> moduleProcessThread;
      std::unique_ptr<webrtc::ProcessThread> pacerThread;
      std::unique_ptr<webrtc::BitrateAllocator> bitrateAllocator;
      std::unique_ptr<webrtc::CallStats> callStats;
      std::unique_ptr<webrtc::CongestionController> congestionController;
      int channel;
      String codecPayloadName;
      BYTE codecPayloadType;

      {
        AutoRecursiveLock lock(*this);

        parameters = mParameters;
      }

      if (!parameters) {
        notifyPromisesReject();
        return;
      }

      moduleProcessThread = webrtc::ProcessThread::Create("AudioReceiverChannelResourceModuleProcessThread");
      pacerThread = webrtc::ProcessThread::Create("AudioReceiverChannelResourcePacerThread");

      bitrateAllocator = std::unique_ptr<webrtc::BitrateAllocator>(new webrtc::BitrateAllocator(this));
      callStats = std::unique_ptr<webrtc::CallStats>(new webrtc::CallStats(mClock));
      congestionController =
        std::unique_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mClock,
                                                                                       this,
                                                                                       &mRemb,
                                                                                       mEventLog.get()
                                                                                       ));

      callStats->RegisterStatsObserver(congestionController.get());

      channel = webrtc::VoEBase::GetInterface(voiceEngine)->CreateChannel();

      bool audioCodecSet = false;
      webrtc::CodecInst codec;
      for (auto codecIter = parameters->mCodecs.begin(); codecIter != parameters->mCodecs.end(); codecIter++) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::getCodecKind(supportedCodec) == IRTPTypes::CodecKind_Audio && audioCodecSet)
          continue;
        codec = getAudioCodec(voiceEngine, codecIter->mName);
        codec.pltype = codecIter->mPayloadType;
        if (codecIter->mPTime != Milliseconds::zero())
          codec.pacsize = (int)((codec.plfreq / 1000) * codecIter->mPTime.count());
        if (codecIter->mNumChannels.hasValue())
          codec.channels = codecIter->mNumChannels;
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_Opus:
            codec.rate = 48000;
            webrtc::VoECodec::GetInterface(voiceEngine)->SetRecPayloadType(channel, codec);
            goto set_rtcp_feedback;
          case IRTPTypes::SupportedCodec_Isac:
          case IRTPTypes::SupportedCodec_G722:
          case IRTPTypes::SupportedCodec_ILBC:
          case IRTPTypes::SupportedCodec_PCMU:
          case IRTPTypes::SupportedCodec_PCMA:
            webrtc::VoECodec::GetInterface(voiceEngine)->SetRecPayloadType(channel, codec);
            goto set_rtcp_feedback;
          case IRTPTypes::SupportedCodec_RED:
            break;
        }
        continue;

      set_rtcp_feedback:
        for (auto rtcpFeedbackIter = codecIter->mRTCPFeedback.begin(); rtcpFeedbackIter != codecIter->mRTCPFeedback.end(); rtcpFeedbackIter++) {
          IRTPTypes::KnownFeedbackTypes feedbackType = IRTPTypes::toKnownFeedbackType(rtcpFeedbackIter->mType);
          IRTPTypes::KnownFeedbackParameters feedbackParameter = IRTPTypes::toKnownFeedbackParameter(rtcpFeedbackIter->mParameter);
          if (IRTPTypes::KnownFeedbackType_NACK == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetNACKStatus(channel, true, 250);
          }
        }
        audioCodecSet = true;
      }

      codecPayloadName = codec.plname;
      codecPayloadType = (BYTE)codec.pltype;

      config.voe_channel_id = channel;
      config.decoder_factory = audioDecoderFactory;

      for (auto encodingParamIter = parameters->mEncodings.begin(); encodingParamIter != parameters->mEncodings.end(); encodingParamIter++) {

        IRTPTypes::PayloadType codecPayloadType{};
        if (encodingParamIter->mCodecPayloadType.hasValue())
          codecPayloadType = encodingParamIter->mCodecPayloadType;
        else
          codecPayloadType = (BYTE)codec.pltype;

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

      for (auto headerExtensionIter = parameters->mHeaderExtensions.begin(); headerExtensionIter != parameters->mHeaderExtensions.end(); headerExtensionIter++) {
        IRTPTypes::HeaderExtensionURIs headerExtensionURI = IRTPTypes::toHeaderExtensionURI(headerExtensionIter->mURI);
        switch (headerExtensionURI) {
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_ClienttoMixerAudioLevelIndication:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetReceiveAudioLevelIndicationStatus(channel, true, (BYTE)headerExtensionIter->mID);
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_AbsoluteSendTime:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetReceiveAbsoluteSenderTimeStatus(channel, true, (BYTE)headerExtensionIter->mID);
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          default:
            break;
        }
      }

      uint32_t localSSRC = mParameters->mRTCP.mSSRC;
      if (localSSRC == 0)
        localSSRC = 1;
      webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetLocalSSRC(channel, localSSRC);
      config.rtp.local_ssrc = localSSRC;
      config.rtcp_send_transport = mTransport.get();

      {
        AutoRecursiveLock lock(*this);

        mModuleProcessThread.swap(moduleProcessThread);
        mPacerThread.swap(pacerThread);
        mBitrateAllocator.swap(bitrateAllocator);
        mCallStats.swap(callStats);
        mCongestionController.swap(congestionController);
        mChannel = channel;
        mCodecPayloadName = codecPayloadName;
        mCodecPayloadType = codecPayloadType;
      }

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());
      mModuleProcessThread->RegisterModule(mCongestionController.get());
      mPacerThread->RegisterModule(mCongestionController->pacer());
      mPacerThread->RegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mPacerThread->Start();

      webrtc::internal::AudioReceiveStream *receiveStream =
        new webrtc::internal::AudioReceiveStream(
                                                 mCongestionController.get(),
                                                 config,
                                                 audioState,
                                                 mEventLog.get()
                                                 );

      {
        AutoRecursiveLock lock(*this);

        mReceiveStream = receiveStream;
      }

      webrtc::VoENetwork::GetInterface(voiceEngine)->RegisterExternalTransport(channel, *mTransport);

      if (mTransportState == ISecureTransport::State_Connected) {
        webrtc::VoEBase::GetInterface(voiceEngine)->StartReceive(channel);
        webrtc::VoEBase::GetInterface(voiceEngine)->StartPlayout(channel);
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

      int channel;
      {
        AutoRecursiveLock lock(*this);

        channel = mChannel;
      }

      auto outer = mMediaEngine.lock();

      if (outer) {
        auto voiceEngine = outer->getVoiceEngine();
        if (voiceEngine) {
          if (mTransportState == ISecureTransport::State_Connected) {
            webrtc::VoEBase::GetInterface(voiceEngine)->StopPlayout(channel);
            webrtc::VoEBase::GetInterface(voiceEngine)->StopReceive(channel);
          }
          webrtc::VoENetwork::GetInterface(voiceEngine)->DeRegisterExternalTransport(channel);
        }
      }

      {
        AutoRecursiveLock lock(*this);

        mPacerThread->Stop();
        mPacerThread->DeRegisterModule(mCongestionController->pacer());
        mPacerThread->DeRegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
        mModuleProcessThread->DeRegisterModule(mCongestionController.get());
        mModuleProcessThread->DeRegisterModule(mCallStats.get());
        mModuleProcessThread->Stop();

        mCallStats->DeregisterStatsObserver(mCongestionController.get());

        if (mReceiveStream)
          delete reinterpret_cast<webrtc::internal::AudioReceiveStream*>(mReceiveStream);
        mCongestionController.reset();
        mCallStats.reset();
        mBitrateAllocator.reset();
        mModuleProcessThread.reset();
        mPacerThread.reset();
      }

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
                                                                           UseMediaStreamTrackPtr track,
                                                                           ParametersPtr parameters,
                                                                           IDTMFSenderDelegatePtr dtmfDelegate
                                                                           ) :
      ChannelResource(priv, registration),
      mTransport(transport),
      mTrack(track),
      mParameters(parameters),
      mWorkerQueue("AudioSenderChannelResourceWorkerQueue"),
      mDTMFSenderDelegate(IDTMFSenderDelegateProxy::createWeak(dtmfDelegate))
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
                                                                                                     UseMediaStreamTrackPtr track,
                                                                                                     ParametersPtr parameters,
                                                                                                     IDTMFSenderDelegatePtr dtmfDelegate
                                                                                                     )
    {
      auto pThis = make_shared<AudioSenderChannelResource>(
                                                           make_private{},
                                                           registration,
                                                           transport,
                                                           track,
                                                           parameters,
                                                           dtmfDelegate
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
      if (state == mTransportState) return;

      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      ParametersPtr parameters;
      IRTPTypes::EncodingParametersList encodings;
      ISecureTransport::States previousState;
      {
        AutoRecursiveLock lock(*this);

        parameters = mParameters;
        if (parameters)
          encodings = mParameters->mEncodings;
        previousState = mTransportState;
        mTransportState = state;
      }

      if (parameters && (encodings.size() == 0 ||
        encodings.size() > 0 && encodings.begin()->mActive)) {
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
      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      bool previousActive = false;
      bool currentActive = false;
      {
        AutoRecursiveLock lock(*this);

        ParametersPtr previousParams = mParameters;
        mParameters = params;
        if (previousParams->mEncodings.size() == 0 ||
          (previousParams->mEncodings.size() > 0 && previousParams->mEncodings.begin()->mActive)) {
          previousActive = true;
        }
        if (params->mEncodings.size() == 0 ||
          (params->mEncodings.size() > 0 && params->mEncodings.begin()->mActive)) {
          currentActive = true;
        }
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
      auto track = mTrack.lock();
      if (!track) {
        notifyPromisesReject();
        return;
      }

      AutoRecursiveLock lock(*this);

      if (!mSendStream) {
        notifyPromisesReject();
        return;
      }

      UseStatsReport::StatMap reportStats;

      webrtc::AudioSendStream::Stats sendStreamStats = mSendStream->GetStats();

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_OutboundRTP)) {

        auto report = make_shared<IStatsReport::OutboundRTPStreamStats>();

        report->mID = string(sendStreamStats.local_ssrc) + "_send";

        report->mSSRC = sendStreamStats.local_ssrc;
        report->mIsRemote = false;
        report->mMediaType = "audio";
        report->mMediaTrackID = track->id();
        report->mCodecID = mCodecPayloadName;
        report->mPacketsSent = sendStreamStats.packets_sent;
        report->mBytesSent = sendStreamStats.bytes_sent;
        report->mTargetBitrate = (zsLib::DOUBLE)mCurrentTargetBitrate;
        report->mRoundTripTime = (zsLib::DOUBLE)mCallStats->rtcp_rtt_stats()->LastProcessedRtt();

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

    void RTPMediaEngine::AudioSenderChannelResource::onSendDTMFTone()
    {
      AutoRecursiveLock lock(*this);

      if (mDTMFPayloadType == 0)
        return;

      size_t firstTonePosition = mDTMFTones.find_first_of(dtmfValidTones);
      int code = 0;
      if (firstTonePosition == std::string::npos) {
        mDTMFTones.clear();
        notifyToneEvent("");
        return;
      } else {
        char tone = mDTMFTones[firstTonePosition];
        char event = (char)toupper(tone);
        const char* p = strchr(dtmfTonesTable, event);
        if (!p) {
          mDTMFTones.clear();
          notifyToneEvent("");
          return;
        }
        code = p - dtmfTonesTable - 1;
      }

      int toneGap = (int)mDTMFInterToneGap.count();
      if (code == dtmfTwoSecondsDelayCode) {
        toneGap = dtmfTwoSecondInMs;
      } else {
        mSendStream->SendTelephoneEvent(mDTMFPayloadType, code, (int)mDTMFDuration.count());
        toneGap += (int)mDTMFDuration.count();
      }

      notifyToneEvent(mDTMFTones.substr(firstTonePosition, 1).c_str());

      mDTMFTones.erase(0, firstTonePosition + 1);

      auto pThis = ZS_DYNAMIC_PTR_CAST(AudioSenderChannelResource, mThisWeak.lock());
      mDTMFTimer = ITimer::create(pThis, Milliseconds(toneGap), false);
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
    void RTPMediaEngine::AudioSenderChannelResource::insertDTMF(
                                                                const char *tones,
                                                                Milliseconds duration,
                                                                Milliseconds interToneGap
                                                                )
    {
      AutoRecursiveLock lock(*this);

      auto pThis = ZS_DYNAMIC_PTR_CAST(AudioSenderChannelResource, mThisWeak.lock());
      mDTMFTones = tones;
      mDTMFDuration = duration;
      mDTMFInterToneGap = interToneGap;
      mDTMFTimer = ITimer::create(pThis, Milliseconds(0), false);
    }

    //-------------------------------------------------------------------------
    String RTPMediaEngine::AudioSenderChannelResource::toneBuffer() const
    {
      AutoRecursiveLock lock(*this);

      return mDTMFTones;
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPMediaEngine::AudioSenderChannelResource::duration() const
    {
      AutoRecursiveLock lock(*this);

      return mDTMFDuration;
    }

    //-------------------------------------------------------------------------
    Milliseconds RTPMediaEngine::AudioSenderChannelResource::interToneGap() const
    {
      AutoRecursiveLock lock(*this);

      return mDTMFInterToneGap;
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

      auto stream = reinterpret_cast<webrtc::internal::AudioSendStream*>(mSendStream);
      if (NULL == stream) return;

      bool result = stream->DeliverRtcp(buffer->BytePtr(), buffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::onTimer(ITimerPtr timer)
    {
      AutoRecursiveLock lock(*this);

      auto pThis = ZS_DYNAMIC_PTR_CAST(AudioSenderChannelResource, mThisWeak.lock());
      IRTPMediaEngineChannelResourceAsyncDelegateProxy::create(pThis)->onSendDTMFTone();

      mDTMFTimer->cancel();
      mDTMFTimer.reset();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => webrtc::CongestionController::Observer
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs)
    {
      if (!mWorkerQueue.IsCurrent()) {
        mWorkerQueue.PostTask([this, targetBitrateBps, fractionLoss, rttMs] {
          OnNetworkChanged(targetBitrateBps, fractionLoss, rttMs);
        });
        return;
      }

      mBitrateAllocator->OnNetworkChanged(
                                          targetBitrateBps,
                                          fractionLoss,
                                          rttMs
                                          );

      AutoRecursiveLock lock(*this);

      mCurrentTargetBitrate = targetBitrateBps;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::AudioSenderChannelResource => webrtc::BitrateAllocator::LimitObserver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::AudioSenderChannelResource::OnAllocationLimitsChanged(uint32_t min_send_bitrate_bps, uint32_t max_padding_bitrate_bps)
    {
      AutoRecursiveLock lock(*this);

      mCongestionController->SetAllocatedSendBitrateLimits(min_send_bitrate_bps, max_padding_bitrate_bps);
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

      ParametersPtr parameters;
      webrtc::AudioSendStream::Config config(mTransport.get());

      std::unique_ptr<webrtc::ProcessThread> moduleProcessThread;
      std::unique_ptr<webrtc::ProcessThread> pacerThread;
      std::unique_ptr<webrtc::BitrateAllocator> bitrateAllocator;
      std::unique_ptr<webrtc::CallStats> callStats;
      std::unique_ptr<webrtc::CongestionController> congestionController;
      int channel;
      String codecPayloadName;
      BYTE codecPayloadType;
      int dtmfPayloadType;

      {
        AutoRecursiveLock lock(*this);

        parameters = mParameters;
      }

      if (!parameters) {
        notifyPromisesReject();
        return;
      }

      moduleProcessThread = webrtc::ProcessThread::Create("AudioSenderChannelResourceModuleProcessThread");
      pacerThread = webrtc::ProcessThread::Create("AudioSenderChannelResourcePacerThread");

      bitrateAllocator = std::unique_ptr<webrtc::BitrateAllocator>(new webrtc::BitrateAllocator(this));
      callStats = std::unique_ptr<webrtc::CallStats>(new webrtc::CallStats(mClock));
      congestionController =
        std::unique_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mClock,
                                                                                       this,
                                                                                       &mRemb,
                                                                                       mEventLog.get()
                                                                                       ));

      callStats->RegisterStatsObserver(congestionController.get());

      channel = webrtc::VoEBase::GetInterface(voiceEngine)->CreateChannel();

      bool audioCodecSet = false;
      webrtc::CodecInst codec;
      for (auto codecIter = parameters->mCodecs.begin(); codecIter != parameters->mCodecs.end(); codecIter++) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::getCodecKind(supportedCodec) == IRTPTypes::CodecKind_Audio && audioCodecSet)
          continue;
        codec = getAudioCodec(voiceEngine, codecIter->mName);
        codec.pltype = codecIter->mPayloadType;
        if (codecIter->mPTime != Milliseconds::zero())
          codec.pacsize = (int)((codec.plfreq / 1000) * codecIter->mPTime.count());
        if (codecIter->mNumChannels.hasValue())
          codec.channels = codecIter->mNumChannels;
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_Opus:
          {
            codec.rate = 32000 * codec.channels;
            webrtc::VoECodec::GetInterface(voiceEngine)->SetSendCodec(channel, codec);
            auto parameters = IRTPTypes::OpusCodecParameters::convert(codecIter->mParameters);
            if (parameters->mUseInbandFEC.hasValue())
              webrtc::VoECodec::GetInterface(voiceEngine)->SetFECStatus(channel, parameters->mUseInbandFEC);
            if (parameters->mUseDTX.hasValue())
              webrtc::VoECodec::GetInterface(voiceEngine)->SetOpusDtx(channel, parameters->mUseDTX);
            webrtc::VoECodec::GetInterface(voiceEngine)->SetOpusMaxPlaybackRate(channel, 48000);
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_Isac:
          case IRTPTypes::SupportedCodec_G722:
          case IRTPTypes::SupportedCodec_ILBC:
          case IRTPTypes::SupportedCodec_PCMU:
          case IRTPTypes::SupportedCodec_PCMA:
            webrtc::VoECodec::GetInterface(voiceEngine)->SetSendCodec(channel, codec);
            goto set_rtcp_feedback;
          case IRTPTypes::SupportedCodec_RED:
            break;
          case IRTPTypes::SupportedCodec_TelephoneEvent:
            dtmfPayloadType = codecIter->mPayloadType;
            break;
        }
        continue;

      set_rtcp_feedback:
        for (auto rtcpFeedbackIter = codecIter->mRTCPFeedback.begin(); rtcpFeedbackIter != codecIter->mRTCPFeedback.end(); rtcpFeedbackIter++) {
          IRTPTypes::KnownFeedbackTypes feedbackType = IRTPTypes::toKnownFeedbackType(rtcpFeedbackIter->mType);
          IRTPTypes::KnownFeedbackParameters feedbackParameter = IRTPTypes::toKnownFeedbackParameter(rtcpFeedbackIter->mParameter);
          if (IRTPTypes::KnownFeedbackType_NACK == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetNACKStatus(channel, true, 250);
          }
        }
        audioCodecSet = true;
      }

      codecPayloadName = codec.plname;
      codecPayloadType = (BYTE)codec.pltype;

      config.voe_channel_id = mChannel;

      for (auto encodingParamIter = parameters->mEncodings.begin(); encodingParamIter != parameters->mEncodings.end(); encodingParamIter++) {

        IRTPTypes::PayloadType codecPayloadType{};
        if (encodingParamIter->mCodecPayloadType.hasValue())
          codecPayloadType = encodingParamIter->mCodecPayloadType;
        else
          codecPayloadType = (BYTE)codec.pltype;

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
          webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetLocalSSRC(channel, ssrc);
          config.rtp.ssrc = ssrc;
        }
      }
      if (config.rtp.ssrc == 0) {
        uint32_t ssrc = SafeInt<uint32_t>(ortc::services::IHelper::random(1, 0xFFFFFFFF));
        webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetLocalSSRC(channel, ssrc);
        config.rtp.ssrc = ssrc;
      }

      for (auto headerExtensionIter = parameters->mHeaderExtensions.begin(); headerExtensionIter != parameters->mHeaderExtensions.end(); headerExtensionIter++) {
        IRTPTypes::HeaderExtensionURIs headerExtensionURI = IRTPTypes::toHeaderExtensionURI(headerExtensionIter->mURI);
        switch (headerExtensionURI) {
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_ClienttoMixerAudioLevelIndication:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetSendAudioLevelIndicationStatus(channel, true, (BYTE)headerExtensionIter->mID);
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_AbsoluteSendTime:
            webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetSendAbsoluteSenderTimeStatus(channel, true, (BYTE)headerExtensionIter->mID);
            config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
            break;
          default:
            break;
        }
      }

      webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetRTCPStatus(channel, true);
      webrtc::VoERTP_RTCP::GetInterface(voiceEngine)->SetRTCP_CNAME(channel, parameters->mRTCP.mCName);

      {
        AutoRecursiveLock lock(*this);

        mModuleProcessThread.swap(moduleProcessThread);
        mPacerThread.swap(pacerThread);
        mBitrateAllocator.swap(bitrateAllocator);
        mCallStats.swap(callStats);
        mCongestionController.swap(congestionController);
        mChannel = channel;
        mCodecPayloadName = codecPayloadName;
        mCodecPayloadType = codecPayloadType;
        mDTMFPayloadType = dtmfPayloadType;
      }

      mCongestionController->SetBweBitrates(10000, 40000, 100000);

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());
      mModuleProcessThread->RegisterModule(mCongestionController.get());
      mPacerThread->RegisterModule(mCongestionController->pacer());
      mPacerThread->RegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mPacerThread->Start();

      webrtc::internal::AudioSendStream *sendStream =
        new webrtc::internal::AudioSendStream(
                                              config,
                                              audioState,
                                              &mWorkerQueue,
                                              mCongestionController.get(),
                                              mBitrateAllocator.get()
                                              );

      {
        AutoRecursiveLock lock(*this);

        mSendStream = sendStream;
      }

      webrtc::VoENetwork::GetInterface(voiceEngine)->RegisterExternalTransport(channel, *mTransport);

      if (mTransportState == ISecureTransport::State_Connected)
        webrtc::VoEBase::GetInterface(voiceEngine)->StartSend(channel);

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

      int channel;
      {
        AutoRecursiveLock lock(*this);

        channel = mChannel;
      }

      auto engine = mMediaEngine.lock();
      if (engine) {
        auto voiceEngine = engine->getVoiceEngine();
        if (voiceEngine) {
          if (mTransportState == ISecureTransport::State_Connected)
            webrtc::VoEBase::GetInterface(voiceEngine)->StopSend(channel);
          webrtc::VoENetwork::GetInterface(voiceEngine)->DeRegisterExternalTransport(channel);
        }
      }

      {
        AutoRecursiveLock lock(*this);

        mPacerThread->Stop();
        mPacerThread->DeRegisterModule(mCongestionController->pacer());
        mPacerThread->DeRegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
        mModuleProcessThread->DeRegisterModule(mCongestionController.get());
        mModuleProcessThread->DeRegisterModule(mCallStats.get());
        mModuleProcessThread->Stop();

        mCallStats->DeregisterStatsObserver(mCongestionController.get());

        if (mSendStream) {
					webrtc::AudioSendStream *temp = mSendStream;
					mWorkerQueue.PostTask([temp]() {delete reinterpret_cast<webrtc::internal::AudioSendStream*>(temp); });
					mSendStream = NULL;
				}

        mCongestionController.reset();
        mCallStats.reset();
        mBitrateAllocator.reset();
        mModuleProcessThread.reset();
        mPacerThread.reset();
      }

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
    void RTPMediaEngine::AudioSenderChannelResource::notifyToneEvent(const char *tone)
    {
      if (!mDTMFSenderDelegate) return;

      try {
        mDTMFSenderDelegate->onDTMFSenderToneChanged(IDTMFSenderPtr(), tone);
      } catch (const IDTMFSenderDelegateProxy::Exceptions::DelegateGone &) {
      }
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
    void RTPMediaEngine::VideoReceiverChannelResource::ReceiverVideoRenderer::OnFrame(const webrtc::VideoFrame& frame)
    {
      auto frameCopy = make_shared<webrtc::VideoFrame>();
      frameCopy->ShallowCopy(frame);

      mVideoTrack.lock()->renderVideoFrame(frameCopy);
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
                                                                               UseMediaStreamTrackPtr track,
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
                                                                                                         UseMediaStreamTrackPtr track,
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
      if (state == mTransportState) return;

      ParametersPtr parameters;
      IRTPTypes::EncodingParametersList encodings;
      webrtc::VideoReceiveStream *receiveStream;
      ISecureTransport::States previousState;
      {
        AutoRecursiveLock lock(*this);

        receiveStream = mReceiveStream;

        parameters = mParameters;
        if (parameters)
          encodings = mParameters->mEncodings;
        previousState = mTransportState;
        mTransportState = state;
      }

      if (parameters && receiveStream && (encodings.size() == 0 ||
        encodings.size() > 0 && encodings.begin()->mActive)) {
        if (state == ISecureTransport::State_Connected)
          receiveStream->Start();
        else if (previousState == ISecureTransport::State_Connected)
          receiveStream->Stop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::onUpdate(ParametersPtr params)
    {
      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      webrtc::VideoReceiveStream *receiveStream;
      bool previousActive = false;
      bool currentActive = false;
      {
        AutoRecursiveLock lock(*this);

        receiveStream = mReceiveStream;

        ParametersPtr previousParams = mParameters;
        mParameters = params;

        if (previousParams->mEncodings.size() == 0 ||
          (previousParams->mEncodings.size() > 0 && previousParams->mEncodings.begin()->mActive)) {
          previousActive = true;
        }
        if (params->mEncodings.size() == 0 ||
          (params->mEncodings.size() > 0 && params->mEncodings.begin()->mActive)) {
          currentActive = true;
        }
      }

      if (mTransportState == ISecureTransport::State_Connected && receiveStream) {
        if (!previousActive && currentActive)
          receiveStream->Start();
        else if (previousActive && !currentActive)
          receiveStream->Stop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::onProvideStats(PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      auto track = mTrack.lock();
      if (!track) {
        notifyPromisesReject();
        return;
      }

      AutoRecursiveLock lock(*this);

      if (!mReceiveStream) {
        notifyPromisesReject();
        return;
      }

      UseStatsReport::StatMap reportStats;

      webrtc::VideoReceiveStream::Stats receiveStreamStats = mReceiveStream->GetStats();

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_InboundRTP)) {

        auto report = make_shared<IStatsReport::InboundRTPStreamStats>();

        report->mID = string(receiveStreamStats.ssrc) + "_recv";

        report->mSSRC = receiveStreamStats.ssrc;
        report->mIsRemote = true;
        report->mMediaType = "video";
        report->mMediaTrackID = track->id();
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
#ifdef WINRT
        report->mEndToEndDelay = Milliseconds(receiveStreamStats.current_endtoend_delay_ms);
#endif
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

      auto stream = reinterpret_cast<webrtc::internal::VideoReceiveStream*>(mReceiveStream);
      if (NULL == stream) return;

      webrtc::PacketTime time(timestamp, 0);
      bool result = stream->DeliverRtp(buffer->BytePtr(), buffer->SizeInBytes(), time);
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::onHandleRTCPPacket(SecureByteBlockPtr buffer)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);

      if (mDenyNonLockedAccess) return;

      auto stream = reinterpret_cast<webrtc::internal::VideoReceiveStream*>(mReceiveStream);
      if (NULL == stream) return;

      bool result = stream->DeliverRtcp(buffer->BytePtr(), buffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource => webrtc::CongestionController::Observer
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs)
    {
      mBitrateAllocator->OnNetworkChanged(
                                          targetBitrateBps,
                                          fractionLoss,
                                          rttMs
                                          );

      AutoRecursiveLock lock(*this);

      mCurrentTargetBitrate = targetBitrateBps;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoReceiverChannelResource => webrtc::BitrateAllocator::LimitObserver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoReceiverChannelResource::OnAllocationLimitsChanged(uint32_t min_send_bitrate_bps, uint32_t max_padding_bitrate_bps)
    {
      AutoRecursiveLock lock(*this);

      mCongestionController->SetAllocatedSendBitrateLimits(min_send_bitrate_bps, max_padding_bitrate_bps);
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
      auto track = mTrack.lock();
      if (!track) {
        return;
      }

      int numCpuCores;
      ParametersPtr parameters;
      webrtc::VideoReceiveStream::Config config(mTransport.get());

      std::unique_ptr<webrtc::ProcessThread> moduleProcessThread;
      std::unique_ptr<webrtc::ProcessThread> pacerThread;
      std::unique_ptr<webrtc::BitrateAllocator> bitrateAllocator;
      std::unique_ptr<webrtc::CallStats> callStats;
      std::unique_ptr<webrtc::CongestionController> congestionController;
      String codecPayloadName;
      BYTE codecPayloadType;

      {
        AutoRecursiveLock lock(*this);

        parameters = mParameters;
      }

      if (!parameters) {
        notifyPromisesReject();
        return;
      }

      moduleProcessThread = webrtc::ProcessThread::Create("VideoReceiverChannelResourceModuleProcessThread");
      pacerThread = webrtc::ProcessThread::Create("VideoReceiverChannelResourcePacerThread");

      mReceiverVideoRenderer.setMediaStreamTrack(track);

      bitrateAllocator = std::unique_ptr<webrtc::BitrateAllocator>(new webrtc::BitrateAllocator(this));
      callStats = std::unique_ptr<webrtc::CallStats>(new webrtc::CallStats(mClock));
      congestionController =
        std::unique_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mClock,
                                                                                       this,
                                                                                       &mRemb,
                                                                                       mEventLog.get()
                                                                                       ));

      callStats->RegisterStatsObserver(congestionController.get());

      numCpuCores = webrtc::CpuInfo::DetectNumberOfCores();

      webrtc::VideoReceiveStream::Decoder decoder;
      std::vector<IRTPTypes::PayloadType> rtxPayloadTypes;

      bool videoCodecSet = false;
      bool rtxCodecSet = false;
      bool redCodecSet = false;
      bool fecCodecSet = false;
      for (auto codecIter = parameters->mCodecs.begin(); codecIter != parameters->mCodecs.end(); ++codecIter) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_VP8:
          {
            if (videoCodecSet)
              continue;
            webrtc::VideoDecoder* videoDecoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::kVp8);
            decoder.decoder = videoDecoder;
            decoder.payload_name = IRTPTypes::toString(supportedCodec);
            decoder.payload_type = codecIter->mPayloadType;
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_VP9:
          {
            if (videoCodecSet)
              continue;
            webrtc::VideoDecoder* videoDecoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::kVp9);
            decoder.decoder = videoDecoder;
            decoder.payload_name = IRTPTypes::toString(supportedCodec);
            decoder.payload_type = codecIter->mPayloadType;
            goto set_rtcp_feedback;
          }
          case IRTPTypes::SupportedCodec_H264:
          {
            if (videoCodecSet)
              continue;
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
            if (rtxCodecSet)
              continue;
            rtxPayloadTypes.push_back(codecIter->mPayloadType);
            rtxCodecSet = true;
            break;
          }
          case IRTPTypes::SupportedCodec_RED:
          {
            if (redCodecSet)
              continue;
            config.rtp.fec.red_payload_type = codecIter->mPayloadType;
            redCodecSet = true;
            break;
          }
          case IRTPTypes::SupportedCodec_ULPFEC:
          {
            if (fecCodecSet)
              continue;
            config.rtp.fec.ulpfec_payload_type = codecIter->mPayloadType;
            fecCodecSet = true;
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

      codecPayloadName = decoder.payload_name;
      codecPayloadType = (BYTE)decoder.payload_type;

      for (auto encodingParamIter = parameters->mEncodings.begin(); encodingParamIter != parameters->mEncodings.end(); encodingParamIter++) {

        IRTPTypes::PayloadType codecPayloadType {};
        if (encodingParamIter->mCodecPayloadType.hasValue())
          codecPayloadType = encodingParamIter->mCodecPayloadType;
        else
          codecPayloadType = (BYTE)decoder.payload_type;

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
      uint32_t localSSRC = parameters->mRTCP.mSSRC;
      if (localSSRC == 0)
        localSSRC = 1;
      config.rtp.local_ssrc = localSSRC;

      for (auto headerExtensionIter = parameters->mHeaderExtensions.begin(); headerExtensionIter != parameters->mHeaderExtensions.end(); headerExtensionIter++) {
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

      if (parameters->mRTCP.mReducedSize)
        config.rtp.rtcp_mode = webrtc::RtcpMode::kReducedSize;
      config.decoders.push_back(decoder);
      config.renderer = &mReceiverVideoRenderer;

      {
        AutoRecursiveLock lock(*this);

        mModuleProcessThread.swap(moduleProcessThread);
        mPacerThread.swap(pacerThread);
        mBitrateAllocator.swap(bitrateAllocator);
        mCallStats.swap(callStats);
        mCongestionController.swap(congestionController);
        mCodecPayloadName = codecPayloadName;
        mCodecPayloadType = codecPayloadType;
      }

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());
      mModuleProcessThread->RegisterModule(mCongestionController.get());
      mPacerThread->RegisterModule(mCongestionController->pacer());
      mPacerThread->RegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mPacerThread->Start();

      webrtc::internal::VideoReceiveStream *receiveStream =
        new webrtc::internal::VideoReceiveStream(
                                                 numCpuCores,
                                                 mCongestionController.get(),
                                                 std::move(config),
                                                 NULL,
                                                 mModuleProcessThread.get(),
                                                 mCallStats.get(),
                                                 &mRemb
                                                 );

      {
        AutoRecursiveLock lock(*this);

        mReceiveStream = receiveStream;
      }

      if (mTransportState == ISecureTransport::State_Connected)
        receiveStream->Start();

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

      webrtc::VideoReceiveStream *receiveStream;
      {
        AutoRecursiveLock lock(*this);

        receiveStream = mReceiveStream;
      }

      if (receiveStream && mTransportState == ISecureTransport::State_Connected)
        receiveStream->Stop();

      {
        AutoRecursiveLock lock(*this);

        mPacerThread->Stop();
        mPacerThread->DeRegisterModule(mCongestionController->pacer());
        mPacerThread->DeRegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
        mModuleProcessThread->DeRegisterModule(mCongestionController.get());
        mModuleProcessThread->DeRegisterModule(mCallStats.get());
        mModuleProcessThread->Stop();

        mCallStats->DeregisterStatsObserver(mCongestionController.get());

        if (mReceiveStream)
          delete reinterpret_cast<webrtc::internal::VideoReceiveStream*>(mReceiveStream);
        mCongestionController.reset();
        mCallStats.reset();
        mBitrateAllocator.reset();
        mModuleProcessThread.reset();
        mPacerThread.reset();
      }

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
                                                                           UseMediaStreamTrackPtr track,
                                                                           ParametersPtr parameters
                                                                           ) :
      ChannelResource(priv, registration),
      mTransport(transport),
      mTrack(track),
      mParameters(parameters),
      mWorkerQueue("VideoSenderChannelResourceWorkerQueue"),
      mVideoSendDelayStats(new webrtc::SendDelayStats(mClock))
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
                                                                                                     UseMediaStreamTrackPtr track,
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
      if (state == mTransportState) return;

      if (!mSendStream) return;

      ParametersPtr parameters;
      IRTPTypes::EncodingParametersList encodings;
      webrtc::VideoSendStream *sendStream;
      ISecureTransport::States previousState;
      {
        AutoRecursiveLock lock(*this);

        sendStream = mSendStream;

        parameters = mParameters;
        if (parameters)
          encodings = mParameters->mEncodings;
        previousState = mTransportState;
        mTransportState = state;
      }

      if (parameters && sendStream && (encodings.size() == 0 ||
        encodings.size() > 0 && encodings.begin()->mActive)) {
        if (state == ISecureTransport::State_Connected)
          sendStream->Start();
        else if (previousState == ISecureTransport::State_Connected)
          sendStream->Stop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::onUpdate(ParametersPtr params)
    {
      auto engine = mMediaEngine.lock();
      if (!engine) return;

      auto voiceEngine = engine->getVoiceEngine();
      if (!voiceEngine) return;

      webrtc::VideoSendStream *sendStream;
      bool previousActive = false;
      bool currentActive = false;
      {
        AutoRecursiveLock lock(*this);

        sendStream = mSendStream;

        ParametersPtr previousParams = mParameters;
        mParameters = params;

        if (previousParams->mEncodings.size() == 0 ||
          (previousParams->mEncodings.size() > 0 && previousParams->mEncodings.begin()->mActive)) {
          previousActive = true;
        }
        if (params->mEncodings.size() == 0 ||
          (params->mEncodings.size() > 0 && params->mEncodings.begin()->mActive)) {
          currentActive = true;
        }
      }

      if (mTransportState == ISecureTransport::State_Connected && sendStream) {
        if (!previousActive && currentActive)
          sendStream->Start();
        else if (previousActive && !currentActive)
          sendStream->Stop();
      }
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::onProvideStats(PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      auto track = mTrack.lock();
      if (!track) {
        notifyPromisesReject();
        return;
      }

      AutoRecursiveLock lock(*this);

      if (!mSendStream) {
        notifyPromisesReject();
        return;
      }

      UseStatsReport::StatMap reportStats;

      webrtc::VideoSendStream::Stats sendStreamStats = mSendStream->GetStats();

      if (stats.hasStatType(IStatsReportTypes::StatsTypes::StatsType_OutboundRTP)) {

        for (auto statsIter = sendStreamStats.substreams.begin(); statsIter != sendStreamStats.substreams.end(); statsIter++) {

          auto report = make_shared<IStatsReport::OutboundRTPStreamStats>();

          report->mID = string((*statsIter).first) + "_send";

          report->mSSRC = (*statsIter).first;
          report->mIsRemote = false;
          report->mMediaType = "video";
          report->mMediaTrackID = track->id();
          report->mCodecID = mCodecPayloadName;
          report->mFIRCount = (*statsIter).second.rtcp_packet_type_counts.fir_packets;
          report->mPLICount = (*statsIter).second.rtcp_packet_type_counts.pli_packets;
          report->mNACKCount = (*statsIter).second.rtcp_packet_type_counts.nack_packets;
          report->mPacketsSent = (*statsIter).second.rtp_stats.transmitted.packets;
          report->mBytesSent = (*statsIter).second.rtp_stats.transmitted.header_bytes +
            (*statsIter).second.rtp_stats.transmitted.payload_bytes +
            (*statsIter).second.rtp_stats.transmitted.padding_bytes;
          report->mTargetBitrate = (zsLib::DOUBLE)mCurrentTargetBitrate;
          report->mRoundTripTime = (zsLib::DOUBLE)mCallStats->rtcp_rtt_stats()->LastProcessedRtt();

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
    void RTPMediaEngine::VideoSenderChannelResource::sendVideoFrame(VideoFramePtr videoFrame)
    {
      IRTPMediaEngineHandlePacketAsyncDelegateProxy::createUsingQueue(mHandlePacketQueue, getThis<VideoSenderChannelResource>())->onSendVideoFrame(videoFrame);
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

      auto stream = reinterpret_cast<webrtc::internal::VideoSendStream*>(mSendStream);
      if (NULL == stream) return;

      bool result = stream->DeliverRtcp(buffer->BytePtr(), buffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::onSendVideoFrame(VideoFramePtr videoFrame)
    {
      AutoIncrementLock incLock(mAccessFromNonLockedMethods);

      if (mDenyNonLockedAccess) return;

      if (NULL == mSendStream) return;

      mSendStream->Input()->IncomingCapturedFrame(*videoFrame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource => webrtc::CongestionController::Observer
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::OnNetworkChanged(uint32_t targetBitrateBps, uint8_t fractionLoss, int64_t rttMs)
    {
      if (!mWorkerQueue.IsCurrent()) {
        mWorkerQueue.PostTask([this, targetBitrateBps, fractionLoss, rttMs] {
          OnNetworkChanged(targetBitrateBps, fractionLoss, rttMs);
        });
        return;
      }

      mBitrateAllocator->OnNetworkChanged(
                                          targetBitrateBps,
                                          fractionLoss,
                                          rttMs
                                          );

      AutoRecursiveLock lock(*this);

      mCurrentTargetBitrate = targetBitrateBps;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPMediaEngine::VideoSenderChannelResource => webrtc::BitrateAllocator::LimitObserver
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPMediaEngine::VideoSenderChannelResource::OnAllocationLimitsChanged(uint32_t min_send_bitrate_bps, uint32_t max_padding_bitrate_bps)
    {
      AutoRecursiveLock lock(*this);

      mCongestionController->SetAllocatedSendBitrateLimits(min_send_bitrate_bps, max_padding_bitrate_bps);
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
      auto track = mTrack.lock();
      if (!track) return;

      auto trackSettings = track->getSettings();
      if (!trackSettings) return;

      int numCpuCores;
      ParametersPtr parameters;
      webrtc::VideoSendStream::Config config(mTransport.get());
      webrtc::VideoEncoderConfig encoderConfig;
      std::map<uint32_t, webrtc::RtpState> suspendedSSRCs;

      std::unique_ptr<webrtc::ProcessThread> moduleProcessThread;
      std::unique_ptr<webrtc::ProcessThread> pacerThread;
      std::unique_ptr<webrtc::BitrateAllocator> bitrateAllocator;
      std::unique_ptr<webrtc::CallStats> callStats;
      std::unique_ptr<webrtc::CongestionController> congestionController;
      String codecPayloadName;
      BYTE codecPayloadType;

      {
        AutoRecursiveLock lock(*this);

        parameters = mParameters;
      }

      if (!parameters) {
        notifyPromisesReject();
        return;
      }

      moduleProcessThread = webrtc::ProcessThread::Create("VideoSenderChannelResourceModuleProcessThread");
      pacerThread = webrtc::ProcessThread::Create("VideoSenderChannelResourcePacerThread");

      bitrateAllocator = std::unique_ptr<webrtc::BitrateAllocator>(new webrtc::BitrateAllocator(this));
      callStats = std::unique_ptr<webrtc::CallStats>(new webrtc::CallStats(mClock));
      congestionController =
        std::unique_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mClock,
                                                                                       this,
                                                                                       &mRemb,
                                                                                       mEventLog.get()
                                                                                       ));

      callStats->RegisterStatsObserver(congestionController.get());

      numCpuCores = webrtc::CpuInfo::DetectNumberOfCores();

      size_t sourceWidth = 640;
      size_t sourceHeight = 480;
      int sourceMaxFramerate = 15;
      if (trackSettings->mWidth.hasValue())
        sourceWidth = trackSettings->mWidth.value();
      if (trackSettings->mHeight.hasValue())
        sourceHeight = trackSettings->mHeight.value();
      if (trackSettings->mFrameRate.hasValue())
        sourceMaxFramerate = (int)trackSettings->mFrameRate.value();

      encoderConfig.min_transmit_bitrate_bps = 0;
      encoderConfig.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;

      bool videoCodecSet = false;
      bool rtxCodecSet = false;
      bool redCodecSet = false;
      bool fecCodecSet = false;
      for (auto codecIter = parameters->mCodecs.begin(); codecIter != parameters->mCodecs.end(); codecIter++) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_VP8:
          {
            if (videoCodecSet)
              continue;
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
            if (videoCodecSet)
              continue;
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
            if (videoCodecSet)
              continue;
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
            if (rtxCodecSet)
              continue;
            config.rtp.rtx.payload_type = codecIter->mPayloadType;
            rtxCodecSet = true;
            break;
          }
          case IRTPTypes::SupportedCodec_RED:
          {
            if (redCodecSet)
              continue;
            config.rtp.fec.red_payload_type = codecIter->mPayloadType;
            redCodecSet = true;
            break;
          }
          case IRTPTypes::SupportedCodec_ULPFEC:
          {
            if (fecCodecSet)
              continue;
            config.rtp.fec.ulpfec_payload_type = codecIter->mPayloadType;
            fecCodecSet = true;
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

      codecPayloadName = config.encoder_settings.payload_name;
      codecPayloadType = (BYTE)config.encoder_settings.payload_type;

      int totalMinBitrate = 0;
      int totalMaxBitrate = 0;
      int totalTargetBitrate = 0;
      int numberOfTemporalLayers[webrtc::kMaxSimulcastStreams] = {};
      int numberOfSpatialLayers[webrtc::kMaxSimulcastStreams] = {};
      int streamIndex = 0;
      // Vectors of both spatial and temporal layer encoding IDs, one vector per each simulcast stream.
      // Base layer is first in a row and enhancement layers are positioned after.
      std::vector<std::vector<String>> encodingIDs;
      for (auto encodingParamIter = parameters->mEncodings.begin(); encodingParamIter != parameters->mEncodings.end(); encodingParamIter++) {

        IRTPTypes::PayloadType codecPayloadType{};
        if (encodingParamIter->mCodecPayloadType.hasValue())
          codecPayloadType = encodingParamIter->mCodecPayloadType;
        else
          codecPayloadType = (BYTE)config.encoder_settings.payload_type;

        if (codecPayloadType != config.encoder_settings.payload_type)
          continue;

        if (encodingParamIter->mDependencyEncodingIDs.size() == 0) {
          encodingIDs.push_back(std::vector<String>());
          numberOfTemporalLayers[streamIndex] = 1;
          numberOfSpatialLayers[streamIndex] = 1;
          if (!encodingParamIter->mEncodingID.empty())
            encodingIDs[streamIndex].push_back(encodingParamIter->mEncodingID);
        }
        int dependencyStreamIndex = 0;
        bool isTemporalLayer = false;
        bool isSpatialLayer = false;
        for (auto dependencyEncodingIDIter = encodingParamIter->mDependencyEncodingIDs.begin(); dependencyEncodingIDIter != encodingParamIter->mDependencyEncodingIDs.end(); dependencyEncodingIDIter++) {
          dependencyStreamIndex = 0;
          for (auto encodingIDVectorIter = encodingIDs.begin(); encodingIDVectorIter != encodingIDs.end(); encodingIDVectorIter++) {
            for (auto encodingIDStringIter = encodingIDVectorIter->begin(); encodingIDStringIter != encodingIDVectorIter->end(); encodingIDStringIter++) {
              if (encodingIDStringIter->compare(*dependencyEncodingIDIter) != 0)
                continue;
              for (auto dependencyEncodingParamIter = parameters->mEncodings.begin(); dependencyEncodingParamIter != encodingParamIter; dependencyEncodingParamIter++) {
                if (dependencyEncodingParamIter->mEncodingID.compare(*dependencyEncodingIDIter) != 0)
                  continue;
                if (encodingParamIter->mFramerateScale.hasValue() && dependencyEncodingParamIter->mFramerateScale.hasValue() &&
                  encodingParamIter->mFramerateScale.value() != dependencyEncodingParamIter->mFramerateScale.value() ||
                  !encodingParamIter->mFramerateScale.hasValue() && dependencyEncodingParamIter->mFramerateScale.hasValue() &&
                  dependencyEncodingParamIter->mFramerateScale.value() != 1.0 ||
                  encodingParamIter->mFramerateScale.hasValue() && !dependencyEncodingParamIter->mFramerateScale.hasValue() &&
                  encodingParamIter->mFramerateScale.value() != 1.0
                  ) {
                  isTemporalLayer = true;
                }
                if (encodingParamIter->mResolutionScale.hasValue() && dependencyEncodingParamIter->mResolutionScale.hasValue() &&
                  encodingParamIter->mResolutionScale.value() != dependencyEncodingParamIter->mResolutionScale.value() ||
                  !encodingParamIter->mResolutionScale.hasValue() && dependencyEncodingParamIter->mResolutionScale.hasValue() &&
                  dependencyEncodingParamIter->mResolutionScale.value() != 1.0 ||
                  encodingParamIter->mResolutionScale.hasValue() && !dependencyEncodingParamIter->mResolutionScale.hasValue() &&
                  encodingParamIter->mResolutionScale.value() != 1.0
                  ) {
                  isSpatialLayer = true;
                }
                break;
              }
              break;
            }
            if (isTemporalLayer || isSpatialLayer)
              break;
            dependencyStreamIndex++;
          }
        }
        if (isTemporalLayer)
          numberOfTemporalLayers[dependencyStreamIndex]++;
        if (isSpatialLayer)
          numberOfSpatialLayers[dependencyStreamIndex]++;
        if ((isTemporalLayer || isSpatialLayer) && !encodingParamIter->mEncodingID.empty())
          encodingIDs[dependencyStreamIndex].push_back(encodingParamIter->mEncodingID);

        if (encodingParamIter->mDependencyEncodingIDs.size() == 0)
          streamIndex++;
        else
          continue;

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
          ssrc = SafeInt<uint32_t>(ortc::services::IHelper::random(1, 0xFFFFFFFF));
        config.rtp.ssrcs.push_back(ssrc);
        if (encodingParamIter->mRTX.hasValue()) {
          IRTPTypes::RTXParameters rtx = encodingParamIter->mRTX;
          if (rtx.mSSRC.hasValue())
            config.rtp.rtx.ssrcs.push_back(rtx.mSSRC);
        }
        webrtc::VideoStream stream;
        stream.width = encodingParamIter->mResolutionScale.hasValue() ? (size_t)(sourceWidth / encodingParamIter->mResolutionScale) : sourceWidth;
        stream.height = encodingParamIter->mResolutionScale.hasValue() ? (size_t)(sourceHeight / encodingParamIter->mResolutionScale) : sourceHeight;
        stream.max_framerate = encodingParamIter->mFramerateScale.hasValue() ? (int)(sourceMaxFramerate / encodingParamIter->mFramerateScale) : sourceMaxFramerate;
        stream.min_bitrate_bps = 30000;
        stream.max_bitrate_bps = encodingParamIter->mMaxBitrate.hasValue() ? (int)encodingParamIter->mMaxBitrate : 2000000;
        stream.target_bitrate_bps = stream.max_bitrate_bps / 2;
        stream.max_qp = 56;
        encoderConfig.streams.push_back(stream);
        totalMinBitrate += stream.min_bitrate_bps;
        totalMaxBitrate += stream.max_bitrate_bps;
        totalTargetBitrate += stream.target_bitrate_bps;
      }
      if (encoderConfig.streams.size() == 0) {
        config.rtp.ssrcs.push_back(SafeInt<uint32_t>(ortc::services::IHelper::random(1, 0xFFFFFFFF)));
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
      auto supportedCodec = IRTPTypes::toSupportedCodec(codecPayloadName);
      if (supportedCodec == IRTPTypes::SupportedCodec_VP8) {
        if (encoderConfig.streams.size() > 1)
          mVideoEncoderSettings.mVp8.automaticResizeOn = false;
        for (size_t i = 0; i < encoderConfig.streams.size(); i++) {
          if (numberOfTemporalLayers[i] > 0)
            encoderConfig.streams[i].temporal_layer_thresholds_bps.resize(numberOfTemporalLayers[i] - 1);
        }
      } else if (supportedCodec == IRTPTypes::SupportedCodec_VP9) {
        for (size_t i = 0; i < encoderConfig.streams.size(); i++) {
          if (numberOfTemporalLayers[i] > 0)
            encoderConfig.streams[i].temporal_layer_thresholds_bps.resize(numberOfTemporalLayers[i] - 1);
        }
        mVideoEncoderSettings.mVp9.numberOfSpatialLayers = (unsigned char)numberOfSpatialLayers[0];
      }

      for (auto headerExtensionIter = parameters->mHeaderExtensions.begin(); headerExtensionIter != parameters->mHeaderExtensions.end(); headerExtensionIter++) {
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

      config.rtp.c_name = parameters->mRTCP.mCName;

      {
        AutoRecursiveLock lock(*this);

        mModuleProcessThread.swap(moduleProcessThread);
        mPacerThread.swap(pacerThread);
        mBitrateAllocator.swap(bitrateAllocator);
        mCallStats.swap(callStats);
        mCongestionController.swap(congestionController);
        mCodecPayloadName = codecPayloadName;
        mCodecPayloadType = codecPayloadType;
      }

      mCongestionController->SetBweBitrates(totalMinBitrate, totalTargetBitrate, totalMaxBitrate);
      mVideoSendDelayStats->AddSsrcs(config);

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());
      mModuleProcessThread->RegisterModule(mCongestionController.get());
      mPacerThread->RegisterModule(mCongestionController->pacer());
      mPacerThread->RegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
      mPacerThread->Start();

      webrtc::internal::VideoSendStream *sendStream =
        new webrtc::internal::VideoSendStream(
                                              numCpuCores,
                                              mModuleProcessThread.get(),
                                              &mWorkerQueue,
                                              mCallStats.get(),
                                              mCongestionController.get(),
                                              mBitrateAllocator.get(),
                                              mVideoSendDelayStats.get(),
                                              &mRemb,
                                              mEventLog.get(),
                                              std::move(config),
                                              std::move(encoderConfig),
                                              suspendedSSRCs
                                              );

      {
        AutoRecursiveLock lock(*this);

        mSendStream = sendStream;
      }

      if (mTransportState == ISecureTransport::State_Connected)
        sendStream->Start();

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

      webrtc::VideoSendStream *sendStream;
      {
        AutoRecursiveLock lock(*this);

        sendStream = mSendStream;
      }

      if (sendStream && mTransportState == ISecureTransport::State_Connected)
        sendStream->Stop();

      {
        AutoRecursiveLock lock(*this);

        mPacerThread->Stop();
        mPacerThread->DeRegisterModule(mCongestionController->pacer());
        mPacerThread->DeRegisterModule(mCongestionController->GetRemoteBitrateEstimator(true));
        mModuleProcessThread->DeRegisterModule(mCongestionController.get());
        mModuleProcessThread->DeRegisterModule(mCallStats.get());
        mModuleProcessThread->Stop();

        mCallStats->DeregisterStatsObserver(mCongestionController.get());

				if (mSendStream) {
					webrtc::VideoSendStream *temp = mSendStream;
					mWorkerQueue.PostTask([temp]() {delete reinterpret_cast<webrtc::internal::VideoSendStream*>(temp); });
					mSendStream = NULL;
				}
        mCongestionController.reset();
        mCallStats.reset();
        mBitrateAllocator.reset();
        mModuleProcessThread.reset();
        mPacerThread.reset();
      }
      
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
