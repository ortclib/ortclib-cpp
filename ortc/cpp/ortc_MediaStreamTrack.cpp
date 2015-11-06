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
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <limits>

#include <webrtc/modules/video_capture/include/video_capture_factory.h>

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

  using zsLib::SingletonManager;
  using zsLib::DOUBLE;

  namespace internal
  {
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
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IMediaStreamTrackForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
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
      mKind(kind),
      mRemote(remote),
      mConstraints(constraints),
      mVideoCaptureModule(NULL),
      mVideoRenderModule(NULL),
      mVideoRendererCallback(NULL)
    {
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      if (mKind == Kind_Video && !mRemote) {
        String videoDeviceID = mConstraints->mAdvanced.front()->mDeviceID.mValue.value().mValue.value();
        mVideoCaptureModule = webrtc::VideoCaptureFactory::Create(0, videoDeviceID.c_str());
        if (!mVideoCaptureModule) {
          return;
        }

        mVideoCaptureModule->AddRef();

        mVideoCaptureModule->RegisterCaptureDataCallback(*this);

        webrtc::VideoCaptureModule::DeviceInfo* info = webrtc::VideoCaptureFactory::CreateDeviceInfo(0);
        if (!info) {
          return;
        }

        LONG desiredWidth = mConstraints->mAdvanced.front()->mWidth.mValue.value();
        LONG desiredHeight = mConstraints->mAdvanced.front()->mHeight.mValue.value();
        DOUBLE desiredMaxFPS = mConstraints->mAdvanced.front()->mFrameRate.mValue.value();
        LONG minWidthDiff = std::numeric_limits<LONG>::max();
        LONG minHeightDiff = std::numeric_limits<LONG>::max();
        DOUBLE minFpsDiff = std::numeric_limits<double>::max();
        webrtc::VideoCaptureCapability bestCap;
        int32_t numCaps = info->NumberOfCapabilities(videoDeviceID.c_str());
        for (int32_t i = 0; i < numCaps; ++i) {
          webrtc::VideoCaptureCapability cap;
          if (info->GetCapability(videoDeviceID.c_str(), i, cap) != -1) {
            if (cap.rawType == webrtc::kVideoMJPEG || cap.rawType == webrtc::kVideoUnknown)
              continue;
            LONG widthDiff = abs((LONG)(cap.width - desiredWidth));
            LONG heightDiff = abs((LONG)(cap.height - desiredHeight));
            DOUBLE fpsDiff = abs((DOUBLE)(cap.maxFPS - desiredMaxFPS));
            if (widthDiff < minWidthDiff) {
              bestCap = cap;
              minWidthDiff = widthDiff;
              minHeightDiff = heightDiff;
              minFpsDiff = fpsDiff;
            } else if (widthDiff == minWidthDiff) {
              if (heightDiff < minHeightDiff) {
                bestCap = cap;
                minHeightDiff = heightDiff;
                minFpsDiff = fpsDiff;
              } else if (heightDiff == minHeightDiff) {
                if (fpsDiff < minFpsDiff) {
                  bestCap = cap;
                  minFpsDiff = fpsDiff;
                }
              }
            }
          }
        }
        delete info;

        if (mVideoCaptureModule->StartCapture(bestCap) != 0) {
          mVideoCaptureModule->DeRegisterCaptureDataCallback();
          return;
        }
      } else if (mKind == Kind_Video && mRemote) {

      }
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
    MediaStreamTrackPtr MediaStreamTrack::convert(ForSettingsPtr object)
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
    MediaStreamTrackPtr MediaStreamTrack::convert(ForMediaDevicesPtr object)
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
    IStatsProvider::PromiseWithStatsReportPtr MediaStreamTrack::getStats() const throw(InvalidStateError)
    {
      AutoRecursiveLock lock(*this);
      ORTC_THROW_INVALID_STATE_IF(isShutdown() || isShuttingDown())

      PromiseWithStatsReportPtr promise = PromiseWithStatsReport::create(IORTCForInternal::queueDelegate());
      IMediaStreamTrackAsyncDelegateProxy::create(mThisWeak.lock())->onResolveStatsPromise(promise);
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
#define TODO 1
#define TODO 2
      return Kind_First;
    }

    //-------------------------------------------------------------------------
    String MediaStreamTrack::id() const
    {
#define TODO 1
#define TODO 2
      return String();
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
    void MediaStreamTrack::enabeld(bool enabled)
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
    bool MediaStreamTrack::readOnly() const
    {
#define TODO 1
#define TODO 2
      return false;
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::remote() const
    {
#define TODO 1
#define TODO 2
      return mRemote;
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
      if (mVideoCaptureModule) {
        mVideoCaptureModule->StopCapture();
        mVideoCaptureModule->DeRegisterCaptureDataCallback();
      }
      if (mVideoRenderModule)
        mVideoRenderModule->StopRender(1);
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::CapabilitiesPtr MediaStreamTrack::getCapabilities() const
    {
      CapabilitiesPtr result(make_shared<Capabilities>());
#define TODO 1
#define TODO 2
      return result;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::TrackConstraintsPtr MediaStreamTrack::getConstraints() const
    {
      auto result = TrackConstraints::create();
#define TODO 1
#define TODO 2
      return result;
    }

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::SettingsPtr MediaStreamTrack::getSettings() const
    {
      SettingsPtr result(make_shared<Settings>());
#define TODO 1
#define TODO 2
      return result;
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
    void MediaStreamTrack::setMediaElement(void* element)
    {
      AutoRecursiveLock lock(*this);

      if (mKind == Kind_Video) {
        mVideoRenderModule = webrtc::VideoRender::CreateVideoRender(1, element, false);

        mVideoRendererCallback = mVideoRenderModule->AddIncomingRenderStream(1, 0, 0.0, 0.0, 1.0, 1.0);

        mVideoRenderModule->StartRender(1);
      }
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
    #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiver
    #pragma mark

    //-------------------------------------------------------------------------
    MediaStreamTrackPtr MediaStreamTrack::create(Kinds kind)
    {
#define TODO_MOSA_VERIFY_THIS_LOGIC 1
#define TODO_MOSA_VERIFY_THIS_LOGIC 2
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
    void MediaStreamTrack::renderVideoFrame(const webrtc::VideoFrame& videoFrame)
    {
      AutoRecursiveLock lock(*this);

      if (mVideoRendererCallback)
        mVideoRendererCallback->RenderFrame(1, videoFrame);
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
    void MediaStreamTrack::onTimer(TimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrackAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::onApplyConstraints(
                                              PromisePtr promise,
                                              TrackConstraintsPtr constraints
                                              )
    {
      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
      promise->reject();  // temporarily reject everything
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
    #pragma mark MediaStreamTrack => webrtc::VideoCaptureDataCallback
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::OnIncomingCapturedFrame(const int32_t id, const webrtc::VideoFrame& videoFrame)
    {
      AutoRecursiveLock lock(*this);

      if (mVideoRendererCallback)
        mVideoRendererCallback->RenderFrame(1, videoFrame);

      if (mSenderChannel.lock())
        mSenderChannel.lock()->sendVideoFrame(videoFrame);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::OnCaptureDelayChanged(const int32_t id, const int32_t delay)
    {

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
      UseServicesHelper::debugAppend(objectEl, "id", mID);
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

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

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
      if (!stepBogusDoSomething()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("dtls is not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
      }
    }

    //-------------------------------------------------------------------------
    bool MediaStreamTrack::stepBogusDoSomething()
    {
      if ( /* step already done */ false ) {
        ZS_LOG_TRACE(log("already completed do something"))
        return true;
      }

      if ( /* cannot do step yet */ false) {
        ZS_LOG_DEBUG(log("waiting for XYZ to complete before continuing"))
        return false;
      }

      ZS_LOG_DEBUG(log("doing step XYZ"))

      // ....
#define TODO 1
#define TODO 2

      return true;
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SESSION_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SESSION_IS_SHUTDOWN 2

        // grace shutdown process done here

        return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Ended);

      mSubscriptions.clear();

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
  const char *IMediaStreamTrackTypes::toString(States state)
  {
    switch (state) {
      case State_Live:    return "live";
      case State_Ended:   return "ended";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::Capabilities
  #pragma mark

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
  ElementPtr IMediaStreamTrackTypes::Capabilities::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::Capabilities");

    UseServicesHelper::debugAppend(resultEl, "width", mWidth.hasValue() ? mWidth.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "height", mHeight.hasValue() ? mHeight.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio.hasValue() ? mAspectRatio.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "frame rate", mFrameRate.hasValue() ? mFrameRate.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "facingMode", mFacingMode.hasValue() ? mFacingMode.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "volume", mVolume.hasValue() ? mVolume.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "sample rate", mSampleRate.hasValue() ? mSampleRate.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation.hasValue() ? mEchoCancellation.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "latency", mLatency.hasValue() ? mLatency.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "device id", mDeviceID);
    UseServicesHelper::debugAppend(resultEl, "group id", mGroupID);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Capabilities::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IMediaStreamTrackTypes::Capabilities:");
    hasher.update(mWidth.hasValue() ? mWidth.value().hash() : String());
    hasher.update(":");
    hasher.update(mHeight.hasValue() ? mHeight.value().hash() : String());
    hasher.update(":");
    hasher.update(mAspectRatio.hasValue() ? mAspectRatio.value().hash() : String());
    hasher.update(":");
    hasher.update(mFrameRate.hasValue() ? mFrameRate.value().hash() : String());
    hasher.update(":");
    hasher.update(mFacingMode.hasValue() ? mFacingMode.value().hash() : String());
    hasher.update(":");
    hasher.update(mVolume.hasValue() ? mVolume.value().hash() : String());
    hasher.update(":");
    hasher.update(mSampleRate.hasValue() ? mSampleRate.value().hash() : String());
    hasher.update(":");
    hasher.update(mEchoCancellation.hasValue() ? mEchoCancellation.value().hash()  : String());
    hasher.update(":");
    hasher.update(mLatency.hasValue() ? mLatency.value().hash()  : String());
    hasher.update(":");
    hasher.update(mDeviceID);
    hasher.update(":");
    hasher.update(mGroupID);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::Settings
  #pragma mark

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
  ElementPtr IMediaStreamTrackTypes::Settings::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaStreamTrackTypes::Settings");

    UseServicesHelper::debugAppend(resultEl, "width", mWidth);
    UseServicesHelper::debugAppend(resultEl, "height", mHeight);
    UseServicesHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio);
    UseServicesHelper::debugAppend(resultEl, "frame rate", mFrameRate);
    UseServicesHelper::debugAppend(resultEl, "facing mode", mFacingMode);
    UseServicesHelper::debugAppend(resultEl, "orientation", mOrientation);
    UseServicesHelper::debugAppend(resultEl, "volume", mVolume);
    UseServicesHelper::debugAppend(resultEl, "sample rate", mSampleRate);
    UseServicesHelper::debugAppend(resultEl, "sample size", mSampleSize);
    UseServicesHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation);
    UseServicesHelper::debugAppend(resultEl, "device id", mDeviceID);
    UseServicesHelper::debugAppend(resultEl, "group id", mGroupID);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Settings::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IMediaStreamTrackTypes::Settings:");

    hasher.update(mWidth);
    hasher.update(":");
    hasher.update(mHeight);
    hasher.update(":");
    hasher.update(mAspectRatio);
    hasher.update(":");
    hasher.update(mFrameRate);
    hasher.update(":");
    hasher.update(mFacingMode, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher.update(":");
    hasher.update(mOrientation, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher.update(":");
    hasher.update(mVolume);
    hasher.update(":");
    hasher.update(mSampleRate);
    hasher.update(":");
    hasher.update(mSampleSize);
    hasher.update(":");
    hasher.update(mEchoCancellation);
    hasher.update(":");
    hasher.update(mLatency);
    hasher.update(":");
    hasher.update(mDeviceID, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");
    hasher.update(":");
    hasher.update(mGroupID, "bogus99255cc407eaf6f82b33a94ab86f588581df9000");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::ConstraintSet
  #pragma mark

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

    UseServicesHelper::debugAppend(resultEl, "width", mWidth.toDebug());
    UseServicesHelper::debugAppend(resultEl, "height", mHeight.toDebug());
    UseServicesHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio.toDebug());
    UseServicesHelper::debugAppend(resultEl, "frame rate", mFrameRate.toDebug());
    UseServicesHelper::debugAppend(resultEl, "facingMode", mFacingMode.toDebug());
    UseServicesHelper::debugAppend(resultEl, "volume", mVolume.toDebug());
    UseServicesHelper::debugAppend(resultEl, "sample rate", mSampleRate.toDebug());
    UseServicesHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation.toDebug());
    UseServicesHelper::debugAppend(resultEl, "latency", mLatency.toDebug());
    UseServicesHelper::debugAppend(resultEl, "device id", mDeviceID.toDebug());
    UseServicesHelper::debugAppend(resultEl, "group id", mGroupID.toDebug());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::ConstraintSet::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IMediaStreamTrackTypes::ConstraintSet:");

    hasher.update(mWidth.hash());
    hasher.update(":");
    hasher.update(mHeight.hash());
    hasher.update(":");
    hasher.update(mAspectRatio.hash());
    hasher.update(":");
    hasher.update(mFrameRate.hash());
    hasher.update(":");
    hasher.update(mFacingMode.hash());
    hasher.update(":");
    hasher.update(mVolume.hash());
    hasher.update(":");
    hasher.update(mSampleRate.hash());
    hasher.update(":");
    hasher.update(mEchoCancellation.hash());
    hasher.update(":");
    hasher.update(mLatency.hash());
    hasher.update(":");
    hasher.update(mDeviceID.hash());
    hasher.update(":");
    hasher.update(mGroupID.hash());

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::ConstraintSet
  #pragma mark

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
      UseServicesHelper::debugAppend(advancedSet, constraintSet ? constraintSet->toDebug() : ElementPtr());
    }
    if (advancedSet->hasChildren()) {
      UseServicesHelper::debugAppend(resultEl, advancedSet);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::TrackConstraints::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IMediaStreamTrackTypes::TrackConstraints:");

    for (auto iter = mAdvanced.begin(); iter != mAdvanced.end(); ++iter) {
      auto constraintSet = (*iter);
      hasher.update(constraintSet ? constraintSet->hash() : String());
      hasher.update(":");
    }

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes::ConstraintSet
  #pragma mark

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

    UseServicesHelper::debugAppend(resultEl, "video", mVideo ? mVideo->toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "audio", mAudio ? mAudio->toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaStreamTrackTypes::Constraints::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IMediaStreamTrackTypes::Constraints:");

    hasher.update(mVideo ? mVideo->hash() : String());
    hasher.update(":");
    hasher.update(mAudio ? mAudio->hash() : String());

    return hasher.final();
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


}
