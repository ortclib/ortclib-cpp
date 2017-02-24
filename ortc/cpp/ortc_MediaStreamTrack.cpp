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

#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <limits>
#include <float.h>
#include <math.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_mediastreamtrack) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  using zsLib::SingletonManager;
  using zsLib::DOUBLE;

  using zsLib::Numeric;
  using zsLib::Log;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MediaStreamTrackSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

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
    #pragma mark MediaStreamTrackSettingsDefaults
    #pragma mark

    class MediaStreamTrackSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaStreamTrackSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaStreamTrackSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<MediaStreamTrackSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaStreamTrackSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<MediaStreamTrackSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        //      ISettings::setUInt(ORTC_SETTING_MEDIA_STREAM_TRACK_, 0);
      }
      
    };

    //-------------------------------------------------------------------------
    void installMediaStreamTrackSettingsDefaults()
    {
      MediaStreamTrackSettingsDefaults::singleton();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiver
    #pragma mark

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr IMediaStreamTrackForRTPReceiver::create(Kinds kind)
    {
      return internal::IMediaStreamTrackFactory::singleton().create(kind);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    MediaStreamTrack::MediaStreamTrack(
                                       const make_private &,
                                       IMessageQueuePtr queue,
                                       Kinds kind,
                                       bool remote,
                                       TrackConstraintsPtr constraints
                                       ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mTrackID(String(IMediaStreamTrackTypes::toString(kind)) + "_label_" + string(zsLib::createUUID())),
      mKind(kind),
      mRemote(remote),
      mConstraints(constraints)
    {
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::init()
    {
      AutoRecursiveLock lock(*this);

      mStatsTimer = ITimer::create(mThisWeak.lock(), Seconds(1));

      mCapabilities = make_shared<Capabilities>();
      mSettings = make_shared<TrackSettings>();


      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaStreamTrack::~MediaStreamTrack()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::create(
                                                 Kinds kind,
                                                 bool remote,
                                                 TrackConstraintsPtr constraints
                                                 )
    {
      MediaStreamTrackPtr pThis(make_shared<MediaStreamTrack>(make_private{}, IORTCForInternal::queueORTC(), kind, remote, constraints));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(IMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForSenderChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForSenderChannelMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForSenderChannelAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForSenderChannelVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForReceiverPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForReceiverChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForReceiverChannelMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForReceiverChannelAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForReceiverChannelVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaDevicesPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaStreamPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaStreamTrack, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr MediaStreamTrack::getStats(const StatsTypeSet &stats) const
    {
      if (!stats.hasStatType(IStatsReportTypes::StatsType_Track)) {
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }
      AutoRecursiveLock lock(*this);

      if ((isShutdown()) ||
          (isShuttingDown())) {
        ZS_LOG_WARNING(Debug, log("can not fetch stats while shutdown / shutting down"));
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }

      PromiseWithStatsReportPtr promise = PromiseWithStatsReport::create(IORTCForInternal::queueDelegate());
      IMediaStreamTrackAsyncDelegateProxy::create(mThisWeak.lock())->onResolveStatsPromise(promise, stats);
      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr MediaStreamTrack::toDebug(MediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return object->toDebug();
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackSubscriptionPtr MediaStreamTrack::subscribe(IMediaStreamTrackDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(slog("subscribing to media stream track"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return IMediaStreamTrackSubscriptionPtr();

      IMediaStreamTrackSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IMediaStreamTrackDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        auto pThis = mThisWeak.lock();

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
        if (isShutdown()) {
          delegate->onMediaStreamTrackEnded(pThis);
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::Kinds MediaStreamTrack::kind() const
    {
      return mKind;
    }

    //-------------------------------------------------------------------------
    String MediaStreamTrack::id() const
    {
      return mTrackID;
    }

    //-------------------------------------------------------------------------
    String MediaStreamTrack::deviceID() const
    {
      AutoRecursiveLock lock(*this);

      return mDeviceID;
    }

    //-------------------------------------------------------------------------
    String MediaStreamTrack::label() const
    {
#define TODO 1
#define TODO 2
      return String();
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::enabled() const
    {
#define TODO 1
#define TODO 2
      return false;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::enabled(bool enabled)
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::muted() const
    {
#define TODO 1
#define TODO 2
      return false;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::muted(bool muted)
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::remote() const
    {
      return mRemote; // no lock needed
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::States MediaStreamTrack::readyState() const
    {
#define TODO 1
#define TODO 2
      return State_First;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr MediaStreamTrack::clone() const
    {
#define TODO 1
#define TODO 2
      return IMediaStreamTrackPtr();
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::stop()
    {
      AutoRecursiveLock lock(*this);
      if (mDeviceResource)
        mDeviceResource->stop();
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::CapabilitiesPtr MediaStreamTrack::getCapabilities() const
    {
      return mCapabilities;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::TrackConstraintsPtr MediaStreamTrack::getConstraints() const
    {
      AutoRecursiveLock lock(*this);
      return mConstraints;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::SettingsPtr MediaStreamTrack::getSettings() const
    {
      AutoRecursiveLock lock(*this);
      return mSettings;
    }

    //-------------------------------------------------------------------------
    PromisePtr MediaStreamTrack::applyConstraints(const TrackConstraints &inConstraints)
    {
      PromisePtr promise = Promise::createRejected(IORTCForInternal::queueDelegate());

      auto constraints = TrackConstraints::create(inConstraints);

      IMediaStreamTrackAsyncDelegateProxy::create(mThisWeak.lock())->onApplyConstraints(promise, constraints);

      return promise;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::setVideoRenderCallback(IMediaStreamTrackRenderCallbackPtr callback)
    {
      AutoRecursiveLock lock(*this);

      mVideoRendererCallback = callback;
      if (mDeviceResource)
        mDeviceResource->setVideoRenderCallback(callback);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::setH264Rendering(bool h264Rendering)
    {
      AutoRecursiveLock lock(*this);

      mH264Rendering = h264Rendering;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isH264Rendering()
    {
      AutoRecursiveLock lock(*this);

      return mH264Rendering;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::setSender(IRTPSenderPtr sender)
    {
      AutoRecursiveLock lock(*this);

      mSender = RTPSender::convert(sender);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::notifyAttachSenderChannel(RTPSenderChannelPtr channel)
    {
      IMediaStreamTrackAsyncDelegateProxy::create(mThisWeak.lock())->onAttachSenderChannel(channel);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::notifyDetachSenderChannel(RTPSenderChannelPtr channel)
    {
      IMediaStreamTrackAsyncDelegateProxy::create(mThisWeak.lock())->onDetachSenderChannel(channel);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannelMediaBase
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannelAudio
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSenderChannelVideo
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiver
    #pragma mark

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::create(Kinds kind)
    {
      return create(kind, true, TrackConstraintsPtr());
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::setReceiver(IRTPReceiverPtr receiver)
    {
      AutoRecursiveLock lock(*this);

      mReceiver = RTPReceiver::convert(receiver);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::notifyActiveReceiverChannel(RTPReceiverChannelPtr inChannel)
    {
      IMediaStreamTrackAsyncDelegateProxy::create(mThisWeak.lock())->onSetActiveReceiverChannel(inChannel);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelMediaBase
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelAudio
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelVideo
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::renderVideoFrame(VideoFramePtr videoFrame)
    {
      AutoRecursiveLock lock(*this);

      if (mDeviceResource) mDeviceResource->renderVideoFrame(videoFrame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::sendCapturedVideoFrame(VideoFramePtr videoFrame)
    {
      UseSenderChannelPtr channel;

      {
        AutoRecursiveLock lock(*this);

        channel = mSenderChannel.lock();
      }

      if (!channel) return;

      channel->sendVideoFrame(videoFrame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaDevices
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaStream
    #pragma mark

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isEnded() const
    {
      AutoRecursiveLock lock(*this);
      return isShutdown();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onWake()
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
    #pragma mark MediaStreamTrack => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onTimer(ITimerPtr timer)
    {
      ZS_LOG_TRACE(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);

      if (mStatsTimer) {
        if (mDeviceResource && timer->getID() == mStatsTimer->getID()) {
          mDeviceResource->setFrameCount();
        }
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onPromiseSettled(PromisePtr promise)
    {
      ZS_LOG_DEBUG(log("promise settled") + ZS_PARAM("promise", promise->getID()))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise, IStatsReportTypes::StatsTypeSet stats)
    {
      AutoRecursiveLock lock(*this);

      if (mDeviceResource)
        mDeviceResource->requestStats(promise, stats);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onApplyConstraints(
                                              PromisePtr promise,
                                              TrackConstraintsPtr constraints
                                              )
    {
      AutoRecursiveLock lock(*this);
        
      if (mDeviceResource)
        mDeviceResource->updateConstraints(promise, constraints);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onSetActiveReceiverChannel(UseReceiverChannelPtr channel)
    {
      AutoRecursiveLock lock(*this);

      if (!channel) {
        ZS_LOG_DEBUG(log("active receiver channel is being removed"))
#define TODO 1
#define TODO 2
        return;
      }

      ZS_LOG_DEBUG(log("setting to active receiver channel") + ZS_PARAM("channel", channel->getID()))

      mReceiverChannel = channel;

#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onAttachSenderChannel(UseSenderChannelPtr channel)
    {
      ZS_LOG_DEBUG(log("attaching sender channel") + ZS_PARAM("channel", channel->getID()))

      AutoRecursiveLock lock(*this);

      mSenderChannel = channel;

#define TODO 1
#define TODO 2
    }
    
    //-------------------------------------------------------------------------
    void MediaStreamTrack::onDetachSenderChannel(UseSenderChannelPtr channel)
    {
      ZS_LOG_DEBUG(log("detaching sender channel") + ZS_PARAM("channel", channel->getID()))

      AutoRecursiveLock lock(*this);

      mSenderChannel.reset();

#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params MediaStreamTrack::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::MediaStreamTrack");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params MediaStreamTrack::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::MediaStreamTrack");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params MediaStreamTrack::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr MediaStreamTrack::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::MediaStreamTrack");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());

      IHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isShuttingDown() const
    {
      if (mGracefulShutdownReference) return true;
      return State_Ended == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return State_Ended == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepDevicePromise()) goto not_ready;
      if (!stepSetupDevice()) goto not_ready;
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
      }
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::stepDevicePromise()
    {
      if (mDeviceResourceLifetimeHolderPromise) {
        ZS_LOG_TRACE(log("already setup device promise"))
          return true;
      }

      mDeviceResourceLifetimeHolderPromise = UseMediaEngine::setupDevice(mThisWeak.lock());

      mDeviceResourceLifetimeHolderPromise->thenWeak(mThisWeak.lock());

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::stepSetupDevice()
    {
      if (mDeviceResource) {
        ZS_LOG_TRACE(log("already setup device resource"))
        return true;
      }

      if (!mDeviceResourceLifetimeHolderPromise->isSettled()) {
        ZS_LOG_TRACE(log("waiting for setup device promise to be set up"))
        return false;
      }

      if (mDeviceResourceLifetimeHolderPromise->isRejected()) {
        ZS_LOG_WARNING(Debug, log("media engine rejected device setup"))
        cancel();
        return false;
      }

      mDeviceResource = ZS_DYNAMIC_PTR_CAST(UseDeviceResource, mDeviceResourceLifetimeHolderPromise->value());

      if (!mDeviceResource) {
        ZS_LOG_WARNING(Detail, log("failed to initialize device resource"))
        cancel();
        return false;
      }

      if (mVideoRendererCallback)
        mDeviceResource->setVideoRenderCallback(mVideoRendererCallback);

      ZS_LOG_DEBUG(log("media device is setup") + ZS_PARAM("device", mDeviceResource->getID()))

      return true;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (!mCloseDevicePromise) {
        if (mDeviceResource) {
          mCloseDevicePromise = mDeviceResource->shutdown();
          mCloseDevicePromise->thenWeak(mGracefulShutdownReference);
        }
      }

      if (mGracefulShutdownReference) {
        if (mCloseDevicePromise) {
          if (!mCloseDevicePromise->isSettled()) {
            ZS_LOG_DEBUG(log("waiting for close device promise"))
            return;
          }
        }
      }

      //.......................................................................
      // final cleanup

      setState(State_Ended);

      mSubscriptions.clear();
      
      if (mStatsTimer) {
        mStatsTimer->cancel();
        mStatsTimer.reset();
      }

      mDeviceResourceLifetimeHolderPromise.reset();

      mDeviceResource.reset();
      mCloseDevicePromise.reset();

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      MediaStreamTrackPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onMediaStreamTrackStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IMediaStreamTrackFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaStreamTrackFactory &IMediaStreamTrackFactory::singleton()
    {
      return MediaStreamTrackFactory::singleton();
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr IMediaStreamTrackFactory::create(
                                                         IMediaStreamTrackTypes::Kinds kind,
                                                         bool remote,
                                                         TrackConstraintsPtr constraints
                                                         )
    {
      if (this) {}
      return internal::MediaStreamTrack::create(kind, remote, constraints);
    }

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr IMediaStreamTrackFactory::create(IMediaStreamTrackTypes::Kinds kind)
    {
      if (this) {}
      return internal::MediaStreamTrack::create(kind);
    }


  } // internal namespace

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark (helpers)
  #pragma mark

  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message)
  {
    return Log::Params(message, "ortc::IMediaStreamTrackTypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IMediaStreamTrackTypes::toString(Kinds kind)
  {
    switch (kind) {
      case Kind_Audio:   return "audio";
      case Kind_Video:   return "video";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Kinds IMediaStreamTrackTypes::toKind(const char *inputStr)
  {
    String str(inputStr);
    for (IMediaStreamTrackTypes::Kinds index = IMediaStreamTrackTypes::Kind_First; index <= IMediaStreamTrackTypes::Kind_Last; index = static_cast<IMediaStreamTrackTypes::Kinds>(static_cast<std::underlying_type<IMediaStreamTrackTypes::Kinds>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IMediaStreamTrackTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return IMediaStreamTrackTypes::Kind_First;
  }

  //---------------------------------------------------------------------------
  const char *IMediaStreamTrackTypes::toString(States state)
  {
    switch (state) {
      case State_Live:    return "live";
      case State_Ended:   return "ended";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::States IMediaStreamTrackTypes::toState(const char *inputStr)
  {
    String str(inputStr);
    for (IMediaStreamTrackTypes::States index = IMediaStreamTrackTypes::State_First; index <= IMediaStreamTrackTypes::State_Last; index = static_cast<IMediaStreamTrackTypes::States>(static_cast<std::underlying_type<IMediaStreamTrackTypes::States>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IMediaStreamTrackTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return IMediaStreamTrackTypes::State_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::Capabilities
  #pragma mark

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Capabilities::Capabilities(ElementPtr elem)
  {
    if (!elem) return;

    {
      ElementPtr entryEl = elem->findFirstChildElement("width");
      if (entryEl) {
        mWidth = CapabilityLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("height");
      if (entryEl) {
        mHeight = CapabilityLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("aspectRatio");
      if (entryEl) {
        mAspectRatio = CapabilityDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("frameRate");
      if (entryEl) {
        mFrameRate = CapabilityDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("facingMode");
      if (entryEl) {
        mFacingMode = CapabilityString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("orientation");
      if (entryEl) {
        mOrientation = CapabilityString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("volume");
      if (entryEl) {
        mVolume = CapabilityDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("sampleRate");
      if (entryEl) {
        mSampleRate = CapabilityLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("sampleSize");
      if (entryEl) {
        mSampleSize = CapabilityLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("echoCancellation");
      if (entryEl) {
        mEchoCancellation = CapabilityBoolean(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("latency");
      if (entryEl) {
        mLatency = CapabilityDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("channelCount");
      if (entryEl) {
        mChannelCount = CapabilityLong(entryEl);
      }
    }

    IHelper::getElementValue(elem, "deviceId", "ortc::IMediaStreamTrackTypes::Capabilities", mDeviceID);
    IHelper::getElementValue(elem, "groupId", "ortc::IMediaStreamTrackTypes::Capabilities", mGroupID);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Capabilities::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mWidth.hasValue()) {
      elem->adoptAsLastChild(mWidth.value().createElement("width"));
    }
    if (mHeight.hasValue()) {
      elem->adoptAsLastChild(mHeight.value().createElement("height"));
    }
    if (mAspectRatio.hasValue()) {
      elem->adoptAsLastChild(mAspectRatio.value().createElement("aspectRatio"));
    }
    if (mFrameRate.hasValue()) {
      elem->adoptAsLastChild(mFrameRate.value().createElement("frameRate"));
    }
    if (mFacingMode.hasValue()) {
      elem->adoptAsLastChild(mFacingMode.value().createElement("facingMode"));
    }
    if (mOrientation.hasValue()) {
      elem->adoptAsLastChild(mOrientation.value().createElement("orientation"));
    }
    if (mVolume.hasValue()) {
      elem->adoptAsLastChild(mVolume.value().createElement("volume"));
    }
    if (mSampleRate.hasValue()) {
      elem->adoptAsLastChild(mSampleRate.value().createElement("sampleRate"));
    }
    if (mSampleSize.hasValue()) {
      elem->adoptAsLastChild(mSampleSize.value().createElement("sampleSize"));
    }
    if (mEchoCancellation.hasValue()) {
      elem->adoptAsLastChild(mEchoCancellation.value().createElement("echoCancellation"));
    }
    if (mLatency.hasValue()) {
      elem->adoptAsLastChild(mLatency.value().createElement("latency"));
    }
    if (mChannelCount.hasValue()) {
      elem->adoptAsLastChild(mChannelCount.value().createElement("channelCount"));
    }

    IHelper::adoptElementValue(elem, "deviceId", mDeviceID, false);
    IHelper::adoptElementValue(elem, "groupId", mGroupID, false);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::CapabilitiesPtr IMediaStreamTrackTypes::Capabilities::create()
  {
    return make_shared<Capabilities>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::CapabilitiesPtr IMediaStreamTrackTypes::Capabilities::create(const CapabilitiesPtr &value)
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::CapabilitiesPtr IMediaStreamTrackTypes::Capabilities::create(const Capabilities &value)
  {
    return make_shared<Capabilities>(value);
  }
  
  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::CapabilitiesPtr IMediaStreamTrackTypes::Capabilities::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(Capabilities, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Capabilities::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::Capabilities");

    IHelper::debugAppend(resultEl, "width", mWidth.hasValue() ? mWidth.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "height", mHeight.hasValue() ? mHeight.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio.hasValue() ? mAspectRatio.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "frame rate", mFrameRate.hasValue() ? mFrameRate.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "facingMode", mFacingMode.hasValue() ? mFacingMode.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "volume", mVolume.hasValue() ? mVolume.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "sample rate", mSampleRate.hasValue() ? mSampleRate.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation.hasValue() ? mEchoCancellation.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "latency", mLatency.hasValue() ? mLatency.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "channel acount", mChannelCount.hasValue() ? mChannelCount.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "device id", mDeviceID);
    IHelper::debugAppend(resultEl, "group id", mGroupID);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Capabilities::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::Capabilities:");
    hasher->update(mWidth.hasValue() ? mWidth.value().hash() : String());
    hasher->update(":");
    hasher->update(mHeight.hasValue() ? mHeight.value().hash() : String());
    hasher->update(":");
    hasher->update(mAspectRatio.hasValue() ? mAspectRatio.value().hash() : String());
    hasher->update(":");
    hasher->update(mFrameRate.hasValue() ? mFrameRate.value().hash() : String());
    hasher->update(":");
    hasher->update(mFacingMode.hasValue() ? mFacingMode.value().hash() : String());
    hasher->update(":");
    hasher->update(mVolume.hasValue() ? mVolume.value().hash() : String());
    hasher->update(":");
    hasher->update(mSampleRate.hasValue() ? mSampleRate.value().hash() : String());
    hasher->update(":");
    hasher->update(mEchoCancellation.hasValue() ? mEchoCancellation.value().hash()  : String());
    hasher->update(":");
    hasher->update(mLatency.hasValue() ? mLatency.value().hash()  : String());
    hasher->update(":");
    hasher->update(mChannelCount.hasValue() ? mChannelCount.value().hash() : String());
    hasher->update(":");
    hasher->update(mDeviceID);
    hasher->update(":");
    hasher->update(mGroupID);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::Settings
  #pragma mark

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Settings::Settings(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "width", mWidth);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "height", mHeight);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "aspectRatio", mAspectRatio);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "frameRate", mFrameRate);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "facingMode", mFacingMode);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "orientation", mOrientation);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "volume", mVolume);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "sampleRate", mSampleRate);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "sampleSize", mSampleSize);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "echoCancellation", mEchoCancellation);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "latency", mLatency);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "channelCount", mChannelCount);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "deviceId", mDeviceID);
    IHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "groupId", mGroupID);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Settings::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "width", mWidth);
    IHelper::adoptElementValue(elem, "height", mHeight);
    IHelper::adoptElementValue(elem, "aspectRatio", mAspectRatio);
    IHelper::adoptElementValue(elem, "frameRate", mFrameRate);
    IHelper::adoptElementValue(elem, "facingMode", mFacingMode);
    IHelper::adoptElementValue(elem, "orientation", mOrientation);
    IHelper::adoptElementValue(elem, "volume", mVolume);
    IHelper::adoptElementValue(elem, "sampleRate", mSampleRate);
    IHelper::adoptElementValue(elem, "sampleSize", mSampleSize);
    IHelper::adoptElementValue(elem, "echoCancellation", mEchoCancellation);
    IHelper::adoptElementValue(elem, "latency", mLatency);
    IHelper::adoptElementValue(elem, "channelCount", mChannelCount);
    IHelper::adoptElementValue(elem, "deviceId", mDeviceID);
    IHelper::adoptElementValue(elem, "groupId", mGroupID);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::SettingsPtr IMediaStreamTrackTypes::Settings::create()
  {
    return make_shared<Settings>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::SettingsPtr IMediaStreamTrackTypes::Settings::create(const SettingsPtr &value)
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::SettingsPtr IMediaStreamTrackTypes::Settings::create(const Settings &value)
  {
    return make_shared<Settings>(value);
  }
  
  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::SettingsPtr IMediaStreamTrackTypes::Settings::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(Settings, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Settings::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::Settings");

    IHelper::debugAppend(resultEl, "width", mWidth);
    IHelper::debugAppend(resultEl, "height", mHeight);
    IHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio);
    IHelper::debugAppend(resultEl, "frame rate", mFrameRate);
    IHelper::debugAppend(resultEl, "facing mode", mFacingMode);
    IHelper::debugAppend(resultEl, "orientation", mOrientation);
    IHelper::debugAppend(resultEl, "volume", mVolume);
    IHelper::debugAppend(resultEl, "sample rate", mSampleRate);
    IHelper::debugAppend(resultEl, "sample size", mSampleSize);
    IHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation);
    IHelper::debugAppend(resultEl, "latency", mLatency);
    IHelper::debugAppend(resultEl, "channel count", mChannelCount);
    IHelper::debugAppend(resultEl, "device id", mDeviceID);
    IHelper::debugAppend(resultEl, "group id", mGroupID);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Settings::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::Settings:");

    hasher->update(mWidth);
    hasher->update(":");
    hasher->update(mHeight);
    hasher->update(":");
    hasher->update(mAspectRatio);
    hasher->update(":");
    hasher->update(mFrameRate);
    hasher->update(":");
    hasher->update(mFacingMode, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher->update(":");
    hasher->update(mOrientation, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher->update(":");
    hasher->update(mVolume);
    hasher->update(":");
    hasher->update(mSampleRate);
    hasher->update(":");
    hasher->update(mSampleSize);
    hasher->update(":");
    hasher->update(mEchoCancellation);
    hasher->update(":");
    hasher->update(mLatency);
    hasher->update(":");
    hasher->update(mChannelCount);
    hasher->update(":");
    hasher->update(mDeviceID, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher->update(":");
    hasher->update(mGroupID, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::ConstraintSet
  #pragma mark

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintSet::ConstraintSet(ElementPtr elem)
  {
    if (!elem) return;

    {
      ElementPtr entryEl = elem->findFirstChildElement("width");
      if (entryEl) {
        mWidth = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("height");
      if (entryEl) {
        mHeight = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("aspectRatio");
      if (entryEl) {
        mAspectRatio = ConstrainDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("frameRate");
      if (entryEl) {
        mFrameRate = ConstrainDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("facingMode");
      if (entryEl) {
        mFacingMode = ConstrainString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("orientation");
      if (entryEl) {
        mOrientation = ConstrainString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("volume");
      if (entryEl) {
        mVolume = ConstrainDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("sampleRate");
      if (entryEl) {
        mSampleRate = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("sampleSize");
      if (entryEl) {
        mSampleSize = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("echoCancellation");
      if (entryEl) {
        mEchoCancellation = ConstrainBoolean(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("latency");
      if (entryEl) {
        mLatency = ConstrainDouble(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("channelCount");
      if (entryEl) {
        mChannelCount = ConstrainLong(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("deviceId");
      if (entryEl) {
        mDeviceID = ConstrainString(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("groupId");
      if (entryEl) {
        mGroupID = ConstrainString(entryEl);
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::ConstraintSet::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    elem->adoptAsLastChild(mWidth.createElement("width"));
    elem->adoptAsLastChild(mHeight.createElement("height"));
    elem->adoptAsLastChild(mAspectRatio.createElement("aspectRatio"));
    elem->adoptAsLastChild(mFrameRate.createElement("frameRate"));
    elem->adoptAsLastChild(mFacingMode.createElement("facingMode"));
    elem->adoptAsLastChild(mOrientation.createElement("orientation"));
    elem->adoptAsLastChild(mVolume.createElement("volume"));
    elem->adoptAsLastChild(mSampleRate.createElement("sampleRate"));
    elem->adoptAsLastChild(mSampleSize.createElement("sampleSize"));
    elem->adoptAsLastChild(mEchoCancellation.createElement("echoCancellation"));
    elem->adoptAsLastChild(mLatency.createElement("latency"));
    elem->adoptAsLastChild(mChannelCount.createElement("channelCount"));
    elem->adoptAsLastChild(mDeviceID.createElement("deviceId"));
    elem->adoptAsLastChild(mGroupID.createElement("groupId"));

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintSetPtr IMediaStreamTrackTypes::ConstraintSet::create()
  {
    return make_shared<ConstraintSet>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintSetPtr IMediaStreamTrackTypes::ConstraintSet::create(const ConstraintSetPtr &value)
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes:: ConstraintSetPtr IMediaStreamTrackTypes::ConstraintSet::create(const ConstraintSet &value)
  {
    return make_shared<ConstraintSet>(value);
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::ConstraintSet::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::ConstraintSet");

    IHelper::debugAppend(resultEl, "width", mWidth.toDebug());
    IHelper::debugAppend(resultEl, "height", mHeight.toDebug());
    IHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio.toDebug());
    IHelper::debugAppend(resultEl, "frame rate", mFrameRate.toDebug());
    IHelper::debugAppend(resultEl, "facingMode", mFacingMode.toDebug());
    IHelper::debugAppend(resultEl, "volume", mVolume.toDebug());
    IHelper::debugAppend(resultEl, "sample rate", mSampleRate.toDebug());
    IHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation.toDebug());
    IHelper::debugAppend(resultEl, "latency", mLatency.toDebug());
    IHelper::debugAppend(resultEl, "channel count", mChannelCount.toDebug());
    IHelper::debugAppend(resultEl, "device id", mDeviceID.toDebug());
    IHelper::debugAppend(resultEl, "group id", mGroupID.toDebug());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::ConstraintSet::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::ConstraintSet:");

    hasher->update(mWidth.hash());
    hasher->update(":");
    hasher->update(mHeight.hash());
    hasher->update(":");
    hasher->update(mAspectRatio.hash());
    hasher->update(":");
    hasher->update(mFrameRate.hash());
    hasher->update(":");
    hasher->update(mFacingMode.hash());
    hasher->update(":");
    hasher->update(mVolume.hash());
    hasher->update(":");
    hasher->update(mSampleRate.hash());
    hasher->update(":");
    hasher->update(mEchoCancellation.hash());
    hasher->update(":");
    hasher->update(mLatency.hash());
    hasher->update(":");
    hasher->update(mChannelCount.hash());
    hasher->update(":");
    hasher->update(mDeviceID.hash());
    hasher->update(":");
    hasher->update(mGroupID.hash());

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::ConstraintSet
  #pragma mark

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::TrackConstraints::TrackConstraints(ElementPtr elem)
  {
    if (!elem) return;

    ElementPtr constraintSetsEl = elem->findFirstChildElement("constraintSets");

    ElementPtr constraintSetEl;
    if (constraintSetsEl) {
      constraintSetEl = constraintSetEl->findFirstChildElement("constraintSet");
    } else {
      constraintSetEl = elem->findFirstChildElement("constraintSet");
    }

    while (constraintSetEl) {
      mAdvanced.push_back(make_shared<ConstraintSet>(constraintSetEl));
      constraintSetEl = constraintSetEl->findNextSiblingElement("constraintSet");
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::TrackConstraints::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    ElementPtr constraintSetsEl = Element::create("constraintSets");

    for (auto iter = mAdvanced.begin(); iter != mAdvanced.end(); ++iter) {
      auto value = (*iter);

      constraintSetsEl->adoptAsLastChild(value->createElement("constraintSet"));
    }

    if (constraintSetsEl->hasChildren()) {
      elem->adoptAsLastChild(constraintSetsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::TrackConstraintsPtr IMediaStreamTrackTypes::TrackConstraints::create()
  {
    return make_shared<TrackConstraints>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::TrackConstraintsPtr IMediaStreamTrackTypes::TrackConstraints::create(const TrackConstraintsPtr &value)
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::TrackConstraintsPtr IMediaStreamTrackTypes::TrackConstraints::create(const TrackConstraints &value)
  {
    auto result = make_shared<TrackConstraints>();

    for (auto iter = value.mAdvanced.begin(); iter != value.mAdvanced.end(); ++iter) {
      auto clone = ConstraintSet::create(*iter);
      result->mAdvanced.push_back(clone);
    }

    return result;
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::TrackConstraints::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::TrackConstraints");

    ElementPtr advancedSet = Element::create("advanced set");

    for (auto iter = mAdvanced.begin(); iter != mAdvanced.end(); ++iter) {
      auto constraintSet = (*iter);
      IHelper::debugAppend(advancedSet, constraintSet ? constraintSet->toDebug() : ElementPtr());
    }
    if (advancedSet->hasChildren()) {
      IHelper::debugAppend(resultEl, advancedSet);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::TrackConstraints::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::TrackConstraints:");

    for (auto iter = mAdvanced.begin(); iter != mAdvanced.end(); ++iter) {
      auto constraintSet = (*iter);
      hasher->update(constraintSet ? constraintSet->hash() : String());
      hasher->update(":");
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::ConstraintSet
  #pragma mark

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Constraints::Constraints(ElementPtr elem)
  {
    if (!elem) return;

    {
      ElementPtr entryEl = elem->findFirstChildElement("video");
      if (entryEl) {
        mVideo = make_shared<TrackConstraints>(entryEl);
      }
    }
    {
      ElementPtr entryEl = elem->findFirstChildElement("audio");
      if (entryEl) {
        mAudio = make_shared<TrackConstraints>(entryEl);
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Constraints::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mVideo) {
      elem->adoptAsLastChild(mVideo->createElement("video"));
    }
    if (mAudio) {
      elem->adoptAsLastChild(mVideo->createElement("audio"));
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintsPtr IMediaStreamTrackTypes::Constraints::create()
  {
    return make_shared<Constraints>();
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintsPtr IMediaStreamTrackTypes::Constraints::create(const ConstraintsPtr &value)
  {
    if (!value) return create();
    return create(*value);
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::ConstraintsPtr IMediaStreamTrackTypes::Constraints::create(const Constraints &value)
  {
    auto result = make_shared<Constraints>();
    result->mVideo = value.mVideo ? TrackConstraints::create(value.mVideo) : TrackConstraintsPtr();
    result->mAudio = value.mAudio ? TrackConstraints::create(value.mAudio) : TrackConstraintsPtr();
    return result;
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Constraints::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::Constraints");

    IHelper::debugAppend(resultEl, "video", mVideo ? mVideo->toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, "audio", mAudio ? mAudio->toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Constraints::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaStreamTrackTypes::Constraints:");

    hasher->update(mVideo ? mVideo->hash() : String());
    hasher->update(":");
    hasher->update(mAudio ? mAudio->hash() : String());

    return hasher->finalizeAsString();
  }
  

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrack
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrack::toDebug(IMediaStreamTrackPtr object)
  {
    return internal::MediaStreamTrack::toDebug(internal::MediaStreamTrack::convert(object));
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackPtr IMediaStreamTrack::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(IMediaStreamTrack, any);
  }

}
