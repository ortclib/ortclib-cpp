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

#include <ortc/internal/ortc_MediaDevices.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <ortc/services/IHelper.h>
#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <webrtc/modules/video_capture/video_capture_factory.h>
#include <webrtc/modules/audio_device/audio_device_impl.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_mediadevices) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  using zsLib::SingletonManager;

  using zsLib::Numeric;
  using zsLib::Log;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MediaDevicesSettingsDefaults);

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
    #pragma mark MediaDevicesSettingsDefaults
    #pragma mark

    class MediaDevicesSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaDevicesSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaDevicesSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<MediaDevicesSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaDevicesSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<MediaDevicesSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        //      ISettings::setUInt(ORTC_SETTING_MEDIA_DEVICES, 0);
      }
      
    };

    //-------------------------------------------------------------------------
    void installMediaDevicesSettingsDefaults()
    {
      MediaDevicesSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDevices
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *MediaDevices::toString(States state)
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
    MediaDevices::MediaDevices(
                               const make_private &,
                               IMessageQueuePtr queue
                               ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void MediaDevices::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaDevices::~MediaDevices()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDevicesPtr MediaDevices::create()
    {
      MediaDevicesPtr pThis(make_shared<MediaDevices>(make_private {}, IORTCForInternal::queueORTC()));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    MediaDevicesPtr MediaDevices::singleton()
    {
      AutoRecursiveLock lock(*IHelper::getGlobalLock());
      static SingletonLazySharedPtr<MediaDevices> singleton(create());
      MediaDevicesPtr result = singleton.singleton();

      static SingletonManager::Register registerSingleton("org.ortc.MediaDevices", result);

      if (!result) {
        ZS_LOG_WARNING(Detail, slog("singleton gone"))
      }

      return result;
    }

    //-------------------------------------------------------------------------
    MediaDevicesPtr MediaDevices::convert(IMediaDevicesPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaDevices, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDevices => IMediaDevices
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr MediaDevices::singletonToDebug()
    {

      MediaDevicesPtr pThis(MediaDevices::singleton());
      if (!pThis) return ElementPtr();

      AutoRecursiveLock lock(*pThis);
      return pThis->toDebug();
    }

    //-------------------------------------------------------------------------
    IMediaDevices::SupportedConstraintsPtr MediaDevices::getSupportedConstraints()
    {
      SupportedConstraintsPtr result(make_shared<SupportedConstraints>());

#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2

      return result;
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithDeviceListPtr MediaDevices::enumerateDevices()
    {
      PromiseWithDeviceListPtr promise = PromiseWithDeviceList::create(IORTCForInternal::queueDelegate());

      MediaDevicesPtr pThis(MediaDevices::singleton());

      if (!pThis) {
        ZS_LOG_WARNING(Basic, slog("media devices singleton is gone"))
        promise->reject();
        return promise;
      }

      IMediaDevicesAsyncDelegateProxy::create(pThis)->onEnumerateDevices(promise);

      return promise;
    }


    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithMediaStreamTrackListPtr MediaDevices::getUserMedia(const Constraints &constraints)
    {
      PromiseWithMediaStreamTrackListPtr promise = PromiseWithMediaStreamTrackList::create(IORTCForInternal::queueDelegate());

      MediaDevicesPtr pThis(MediaDevices::singleton());

      if (!pThis) {
        ZS_LOG_WARNING(Basic, slog("media devices singleton is gone"))
        promise->reject();
        return promise;
      }

      ConstraintsPtr constraintsCopy = Constraints::create(constraints);

      IMediaDevicesAsyncDelegateProxy::create(pThis)->onGetUserMedia(promise, constraintsCopy);

      return promise;
    }

    //-------------------------------------------------------------------------
    IMediaDevicesSubscriptionPtr MediaDevices::subscribe(IMediaDevicesDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(slog("subscribing to media devices"))

      auto pThis = singleton();
      if (!pThis) {
        ZS_DECLARE_STRUCT_PTR(BogusSubscription)

        struct BogusSubscription : public IMediaDevicesSubscription {
          virtual PUID getID() const override {return mID;}
          virtual void cancel() override {}
          virtual void background() override {}

          AutoPUID mID;
        };

        return make_shared<BogusSubscription>();
      }

      AutoRecursiveLock lock(*pThis);
      if (!originalDelegate) return IMediaDevicesSubscriptionPtr();

      IMediaDevicesSubscriptionPtr subscription = pThis->mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IMediaDevicesDelegatePtr delegate = pThis->mSubscriptions.delegate(subscription, true);

      if (delegate) {
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
      }

      if (pThis->isShutdown()) {
        pThis->mSubscriptions.clear();
      }
      
      return subscription;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDevices => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDevices::onWake()
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
    #pragma mark MediaDevices => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDevices::onTimer(ITimerPtr timer)
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
    #pragma mark MediaDevices => IMediaDevicesAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDevices::onEnumerateDevices(PromiseWithDeviceListPtr promise)
    {
      AutoRecursiveLock lock(*this);

      DeviceListPtr value(std::make_shared<DeviceList>());

      webrtc::VideoCaptureModule::DeviceInfo* info = webrtc::VideoCaptureFactory::CreateDeviceInfo(0);
      if (!info) {
        promise->reject();
        return;
      }

      int numCams = info->NumberOfDevices();
      for (int index = 0; index < numCams; ++index) {
        char deviceName[256];
        char deviceUniqueId[256];
        if (info->GetDeviceName(index, deviceName, sizeof(deviceName),
          deviceUniqueId, sizeof(deviceUniqueId)) != -1) {
          Device device;
          device.mKind = DeviceKind_VideoInput;
          device.mDeviceID = deviceUniqueId;
          device.mLabel = deviceName;
          value->push_back(device);
        }
      }
      delete info;

      webrtc::AudioDeviceModule* audioDevice =
        webrtc::AudioDeviceModuleImpl::Create(1, webrtc::AudioDeviceModule::kWindowsWasapiAudio);
      if (!audioDevice) {
        promise->reject();
        return;
      }

      audioDevice->AddRef();

      audioDevice->Init();

      int numMics = audioDevice->RecordingDevices();
      for (int index = 0; index < numMics; ++index) {
        char deviceName[webrtc::kAdmMaxDeviceNameSize];
        char deviceGuid[webrtc::kAdmMaxGuidSize];
        if (audioDevice->RecordingDeviceName(index, deviceName, deviceGuid) != -1) {
          Device device;
          device.mKind = DeviceKind_AudioInput;
          device.mDeviceID = deviceGuid;
          device.mLabel = deviceName;
          value->push_back(device);
        }
      }

      int numSpeaks = audioDevice->PlayoutDevices();
      for (int index = 0; index < numSpeaks; ++index) {
        char deviceName[webrtc::kAdmMaxDeviceNameSize];
        char deviceGuid[webrtc::kAdmMaxGuidSize];
        if (audioDevice->PlayoutDeviceName(index, deviceName, deviceGuid) != -1) {
          Device device;
          device.mKind = DeviceKind_AudioOutput;
          device.mDeviceID = deviceGuid;
          device.mLabel = deviceName;
          value->push_back(device);
        }
      }

      audioDevice->Terminate();

      promise->resolve(value);
    }

    //-------------------------------------------------------------------------
    void MediaDevices::onGetUserMedia(PromiseWithMediaStreamTrackListPtr promise, ConstraintsPtr constraints)
    {
      AutoRecursiveLock lock(*this);

      MediaStreamTrackListPtr value(std::make_shared<MediaStreamTrackList>());

      if (constraints->mVideo)
      {
        MediaStreamTrackPtr videoTrack = IMediaStreamTrackFactory::singleton().create(
                                                                                      IMediaStreamTrackTypes::Kinds::Kind_Video,
                                                                                      false,
                                                                                      constraints->mVideo
                                                                                      );
        value->push_back(videoTrack);
      }

      if (constraints->mAudio)
      {
        MediaStreamTrackPtr audioTrack = IMediaStreamTrackFactory::singleton().create(
                                                                                      IMediaStreamTrackTypes::Kinds::Kind_Audio,
                                                                                      false,
                                                                                      constraints->mAudio
                                                                                      );
        value->push_back(audioTrack);
      }

      promise->resolve(value);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDevices => ISingletonManagerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDevices::notifySingletonCleanup()
    {
      ZS_LOG_DEBUG(log("notify singleton cleanup"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDevices => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params MediaDevices::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::MediaDevices");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params MediaDevices::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::MediaDevices");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params MediaDevices::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr MediaDevices::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::MediaDevices");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());

      IHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool MediaDevices::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool MediaDevices::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void MediaDevices::step()
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
        ZS_LOG_TRACE(debug("not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
      }
    }

    //-------------------------------------------------------------------------
    bool MediaDevices::stepBogusDoSomething()
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
    void MediaDevices::cancel()
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

      setState(State_Shutdown);

      mSubscriptions.clear();

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void MediaDevices::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      MediaDevicesPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onMediaDevicesStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void MediaDevices::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IMediaDevicesFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaDevicesFactory &IMediaDevicesFactory::singleton()
    {
      return MediaDevicesFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ElementPtr IMediaDevicesFactory::singletonToDebug()
    {
      if (this) {}
      return internal::MediaDevices::singletonToDebug();
    }

    //-------------------------------------------------------------------------
    MediaDevicesPtr IMediaDevicesFactory::create()
    {
      if (this) {}
      return internal::MediaDevices::create();
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::SupportedConstraintsPtr IMediaDevicesFactory::getSupportedConstraints()
    {
      if (this) {}
      return internal::MediaDevices::getSupportedConstraints();
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithDeviceListPtr IMediaDevicesFactory::enumerateDevices()
    {
      if (this) {}
      return internal::MediaDevices::enumerateDevices();
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithMediaStreamTrackListPtr IMediaDevicesFactory::getUserMedia(const Constraints &constraints)
    {
      if (this) {}
      return internal::MediaDevices::getUserMedia(constraints);
    }

    //-------------------------------------------------------------------------
    IMediaDevicesSubscriptionPtr IMediaDevicesFactory::subscribe(IMediaDevicesDelegatePtr delegate)
    {
      if (this) {}
      return internal::MediaDevices::subscribe(delegate);
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
    return Log::Params(message, "ortc::IMediaDevicesTypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevicesTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IMediaDevicesTypes::toString(DeviceKinds kind)
  {
    switch (kind) {
      case DeviceKind_AudioInput:   return "audioinput";
      case DeviceKind_AudioOutput:  return "audiooutput";
      case DeviceKind_VideoInput:   return "videoinput";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IMediaDevicesTypes::DeviceKinds IMediaDevicesTypes::toDeviceKind(const char *deviceKindStr)
  {
    String str(deviceKindStr);
    for (IMediaDevicesTypes::DeviceKinds index = IMediaDevicesTypes::DeviceKind_First; index <= IMediaDevicesTypes::DeviceKind_Last; index = static_cast<IMediaDevicesTypes::DeviceKinds>(static_cast<std::underlying_type<IMediaDevicesTypes::DeviceKinds>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IMediaDevicesTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return IMediaDevicesTypes::DeviceKind_First;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Kinds IMediaDevicesTypes::toKind(DeviceKinds kind)
  {
    switch (kind) {
      case DeviceKind_AudioInput:   return Kind_Audio;
      case DeviceKind_AudioOutput:  return Kind_Audio;
      case DeviceKind_VideoInput:   return Kind_Video;
    }

    ASSERT(false)
    return Kind_Audio;
  }

  //---------------------------------------------------------------------------
  bool IMediaDevicesTypes::isAudio(DeviceKinds kind)
  {
    switch (kind) {
      case DeviceKind_AudioInput:   return true;
      case DeviceKind_AudioOutput:  return true;
      case DeviceKind_VideoInput:   return false;
    }

    ASSERT(false)
    return false;
  }

  //---------------------------------------------------------------------------
  bool IMediaDevicesTypes::isVideo(DeviceKinds kind)
  {
    switch (kind) {
      case DeviceKind_AudioInput:   return false;
      case DeviceKind_AudioOutput:  return false;
      case DeviceKind_VideoInput:   return true;
    }

    ASSERT(false)
    return false;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevicesTypes::SupportedConstraints
  #pragma mark


  //---------------------------------------------------------------------------
  IMediaDevicesTypes::SupportedConstraints::SupportedConstraints(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "width", mWidth);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "height", mHeight);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "aspectRatio", mAspectRatio);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "frameRate", mFrameRate);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "facingMode", mFacingMode);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "volume", mVolume);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "sampleRate", mSampleRate);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "sampleSize", mSampleSize);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "echoCancellation", mEchoCancellation);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "latency", mLatency);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "deviceId", mDeviceID);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::SupportedConstraints", "groupId", mGroupID);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevicesTypes::SupportedConstraints::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "width", mWidth);
    IHelper::adoptElementValue(elem, "height", mHeight);
    IHelper::adoptElementValue(elem, "aspectRatio", mAspectRatio);
    IHelper::adoptElementValue(elem, "frameRate", mFrameRate);
    IHelper::adoptElementValue(elem, "facingMode", mFacingMode);
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
  ElementPtr IMediaDevicesTypes::SupportedConstraints::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaDevicesTypes::SupportedConstraints");

    IHelper::debugAppend(resultEl, "width", mWidth);
    IHelper::debugAppend(resultEl, "height", mHeight);
    IHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio);
    IHelper::debugAppend(resultEl, "frame rate", mFrameRate);
    IHelper::debugAppend(resultEl, "facingMode", mFacingMode);
    IHelper::debugAppend(resultEl, "volume", mVolume);
    IHelper::debugAppend(resultEl, "sample rate", mSampleRate);
    IHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation);
    IHelper::debugAppend(resultEl, "latency", mLatency);
    IHelper::debugAppend(resultEl, "channel count", mChannelCount);
    IHelper::debugAppend(resultEl, "device id", mDeviceID);
    IHelper::debugAppend(resultEl, "group id", mGroupID);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaDevicesTypes::SupportedConstraints::hash() const
  {
    auto hasher = IHasher::sha1();;

    hasher->update("ortc::IMediaDevicesTypes::SupportedConstraints:");
    hasher->update(mWidth);
    hasher->update(":");
    hasher->update(mHeight);
    hasher->update(":");
    hasher->update(mAspectRatio);
    hasher->update(":");
    hasher->update(mFrameRate);
    hasher->update(":");
    hasher->update(mFacingMode);
    hasher->update(":");
    hasher->update(mVolume);
    hasher->update(":");
    hasher->update(mSampleRate);
    hasher->update(":");
    hasher->update(mEchoCancellation);
    hasher->update(":");
    hasher->update(mLatency);
    hasher->update(":");
    hasher->update(mChannelCount);
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
  #pragma mark IMediaDevicesTypes::Device
  #pragma mark


  //---------------------------------------------------------------------------
  IMediaDevicesTypes::Device::Device(ElementPtr elem)
  {
    if (!elem) return;

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("deviceKind"));
      if (str.hasData()) {
        try {
          mKind = IMediaDevicesTypes::toDeviceKind(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("priority value out of range") + ZS_PARAM("value", str))
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::Device", "label", mLabel);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::Device", "deviceId", mDeviceID);
    IHelper::getElementValue(elem, "ortc::IMediaDevicesTypes::Device", "groupId", mGroupID);

    ElementPtr supportedConstraintsEl = elem->findFirstChildElement("supportedConstraints");
    if (supportedConstraintsEl) {
      mSupportedConstraints = SupportedConstraints(supportedConstraintsEl);
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevicesTypes::Device::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "deviceKind", IMediaDevicesTypes::toString(mKind), false);

    IHelper::adoptElementValue(elem, "label", mLabel, false);
    IHelper::adoptElementValue(elem, "deviceId", mDeviceID, false);
    IHelper::adoptElementValue(elem, "groupId", mGroupID, false);

    elem->adoptAsLastChild(mSupportedConstraints.createElement("supportedConstraints"));

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevicesTypes::Device::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaDevicesTypes::Device");

    IHelper::debugAppend(resultEl, "device kind", toString(mKind));
    IHelper::debugAppend(resultEl, "label", mLabel);
    IHelper::debugAppend(resultEl, "device id", mDeviceID);
    IHelper::debugAppend(resultEl, "group id", mGroupID);
    IHelper::debugAppend(resultEl, mSupportedConstraints.toDebug());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaDevicesTypes::Device::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaDevicesTypes::Device:");

    hasher->update(toString(mKind));
    hasher->update(":");
    hasher->update(mLabel);
    hasher->update(":");
    hasher->update(mDeviceID);
    hasher->update(":");
    hasher->update(mGroupID);
    hasher->update(":");
    hasher->update(mSupportedConstraints.hash());

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevices::MediaStreamTrackList
  #pragma mark

  //---------------------------------------------------------------------------
  IMediaDevices::MediaStreamTrackListPtr IMediaDevices::MediaStreamTrackList::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(MediaStreamTrackList, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevicesTypes::MediaStreamTrackList::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaDevicesTypes::MediaStreamTrackList");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto &track = (*iter);
      IHelper::debugAppend(resultEl, IMediaStreamTrack::toDebug(track));
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaDevicesTypes::MediaStreamTrackList::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaDevicesTypes::MediaStreamTrackList:");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto &track = (*iter);
      hasher->update(track ? track->getID() : 0);
      hasher->update(":");
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevices::DeviceList
  #pragma mark


  //---------------------------------------------------------------------------
  IMediaDevices::DeviceList::DeviceList(ElementPtr elem)
  {
    if (!elem) return;

    ElementPtr deviceEl = elem->findFirstChildElement("device");
    while (deviceEl) {
      Device device(deviceEl);
      push_back(device);
      deviceEl = deviceEl->findNextSiblingElement("device");
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevices::DeviceList::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (size() < 1) return ElementPtr();

    for (auto iter = begin(); iter != end(); ++iter) {
      auto &device = (*iter);
      elem->adoptAsLastChild(device.createElement());
    }

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  IMediaDevices::DeviceListPtr IMediaDevices::DeviceList::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(DeviceList, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevicesTypes::DeviceList::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IMediaDevicesTypes::DeviceList");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto &device = (*iter);
      IHelper::debugAppend(resultEl, device.toDebug());
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaDevicesTypes::DeviceList::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IMediaDevicesTypes::DeviceList:");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto &device = (*iter);
      hasher->update(device.hash());
      hasher->update(":");
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaDevices
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevices::toDebug()
  {
    return internal::IMediaDevicesFactory::singleton().singletonToDebug();
  }

  //---------------------------------------------------------------------------
  IMediaDevicesTypes::SupportedConstraintsPtr IMediaDevices::getSupportedConstraints()
  {
    return internal::IMediaDevicesFactory::singleton().getSupportedConstraints();
  }

  //---------------------------------------------------------------------------
  IMediaDevicesTypes::PromiseWithDeviceListPtr IMediaDevices::enumerateDevices()
  {
    return internal::IMediaDevicesFactory::singleton().enumerateDevices();
  }

  //---------------------------------------------------------------------------
  IMediaDevicesTypes::PromiseWithMediaStreamTrackListPtr IMediaDevices::getUserMedia(const Constraints &constraints)
  {
    return internal::IMediaDevicesFactory::singleton().getUserMedia(constraints);
  }

  //---------------------------------------------------------------------------
  IMediaDevicesSubscriptionPtr IMediaDevices::subscribe(IMediaDevicesDelegatePtr delegate)
  {
    return internal::IMediaDevicesFactory::singleton().subscribe(delegate);
  }

} // namespace ortc
