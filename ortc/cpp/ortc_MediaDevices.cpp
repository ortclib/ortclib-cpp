/*

 Copyright (c) 2017, Hookflash Inc. / Optical Tone Ltd.
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

#define ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

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
#include <zsLib/eventing/Log.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <ortc/internal/webrtc_pre_include.h>
#include <modules/video_capture/video_capture_factory.h>
#include <modules/audio_device/audio_device_impl.h>
#include <ortc/internal/webrtc_post_include.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_devices) }

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
    //
    // (helpers)
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDevicesSettingsDefaults
    //

    class MediaDevicesSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaDevicesSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaDevicesSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<MediaDevicesSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaDevicesSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<MediaDevicesSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
        //      ISettings::setUInt(ORTC_SETTING_MEDIA_DEVICES, 0);
      }
      
    };

    //-------------------------------------------------------------------------
    void installMediaDevicesSettingsDefaults() noexcept
    {
      MediaDevicesSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IICETransportForSettings
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDevices
    //
    
    //---------------------------------------------------------------------------
    const char *MediaDevices::toString(States state) noexcept
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
                               ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSubscriptions(decltype(mSubscriptions)::create())
    {
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void MediaDevices::init() noexcept
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaDevices::~MediaDevices() noexcept
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDevicesPtr MediaDevices::create() noexcept
    {
      MediaDevicesPtr pThis(make_shared<MediaDevices>(make_private {}, IORTCForInternal::queueORTC()));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    MediaDevicesPtr MediaDevices::singleton() noexcept
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
    MediaDevicesPtr MediaDevices::convert(IMediaDevicesPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(MediaDevices, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDevices => IMediaDevices
    //
    
    //-------------------------------------------------------------------------
    ElementPtr MediaDevices::singletonToDebug() noexcept
    {

      MediaDevicesPtr pThis(MediaDevices::singleton());
      if (!pThis) return ElementPtr();

      AutoRecursiveLock lock(*pThis);
      return pThis->toDebug();
    }

    //-------------------------------------------------------------------------
    IMediaDevices::SupportedConstraintsPtr MediaDevices::getSupportedConstraints() noexcept
    {
      SupportedConstraintsPtr result(make_shared<SupportedConstraints>());

#pragma ZS_BUILD_NOTE("TODO","Implement media devices getSupportedConstraints")

      return result;
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithDeviceListPtr MediaDevices::enumerateDevices() noexcept
    {
      PromiseWithDeviceListPtr promise = PromiseWithDeviceList::create(IORTCForInternal::queueDelegate());

      MediaDevicesPtr pThis(MediaDevices::singleton());

      if (!pThis) {
        ZS_LOG_WARNING(Basic, slog("media devices singleton is gone"));
        promise->reject();
        return promise;
      }

      IMediaDevicesAsyncDelegateProxy::create(pThis)->onEnumerateDevices(promise);
      return promise;
    }


    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithSettingsListPtr MediaDevices::enumerateDefaultModes(const char *deviceID) noexcept
    {
      PromiseWithSettingsListPtr promise = PromiseWithSettingsList::create(IORTCForInternal::queueDelegate());

      MediaDevicesPtr pThis(MediaDevices::singleton());

      if (!pThis) {
        ZS_LOG_WARNING(Basic, slog("media devices singleton is gone"))
        promise->reject();
        return promise;
      }

      IMediaDevicesAsyncDelegateProxy::create(pThis)->onEnumerateDefaultModes(promise, deviceID);
      return promise;
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithMediaStreamTrackListPtr MediaDevices::getUserMedia(const Constraints &constraints) noexcept
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
    IMediaDevicesSubscriptionPtr MediaDevices::subscribe(IMediaDevicesDelegatePtr originalDelegate) noexcept
    {
      ZS_LOG_DETAIL(slog("subscribing to media devices"))

      auto pThis = singleton();
      if (!pThis) {
        struct BogusSubscription : public IMediaDevicesSubscription {
          virtual PUID getID() const noexcept override {return mID;}
          virtual void cancel() noexcept override {}
          virtual void background() noexcept override {}

          AutoPUID mID;
        };

        return make_shared<BogusSubscription>();
      }

      AutoRecursiveLock lock(*pThis);
      if (!originalDelegate) return IMediaDevicesSubscriptionPtr();

      IMediaDevicesSubscriptionPtr subscription = pThis->mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IMediaDevicesDelegatePtr delegate = pThis->mSubscriptions.delegate(subscription, true);

      if (delegate) {
#pragma ZS_BUILD_NOTE("NEEDED?","Do we need to tell about any missing events?")
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
    //
    // MediaDevices => IWakeDelegate
    //

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
    //
    // MediaDevices => ITimerDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDevices::onTimer(ITimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
#pragma ZS_BUILD_NOTE("TODO","Media devices timer (if needed)")
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDevices => IMediaDevicesAsyncDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDevices::onEnumerateDevices(PromiseWithDeviceListPtr promise)
    {
      AutoRecursiveLock lock(*this);

      DeviceListPtr value(std::make_shared<DeviceList>());

      webrtc::VideoCaptureModule::DeviceInfo* info = webrtc::VideoCaptureFactory::CreateDeviceInfo();
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

      webrtc::AudioDeviceModule::AudioLayer audioLayer;
#ifdef WINUWP
      audioLayer = webrtc::AudioDeviceModule::kWindowsWasapiAudio;
#else
      audioLayer = webrtc::AudioDeviceModule::kPlatformDefaultAudio;
#endif

      rtc::scoped_refptr<webrtc::AudioDeviceModule> audioDevice =
        webrtc::AudioDeviceModule::Create(audioLayer);
      if (!audioDevice) {
        promise->reject();
        return;
      }

      audioDevice->Init();

      int numMics = audioDevice->RecordingDevices();
      for (int index = 0; index < numMics; ++index) {
        char deviceName[webrtc::kAdmMaxDeviceNameSize];
        char deviceGuid[webrtc::kAdmMaxGuidSize];
        if (audioDevice->RecordingDeviceName(static_cast<uint16_t>(index), deviceName, deviceGuid) != -1) {
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
        if (audioDevice->PlayoutDeviceName(static_cast<uint16_t>(index), deviceName, deviceGuid) != -1) {
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
    void MediaDevices::onEnumerateDefaultModes(
                                               PromiseWithSettingsListPtr promise,
                                               ZS_MAYBE_USED() const char *deviceID
                                               )
    {
      ZS_MAYBE_USED(deviceID);
      AutoRecursiveLock lock(*this);
#pragma ZS_BUILD_NOTE("TODO","Enumerate devices")
      promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_NotImplemented, "method not implemented"));
    }

    //-------------------------------------------------------------------------
    void MediaDevices::onGetUserMedia(PromiseWithMediaStreamTrackListPtr promise, ConstraintsPtr constraints)
    {
      AutoRecursiveLock lock(*this);

      MediaStreamTrackListPtr value(std::make_shared<MediaStreamTrackList>());

      if (constraints->mVideo)
      {
        auto videoTrack = IMediaStreamTrackFactory::singleton().createForMediaDevices(
                                                                                      IMediaStreamTrackTypes::Kinds::Kind_Video,
                                                                                      *constraints->mVideo
                                                                                      );
        value->push_back(MediaStreamTrack::convert(videoTrack));
      }

      if (constraints->mAudio)
      {
        auto audioTrack = IMediaStreamTrackFactory::singleton().createForMediaDevices(
                                                                                      IMediaStreamTrackTypes::Kinds::Kind_Audio,
                                                                                      *constraints->mAudio
                                                                                      );
        value->push_back(MediaStreamTrack::convert(audioTrack));
      }

      promise->resolve(value);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDevices => ISingletonManagerDelegate
    //

    //-------------------------------------------------------------------------
    void MediaDevices::notifySingletonCleanup() noexcept
    {
      ZS_LOG_DEBUG(log("notify singleton cleanup"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDevices => (internal)
    //

    //-------------------------------------------------------------------------
    Log::Params MediaDevices::log(const char *message) const noexcept
    {
      ElementPtr objectEl = Element::create("ortc::MediaDevices");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params MediaDevices::slog(const char *message) noexcept
    {
      ElementPtr objectEl = Element::create("ortc::MediaDevices");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params MediaDevices::debug(const char *message) const noexcept
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr MediaDevices::toDebug() const noexcept
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
    bool MediaDevices::isShuttingDown() const noexcept
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool MediaDevices::isShutdown() const noexcept
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void MediaDevices::step() noexcept
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
    bool MediaDevices::stepBogusDoSomething() noexcept
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
#pragma ZS_BUILD_NOTE("TODO","Implement media devices step")

      return true;
    }

    //-------------------------------------------------------------------------
    void MediaDevices::cancel() noexcept
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#pragma ZS_BUILD_NOTE("TODO","Implement media devices graceful shutdown")

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
    void MediaDevices::setState(States state) noexcept
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
    void MediaDevices::setError(WORD errorCode, const char *inReason) noexcept
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
    //
    // IMediaDevicesFactory
    //

    //-------------------------------------------------------------------------
    IMediaDevicesFactory &IMediaDevicesFactory::singleton() noexcept
    {
      return MediaDevicesFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ElementPtr IMediaDevicesFactory::singletonToDebug() noexcept
    {
      if (this) {}
      return internal::MediaDevices::singletonToDebug();
    }

    //-------------------------------------------------------------------------
    MediaDevicesPtr IMediaDevicesFactory::create() noexcept
    {
      if (this) {}
      return internal::MediaDevices::create();
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::SupportedConstraintsPtr IMediaDevicesFactory::getSupportedConstraints() noexcept
    {
      if (this) {}
      return internal::MediaDevices::getSupportedConstraints();
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithDeviceListPtr IMediaDevicesFactory::enumerateDevices() noexcept
    {
      if (this) {}
      return internal::MediaDevices::enumerateDevices();
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithSettingsListPtr IMediaDevicesFactory::enumerateDefaultModes(const char *deviceID) noexcept
    {
      if (this) {}
      return internal::MediaDevices::enumerateDefaultModes(deviceID);
    }

    //-------------------------------------------------------------------------
    IMediaDevicesTypes::PromiseWithMediaStreamTrackListPtr IMediaDevicesFactory::getUserMedia(const Constraints &constraints) noexcept
    {
      if (this) {}
      return internal::MediaDevices::getUserMedia(constraints);
    }

    //-------------------------------------------------------------------------
    IMediaDevicesSubscriptionPtr IMediaDevicesFactory::subscribe(IMediaDevicesDelegatePtr delegate) noexcept
    {
      if (this) {}
      return internal::MediaDevices::subscribe(delegate);
    }

  } // internal namespace

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // (helpers)
  //

  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message) noexcept
  {
    return Log::Params(message, "ortc::IMediaDevicesTypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaDevicesTypes
  //

  //---------------------------------------------------------------------------
  const char *IMediaDevicesTypes::toString(DeviceKinds kind) noexcept
  {
    switch (kind) {
      case DeviceKind_AudioInput:   return "audioinput";
      case DeviceKind_AudioOutput:  return "audiooutput";
      case DeviceKind_VideoInput:   return "videoinput";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  IMediaDevicesTypes::DeviceKinds IMediaDevicesTypes::toDeviceKind(const char *deviceKindStr) noexcept(false)
  {
    String str(deviceKindStr);
    for (IMediaDevicesTypes::DeviceKinds index = IMediaDevicesTypes::DeviceKind_First; index <= IMediaDevicesTypes::DeviceKind_Last; index = static_cast<IMediaDevicesTypes::DeviceKinds>(static_cast<std::underlying_type<IMediaDevicesTypes::DeviceKinds>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IMediaDevicesTypes::toString(index))) return index;
    }
    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str);
    return IMediaDevicesTypes::DeviceKind_First;
  }

  //---------------------------------------------------------------------------
  IMediaStreamTrackTypes::Kinds IMediaDevicesTypes::toKind(DeviceKinds kind) noexcept
  {
    switch (kind) {
      case DeviceKind_AudioInput:   return Kind_Audio;
      case DeviceKind_AudioOutput:  return Kind_Audio;
      case DeviceKind_VideoInput:   return Kind_Video;
    }

    ZS_ASSERT_FAIL("unknown device kind");
    return Kind_Audio;
  }

  //---------------------------------------------------------------------------
  bool IMediaDevicesTypes::isAudio(DeviceKinds kind) noexcept
  {
    switch (kind) {
      case DeviceKind_AudioInput:   return true;
      case DeviceKind_AudioOutput:  return true;
      case DeviceKind_VideoInput:   return false;
    }

    ZS_ASSERT_FAIL("unknown device kind");
    return false;
  }

  //---------------------------------------------------------------------------
  bool IMediaDevicesTypes::isVideo(DeviceKinds kind) noexcept
  {
    switch (kind) {
      case DeviceKind_AudioInput:   return false;
      case DeviceKind_AudioOutput:  return false;
      case DeviceKind_VideoInput:   return true;
    }

    ZS_ASSERT_FAIL("unknown device kind");
    return false;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaDevicesTypes::SupportedConstraints
  //


  //---------------------------------------------------------------------------
  IMediaDevicesTypes::SupportedConstraints::SupportedConstraints(ElementPtr elem) noexcept
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
  ElementPtr IMediaDevicesTypes::SupportedConstraints::createElement(const char *objectName) const noexcept
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
  ElementPtr IMediaDevicesTypes::SupportedConstraints::toDebug() const noexcept
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
  String IMediaDevicesTypes::SupportedConstraints::hash() const noexcept
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
  //
  // IMediaDevicesTypes::Device
  //


  //---------------------------------------------------------------------------
  IMediaDevicesTypes::Device::Device(ElementPtr elem) noexcept
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
  ElementPtr IMediaDevicesTypes::Device::createElement(const char *objectName) const noexcept
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
  ElementPtr IMediaDevicesTypes::Device::toDebug() const noexcept
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
  String IMediaDevicesTypes::Device::hash() const noexcept
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
  //
  // IMediaDevices::MediaStreamTrackList
  //

  //---------------------------------------------------------------------------
  IMediaDevices::MediaStreamTrackListPtr IMediaDevices::MediaStreamTrackList::convert(AnyPtr any) noexcept
  {
    return ZS_DYNAMIC_PTR_CAST(MediaStreamTrackList, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevicesTypes::MediaStreamTrackList::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IMediaDevicesTypes::MediaStreamTrackList");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto &track = (*iter);
      ZS_EVENTING_TRACE_OBJECT_PTR(Trace, track, "media stream track list debug");
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IMediaDevicesTypes::MediaStreamTrackList::hash() const noexcept
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
  //
  // IMediaDevices::DeviceList
  //


  //---------------------------------------------------------------------------
  IMediaDevices::DeviceList::DeviceList(ElementPtr elem) noexcept
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
  ElementPtr IMediaDevices::DeviceList::createElement(const char *objectName) const noexcept
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
  IMediaDevices::DeviceListPtr IMediaDevices::DeviceList::convert(AnyPtr any) noexcept
  {
    return ZS_DYNAMIC_PTR_CAST(DeviceList, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevicesTypes::DeviceList::toDebug() const noexcept
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
  String IMediaDevicesTypes::DeviceList::hash() const noexcept
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
  //
  // IMediaDevices
  //

  //---------------------------------------------------------------------------
  ElementPtr IMediaDevices::toDebug() noexcept
  {
    return internal::IMediaDevicesFactory::singleton().singletonToDebug();
  }

  //---------------------------------------------------------------------------
  IMediaDevicesTypes::SupportedConstraintsPtr IMediaDevices::getSupportedConstraints() noexcept
  {
    return internal::IMediaDevicesFactory::singleton().getSupportedConstraints();
  }

  //---------------------------------------------------------------------------
  IMediaDevicesTypes::PromiseWithDeviceListPtr IMediaDevices::enumerateDevices() noexcept
  {
    return internal::IMediaDevicesFactory::singleton().enumerateDevices();
  }

  //---------------------------------------------------------------------------
  IMediaDevices::PromiseWithSettingsListPtr IMediaDevices::enumerateDefaultModes(const char *deviceID) noexcept
  {
    return internal::IMediaDevicesFactory::singleton().enumerateDefaultModes(deviceID);
  }

  //---------------------------------------------------------------------------
  IMediaDevicesTypes::PromiseWithMediaStreamTrackListPtr IMediaDevices::getUserMedia(const Constraints &constraints) noexcept
  {
    return internal::IMediaDevicesFactory::singleton().getUserMedia(constraints);
  }

  //---------------------------------------------------------------------------
  IMediaDevicesSubscriptionPtr IMediaDevices::subscribe(IMediaDevicesDelegatePtr delegate) noexcept
  {
    return internal::IMediaDevicesFactory::singleton().subscribe(delegate);
  }

} // namespace ortc
