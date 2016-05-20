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
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <limits>
#include <float.h>
#include <math.h>

#include <webrtc/modules/video_capture/video_capture_factory.h>
#include <webrtc/modules/audio_device/audio_device_impl.h>

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

  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  using zsLib::SingletonManager;
  using zsLib::DOUBLE;

  using zsLib::Numeric;
  using zsLib::Log;

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
      mTrackID(string(zsLib::createUUID())),
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

      mCapabilities = make_shared<Capabilities>();

      mSettings = make_shared<Settings>();

      mTransport = Transport::create(mThisWeak.lock());

      if (mKind == Kind_Video && !mRemote) {
        mDeviceID = mConstraints->mAdvanced.front()->mDeviceID.mValue.value().mValue.value();
        mVideoCaptureModule = webrtc::VideoCaptureFactory::Create(0, mDeviceID.c_str());
        if (!mVideoCaptureModule) {
          return;
        }

        mVideoCaptureModule->AddRef();

        mVideoCaptureModule->RegisterCaptureDataCallback(*mTransport);

        webrtc::VideoCaptureModule::DeviceInfo* info = webrtc::VideoCaptureFactory::CreateDeviceInfo(0);
        if (!info) {
          return;
        }
        
        std::list<VideoCaptureCapabilityWithDistance> capabilityCandidates;
        int32_t numCapabilities = info->NumberOfCapabilities(mDeviceID.c_str());
        for (INT i = 0; i < numCapabilities; ++i) {
          webrtc::VideoCaptureCapability capability;
          if (info->GetCapability(mDeviceID.c_str(), i, capability) == -1)
            continue;
          ConstrainLongRange widthRange;
          ConstrainLongRange heightRange;
          if (mConstraints->mWidth.mValue.hasValue())
            widthRange.mIdeal = mConstraints->mWidth.mValue.value();
          if (mConstraints->mHeight.mValue.hasValue())
            heightRange.mIdeal = mConstraints->mHeight.mValue.value();
          if (mConstraints->mWidth.mRange.hasValue())
            widthRange = mConstraints->mWidth.mRange;
          if (mConstraints->mHeight.mRange.hasValue())
            heightRange = mConstraints->mHeight.mRange;
          FLOAT sizeDistance = calculateSizeDistance(widthRange, heightRange, capability);
          if (sizeDistance == FLT_MAX)
            continue;
          ConstrainDoubleRange frameRateRange;
          if (mConstraints->mFrameRate.mValue.hasValue())
            frameRateRange.mIdeal = mConstraints->mFrameRate.mValue.value();
          if (mConstraints->mFrameRate.mRange.hasValue())
            frameRateRange = mConstraints->mFrameRate.mRange;
          FLOAT frameRateDistance = calculateFrameRateDistance(frameRateRange, capability);
          if (frameRateDistance == FLT_MAX)
            continue;
          ConstrainDoubleRange aspectRatioRange;
          if (mConstraints->mAspectRatio.mValue.hasValue())
            aspectRatioRange.mIdeal = mConstraints->mAspectRatio.mValue.value();
          if (mConstraints->mAspectRatio.mRange.hasValue())
            aspectRatioRange = mConstraints->mAspectRatio.mRange;
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
        std::list<ConstraintSetPtr>::iterator constraintsIter = mConstraints->mAdvanced.begin();
        while (constraintsIter != mConstraints->mAdvanced.end()) {
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
        
        if (bestDistance == FLT_MAX)
          return;

        mSettings->mWidth = bestCapability.width;
        mSettings->mHeight = bestCapability.height;
        mSettings->mFrameRate = bestCapability.maxFPS;
        mSettings->mDeviceID = mDeviceID;

        if (mVideoCaptureModule->StartCapture(bestCapability) != 0) {
          mVideoCaptureModule->DeRegisterCaptureDataCallback();
          return;
        }
      } else if (mKind == Kind_Video && mRemote) {

      } else if (mKind == Kind_Audio) {
        if (!mRemote)
          mDeviceID = mConstraints->mAdvanced.front()->mDeviceID.mValue.value().mValue.value();
        mAudioDeviceModule = webrtc::AudioDeviceModuleImpl::Create(1, webrtc::AudioDeviceModule::kWindowsWasapiAudio);
        if (!mAudioDeviceModule) {
          return;
        }
        mAudioDeviceModule->AddRef();
        //mAudioDeviceModule->RegisterAudioCallback(mTransport.get());
        mAudioDeviceModule->Init();

        mSettings->mDeviceID = mDeviceID;
      }

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
      AutoRecursiveLock lock(*this);

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
      AutoRecursiveLock lock(*this);

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
      AutoRecursiveLock lock(*this);

      if (mVideoCaptureModule) {
        mVideoCaptureModule->StopCapture();
        mVideoCaptureModule->DeRegisterCaptureDataCallback();
      }

      if (mAudioDeviceModule) {
        if (!mRemote)
          mAudioDeviceModule->StopRecording();
        else
          mAudioDeviceModule->StopPlayout();
        //mAudioDeviceModule->RegisterAudioCallback(nullptr);
        mAudioDeviceModule->Terminate();
      }
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
    void MediaStreamTrack::setVideoRenderCallback(void* callback)
    {
      AutoRecursiveLock lock(*this);

      if (mKind == Kind_Video)
        mVideoRendererCallback = (webrtc::VideoRenderCallback*)callback;
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
    void MediaStreamTrack::renderVideoFrame(const webrtc::VideoFrame& videoFrame)
    {
      AutoRecursiveLock lock(*this);

      if (mVideoRendererCallback) mVideoRendererCallback->RenderFrame(1, videoFrame);
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

    webrtc::AudioDeviceModule* MediaStreamTrack::getAudioDeviceModule()
    {
      AutoRecursiveLock lock(*this);

      return mAudioDeviceModule;
    }

    void MediaStreamTrack::start()
    {
      AutoRecursiveLock lock(*this);
      if (mAudioDeviceModule) {
        if (!mRemote) {
          mAudioDeviceModule->SetRecordingDevice(webrtc::AudioDeviceModule::kDefaultCommunicationDevice);
          mAudioDeviceModule->InitRecording();
          mAudioDeviceModule->StartRecording();
        } else {
          mAudioDeviceModule->SetPlayoutDevice(webrtc::AudioDeviceModule::kDefaultCommunicationDevice);
          mAudioDeviceModule->InitPlayout();
          mAudioDeviceModule->StartPlayout();
        }
      }
    }

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
      UseSenderChannelPtr channel;
      
      {
        AutoRecursiveLock lock(*this);

        if (mVideoRendererCallback) mVideoRendererCallback->RenderFrame(1, videoFrame);
        
        channel = mSenderChannel.lock();
      }
      
      if (!channel) return;
      
      channel->sendVideoFrame(videoFrame);
    }

    //-------------------------------------------------------------------------
    void MediaStreamTrack::OnCaptureDelayChanged(const int32_t id, const int32_t delay)
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => webrtc::AudioTransport
    #pragma mark

    //-------------------------------------------------------------------------
    int32_t MediaStreamTrack::RecordedDataIsAvailable(
                                                      const void* audioSamples,
                                                      const size_t nSamples,
                                                      const size_t nBytesPerSample,
                                                      const uint8_t nChannels,
                                                      const uint32_t samplesPerSec,
                                                      const uint32_t totalDelayMS,
                                                      const int32_t clockDrift,
                                                      const uint32_t currentMicLevel,
                                                      const bool keyPressed,
                                                      uint32_t& newMicLevel
                                                      )
    {
      UseSenderChannelPtr channel;
      {
        AutoRecursiveLock lock(*this);
        channel = mSenderChannel.lock();
      }

#define TODO_VERIFY_RETURN_RESULT 1
#define TODO_VERIFY_RETURN_RESULT 2
      if (!channel) return 0;
      
      return channel->sendAudioSamples(audioSamples, nSamples, nChannels);
    }

    //-------------------------------------------------------------------------
    int32_t MediaStreamTrack::NeedMorePlayData(
                                               const size_t nSamples,
                                               const size_t nBytesPerSample,
                                               const uint8_t nChannels,
                                               const uint32_t samplesPerSec,
                                               void* audioSamples,
                                               size_t& nSamplesOut,
                                               int64_t* elapsed_time_ms,
                                               int64_t* ntp_time_ms
                                               )
    {
      nSamplesOut = 0;  // no samples
      
      UseReceiverChannelPtr channel;
      
      {
        AutoRecursiveLock lock(*this);
        channel = mReceiverChannel.lock();
      }
      
#define TODO_VERIFY_RETURN_RESULT 1
#define TODO_VERIFY_RETURN_RESULT 2
      if (!channel) return 0;
      
      return channel->getAudioSamples(nSamples, nChannels, audioSamples, nSamplesOut);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    MediaStreamTrack::Transport::Transport(
                                           const make_private &,
                                           MediaStreamTrackPtr outer
                                           ) :
      mOuter(outer)
    {
    }
        
    //-------------------------------------------------------------------------
    MediaStreamTrack::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void MediaStreamTrack::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    MediaStreamTrack::TransportPtr MediaStreamTrack::Transport::create(MediaStreamTrackPtr outer)
    {
      TransportPtr pThis(make_shared<Transport>(make_private{}, outer));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack::Transport => webrtc::VideoCaptureDataCallback
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaStreamTrack::Transport::OnIncomingCapturedFrame(const int32_t id, const webrtc::VideoFrame& videoFrame)
    {
      auto outer = mOuter.lock();
      if (!outer) return;
      return outer->OnIncomingCapturedFrame(id, videoFrame);
    }
    
    //-------------------------------------------------------------------------
    void MediaStreamTrack::Transport::OnCaptureDelayChanged(const int32_t id, const int32_t delay)
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
    #pragma mark MediaStreamTrack::Transport => webrtc::AudioTransport
    #pragma mark

    //-------------------------------------------------------------------------
    int32_t MediaStreamTrack::Transport::RecordedDataIsAvailable(
                                                                 const void* audioSamples,
                                                                 const size_t nSamples,
                                                                 const size_t nBytesPerSample,
                                                                 const size_t nChannels,
                                                                 const uint32_t samplesPerSec,
                                                                 const uint32_t totalDelayMS,
                                                                 const int32_t clockDrift,
                                                                 const uint32_t currentMicLevel,
                                                                 const bool keyPressed,
                                                                 uint32_t& newMicLevel
                                                                 )
    {
      auto outer = mOuter.lock();
#define TODO_VERIFY_RESULT 1
#define TODO_VERIFY_RESULT 2
      if (!outer) return 0;
      return outer->RecordedDataIsAvailable(
                                            audioSamples,
                                            nSamples,
                                            nBytesPerSample,
                                            nChannels,
                                            samplesPerSec,
                                            totalDelayMS,
                                            clockDrift,
                                            currentMicLevel,
                                            keyPressed,
                                            newMicLevel
                                            );
    }
    
    //-------------------------------------------------------------------------
    int32_t MediaStreamTrack::Transport::NeedMorePlayData(
                                                          const size_t nSamples,
                                                          const size_t nBytesPerSample,
                                                          const size_t nChannels,
                                                          const uint32_t samplesPerSec,
                                                          void* audioSamples,
                                                          size_t& nSamplesOut,
                                                          int64_t* elapsed_time_ms,
                                                          int64_t* ntp_time_ms
                                                          )
    {
      auto outer = mOuter.lock();
#define TODO_VERIFY_RESULT 1
#define TODO_VERIFY_RESULT 2
      if (!outer) return 0;
      return outer->NeedMorePlayData(
                                     nSamples,
                                     nBytesPerSample,
                                     nChannels,
                                     samplesPerSec,
                                     audioSamples,
                                     nSamplesOut,
                                     elapsed_time_ms,
                                     ntp_time_ms
                                     );
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
        ZS_LOG_TRACE(debug("not ready"))
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
    FLOAT MediaStreamTrack::calculateSizeDistance(
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
    FLOAT MediaStreamTrack::calculateFrameRateDistance(
                                                       ConstrainDoubleRange frameRate,
                                                       webrtc::VideoCaptureCapability capability
                                                       )
    {
      if (frameRate.mMin.hasValue() && frameRate.mMin.value() > (DOUBLE)capability.maxFPS)
        return FLT_MAX;
      if (frameRate.mMax.hasValue() && frameRate.mMax.value() < (DOUBLE)capability.maxFPS)
        return FLT_MAX;
      if (frameRate.mExact.hasValue() && fabs(frameRate.mExact.value() - (DOUBLE)capability.maxFPS) > 0.01F)
        return FLT_MAX;
      FLOAT frameRateDistance = 0.0F;
      if (frameRate.mIdeal.hasValue() && fabs(frameRate.mExact.value() - (DOUBLE)capability.maxFPS) > 0.01F) {
        frameRateDistance = (FLOAT)(abs((int)(capability.maxFPS - frameRate.mIdeal.value()))) / (FLOAT)frameRate.mIdeal.value();
      }
      
      return frameRateDistance;
    }
    
    //-------------------------------------------------------------------------
    FLOAT MediaStreamTrack::calculateAspectRatioDistance(
                                                         ConstrainDoubleRange aspectRatio,
                                                         webrtc::VideoCaptureCapability capability
                                                         )
    {
      DOUBLE capabilityAspectRatio = (DOUBLE)capability.width / (DOUBLE)capability.height;
      
      if (aspectRatio.mMin.hasValue() && aspectRatio.mMin.value() > capabilityAspectRatio)
        return FLT_MAX;
      if (aspectRatio.mMax.hasValue() && aspectRatio.mMax.value() < capabilityAspectRatio)
        return FLT_MAX;
      if (aspectRatio.mExact.hasValue() && fabs(aspectRatio.mExact.value() - capabilityAspectRatio) > 0.001F)
        return FLT_MAX;
      FLOAT aspectRatioDistance = 0.0F;
      if (aspectRatio.mIdeal.hasValue() && fabs(aspectRatio.mIdeal.value() - capabilityAspectRatio) > 0.001F) {
        aspectRatioDistance = (capabilityAspectRatio - aspectRatio.mIdeal.value()) / aspectRatio.mIdeal.value();
      }
      
      return aspectRatioDistance;
    }
    
    //-------------------------------------------------------------------------
    FLOAT MediaStreamTrack::calculateFormatDistance(webrtc::VideoCaptureCapability capability)
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

    UseHelper::getElementValue(elem, "deviceId", "ortc::IMediaStreamTrackTypes::Capabilities", mDeviceID);
    UseHelper::getElementValue(elem, "groupId", "ortc::IMediaStreamTrackTypes::Capabilities", mGroupID);
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

    UseHelper::adoptElementValue(elem, "deviceId", mDeviceID, false);
    UseHelper::adoptElementValue(elem, "groupId", mGroupID, false);

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

    UseServicesHelper::debugAppend(resultEl, "width", mWidth.hasValue() ? mWidth.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "height", mHeight.hasValue() ? mHeight.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio.hasValue() ? mAspectRatio.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "frame rate", mFrameRate.hasValue() ? mFrameRate.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "facingMode", mFacingMode.hasValue() ? mFacingMode.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "volume", mVolume.hasValue() ? mVolume.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "sample rate", mSampleRate.hasValue() ? mSampleRate.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation.hasValue() ? mEchoCancellation.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "latency", mLatency.hasValue() ? mLatency.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "channel acount", mChannelCount.hasValue() ? mChannelCount.value().toDebug() : ElementPtr());
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
    hasher.update(mChannelCount.hasValue() ? mChannelCount.value().hash() : String());
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
  IMediaStreamTrackTypes::Settings::Settings(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "width", mWidth);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "height", mHeight);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "aspectRatio", mAspectRatio);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "frameRate", mFrameRate);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "facingMode", mFacingMode);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "orientation", mOrientation);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "volume", mVolume);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "sampleRate", mSampleRate);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "sampleSize", mSampleSize);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "echoCancellation", mEchoCancellation);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "latency", mLatency);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "channelCount", mChannelCount);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "deviceId", mDeviceID);
    UseHelper::getElementValue(elem, "ortc::IMediaStreamTrackTypes::Settings", "groupId", mGroupID);
  }

  //---------------------------------------------------------------------------
  ElementPtr IMediaStreamTrackTypes::Settings::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "width", mWidth);
    UseHelper::adoptElementValue(elem, "height", mHeight);
    UseHelper::adoptElementValue(elem, "aspectRatio", mAspectRatio);
    UseHelper::adoptElementValue(elem, "frameRate", mFrameRate);
    UseHelper::adoptElementValue(elem, "facingMode", mFacingMode);
    UseHelper::adoptElementValue(elem, "orientation", mOrientation);
    UseHelper::adoptElementValue(elem, "volume", mVolume);
    UseHelper::adoptElementValue(elem, "sampleRate", mSampleRate);
    UseHelper::adoptElementValue(elem, "sampleSize", mSampleSize);
    UseHelper::adoptElementValue(elem, "echoCancellation", mEchoCancellation);
    UseHelper::adoptElementValue(elem, "latency", mLatency);
    UseHelper::adoptElementValue(elem, "channelCount", mChannelCount);
    UseHelper::adoptElementValue(elem, "deviceId", mDeviceID);
    UseHelper::adoptElementValue(elem, "groupId", mGroupID);

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
    UseServicesHelper::debugAppend(resultEl, "latency", mLatency);
    UseServicesHelper::debugAppend(resultEl, "channel count", mChannelCount);
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
    hasher.update(mChannelCount);
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

    UseServicesHelper::debugAppend(resultEl, "width", mWidth.toDebug());
    UseServicesHelper::debugAppend(resultEl, "height", mHeight.toDebug());
    UseServicesHelper::debugAppend(resultEl, "aspect ratio", mAspectRatio.toDebug());
    UseServicesHelper::debugAppend(resultEl, "frame rate", mFrameRate.toDebug());
    UseServicesHelper::debugAppend(resultEl, "facingMode", mFacingMode.toDebug());
    UseServicesHelper::debugAppend(resultEl, "volume", mVolume.toDebug());
    UseServicesHelper::debugAppend(resultEl, "sample rate", mSampleRate.toDebug());
    UseServicesHelper::debugAppend(resultEl, "echo cancellation", mEchoCancellation.toDebug());
    UseServicesHelper::debugAppend(resultEl, "latency", mLatency.toDebug());
    UseServicesHelper::debugAppend(resultEl, "channel count", mChannelCount.toDebug());
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
    hasher.update(mChannelCount.hash());
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

  //---------------------------------------------------------------------------
  IMediaStreamTrackPtr IMediaStreamTrack::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(IMediaStreamTrack, any);
  }

}
