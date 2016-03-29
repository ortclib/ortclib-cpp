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
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_Tracing.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Singleton.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(RTPMediaEngineRegistration)
    ZS_DECLARE_CLASS_PTR(RTPMediaEngineSingleton)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

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
        auto promise = PromiseWithRTPMediaEngine::create(IORTCForInternal::queueDelegate());
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
    #pragma mark IICETransportForSettings
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
      if (!singleton) return PromiseWithRTPMediaEngineDeviceResource::createRejected(IORTCForInternal::queueDelegate());
      return singleton->getEngineRegistration()->getRTPEngine()->getDeviceResource(deviceID);
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineSetupChannelResultPtr IRTPMediaEngineForRTPReceiverChannelMediaBase::setupChannel(UseReceiverChannelMediaBasePtr channel)
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineSetupChannelResult::createRejected(IORTCForInternal::queueDelegate());
      return singleton->getEngineRegistration()->getRTPEngine()->setupChannel(channel);
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineCloseChannelResultPtr IRTPMediaEngineForRTPReceiverChannelMediaBase::closeChannel(UseReceiverChannelMediaBasePtr channel)
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineCloseChannelResult::createRejected(IORTCForInternal::queueDelegate());
      return singleton->getEngineRegistration()->getRTPEngine()->closeChannel(channel);
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
      if (!singleton) return PromiseWithRTPMediaEngineDeviceResource::createRejected(IORTCForInternal::queueDelegate());
      return singleton->getEngineRegistration()->getRTPEngine()->getDeviceResource(deviceID);
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineSetupChannelResultPtr IRTPMediaEngineForRTPSenderChannelMediaBase::setupChannel(UseSenderChannelMediaBasePtr channel)
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineSetupChannelResult::createRejected(IORTCForInternal::queueDelegate());
      return singleton->getEngineRegistration()->getRTPEngine()->setupChannel(channel);
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineCloseChannelResultPtr IRTPMediaEngineForRTPSenderChannelMediaBase::closeChannel(UseSenderChannelMediaBasePtr channel)
    {
      auto singleton = RTPMediaEngineSingleton::singleton();
      if (!singleton) return PromiseWithRTPMediaEngineCloseChannelResult::createRejected(IORTCForInternal::queueDelegate());
      return singleton->getEngineRegistration()->getRTPEngine()->closeChannel(channel);
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
      mRegistration(registration)
    {
      EventWriteOrtcRtpMediaEngineCreate(__func__, mID);
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::~RTPMediaEngine()
    {
      if (isNoop()) return;

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
        return PromiseWithRTPMediaEngine::createRejected(IORTCForInternal::queueDelegate());
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
        promise->resolve(registration);
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
    PromiseWithRTPMediaEngineSetupChannelResultPtr RTPMediaEngine::setupChannel(UseReceiverChannelMediaBasePtr channel)
    {
      auto promise = PromiseWithRTPMediaEngineSetupChannelResult::create(IORTCForInternal::queueDelegate());

      {
        AutoRecursiveLock lock(*this);
        if (ZS_DYNAMIC_PTR_CAST(IRTPReceiverChannelAudioForRTPMediaEngine, channel)) {
          mPendingSetupReceiverAudioChannels[channel->getID()] = channel;
          mPendingSetupReceiverAudioChannelPromises[channel->getID()] = promise;
        } else if (ZS_DYNAMIC_PTR_CAST(IRTPReceiverChannelVideoForRTPMediaEngine, channel)) {
          mPendingSetupReceiverVideoChannels[channel->getID()] = channel;
          mPendingSetupReceiverVideoChannelPromises[channel->getID()] = promise;
        }
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      return promise;
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineCloseChannelResultPtr RTPMediaEngine::closeChannel(UseReceiverChannelMediaBasePtr channel)
    {
      auto promise = PromiseWithRTPMediaEngineCloseChannelResult::create(IORTCForInternal::queueDelegate());

      {
        AutoRecursiveLock lock(*this);
        if (ZS_DYNAMIC_PTR_CAST(IRTPReceiverChannelAudioForRTPMediaEngine, channel)) {
          mPendingCloseReceiverAudioChannels[channel->getID()] = channel;
          mPendingCloseReceiverAudioChannelPromises[channel->getID()] = promise;
        } else if (ZS_DYNAMIC_PTR_CAST(IRTPReceiverChannelVideoForRTPMediaEngine, channel)) {
          mPendingCloseReceiverVideoChannels[channel->getID()] = channel;
          mPendingCloseReceiverVideoChannelPromises[channel->getID()] = promise;
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
    rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter> RTPMediaEngine::getVoiceEngine()
    {
      AutoRecursiveLock lock(*this);

      return rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter>(mVoiceEngine.get());
    }

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
    PromiseWithRTPMediaEngineSetupChannelResultPtr RTPMediaEngine::setupChannel(UseSenderChannelMediaBasePtr channel)
    {
      auto promise = PromiseWithRTPMediaEngineSetupChannelResult::create(IORTCForInternal::queueDelegate());

      {
        AutoRecursiveLock lock(*this);
        if (ZS_DYNAMIC_PTR_CAST(IRTPSenderChannelAudioForRTPMediaEngine, channel)) {
          mPendingSetupSenderAudioChannels[channel->getID()] = channel;
          mPendingSetupSenderAudioChannelPromises[channel->getID()] = promise;
        } else if (ZS_DYNAMIC_PTR_CAST(IRTPSenderChannelVideoForRTPMediaEngine, channel)) {
          mPendingCloseSenderVideoChannels[channel->getID()] = channel;
          mPendingCloseSenderVideoChannelPromises[channel->getID()] = promise;
        }
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      return promise;
    }

    //-------------------------------------------------------------------------
    PromiseWithRTPMediaEngineCloseChannelResultPtr RTPMediaEngine::closeChannel(UseSenderChannelMediaBasePtr channel)
    {
      auto promise = PromiseWithRTPMediaEngineCloseChannelResult::create(IORTCForInternal::queueDelegate());

      {
        AutoRecursiveLock lock(*this);
        if (ZS_DYNAMIC_PTR_CAST(IRTPSenderChannelAudioForRTPMediaEngine, channel)) {
          mPendingSetupSenderAudioChannels[channel->getID()] = channel;
          mPendingSetupSenderAudioChannelPromises[channel->getID()] = promise;
        } else if (ZS_DYNAMIC_PTR_CAST(IRTPSenderChannelVideoForRTPMediaEngine, channel)) {
          mPendingCloseSenderVideoChannels[channel->getID()] = channel;
          mPendingCloseSenderVideoChannelPromises[channel->getID()] = promise;
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

      UseServicesHelper::debugAppend(resultEl, "pending device resources", mExampleDeviceResources.size());

      UseServicesHelper::debugAppend(resultEl, "pending setup receiver audio channel", mPendingSetupReceiverAudioChannels.size());
      UseServicesHelper::debugAppend(resultEl, "pending setup receiver video channel", mPendingSetupReceiverVideoChannels.size());
      UseServicesHelper::debugAppend(resultEl, "pending setup sender audio channel", mPendingSetupSenderAudioChannels.size());
      UseServicesHelper::debugAppend(resultEl, "pending setup sender video channel", mPendingSetupSenderVideoChannels.size());

      UseServicesHelper::debugAppend(resultEl, "pending close receiver audio channel", mPendingCloseReceiverAudioChannels.size());
      UseServicesHelper::debugAppend(resultEl, "pending close receiver video channel", mPendingCloseReceiverVideoChannels.size());
      UseServicesHelper::debugAppend(resultEl, "pending close sender audio channel", mPendingCloseSenderAudioChannels.size());
      UseServicesHelper::debugAppend(resultEl, "pending close sender video channel", mPendingCloseSenderVideoChannels.size());

      return resultEl;
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
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepSetup()) goto not_ready;
      if (!stepExampleSetupDeviceResources()) goto not_ready;
      if (!stepSetupSenderChannel()) goto not_ready;
      if (!stepCloseSenderChannel()) goto not_ready;
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
          deviceResource->notifyReady();
        }

        mExamplePendingDeviceResources.pop_front();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepSetupSenderChannel()
    {
      ReceiverChannelMap pendingSetupReceiverAudioChannels;
      PendingPromiseMap pendingSetupReceiverAudioChannelPromises;
      ReceiverChannelMap pendingSetupReceiverVideoChannels;
      PendingPromiseMap pendingSetupReceiverVideoChannelPromises;
      SenderChannelMap pendingSetupSenderAudioChannels;
      PendingPromiseMap pendingSetupSenderAudioChannelPromises;
      SenderChannelMap pendingSetupSenderVideoChannels;
      PendingPromiseMap pendingSetupSenderVideoChannelPromises;

      {
        AutoRecursiveLock lock(*this);

        pendingSetupReceiverAudioChannels = mPendingSetupReceiverAudioChannels;
        pendingSetupReceiverAudioChannelPromises = mPendingSetupReceiverAudioChannelPromises;
        pendingSetupReceiverVideoChannels = mPendingSetupReceiverVideoChannels;
        pendingSetupReceiverVideoChannelPromises = mPendingSetupReceiverVideoChannelPromises;
        pendingSetupSenderAudioChannels = mPendingSetupSenderAudioChannels;
        pendingSetupSenderAudioChannelPromises = mPendingSetupSenderAudioChannelPromises;
        pendingSetupSenderVideoChannels = mPendingSetupSenderVideoChannels;
        pendingSetupSenderVideoChannelPromises = mPendingSetupSenderVideoChannelPromises;

        mPendingSetupReceiverAudioChannels.clear();
        mPendingSetupReceiverAudioChannelPromises.clear();
        mPendingSetupReceiverVideoChannels.clear();
        mPendingSetupReceiverVideoChannelPromises.clear();
        mPendingSetupSenderAudioChannels.clear();
        mPendingSetupSenderAudioChannelPromises.clear();
        mPendingSetupSenderVideoChannels.clear();
        mPendingSetupSenderVideoChannelPromises.clear();
      }

      ReceiverChannelMap::iterator receiverChannelIter;
      SenderChannelMap::iterator senderChannelIter;

      receiverChannelIter = pendingSetupReceiverAudioChannels.begin();
      while (receiverChannelIter != pendingSetupReceiverAudioChannels.end()) {

        PUID receiverChannelPUID = receiverChannelIter->first;
        UseReceiverChannelMediaBasePtr receiverChannel = receiverChannelIter->second.lock();

        if (receiverChannel) {
          receiverChannel->setupChannel();
          PromisePtr channelPromise = pendingSetupReceiverAudioChannelPromises[receiverChannelPUID].lock();
          ASSERT(channelPromise)
          channelPromise->resolve();
        }

        receiverChannelIter++;
      }

      receiverChannelIter = pendingSetupReceiverVideoChannels.begin();
      while (receiverChannelIter != pendingSetupReceiverVideoChannels.end()) {

        PUID receiverChannelPUID = receiverChannelIter->first;
        UseReceiverChannelMediaBasePtr receiverChannel = receiverChannelIter->second.lock();

        if (receiverChannel) {
          receiverChannel->setupChannel();
          PromisePtr channelPromise = pendingSetupReceiverVideoChannelPromises[receiverChannelPUID].lock();
          ASSERT(channelPromise)
            channelPromise->resolve();
        }

        receiverChannelIter++;
      }

      senderChannelIter = pendingSetupSenderAudioChannels.begin();
      while (senderChannelIter != pendingSetupSenderAudioChannels.end()) {

        PUID senderChannelPUID = senderChannelIter->first;
        UseSenderChannelMediaBasePtr senderChannel = senderChannelIter->second.lock();

        if (senderChannel) {
          senderChannel->setupChannel();
          PromisePtr channelPromise = pendingSetupSenderAudioChannelPromises[senderChannelPUID].lock();
          ASSERT(channelPromise)
            channelPromise->resolve();
        }

        senderChannelIter++;
      }

      senderChannelIter = pendingSetupSenderVideoChannels.begin();
      while (senderChannelIter != pendingSetupSenderVideoChannels.end()) {

        PUID senderChannelPUID = senderChannelIter->first;
        UseSenderChannelMediaBasePtr senderChannel = senderChannelIter->second.lock();

        if (senderChannel) {
          senderChannel->setupChannel();
          PromisePtr channelPromise = pendingSetupSenderVideoChannelPromises[senderChannelPUID].lock();
          ASSERT(channelPromise)
            channelPromise->resolve();
        }

        senderChannelIter++;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPMediaEngine::stepCloseSenderChannel()
    {
      ReceiverChannelMap pendingCloseReceiverAudioChannels;
      PendingPromiseMap pendingCloseReceiverAudioChannelPromises;
      ReceiverChannelMap pendingCloseReceiverVideoChannels;
      PendingPromiseMap pendingCloseReceiverVideoChannelPromises;
      SenderChannelMap pendingCloseSenderAudioChannels;
      PendingPromiseMap pendingCloseSenderAudioChannelPromises;
      SenderChannelMap pendingCloseSenderVideoChannels;
      PendingPromiseMap pendingCloseSenderVideoChannelPromises;

      {
        AutoRecursiveLock lock(*this);

        pendingCloseReceiverAudioChannels = mPendingCloseReceiverAudioChannels;
        pendingCloseReceiverAudioChannelPromises = mPendingCloseReceiverAudioChannelPromises;
        pendingCloseReceiverVideoChannels = mPendingCloseReceiverVideoChannels;
        pendingCloseReceiverVideoChannelPromises = mPendingCloseReceiverVideoChannelPromises;
        pendingCloseSenderAudioChannels = mPendingCloseSenderAudioChannels;
        pendingCloseSenderAudioChannelPromises = mPendingCloseSenderAudioChannelPromises;
        pendingCloseSenderVideoChannels = mPendingCloseSenderVideoChannels;
        pendingCloseSenderVideoChannelPromises = mPendingCloseSenderVideoChannelPromises;

        mPendingCloseReceiverAudioChannels.clear();
        mPendingCloseReceiverAudioChannelPromises.clear();
        mPendingCloseReceiverVideoChannels.clear();
        mPendingCloseReceiverVideoChannelPromises.clear();
        mPendingCloseSenderAudioChannels.clear();
        mPendingCloseSenderAudioChannelPromises.clear();
        mPendingCloseSenderVideoChannels.clear();
        mPendingCloseSenderVideoChannelPromises.clear();
      }

      ReceiverChannelMap::iterator receiverChannelIter;
      SenderChannelMap::iterator senderChannelIter;

      receiverChannelIter = pendingCloseReceiverAudioChannels.begin();
      while (receiverChannelIter != pendingCloseReceiverAudioChannels.end()) {

        PUID receiverChannelPUID = receiverChannelIter->first;
        UseReceiverChannelMediaBasePtr receiverChannel = receiverChannelIter->second.lock();

        if (receiverChannel) {
          receiverChannel->setupChannel();
          PromisePtr channelPromise = pendingCloseReceiverAudioChannelPromises[receiverChannelPUID].lock();
          ASSERT(channelPromise)
            channelPromise->resolve();
        }

        receiverChannelIter++;
      }

      receiverChannelIter = pendingCloseReceiverVideoChannels.begin();
      while (receiverChannelIter != pendingCloseReceiverVideoChannels.end()) {

        PUID receiverChannelPUID = receiverChannelIter->first;
        UseReceiverChannelMediaBasePtr receiverChannel = receiverChannelIter->second.lock();

        if (receiverChannel) {
          receiverChannel->setupChannel();
          PromisePtr channelPromise = pendingCloseReceiverVideoChannelPromises[receiverChannelPUID].lock();
          ASSERT(channelPromise)
            channelPromise->resolve();
        }

        receiverChannelIter++;
      }

      senderChannelIter = pendingCloseSenderAudioChannels.begin();
      while (senderChannelIter != pendingCloseSenderAudioChannels.end()) {

        PUID senderChannelPUID = senderChannelIter->first;
        UseSenderChannelMediaBasePtr senderChannel = senderChannelIter->second.lock();

        if (senderChannel) {
          senderChannel->setupChannel();
          PromisePtr channelPromise = pendingCloseSenderAudioChannelPromises[senderChannelPUID].lock();
          ASSERT(channelPromise)
            channelPromise->resolve();
        }

        senderChannelIter++;
      }

      senderChannelIter = pendingCloseSenderVideoChannels.begin();
      while (senderChannelIter != pendingCloseSenderVideoChannels.end()) {

        PUID senderChannelPUID = senderChannelIter->first;
        UseSenderChannelMediaBasePtr senderChannel = senderChannelIter->second.lock();

        if (senderChannel) {
          senderChannel->setupChannel();
          PromisePtr channelPromise = pendingCloseSenderVideoChannelPromises[senderChannelPUID].lock();
          ASSERT(channelPromise)
            channelPromise->resolve();
        }

        senderChannelIter++;
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
            front->resolve(registration);
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
    #pragma mark RTPMediaEngine::DeviceResource
    #pragma mark

    //-------------------------------------------------------------------------
    RTPMediaEngine::BaseResource::BaseResource(
                                               const make_private &,
                                               IMessageQueuePtr queue,
                                               IRTPMediaEngineRegistrationPtr registration
                                               ) :
      SharedRecursiveLock(SharedRecursiveLock::create()),
      MessageQueueAssociator(queue),
      mRegistration(registration),
      mMediaEngine(registration ? registration->getRTPEngine() : RTPMediaEnginePtr())
    {
    }

    //-------------------------------------------------------------------------
    RTPMediaEngine::BaseResource::~BaseResource()
    {
      mThisWeak.reset();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::BaseResource::notifyReady()
    {
      {
        AutoRecursiveLock lock(*this);
        mNotifiedReady = true;
      }
      internalFixState();
    }

    //-------------------------------------------------------------------------
    void RTPMediaEngine::BaseResource::notifyRejected()
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
    #pragma mark RTPMediaEngine::DeviceResource => (internal)
    #pragma mark


    //-------------------------------------------------------------------------
    IMessageQueuePtr RTPMediaEngine::BaseResource::delegateQueue()
    {
      return IORTCForInternal::queueDelegate();
    }

    //-------------------------------------------------------------------------
    PromisePtr RTPMediaEngine::BaseResource::internalSetupPromise(PromisePtr promise)
    {
      promise->setReferenceHolder(mThisWeak.lock());

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
                                                   IMessageQueuePtr queue,
                                                   IRTPMediaEngineRegistrationPtr registration,
                                                   const char *deviceID
                                                   ) :
      BaseResource(priv, queue, registration),
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
      auto pThis = make_shared<DeviceResource>(make_private{}, IORTCForInternal::queueBlockingMediaStartStopThread(), registration, deviceID);
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
