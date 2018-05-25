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

#include <ortc/internal/ortc_MediaEngine.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/RTPPacket.h>
#include <ortc/RTCPPacket.h>
#include <ortc/IStatsReport.h>
#include <ortc/IHelper.h>

#include <zsLib/ISettings.h>
#include <zsLib/Singleton.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>
#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>

#include <rtc_base/event_tracer.h>

#if 0
#include <limits>
#include <float.h>
#include <math.h>

#include <rtc_base/timeutils.h>
#include <call/rtc_event_log.h>
#include <voice_engine/include/voe_codec.h>
#include <voice_engine/include/voe_rtp_rtcp.h>
#include <voice_engine/include/voe_network.h>
#include <voice_engine/include/voe_hardware.h>
#include <system_wrappers/include/cpu_info.h>
#include <voice_engine/include/voe_audio_processing.h>
#include <modules/video_capture/video_capture_factory.h>
#include <modules/audio_coding/codecs/builtin_audio_decoder_factory.h>
#ifdef WINRT
#include <third_party/h264_winrt/h264_winrt_factory.h>
#endif
#endif //0

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_engine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MediaEngineSettingsDefaults);
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
    #pragma mark MediaEngineSettingsDefaults
    #pragma mark

    class MediaEngineSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaEngineSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaEngineSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<MediaEngineSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaEngineSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<MediaEngineSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installMediaEngineSettingsDefaults()
    {
      MediaEngineSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForORTC
    #pragma mark

    //-------------------------------------------------------------------------
    void IMediaEngineForORTC::setLogLevel(Log::Level level)
    {
      IMediaEngineFactory::singleton().setLogLevel(level);
    }

    //-------------------------------------------------------------------------
    void IMediaEngineForORTC::ntpServerTime(const Milliseconds &value)
    {
      IMediaEngineFactory::singleton().ntpServerTime(value);
    }

    //-------------------------------------------------------------------------
    void IMediaEngineForORTC::startMediaTracing()
    {
      IMediaEngineFactory::singleton().startMediaTracing();
    }

    //-------------------------------------------------------------------------
    void IMediaEngineForORTC::stopMediaTracing()
    {
      IMediaEngineFactory::singleton().stopMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool IMediaEngineForORTC::isMediaTracing()
    {
      return IMediaEngineFactory::singleton().isMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool IMediaEngineForORTC::saveMediaTrace(String filename)
    {
      return IMediaEngineFactory::singleton().saveMediaTrace(filename);
    }

    //-------------------------------------------------------------------------
    bool IMediaEngineForORTC::saveMediaTrace(String host, int port)
    {
      return IMediaEngineFactory::singleton().saveMediaTrace(host, port);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDeviceCapture
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaDeviceCapture::MediaDeviceCapturePromisePtr IMediaEngineForMediaDeviceCapture::createMediaDeviceCapture(
                                                                                                                  MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                                                                  Kinds kind,
                                                                                                                  const TrackConstraints &constraints,
                                                                                                                  IMediaDeviceCaptureDelegatePtr delegate
                                                                                                                  )
    {
      return internal::IMediaEngineFactory::singleton().createMediaDeviceCapture(repaceExistingDeviceObjectID, kind, constraints, delegate);
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDeviceRender
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaEngineForMediaDeviceRender::MediaDeviceRenderPromisePtr IMediaEngineForMediaDeviceRender::createMediaDeviceRender(
                                                                                                                            MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                                                                            const TrackConstraints &constraints,
                                                                                                                            IMediaDeviceRenderDelegatePtr delegate
                                                                                                                            )
    {
      return internal::IMediaEngineFactory::singleton().createMediaDeviceRender(repaceExistingDeviceObjectID, constraints, delegate);
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDeviceRenderAudio
    #pragma mark
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTP
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPDecode
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPDecoderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaEngineForRTPDecoderAudio::PromiseWithRTPDecoderAudioPtr IMediaEngineForRTPDecoderAudio::createRTPDecoderAudio(
                                                                                                                        const Parameters &parameters,
                                                                                                                        IRTPDecoderDelegatePtr delegate
                                                                                                                        )
    {
      return internal::IMediaEngineFactory::singleton().createRTPDecoderAudio(parameters, delegate);
    }
        //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPDecoderVideo
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaEngineForRTPDecoderVideo::PromiseWithRTPDecoderVideoPtr IMediaEngineForRTPDecoderVideo::createRTPDecoderVideo(
                                                                                                                        const Parameters &parameters,
                                                                                                                        IRTPDecoderDelegatePtr delegate
                                                                                                                        )
    {
      return internal::IMediaEngineFactory::singleton().createRTPDecoderVideo(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPEncoderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaEngineForRTPEncoderAudio::PromiseWithRTPEncoderAudioPtr IMediaEngineForRTPEncoderAudio::createRTPEncoderAudio(
                                                                                                                        const Parameters &parameters,
                                                                                                                        IRTPEncoderDelegatePtr delegate
                                                                                                                        )
    {
      return internal::IMediaEngineFactory::singleton().createRTPEncoderAudio(parameters, delegate);
    }
        //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPEncoderVideo
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaEngineForRTPEncoderVideo::PromiseWithRTPEncoderVideoPtr IMediaEngineForRTPEncoderVideo::createRTPEncoderVideo(
                                                                                                                        const Parameters &parameters,
                                                                                                                        IRTPEncoderDelegatePtr delegate
                                                                                                                        )
    {
      return internal::IMediaEngineFactory::singleton().createRTPEncoderVideo(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *MediaEngine::toString(States state)
    {
      switch (state) {
        case State_Pending:       return "pending";
        case State_WakingUp:      return "waking up";
        case State_Ready:         return "ready";
        case State_GoingToSleep:  return "going to sleep";
        case State_Sleeping:      return "sleeping";
        case State_ShuttingDown:  return "shutting down";
        case State_Shutdown:      return "shutdown";
      }
      return "UNDEFINED";
    }
    
    //-------------------------------------------------------------------------
    MediaEngine::MediaEngine(
                             const make_private &,
                             IMessageQueuePtr queue
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      //traceCallback_(new WebRtcTraceCallback()),
      logSink_(new WebRtcLogSink())
    {
      ZS_EVENTING_1(x, i, Detail, MediaEngineCreate, ol, MediaEngine, Start, puid, id, id_);
    }

    //-------------------------------------------------------------------------
    void MediaEngine::init()
    {
      //webrtc::Trace::CreateTrace();
      //webrtc::Trace::SetTraceCallback(traceCallback_.get());

      auto level = IORTCForInternal::webrtcLogLevel();
      if (level.hasValue()) {
        internalSetLogLevel(level);
      }

#if defined(WINRT)
      webrtc::SetupEventTracer(&GetCategoryGroupEnabled, &AddTraceEvent);
#endif

      AutoRecursiveLock lock(*this);

      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaEngine::~MediaEngine()
    {
      if (isNoop()) return;

      //webrtc::Trace::SetTraceCallback(nullptr);
      //webrtc::Trace::ReturnTrace();

      thisWeak_.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, MediaEngineDestroy, ol, MediaEngine, Stop, puid, id, id_);
    }

    //-----------------------------------------------------------------------
    MediaEnginePtr MediaEngine::singleton()
    {
      static SingletonLazySharedPtr<MediaEngine> singleton(create());
      return singleton.singleton();
    }

    //-----------------------------------------------------------------------
    MediaEnginePtr MediaEngine::create()
    {
      auto pThis(make_shared<MediaEngine>(make_private{}, IORTCForInternal::queueBlockingMediaStartStopThread()));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForORTCPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForMediaDeviceCapturePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForMediaDeviceCaptureAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForMediaDeviceCaptureVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForMediaDeviceRenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForMediaDeviceRenderAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForRTPPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }
    
    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForRTPDecodePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForRTPDecodeAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForRTPDecodeVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForRTPEncodePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForRTPEncodeAudioPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr MediaEngine::convert(ForRTPEncodeVideoPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaEngine, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForORTC
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::internalSetLogLevel(Log::Level level)
    {
      //webrtc::TraceLevel traceLevel{ webrtc::kTraceAll };
      rtc::LoggingSeverity rtcLevel{ rtc::LS_INFO };

      /*switch (level)
      {
      case Log::Level::None:    rtcLevel = rtc::LS_NONE; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceNone); break;
      case Log::Level::Basic:   rtcLevel = rtc::LS_ERROR; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceCritical | webrtc::kTraceError); break;
      case Log::Level::Detail:  rtcLevel = rtc::LS_WARNING; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceWarning | webrtc::kTraceError | webrtc::kTraceCritical); break;
      case Log::Level::Debug:   rtcLevel = rtc::LS_INFO; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceDefault); break;
      case Log::Level::Trace:   rtcLevel = rtc::LS_VERBOSE; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceDefault | webrtc::kTraceModuleCall | webrtc::kTraceTimer | webrtc::kTraceDebug); break;
      case Log::Level::Insane:  rtcLevel = rtc::LS_SENSITIVE; traceLevel = static_cast<webrtc::TraceLevel>(webrtc::kTraceAll); break;
      }*/

      rtc::LogMessage::RemoveLogToStream(logSink_.get());
      rtc::LogMessage::AddLogToStream(logSink_.get(), rtcLevel);

      //webrtc::Trace::set_level_filter(traceLevel);
    }

    //-------------------------------------------------------------------------
    void MediaEngine::internalNTPServerTime(const Milliseconds &value)
    {
#define TODO 1
#define TODO 2
#if 0
      rtc::SyncWithNtp(value.count());
#endif //0
    }

    //-------------------------------------------------------------------------
    void MediaEngine::internalStartMediaTracing()
    {
      //traceLog_.EnableTraceInternalStorage();
      //traceLog_.StartTracing();
    }

    //-------------------------------------------------------------------------
    void MediaEngine::internalStopMediaTracing()
    {
      //traceLog_.StopTracing();
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::internalIsMediaTracing()
    {
      //return traceLog_.IsTracing();
      return false;
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::internalSaveMediaTrace(String filename)
    {
      //return traceLog_.Save(filename);
      return false;
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::internalSaveMediaTrace(String host, int port)
    {
      //return traceLog_.Save(host, port);
      return false;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForMediaDevice
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::notifyDeviceIsIdleOrShutdownStateChanged()
    {
      checkForIdleDevices_ = true;
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForMediaDeviceCapture
    #pragma mark

    //-------------------------------------------------------------------------
    MediaEngine::MediaDeviceCapturePromisePtr MediaEngine::innerCreateMediaDeviceCapture(
                                                                                         MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                                         Kinds kind,
                                                                                         const TrackConstraints &constraints,
                                                                                         IMediaDeviceCaptureDelegatePtr delegate
                                                                                         )
    {    
      auto pThis = thisWeak_.lock();

      PendingForMediaDeviceCapture info;
      info.repaceExistingDeviceObjectID_ = repaceExistingDeviceObjectID;
      info.kind_ = kind;
      info.promise_ = MediaDeviceCapturePromise::create(IORTCForInternal::queueDelegate());
      info.constraints_ = TrackConstraints::create(constraints);
      info.delegate_ = delegate;

      postClosure([pThis, info] {
        AutoRecursiveLock lock(*pThis);
        pThis->pendingForMediaDeviceCaptures_.push_back(info);
        pThis->step();
      });

      return info.promise_;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForMediaDeviceCaptureAudio
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForMediaDeviceCaptureVideo
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForMediaDeviceRender
    #pragma mark

    //-------------------------------------------------------------------------
    MediaEngine::MediaDeviceRenderPromisePtr MediaEngine::innerCreateMediaDeviceRender(
                                                                                       MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                                       const TrackConstraints &constraints,
                                                                                       IMediaDeviceRenderDelegatePtr delegate
                                                                                       )
    {    
      auto pThis = thisWeak_.lock();

      PendingForMediaDeviceRenderInfo info;
      info.repaceExistingDeviceObjectID_ = repaceExistingDeviceObjectID;
      info.promise_ = MediaDeviceRenderPromise::create(IORTCForInternal::queueDelegate());
      info.constraints_ = TrackConstraints::create(constraints);
      info.delegate_ = delegate;

      postClosure([pThis, info] {
        AutoRecursiveLock lock(*pThis);
        pThis->pendingForMediaDeviceRenders_.push_back(info);
        pThis->step();
      });

      return info.promise_;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForMediaDeviceRenderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForRTP
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::notifyRTPShutdownStateChanged()
    {
      checkForShutdownRTP_ = true;
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForRTPDecoder
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForRTPDecoderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    MediaEngine::PromiseWithRTPDecoderAudioPtr MediaEngine::innerCreateRTPDecoderAudio(
                                                                                       const Parameters &parameters,
                                                                                       IRTPDecoderDelegatePtr delegate
                                                                                       )
    {
      auto pThis = thisWeak_.lock();

      PendingForRTPDecodeAudio info;
      info.promise_ = PromiseWithRTPDecoderAudio::create(IORTCForInternal::queueDelegate());
      info.parameters_ = make_shared<Parameters>(parameters);
      info.delegate_ = delegate;

      postClosure([pThis, info] {
        AutoRecursiveLock lock(*pThis);
        pThis->pendingForRTPDecodeAudios_.push_back(info);
        pThis->step();
      });

      return info.promise_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForRTPDecoderVideo
    #pragma mark

    //-------------------------------------------------------------------------
    MediaEngine::PromiseWithRTPDecoderVideoPtr MediaEngine::innerCreateRTPDecoderVideo(
                                                                                       const Parameters &parameters,
                                                                                       IRTPDecoderDelegatePtr delegate
                                                                                       )
    {
      auto pThis = thisWeak_.lock();

      PendingForRTPDecodeVideo info;
      info.promise_ = PromiseWithRTPDecoderVideo::create(IORTCForInternal::queueDelegate());
      info.parameters_ = make_shared<Parameters>(parameters);
      info.delegate_ = delegate;

      postClosure([pThis, info] {
        AutoRecursiveLock lock(*pThis);
        pThis->pendingForRTPDecodeVideos_.push_back(info);
        pThis->step();
      });

      return info.promise_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForRTPEncoder
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForRTPEncoderAudio
    #pragma mark

    //-------------------------------------------------------------------------
    MediaEngine::PromiseWithRTPEncoderAudioPtr MediaEngine::innerCreateRTPEncoderAudio(
                                                                                       const Parameters &parameters,
                                                                                       IRTPEncoderDelegatePtr delegate
                                                                                       )
    {
      auto pThis = thisWeak_.lock();

      PendingForRTPEncodeAudio info;
      info.promise_ = PromiseWithRTPEncoderAudio::create(IORTCForInternal::queueDelegate());
      info.parameters_ = make_shared<Parameters>(parameters);
      info.delegate_ = delegate;

      postClosure([pThis, info] {
        AutoRecursiveLock lock(*pThis);
        pThis->pendingForRTPEncodeAudios_.push_back(info);
        pThis->step();
      });

      return info.promise_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngineForRTPEncoderVideo
    #pragma mark

    //-------------------------------------------------------------------------
    MediaEngine::PromiseWithRTPEncoderVideoPtr MediaEngine::innerCreateRTPEncoderVideo(
                                                                                       const Parameters &parameters,
                                                                                       IRTPEncoderDelegatePtr delegate
                                                                                       )
    {
      auto pThis = thisWeak_.lock();

      PendingForRTPEncodeVideo info;
      info.promise_ = PromiseWithRTPEncoderVideo::create(IORTCForInternal::queueDelegate());
      info.parameters_ = make_shared<Parameters>(parameters);
      info.delegate_ = delegate;

      postClosure([pThis, info] {
        AutoRecursiveLock lock(*pThis);
        pThis->pendingForRTPEncodeVideos_.push_back(info);
        pThis->step();
      });

      return info.promise_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaDevicesDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::onMediaDevicesChanged()
    {
      ZS_EVENTING_1(x, i, Trace, MediaEngineOnMediaDevicesChanged, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      refreshDevicesList_ = true;
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, MediaEngineOnWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::onTimer(ITimerPtr timer)
    {
      ZS_EVENTING_2(x, i, Trace, MediaEngineOnTimer, ol, MediaEngine, Event, puid, id, id_, puid, timerId, timer->getID());

      AutoRecursiveLock lock(*this);
      // NOTE: ADD IF NEEDED...
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Trace, MediaEngineOnPromiseSettled, ol, MediaEngine, Event, puid, id, id_, puid, promiseId, promise->getID());

      AutoRecursiveLock lock(*this);
      auto found = pendingPromises_.find(promise->getID());
      if (found != pendingPromises_.end()) {
        pendingPromises_.erase(found);

        if (promise->isRejected()) {
          setError(promise);
          cancel();
          return;
        }
      }

      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => webrtc::SetupEventTracer
    #pragma mark

#if defined(WINRT)
    //-------------------------------------------------------------------------
    const unsigned char *MediaEngine::GetCategoryGroupEnabled(const char *categoryGroup)
    {
      return reinterpret_cast<const unsigned char*>("webrtc");
    }

    //-------------------------------------------------------------------------
    void __cdecl MediaEngine::AddTraceEvent(
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
      auto engine = MediaEngine::singleton();
      if (engine) {
        engine->internalAddTraceEvent(phase, categoryGroupEnabled, name, id,
          numArgs, argNames, argTypes, argValues, flags);
      }
    }
#endif //defined(WINRT)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => (internal)
    #pragma mark

#if defined(WINRT)
    //-------------------------------------------------------------------------
    void MediaEngine::internalAddTraceEvent(
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
      traceLog_.Add(phase, categoryGroupEnabled, name, id, numArgs, argNames, argTypes, argValues, flags);
    }
#endif //defined(WINRT)

    //-------------------------------------------------------------------------
    void MediaEngine::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_ShuttingDown);

      if (devicesSubscription_) {
        devicesSubscription_->cancel();
        devicesSubscription_.reset();
      }

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
        // perform any graceful asynchronous shutdown processes needed and
        // re-attempt shutdown again later if needed.

        if (!stepPendingPromises()) return;

        if (!stepShutdownPendingRequests()) return;
        if (!stepShutdownDevicesAndRTP()) return;

#define TODO_IMPLEMENT_MEDIA_GRACEFUL_SHUTDOWN 1
#define TODO_IMPLEMENT_MEDIA_GRACEFUL_SHUTDOWN 2
        //        return;
      }

      //.......................................................................
      // final cleanup (hard shutdown)

      setState(State_Shutdown);
      stepShutdownDevicesAndRTP();

#define TODO_IMPLEMENT_MEDIA_HARD_SHUTDOWN 1
#define TODO_IMPLEMENT_MEDIA_HARD_SHUTDOWN 2

      // make sure to cleanup any final reference to self
      gracefulShutdownReference_.reset();
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepShutdownPendingRequests()
    {
      for (auto iter_doNotUse = pendingForMediaDeviceCaptures_.begin(); iter_doNotUse != pendingForMediaDeviceCaptures_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto pending = (*current).promise_;

        pending->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
        pendingForMediaDeviceCaptures_.erase(current);
      }

      for (auto iter_doNotUse = pendingForMediaDeviceRenders_.begin(); iter_doNotUse != pendingForMediaDeviceRenders_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto pending = (*current).promise_;

        pending->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
        pendingForMediaDeviceRenders_.erase(current);
      }

      for (auto iter_doNotUse = pendingForRTPDecodeAudios_.begin(); iter_doNotUse != pendingForRTPDecodeAudios_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto pending = (*current).promise_;

        pending->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
        pendingForRTPDecodeAudios_.erase(current);
      }

      for (auto iter_doNotUse = pendingForRTPDecodeVideos_.begin(); iter_doNotUse != pendingForRTPDecodeVideos_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto pending = (*current).promise_;

        pending->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
        pendingForRTPDecodeVideos_.erase(current);
      }

      for (auto iter_doNotUse = pendingForRTPEncodeAudios_.begin(); iter_doNotUse != pendingForRTPEncodeAudios_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto pending = (*current).promise_;

        pending->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
        pendingForRTPEncodeAudios_.erase(current);
      }

      for (auto iter_doNotUse = pendingForRTPEncodeVideos_.begin(); iter_doNotUse != pendingForRTPEncodeVideos_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto pending = (*current).promise_;

        pending->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
        pendingForRTPEncodeVideos_.erase(current);
      }

      if (pendingForMediaDeviceCaptures_.size() > 0) return false;
      if (pendingForMediaDeviceRenders_.size() > 0) return false;
      if (pendingForRTPDecodeAudios_.size() > 0) return false;
      if (pendingForRTPDecodeVideos_.size() > 0) return false;
      if (pendingForRTPEncodeAudios_.size() > 0) return false;
      if (pendingForRTPEncodeVideos_.size() > 0) return false;

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepShutdownDevicesAndRTP()
    {
      for (auto iter_doNotUse = captureDevices_.begin(); iter_doNotUse != captureDevices_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto device = (*current).second;

        device->shutdown();
        if (!device->isShutdown()) continue;

        captureDevices_.erase(current);
      }

      for (auto iter_doNotUse = renderDevices_.begin(); iter_doNotUse != renderDevices_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto device = (*current).second;

        device->shutdown();
        if (!device->isShutdown()) continue;

        renderDevices_.erase(current);
      }

      for (auto iter_doNotUse = rtpEncoders_.begin(); iter_doNotUse != rtpEncoders_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto encoder = (*current).second;

        encoder->shutdown();
        if (!encoder->isShutdown()) continue;

        rtpEncoders_.erase(current);
      }

      for (auto iter_doNotUse = rtpDecoders_.begin(); iter_doNotUse != rtpDecoders_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto decoder = (*current).second;

        decoder->shutdown();
        if (!decoder->isShutdown()) continue;

        rtpDecoders_.erase(current);
      }

      if (captureDevices_.size() > 0) return false;
      if (renderDevices_.size() > 0) return false;

      if (rtpEncoders_.size() > 0) return false;
      if (rtpDecoders_.size() > 0) return false;

      return true;
    }

    //-------------------------------------------------------------------------
    void MediaEngine::step()
    {
      ZS_EVENTING_1(x, i, Trace, MediaEngineStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, MediaEngineStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      if (!stepPendingPromises()) return;
      if (!stepRefreshDeviceList()) goto not_ready;

      if ((isPending()) ||
          (isReady()) ||
          (isSleeping())) {
        stepFixState();
      }

      if (isGoingToSleep()) {
        if (!stepGoingToSleep1()) goto not_ready;
      }

      if (isSleeping()) {
        goto sleeping;
      }

      if (isWakingUp()) {
        if (!stepWakeUp1()) goto not_ready;
      }

      if (!stepReady1()) return;

      if (!stepShutdownDevices()) return;
      if (!stepShutdownRTP()) return;
      if (!stepStartPendingDevices()) return;
      if (!stepStartPendingRTP()) return;

      goto ready;

    not_ready:
      {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    sleeping:
      {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "sleeping");
        setState(State_Sleeping);
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
        setState(State_Ready);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepPendingPromises()
    {
      if (pendingPromises_.size() > 0) {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "promises are still pending");
        return false;
      }

      ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no promises are pending");
      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepRefreshDeviceList()
    {
      if (!devicesSubscription_) {
        devicesSubscription_ = UseMediaDevices::subscribe(thisWeak_.lock());
      }

      if ((!refreshDevicesList_) &&
          (!devicesListPromise_)) {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "refresh device list skipped (no change notified)");
        return true;
      }

      if (devicesListPromise_) {
        if (!devicesListPromise_->isSettled()) {
          ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "refresh device list still pending");
          return true;
        }

        if (devicesListPromise_->isRejected()) {
          devicesListPromise_.reset();
          ZS_EVENTING_2(x, w, Debug, MediaEngineStepWarningMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "refresh device list failed");
          return true;
        }

        auto deviceList = devicesListPromise_->value();
        mediaDevices_ = deviceList;
      }

      if (!refreshDevicesList_) {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "refresh device list completed");
        return true;
      }

      ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "refresh device list");

      refreshDevicesList_ = false;

      devicesListPromise_ = UseMediaDevices::enumerateDevices();
      if (!devicesListPromise_) {
        ZS_EVENTING_2(x, w, Debug, MediaEngineStepWarningMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "failed to enumerate devices");
        return true;
      }

      devicesListPromise_->thenWeak(thisWeak_.lock());
      return true;
    }

    //-------------------------------------------------------------------------
    void MediaEngine::stepFixState()
    {
      ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "fix state");

      {
        if (pendingForMediaDeviceCaptures_.size() > 0) goto need_media;
        if (pendingForMediaDeviceRenders_.size() > 0) goto need_media;
        if (pendingForRTPDecodeAudios_.size() > 0) goto need_media;
        if (pendingForRTPDecodeVideos_.size() > 0) goto need_media;
        if (pendingForRTPEncodeAudios_.size() > 0) goto need_media;
        if (pendingForRTPEncodeVideos_.size() > 0) goto need_media;

        if (captureDevices_.size() > 0) goto need_media;
        if (renderDevices_.size() > 0) goto need_media;

        if (rtpEncoders_.size() > 0) goto need_media;
        if (rtpDecoders_.size() > 0) goto need_media;

        goto sleep_now;
      }

    need_media:
      {
        setState(State_WakingUp);
        return;
      }

    sleep_now:
      {
        setState(State_GoingToSleep);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepWakeUp1()
    {
      if (completedWakeUp1_) {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "wake up 1 already completed");
        return true;
      }

      ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "wake up 1");

      performAsync(std::bind(&MediaEngine::onStepWakeUp1, this, std::placeholders::_1));
      return false;
    }

    //-------------------------------------------------------------------------
    void MediaEngine::onStepWakeUp1(PromisePtr promise)
    {
      // NOT in a lock

      completedWakeUp1_ = true;
      promise->resolve();
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepGoingToSleep1()
    {
      if (completedGoingToSleep1_) {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "going to sleep 1 already completed");
        return false;
      }

      ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "going to sleep 1");

      performAsync(std::bind(&MediaEngine::onStepGoingToSleep1, this, std::placeholders::_1));
      return true;
    }

    //-------------------------------------------------------------------------
    void MediaEngine::onStepGoingToSleep1(PromisePtr promise)
    {
      // NOT in a lock
      completedGoingToSleep1_ = true;
      promise->resolve();
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepShutdownDevices()
    {
      if (!((checkForIdleDevices_) || (shuttingDownIdleDevices_))) {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no devices have reported being idle and no devices are in the middle of shutting down");
        return true;
      }

      checkForIdleDevices_ = false;
      shuttingDownIdleDevices_ = false;

      for (auto iter_doNotUse = captureDevices_.begin(); iter_doNotUse != captureDevices_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto &deviceID = (*current).first;
        auto &device = (*current).second;
        if (device->isShutdown()) {
          ZS_EVENTING_3(x, i, Debug, MediaEngineStepShutdownDevice, ol, MediaEngine, Step, puid, id, id_, string, deviceId, deviceID, string, message, "capture device is reported shutdown");
          captureDevices_.erase(current);
          continue;
        }

        if (device->isDeviceIdle()) {
          ZS_EVENTING_3(x, i, Debug, MediaEngineStepShutdownDevice, ol, MediaEngine, Step, puid, id, id_, string, deviceId, deviceID, string, message, "capture device is idle (requesting shutdown)");
          device->shutdown();
          shuttingDownIdleDevices_ = true;
        }
      }

      for (auto iter_doNotUse = renderDevices_.begin(); iter_doNotUse != renderDevices_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto &deviceID = (*current).first;
        auto &device = (*current).second;
        if (device->isShutdown()) {
          ZS_EVENTING_3(x, i, Debug, MediaEngineStepShutdownDevice, ol, MediaEngine, Step, puid, id, id_, string, deviceId, deviceID, string, message, "render device is reported shutdown");
          renderDevices_.erase(current);
          continue;
        }

        if (device->isDeviceIdle()) {
          ZS_EVENTING_3(x, i, Debug, MediaEngineStepShutdownDevice, ol, MediaEngine, Step, puid, id, id_, string, deviceId, deviceID, string, message, "render device is idle (requesting shutdown)");
          device->shutdown();
          shuttingDownIdleDevices_ = true;
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepShutdownRTP()
    {
      if (!checkForShutdownRTP_) {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no rtp notified being shutting down");
        return true;
      }

      checkForShutdownRTP_ = false;

      for (auto iter_doNotUse = rtpDecoders_.begin(); iter_doNotUse != rtpDecoders_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto decoder = (*current).second;

        if (!decoder->isShutdown()) continue;
        rtpDecoders_.erase(current);
      }

      for (auto iter_doNotUse = rtpEncoders_.begin(); iter_doNotUse != rtpEncoders_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto encoder = (*current).second;

        if (!encoder->isShutdown()) continue;
        rtpEncoders_.erase(current);
      }

      ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "rtp shutting down");
      return true;
    }

    //-------------------------------------------------------------------------
    static void getExactDeviceIDs(
                                  const MediaEngine::TrackConstraints &constraints,
                                  MediaEngine::StringList &outValues
                                  )
    {
      constraints.mDeviceID.exact(outValues);
      for (auto iter = constraints.mAdvanced.begin(); iter != constraints.mAdvanced.end(); ++iter) {
        (*iter)->mDeviceID.exact(outValues);
      }
    }

    //-------------------------------------------------------------------------
    static void getIdealDeviceIDs(
                                  const MediaEngine::TrackConstraints &constraints,
                                  MediaEngine::StringList &outValues
                                  )
    {
      constraints.mDeviceID.ideal(outValues);
      for (auto iter = constraints.mAdvanced.begin(); iter != constraints.mAdvanced.end(); ++iter) {
        (*iter)->mDeviceID.ideal(outValues);
      }
    }

    //-------------------------------------------------------------------------
    static void getExactGroupIDs(
                                  const MediaEngine::TrackConstraints &constraints,
                                  MediaEngine::StringList &outValues
                                  )
    {
      constraints.mGroupID.exact(outValues);
      for (auto iter = constraints.mAdvanced.begin(); iter != constraints.mAdvanced.end(); ++iter) {
        (*iter)->mGroupID.exact(outValues);
      }
    }

    //-------------------------------------------------------------------------
    static void getIdealGroupIDs(
                                  const MediaEngine::TrackConstraints &constraints,
                                  MediaEngine::StringList &outValues
                                  )
    {
      constraints.mGroupID.ideal(outValues);
      for (auto iter = constraints.mAdvanced.begin(); iter != constraints.mAdvanced.end(); ++iter) {
        (*iter)->mGroupID.ideal(outValues);
      }
    }

    //-------------------------------------------------------------------------
    static bool doesListContain(const MediaEngine::StringList &values, const String &value)
    {
      for (auto iter = values.begin(); iter != values.end(); ++iter) {
        if (value == (*iter)) return true;
      }
      return false;
    }

    //-------------------------------------------------------------------------
    static bool findDevice(
                           bool capture,
                           MediaEngine::Kinds kind,
                           const MediaEngine::DeviceList &devices,
                           const MediaEngine::TrackConstraints &constraints,
                           String &outUseDeviceID,
                           String &outUseGroupID
                           )
    {
      MediaEngine::StringList exactDeviceIDs;
      MediaEngine::StringList idealDeviceIDs;

      MediaEngine::StringList exactGroupIDs;
      MediaEngine::StringList idealGroupIDs;

      getExactDeviceIDs(constraints, exactDeviceIDs);
      getIdealDeviceIDs(constraints, idealDeviceIDs);

      getExactGroupIDs(constraints, exactGroupIDs);
      getIdealGroupIDs(constraints, idealGroupIDs);

      for (auto iterDeviceList = devices.begin(); iterDeviceList != devices.end(); ++iterDeviceList) {
        auto &device = (*iterDeviceList);

        if (capture) {
          switch (kind) {
            case IMediaStreamTrackTypes::Kind_Audio: if (IMediaDevicesTypes::DeviceKind_AudioInput != device.mKind) continue;
            case IMediaStreamTrackTypes::Kind_Video: if (IMediaDevicesTypes::DeviceKind_VideoInput != device.mKind) continue;
          }
        }
        if (exactDeviceIDs.size() > 0) {
          if (!doesListContain(exactDeviceIDs, device.mDeviceID)) continue;
          if (exactGroupIDs.size() > 0) {
            if (!doesListContain(exactGroupIDs, device.mGroupID)) continue;
          }
          outUseDeviceID = device.mDeviceID;
          outUseGroupID = device.mGroupID;
          return true;
        }

        if (exactGroupIDs.size() > 0) {
          if (!doesListContain(exactGroupIDs, device.mGroupID)) continue;
          outUseDeviceID = device.mDeviceID;
          outUseGroupID = device.mGroupID;
          return true;
        }

        if (outUseDeviceID.hasData()) continue;

        if (idealDeviceIDs.size() > 0) {
          if (!doesListContain(idealDeviceIDs, device.mDeviceID)) continue;
          if (idealGroupIDs.size() > 0) {
            if (!doesListContain(idealGroupIDs, device.mGroupID)) continue;
          }
          outUseDeviceID = device.mDeviceID;
          outUseGroupID = device.mGroupID;
          continue;
        }

        if (idealGroupIDs.size() > 0) {
          if (!doesListContain(idealGroupIDs, device.mGroupID)) continue;
          outUseDeviceID = device.mDeviceID;
          outUseGroupID = device.mGroupID;
          continue;
        }

        outUseDeviceID = device.mDeviceID;
        outUseGroupID = device.mGroupID;
      }
      return outUseDeviceID.hasData();
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepStartPendingDevices()
    {
      if ((checkForIdleDevices_) || (shuttingDownIdleDevices_) || (devicesListPromise_)) {
        ZS_EVENTING_2(x, i, Trace, MediaEngineStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "start pending devices step skipped (cannot start devices while device list is being obtained or when shutting down existing devices)");
        return true;
      }

      for (auto iter_doNotUse = pendingForMediaDeviceCaptures_.begin(); iter_doNotUse != pendingForMediaDeviceCaptures_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto &info = (*current);

        if (!mediaDevices_) {
          info.promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_NoContent, "no media devices were found connected"));
          pendingForMediaDeviceCaptures_.erase(current);
          continue;
        }

        String useDeviceID;
        String useGroupID;

        if (!findDevice(true, info.kind_, *mediaDevices_, *info.constraints_, useDeviceID, useGroupID)) {
          info.promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_NotFound, "media capture device was not found"));
          pendingForMediaDeviceCaptures_.erase(current);
          continue;
        }

        UseMediaDeviceCapturePtr useDevice;

        auto found = captureDevices_.find(useDeviceID);
        if (found == captureDevices_.end()) {
          switch (info.kind_) {
            case IMediaStreamTrackTypes::Kind_Audio: useDevice = UseMediaDeviceCaptureAudio::create(thisWeak_.lock(), useDeviceID); break;
            case IMediaStreamTrackTypes::Kind_Video: useDevice = UseMediaDeviceCaptureVideo::create(thisWeak_.lock(), useDeviceID); break;
          }
          captureDevices_[useDeviceID] = useDevice;
        } else {
          useDevice = (*found).second;
        }

        useDevice->mediaDeviceCaptureSubscribe(info.promise_, info.repaceExistingDeviceObjectID_, info.constraints_, info.delegate_);
        pendingForMediaDeviceCaptures_.erase(current);
      }

      for (auto iter_doNotUse = pendingForMediaDeviceRenders_.begin(); iter_doNotUse != pendingForMediaDeviceRenders_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto &info = (*current);

        if (!mediaDevices_) {
          info.promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_NoContent, "no media devices were found connected"));
          pendingForMediaDeviceRenders_.erase(current);
          continue;
        }

        String useDeviceID;
        String useGroupID;

        if (!findDevice(false, IMediaStreamTrackTypes::Kind_Audio, *mediaDevices_, *info.constraints_, useDeviceID, useGroupID)) {
          info.promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_NotFound, "media render device was not found"));
          pendingForMediaDeviceRenders_.erase(current);
          continue;
        }

        UseMediaDeviceRenderPtr useDevice;

        auto found = renderDevices_.find(useDeviceID);
        if (found == renderDevices_.end()) {
          useDevice = UseMediaDeviceRenderAudio::create(thisWeak_.lock(), useDeviceID);
          renderDevices_[useDeviceID] = useDevice;
        } else {
          useDevice = (*found).second;
        }

        useDevice->mediaDeviceRenderSubscribe(info.promise_, info.repaceExistingDeviceObjectID_, info.constraints_, info.delegate_);
        pendingForMediaDeviceRenders_.erase(current);
      }
        
      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepReady1()
    {
      //...............................

      bool shouldDoThis = false;
      if (shouldDoThis) {
        performAsync(std::bind(&MediaEngine::onReady1, this, std::placeholders::_1));
      }

      //...............................

      bool shouldDoThisOtherThing = false;
      if (shouldDoThisOtherThing) {
        auto pThis = thisWeak_.lock();
        performAsyncClosure([pThis] { /* pThis->doSomethingThatCantFail(); */ });
      }

      //...............................

      bool shouldDoSomethingElseToo = false;
      if (shouldDoSomethingElseToo) {

        struct SomeResult : public Any
        {
          int someResult_ {};
        };

        auto pThis = thisWeak_.lock();
        auto promise = Promise::create(getAssociatedMessageQueue());

        /* perform some step that should not be in a lock */
        performAsyncClosure(promise, [pThis, promise] {
          /* not in a lock */

          bool didFail = false;
          if (didFail)
          {
            promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "I didn't like this value"));
          }

          auto result = make_shared<SomeResult>();
          result->someResult_ = -1;
          promise->resolve(result);
        });

        /* complete the step here */
        promise->thenClosure([promise, pThis] {
          /* because this is locked no other step will be performed at this time */
          AutoRecursiveLock lock(*pThis);

          if (pThis->isShuttingDown() || pThis->isShutdown()) {
            ZS_EVENTING_2(x, w, Debug, MediaEngineStepWarningMessage, ol, MediaEngine, Step, puid, id, pThis->id_, string, message, "will not perform this additional step if shutting down");
            return;
          }

          /* ... continue here ... */

          if (promise->isRejected()) {
            pThis->setError(promise);
            pThis->cancel();
            return;
          }

          auto result = promise->value<SomeResult>();
          if (!result) {
          }
        });

      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::stepStartPendingRTP()
    {
      {
        for (auto iter = pendingForRTPDecodeAudios_.begin(); iter != pendingForRTPDecodeAudios_.end(); ++iter)
        {
          auto &info = (*iter);

          auto result = UseRTPDecoderAudio::create(info.promise_, thisWeak_.lock(), info.parameters_, info.delegate_);
          rtpDecoders_[result->getID()] = result;
        }
        pendingForRTPDecodeAudios_.clear();
      }
      {
        for (auto iter = pendingForRTPDecodeVideos_.begin(); iter != pendingForRTPDecodeVideos_.end(); ++iter)
        {
          auto &info = (*iter);

          auto result = UseRTPDecoderVideo::create(info.promise_, thisWeak_.lock(), info.parameters_, info.delegate_);
          rtpDecoders_[result->getID()] = result;
        }
        pendingForRTPDecodeVideos_.clear();
      }
      {
        for (auto iter = pendingForRTPEncodeAudios_.begin(); iter != pendingForRTPEncodeAudios_.end(); ++iter)
        {
          auto &info = (*iter);

          auto result = UseRTPEncoderAudio::create(info.promise_, thisWeak_.lock(), info.parameters_, info.delegate_);
          rtpEncoders_[result->getID()] = result;
        }
        pendingForRTPEncodeAudios_.clear();
      }
      {
        for (auto iter = pendingForRTPEncodeVideos_.begin(); iter != pendingForRTPEncodeVideos_.end(); ++iter)
        {
          auto &info = (*iter);

          auto result = UseRTPEncoderVideo::create(info.promise_, thisWeak_.lock(), info.parameters_, info.delegate_);
          rtpEncoders_[result->getID()] = result;
        }
        pendingForRTPEncodeVideos_.clear();
      }
      return true;
    }

    //-------------------------------------------------------------------------
    void MediaEngine::onReady1(PromisePtr promise)
    {
      // NOT in a lock
      promise->resolve();
    }

    //-------------------------------------------------------------------------
    void MediaEngine::performAsync(std::function<void(PromisePtr)> func)
    {
      auto pThis = thisWeak_.lock();

      auto promise = Promise::create(getAssociatedMessageQueue());
      promise->thenWeak(pThis);

      pendingPromises_[promise->getID()] = promise;
      postClosure([func, promise] { func(promise); });
    }

    //-------------------------------------------------------------------------
    void MediaEngine::setState(States state)
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaEngineSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, toString(state), string, oldState, toString(currentState_));

      currentState_ = state;

      // reset wakup states
      completedWakeUp1_ = false;

      // reset going to sleep states
      completedGoingToSleep1_ = false;

//      MediaEnginePtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onMediaEngineStateChanged(pThis, currentState_);
//      }
    }

    //-------------------------------------------------------------------------
    void MediaEngine::setError(PromisePtr promise)
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void MediaEngine::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, MediaEngineSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, MediaEngineSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::WebRtcTraceCallback
    #pragma mark

    //-------------------------------------------------------------------------
    /*void MediaEngine::WebRtcTraceCallback::Print(webrtc::TraceLevel trace, const char* message, int length)
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
    }*/

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::WebRtcTraceCallback
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::WebRtcLogSink::OnLogMessage(const std::string& message)
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
    #pragma mark IMediaEngineFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaEngineFactory &IMediaEngineFactory::singleton()
    {
      return MediaEngineFactory::singleton();
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr IMediaEngineFactory::create()
    {
      if (this) {}
      return internal::MediaEngine::create();
    }

    //-------------------------------------------------------------------------
    void IMediaEngineFactory::setLogLevel(Log::Level level)
    {
      if (this) {}
      internal::MediaEngine::setLogLevel(level);
    }

    //-------------------------------------------------------------------------
    void IMediaEngineFactory::ntpServerTime(const Milliseconds &value)
    {
      if (this) {}
      internal::MediaEngine::ntpServerTime(value);
    }

    //-------------------------------------------------------------------------
    void IMediaEngineFactory::startMediaTracing()
    {
      if (this) {}
      return internal::MediaEngine::startMediaTracing();
    }

    //-------------------------------------------------------------------------
    void IMediaEngineFactory::stopMediaTracing()
    {
      if (this) {}
      internal::MediaEngine::stopMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool IMediaEngineFactory::isMediaTracing()
    {
      if (this) {}
      return internal::MediaEngine::isMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool IMediaEngineFactory::saveMediaTrace(String filename)
    {
      if (this) {}
      return internal::MediaEngine::saveMediaTrace(filename);
    }

    //-------------------------------------------------------------------------
    bool IMediaEngineFactory::saveMediaTrace(String host, int port)
    {
      if (this) {}
      return internal::MediaEngine::saveMediaTrace(host, port);
    }

    //-------------------------------------------------------------------------
    IMediaEngineFactory::MediaDeviceCapturePromisePtr IMediaEngineFactory::createMediaDeviceCapture(
                                                                                                    MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                                                    Kinds kind,
                                                                                                    const TrackConstraints &constraints,
                                                                                                    IMediaDeviceCaptureDelegatePtr delegate
                                                                                                    )
    {
      if (this) {}
      return internal::MediaEngine::createMediaDeviceCapture(repaceExistingDeviceObjectID, kind, constraints, delegate);
    }

    //-------------------------------------------------------------------------
    IMediaEngineFactory::MediaDeviceRenderPromisePtr IMediaEngineFactory::createMediaDeviceRender(
                                                                                                  MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                                                  const TrackConstraints &constraints,
                                                                                                  IMediaDeviceRenderDelegatePtr delegate
                                                                                                  )
    {
      if (this) {}
      return internal::MediaEngine::createMediaDeviceRender(repaceExistingDeviceObjectID, constraints, delegate);
    }

    //-------------------------------------------------------------------------
    IMediaEngineFactory::PromiseWithRTPDecoderAudioPtr IMediaEngineFactory::createRTPDecoderAudio(
                                                                                                  const Parameters &parameters,
                                                                                                  IRTPDecoderDelegatePtr delegate
                                                                                                  )
    {
      if (this) {}
      return internal::MediaEngine::createRTPDecoderAudio(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    IMediaEngineFactory::PromiseWithRTPDecoderVideoPtr IMediaEngineFactory::createRTPDecoderVideo(
                                                                                                  const Parameters &parameters,
                                                                                                  IRTPDecoderDelegatePtr delegate
                                                                                                  )
    {
      if (this) {}
      return internal::MediaEngine::createRTPDecoderVideo(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    IMediaEngineFactory::PromiseWithRTPEncoderAudioPtr IMediaEngineFactory::createRTPEncoderAudio(
                                                                                                  const Parameters &parameters,
                                                                                                  IRTPEncoderDelegatePtr delegate
                                                                                                  )
    {
      if (this) {}
      return internal::MediaEngine::createRTPEncoderAudio(parameters, delegate);
    }

    //-------------------------------------------------------------------------
    IMediaEngineFactory::PromiseWithRTPEncoderVideoPtr IMediaEngineFactory::createRTPEncoderVideo(
                                                                                                  const Parameters &parameters,
                                                                                                  IRTPEncoderDelegatePtr delegate
                                                                                                  )
    {
      if (this) {}
      return internal::MediaEngine::createRTPEncoderVideo(parameters, delegate);
    }


  } // internal namespace
}
