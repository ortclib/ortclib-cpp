/*

 Copyright (c) 2017, Optical Tone Ltd.
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

#pragma once

#include <ortc/internal/types.h>

#include <ortc/internal/ortc_IMediaDeviceCapture.h>
#include <ortc/internal/ortc_IMediaDeviceRender.h>
#include <ortc/internal/ortc_IRTPEncoder.h>
#include <ortc/internal/ortc_IRTPDecoder.h>
#include <ortc/internal/ortc_ORTC.h>

#include <ortc/IMediaDevices.h>
#include <ortc/IMediaStreamTrack.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/Promise.h>
#include <zsLib/IWakeDelegate.h>
#include <zsLib/ITimer.h>

#include <ortc/internal/webrtc_pre_include.h>
#include <webrtc/rtc_base/logging.h>
#include <webrtc/system_wrappers/include/trace.h>
#include <ortc/internal/webrtc_post_include.h>
 //#include <webrtc/rtc_base/tracelog.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineFactory);

    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDeviceCapture);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDeviceCaptureAudio);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDeviceCaptureVideo);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDeviceRender);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDeviceRenderAudio);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForRTPSenderChannelAudio);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForRTPSenderChannelVideo);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForRTPReceiverChannelAudio);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForRTPReceiverChannelVideo);

    ZS_DECLARE_INTERACTION_PTR(IMediaDeviceCaptureAudioForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IMediaDeviceCaptureVideoForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IMediaDeviceRenderAudioForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IRTPEncoderForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IRTPEncoderAudioForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IRTPEncoderVideoForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IRTPDecoderForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IRTPDecoderAudioForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IRTPDecoderVideoForMediaEngine);

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //
    // IMediaEngineForORTC
    //

    interaction IMediaEngineForORTC
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForORTC, ForORTC);
    
      static void setLogLevel(Log::Level level) noexcept;
      static void ntpServerTime(const Milliseconds &value) noexcept;
      static void startMediaTracing() noexcept;
      static void stopMediaTracing() noexcept;
      static bool isMediaTracing() noexcept;
      static bool saveMediaTrace(String filename) noexcept;
      static bool saveMediaTrace(String host, int port) noexcept;

      virtual ~IMediaEngineForORTC() noexcept {}
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForMediaDevice
    //

    interaction IMediaEngineForMediaDevice
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDevice, ForDevice);

      virtual void notifyDeviceIsIdleOrShutdownStateChanged() noexcept = 0;

      virtual ~IMediaEngineForMediaDevice() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForMediaDeviceCapture
    //

    interaction IMediaEngineForMediaDeviceCapture : public IMediaEngineForMediaDevice
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::MediaDeviceObjectID, MediaDeviceObjectID);
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceCapture, ForMediaDeviceCapture);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);

      typedef zsLib::PromiseWithHolderPtr<IMediaDeviceCapturePtr> MediaDeviceCapturePromise;
      ZS_DECLARE_PTR(MediaDeviceCapturePromise);

      static MediaDeviceCapturePromisePtr createMediaDeviceCapture(
                                                                   MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                   Kinds kind,
                                                                   const TrackConstraints &constraints,
                                                                   IMediaDeviceCaptureDelegatePtr delegate
                                                                   ) noexcept;

      virtual ~IMediaEngineForMediaDeviceCapture() noexcept {}
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForMediaDeviceCaptureAudio
    //

    interaction IMediaEngineForMediaDeviceCaptureAudio : public IMediaEngineForMediaDeviceCapture
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceCaptureAudio, ForMediaDeviceCaptureAudio);

      virtual ~IMediaEngineForMediaDeviceCaptureAudio() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForMediaDeviceCaptureVideo
    //

    interaction IMediaEngineForMediaDeviceCaptureVideo : public IMediaEngineForMediaDeviceCapture
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceCaptureVideo, ForMediaDeviceCaptureVideo);

      virtual ~IMediaEngineForMediaDeviceCaptureVideo() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForMediaDeviceRender
    //

    interaction IMediaEngineForMediaDeviceRender : public IMediaEngineForMediaDevice
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceRender, ForMediaDeviceRender);

      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::MediaDeviceObjectID, MediaDeviceObjectID);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);

      typedef zsLib::PromiseWithHolderPtr<IMediaDeviceRenderPtr> MediaDeviceRenderPromise;
      ZS_DECLARE_PTR(MediaDeviceRenderPromise);

      static MediaDeviceRenderPromisePtr createMediaDeviceRender(
                                                                 MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                 const TrackConstraints &constraints,
                                                                 IMediaDeviceRenderDelegatePtr delegate
                                                                 ) noexcept;

      virtual ~IMediaEngineForMediaDeviceRender() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForMediaDeviceRenderAudio : public IMediaEngineForMediaDeviceRender
    //

    interaction IMediaEngineForMediaDeviceRenderAudio : public IMediaEngineForMediaDeviceRender
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceRenderAudio, ForMediaDeviceRenderAudio);

      virtual ~IMediaEngineForMediaDeviceRenderAudio() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForRTP
    //

    interaction IMediaEngineForRTP
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTP, ForRTP);

    virtual void notifyRTPShutdownStateChanged() noexcept = 0;

      virtual ~IMediaEngineForRTP() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForRTPDecoder
    //

    interaction IMediaEngineForRTPDecoder : public IMediaEngineForRTP
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoder, ForRTPDecode);

      virtual ~IMediaEngineForRTPDecoder() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForRTPDecoderAudio
    //

    interaction IMediaEngineForRTPDecoderAudio : public IMediaEngineForRTPDecoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderAudio, ForRTPDecodeAudio);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderAudioPtr>, PromiseWithRTPDecoderAudio);

      static PromiseWithRTPDecoderAudioPtr createRTPDecoderAudio(
                                                                 const Parameters &parameters,
                                                                 IRTPDecoderDelegatePtr delegate
                                                                 ) noexcept;

      virtual ~IMediaEngineForRTPDecoderAudio() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForRTPDecoderVideo
    //

    interaction IMediaEngineForRTPDecoderVideo : public IMediaEngineForRTPDecoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderVideo, ForRTPDecodeVideo);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderVideoPtr>, PromiseWithRTPDecoderVideo);

      static PromiseWithRTPDecoderVideoPtr createRTPDecoderVideo(
                                                                 const Parameters &parameters,
                                                                 IRTPDecoderDelegatePtr delegate
                                                                 ) noexcept;

      virtual ~IMediaEngineForRTPDecoderVideo() noexcept {}
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForRTPEncoder
    //

    interaction IMediaEngineForRTPEncoder : public IMediaEngineForRTP
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoder, ForRTPEncode);

      virtual ~IMediaEngineForRTPEncoder() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForRTPEncoderAudio
    //

    interaction IMediaEngineForRTPEncoderAudio : public IMediaEngineForRTPEncoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderAudio, ForRTPEncodeAudio);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderAudioPtr>, PromiseWithRTPEncoderAudio);

      static PromiseWithRTPEncoderAudioPtr createRTPEncoderAudio(
                                                                 const Parameters &parameters,
                                                                 IRTPEncoderDelegatePtr delegate
                                                                 ) noexcept;

      virtual ~IMediaEngineForRTPEncoderAudio() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineForRTPEncoderVideo
    //

    interaction IMediaEngineForRTPEncoderVideo : public IMediaEngineForRTPEncoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderVideo, ForRTPEncodeVideo);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderVideoPtr>, PromiseWithRTPEncoderVideo);

      static PromiseWithRTPEncoderVideoPtr createRTPEncoderVideo(
                                                                 const Parameters &parameters,
                                                                 IRTPEncoderDelegatePtr delegate
                                                                 ) noexcept;

      virtual ~IMediaEngineForRTPEncoderVideo() noexcept {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaEngine
    //

    class MediaEngine : public Any,
                        public Noop,
                        public MessageQueueAssociator,
                        public SharedRecursiveLock,
                        public IMediaEngineForORTC,
                        public IMediaEngineForMediaDeviceCaptureAudio,
                        public IMediaEngineForMediaDeviceCaptureVideo,
                        public IMediaEngineForMediaDeviceRenderAudio,
                        public IMediaEngineForRTPDecoderAudio,
                        public IMediaEngineForRTPDecoderVideo,
                        public IMediaEngineForRTPEncoderAudio,
                        public IMediaEngineForRTPEncoderVideo,
                        public IMediaDevicesDelegate,
                        public zsLib::IWakeDelegate,
                        public zsLib::ITimerDelegate,
                        public zsLib::IPromiseSettledDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaEngineFactory;
      friend interaction IMediaEngineForORTC;
      friend interaction IMediaEngineForMediaDeviceCapture;
      friend interaction IMediaEngineForMediaDeviceCaptureAudio;
      friend interaction IMediaEngineForMediaDeviceCaptureVideo;
      friend interaction IMediaEngineForMediaDeviceRender;
      friend interaction IMediaEngineForMediaDeviceRenderAudio;
      friend interaction IMediaEngineForRTPSenderChannelAudio;
      friend interaction IMediaEngineForRTPSenderChannelVideo;
      friend interaction IMediaEngineForRTPReceiverChannelAudio;
      friend interaction IMediaEngineForRTPReceiverChannelVideo;
      friend interaction IMediaEngineForRTPDecoder;
      friend interaction IMediaEngineForRTPDecoderAudio;
      friend interaction IMediaEngineForRTPDecoderVideo;
      friend interaction IMediaEngineForRTPEncoder;
      friend interaction IMediaEngineForRTPEncoderAudio;
      friend interaction IMediaEngineForRTPEncoderVideo;

      enum States
      {
        State_First,

        State_Pending = State_First,
        State_WakingUp,
        State_Ready,
        State_GoingToSleep,
        State_Sleeping,
        State_ShuttingDown,
        State_Shutdown,

        State_Last = State_Shutdown,
      };

      static const char *toString(States state) noexcept;

      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::MediaDeviceObjectID, MediaDeviceObjectID);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithDeviceList, PromiseWithDeviceList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::DeviceList, DeviceList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<String>, StringList);

      struct PendingForMediaDeviceCapture
      {
        MediaDeviceObjectID repaceExistingDeviceObjectID_ {};
        Kinds kind_ { IMediaStreamTrackTypes::Kind_Audio};
        MediaDeviceCapturePromisePtr promise_;
        TrackConstraintsPtr constraints_;
        IMediaDeviceCaptureDelegatePtr delegate_;
      };

      struct PendingForMediaDeviceRenderInfo
      {
        MediaDeviceObjectID repaceExistingDeviceObjectID_{};
        MediaDeviceRenderPromisePtr promise_;
        TrackConstraintsPtr constraints_;
        IMediaDeviceRenderDelegatePtr delegate_;
      };

      ZS_DECLARE_TYPEDEF_PTR(std::list<PendingForMediaDeviceCapture>, PendingForMediaDeviceCaptureList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PendingForMediaDeviceRenderInfo>, PendingForMediaDeviceRenderList);

      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderAudioPtr>, PromiseWithRTPEncoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderVideoPtr>, PromiseWithRTPEncoderVideo);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderAudioPtr>, PromiseWithRTPDecoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderVideoPtr>, PromiseWithRTPDecoderVideo);

      struct PendingForRTPDecodeAudio
      {
        PromiseWithRTPDecoderAudioPtr promise_;
        ParametersPtr parameters_;
        IRTPDecoderDelegatePtr delegate_;
      };

      struct PendingForRTPDecodeVideo
      {
        PromiseWithRTPDecoderVideoPtr promise_;
        ParametersPtr parameters_;
        IRTPDecoderDelegatePtr delegate_;
      };

      struct PendingForRTPEncodeAudio
      {
        PromiseWithRTPEncoderAudioPtr promise_;
        ParametersPtr parameters_;
        IRTPEncoderDelegatePtr delegate_;
      };

      struct PendingForRTPEncodeVideo
      {
        PromiseWithRTPEncoderVideoPtr promise_;
        ParametersPtr parameters_;
        IRTPEncoderDelegatePtr delegate_;
      };

      ZS_DECLARE_TYPEDEF_PTR(std::list<PendingForRTPDecodeAudio>, PendingForRTPDecodeAudioList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PendingForRTPDecodeVideo>, PendingForRTPDecodeVideoList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PendingForRTPEncodeAudio>, PendingForRTPEncodeAudioList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PendingForRTPEncodeVideo>, PendingForRTPEncodeVideoList);

      ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevices, UseMediaDevices);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceCaptureForMediaEngine, UseMediaDeviceCapture);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceCaptureAudioForMediaEngine, UseMediaDeviceCaptureAudio);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceCaptureVideoForMediaEngine, UseMediaDeviceCaptureVideo);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceRenderForMediaEngine, UseMediaDeviceRender);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceRenderAudioForMediaEngine, UseMediaDeviceRenderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPForMediaEngine, UseRTP);
      ZS_DECLARE_TYPEDEF_PTR(IRTPEncoderForMediaEngine, UseRTPEncoder);
      ZS_DECLARE_TYPEDEF_PTR(IRTPEncoderAudioForMediaEngine, UseRTPEncoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPEncoderVideoForMediaEngine, UseRTPEncoderVideo);
      ZS_DECLARE_TYPEDEF_PTR(IRTPDecoderForMediaEngine, UseRTPDecoder);
      ZS_DECLARE_TYPEDEF_PTR(IRTPDecoderAudioForMediaEngine, UseRTPDecoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPDecoderVideoForMediaEngine, UseRTPDecoderVideo);

      typedef String DeviceID;
      typedef std::map<DeviceID, UseMediaDeviceCapturePtr> DeviceCaptureMap;
      typedef std::map<DeviceID, UseMediaDeviceRenderPtr> DeviceRenderMap;

      typedef PUID ObjectID;
      typedef std::map<ObjectID, UseRTPDecoderPtr> RTPDecoderMap;
      typedef std::map<ObjectID, UseRTPEncoderPtr> RTPEncoderMap;
      
      typedef PUID PromiseID;
      typedef std::map<PromiseID, PromisePtr> PromiseMap;

    public:
      MediaEngine(
                  const make_private &,
                  IMessageQueuePtr queue
                  ) noexcept;

    protected:
      MediaEngine(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

    public:
      virtual ~MediaEngine() noexcept;

      static MediaEnginePtr singleton() noexcept;
      static MediaEnginePtr create() noexcept;

      static MediaEnginePtr convert(ForORTCPtr object) noexcept;
      static MediaEnginePtr convert(ForMediaDeviceCapturePtr object) noexcept;
      static MediaEnginePtr convert(ForMediaDeviceRenderPtr object) noexcept;
      static MediaEnginePtr convert(ForMediaDeviceRenderAudioPtr object) noexcept;
      static MediaEnginePtr convert(ForMediaDeviceCaptureAudioPtr object) noexcept;
      static MediaEnginePtr convert(ForMediaDeviceCaptureVideoPtr object) noexcept;
      static MediaEnginePtr convert(ForRTPPtr object) noexcept;
      static MediaEnginePtr convert(ForRTPDecodePtr object) noexcept;
      static MediaEnginePtr convert(ForRTPDecodeAudioPtr object) noexcept;
      static MediaEnginePtr convert(ForRTPDecodeVideoPtr object) noexcept;
      static MediaEnginePtr convert(ForRTPEncodePtr object) noexcept;
      static MediaEnginePtr convert(ForRTPEncodeAudioPtr object) noexcept;
      static MediaEnginePtr convert(ForRTPEncodeVideoPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForORTC
      //

      static void setLogLevel(Log::Level level) noexcept                  { auto engine = singleton(); if (!engine) return; engine->internalSetLogLevel(level); }
      static void ntpServerTime(const Milliseconds &value)noexcept        { auto engine = singleton(); if (!engine) return; engine->internalNTPServerTime(value); }
      static void startMediaTracing() noexcept                            { auto engine = singleton(); if (!engine) return; engine->internalStartMediaTracing(); }
      static void stopMediaTracing() noexcept                             { auto engine = singleton(); if (!engine) return; engine->internalStopMediaTracing(); }
      static bool isMediaTracing() noexcept                               { auto engine = singleton(); if (!engine) return false; return engine->internalIsMediaTracing(); }
      static bool saveMediaTrace(String filename) noexcept                { auto engine = singleton(); if (!engine) return false; return engine->internalSaveMediaTrace(filename); }
      static bool saveMediaTrace(String host, int port) noexcept          { auto engine = singleton(); if (!engine) return false; return engine->internalSaveMediaTrace(host, port); }

      virtual void internalSetLogLevel(Log::Level level) noexcept;
      virtual void internalNTPServerTime(const Milliseconds &value) noexcept;
      virtual void internalStartMediaTracing() noexcept;
      virtual void internalStopMediaTracing() noexcept;
      virtual bool internalIsMediaTracing() noexcept;
      virtual bool internalSaveMediaTrace(String filename) noexcept;
      virtual bool internalSaveMediaTrace(String host, int port) noexcept;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForMediaDevice
      //

      void notifyDeviceIsIdleOrShutdownStateChanged() noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForMediaDeviceCapture
      //

      static MediaDeviceCapturePromisePtr createMediaDeviceCapture(
                                                                   MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                   Kinds kind,
                                                                   const TrackConstraints &constraints,
                                                                   IMediaDeviceCaptureDelegatePtr delegate
                                                                   ) noexcept                                            { auto engine = singleton(); if (!engine) return MediaDeviceCapturePromise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateMediaDeviceCapture(repaceExistingDeviceObjectID, kind, constraints, delegate); }

      virtual MediaDeviceCapturePromisePtr innerCreateMediaDeviceCapture(
                                                                         MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                         Kinds kind,
                                                                         const TrackConstraints &constraints,
                                                                         IMediaDeviceCaptureDelegatePtr delegate
                                                                         ) noexcept;


      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForMediaDeviceCaptureAudio
      //

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForMediaDeviceCaptureVideo
      //

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForMediaDeviceRender
      //

      
      static MediaDeviceRenderPromisePtr createMediaDeviceRender(
                                                                 MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                 const TrackConstraints &constraints,
                                                                 IMediaDeviceRenderDelegatePtr delegate
                                                                 ) noexcept                                            { auto engine = singleton(); if (!engine) return MediaDeviceRenderPromise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateMediaDeviceRender(repaceExistingDeviceObjectID, constraints, delegate); }

      virtual MediaDeviceRenderPromisePtr innerCreateMediaDeviceRender(
                                                                       MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                       const TrackConstraints &constraints,
                                                                       IMediaDeviceRenderDelegatePtr delegate
                                                                       ) noexcept;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForMediaDeviceRenderAudio
      //

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForRTP
      //

      void notifyRTPShutdownStateChanged() noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForRTPDecoder
      //

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForRTPDecoderAudio
      //

      static PromiseWithRTPDecoderAudioPtr createRTPDecoderAudio(
                                                                 const Parameters &parameters,
                                                                 IRTPDecoderDelegatePtr delegate
                                                                 ) noexcept                                               { auto engine = singleton(); if (!engine) return PromiseWithRTPDecoderAudio::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateRTPDecoderAudio(parameters, delegate); }

      virtual PromiseWithRTPDecoderAudioPtr innerCreateRTPDecoderAudio(
                                                                       const Parameters &parameters,
                                                                       IRTPDecoderDelegatePtr delegate
                                                                       ) noexcept;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForRTPDecoderVideo
      //

      static PromiseWithRTPDecoderVideoPtr createRTPDecoderVideo(
                                                                 const Parameters &parameters,
                                                                 IRTPDecoderDelegatePtr delegate
                                                                 ) noexcept                                               { auto engine = singleton(); if (!engine) return PromiseWithRTPDecoderVideo::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateRTPDecoderVideo(parameters, delegate); }

      virtual PromiseWithRTPDecoderVideoPtr innerCreateRTPDecoderVideo(
                                                                       const Parameters &parameters,
                                                                       IRTPDecoderDelegatePtr delegate
                                                                       ) noexcept;


      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForRTPEncoder
      //

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForRTPEncoderAudio
      //

      static PromiseWithRTPEncoderAudioPtr createRTPEncoderAudio(
                                                                 const Parameters &parameters,
                                                                 IRTPEncoderDelegatePtr delegate
                                                                 ) noexcept                                               { auto engine = singleton(); if (!engine) return PromiseWithRTPEncoderAudio::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->createRTPEncoderAudio(parameters, delegate); }

      virtual PromiseWithRTPEncoderAudioPtr innerCreateRTPEncoderAudio(
                                                                       const Parameters &parameters,
                                                                       IRTPEncoderDelegatePtr delegate
                                                                       ) noexcept;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaEngineForRTPEncoderVideo
      //

      static PromiseWithRTPEncoderVideoPtr createRTPEncoderVideo(
                                                                 const Parameters &parameters,
                                                                 IRTPEncoderDelegatePtr delegate
                                                                 ) noexcept                                               { auto engine = singleton(); if (!engine) return PromiseWithRTPEncoderVideo::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateRTPEncoderVideo(parameters, delegate); }

      virtual PromiseWithRTPEncoderVideoPtr innerCreateRTPEncoderVideo(
                                                                       const Parameters &parameters,
                                                                       IRTPEncoderDelegatePtr delegate
                                                                       ) noexcept;

    protected:
      
      //-----------------------------------------------------------------------
      //
      // MediaEngine => IMediaDevicesDelegate
      //

      //-----------------------------------------------------------------------
      void onMediaDevicesChanged() override;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      //
      // MediaEngine => IPromiseSettledDelegate
      //

      void onPromiseSettled(PromisePtr promise) override;

    protected:
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaEngine::WebRtcTraceCallback
      //

      class WebRtcTraceCallback : public webrtc::TraceCallback
      {
      public:
        void Print(webrtc::TraceLevel level, const char* message, int length) override;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaEngine::WebRtcTraceCallback
      //

      class WebRtcLogSink : public rtc::LogSink
      {
      public:
        void OnLogMessage(const std::string& message) override;
      };

      //-----------------------------------------------------------------------
      //
      // MediaEngine => webrtc::SetupEventTracer
      //

#if defined(WINUWP)
      static const unsigned char *GetCategoryGroupEnabled(const char *categoryGroup);

      static void __cdecl AddTraceEvent(
                                        char phase,
                                        const unsigned char *categoryGroupEnabled,
                                        const char *name,
                                        uint64 id,
                                        int numArgs,
                                        const char **argNames,
                                        const unsigned char *argTypes,
                                        const uint64 *argValues,
                                        unsigned char flags
                                        );
#endif //defined(WINUWP)

      //-----------------------------------------------------------------------
      //
      // (internal)
      //

#if defined(WINUWP)
      virtual void internalAddTraceEvent(
                                         char phase,
                                         const unsigned char *categoryGroupEnabled,
                                         const char *name,
                                         uint64_t id,
                                         int numArgs,
                                         const char **argNames,
                                         const unsigned char *argTypes,
                                         const uint64_t *argValues,
                                         unsigned char flags
                                         ) noexcept;
#endif //defined(WINUWP)

      bool isPending() const noexcept { return State_Pending == currentState_; }
      bool isWakingUp() const noexcept { return State_WakingUp == currentState_; }
      bool isReady() const noexcept { return State_Ready == currentState_; }
      bool isGoingToSleep() const noexcept { return State_GoingToSleep == currentState_; }
      bool isSleeping() const noexcept { return State_Sleeping == currentState_; }
      bool isShuttingDown() const noexcept { return State_ShuttingDown == currentState_; }
      bool isShutdown() const noexcept { return State_Shutdown == currentState_; }

      void cancel() noexcept;
      bool stepShutdownPendingRequests() noexcept;
      bool stepShutdownDevicesAndRTP() noexcept;

      void step() noexcept;

      bool stepPendingPromises() noexcept;

      bool stepRefreshDeviceList() noexcept;

      void stepFixState() noexcept;

      bool stepWakeUp1() noexcept;
      void onStepWakeUp1(PromisePtr promise);

      bool stepGoingToSleep1() noexcept;
      void onStepGoingToSleep1(PromisePtr promise);

      bool stepShutdownDevices() noexcept;
      bool stepShutdownRTP() noexcept;
      bool stepStartPendingDevices() noexcept;
      bool stepStartPendingRTP() noexcept;
      bool stepReady1() noexcept;
      void onReady1(PromisePtr promise) noexcept;

      void performAsync(std::function<void(PromisePtr)> func) noexcept;

      template <class Closure>
      PromisePtr performAsyncClosure(const Closure &closure) noexcept                { auto promise = Promise::create(); pendingPromises_[promise->getID()] = promise; postClosure([promise, closure] { closure(); promise->resolve(); }); return promise; }

      template <class Closure>
      void performAsyncClosure(PromisePtr promise, const Closure &closure) noexcept  { pendingPromises_[promise->getID()] = promise; postClosure([closure] { closure(); }); }

      void setState(States state) noexcept;
      void setError(PromisePtr promise) noexcept;
      void setError(WORD errorCode, const char *inReason) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // (data)
      //

      AutoPUID id_;
      MediaEngineWeakPtr thisWeak_;

      MediaEnginePtr gracefulShutdownReference_;

      std::unique_ptr<WebRtcTraceCallback> traceCallback_;
      std::unique_ptr<WebRtcLogSink> logSink_;
      //rtc::TraceLog traceLog_;

      States currentState_ {State_Pending};
      WORD lastError_ {};
      String lastErrorReason_;

      bool refreshDevicesList_ {true};
      IMediaDevicesSubscriptionPtr devicesSubscription_;
      PromiseWithDeviceListPtr devicesListPromise_;
      DeviceListPtr mediaDevices_;

      PendingForMediaDeviceCaptureList pendingForMediaDeviceCaptures_;
      PendingForMediaDeviceRenderList pendingForMediaDeviceRenders_;

      PendingForRTPDecodeAudioList pendingForRTPDecodeAudios_;
      PendingForRTPDecodeVideoList pendingForRTPDecodeVideos_;
      PendingForRTPEncodeAudioList pendingForRTPEncodeAudios_;
      PendingForRTPEncodeVideoList pendingForRTPEncodeVideos_;

      std::atomic<bool> checkForIdleDevices_ {};
      std::atomic<bool> shuttingDownIdleDevices_ {};
      DeviceCaptureMap captureDevices_;
      DeviceRenderMap renderDevices_;

      std::atomic<bool> checkForShutdownRTP_ {};
      RTPEncoderMap rtpEncoders_;
      RTPDecoderMap rtpDecoders_;

      std::atomic<bool> completedWakeUp1_ {};

      std::atomic<bool> completedGoingToSleep1_ {};

      PromiseMap pendingPromises_;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaEngineFactory
    //

    interaction IMediaEngineFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceCapture::MediaDeviceCapturePromise, MediaDeviceCapturePromise);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceRender::MediaDeviceRenderPromise, MediaDeviceRenderPromise);
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderAudio::PromiseWithRTPDecoderAudio, PromiseWithRTPDecoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderVideo::PromiseWithRTPDecoderVideo, PromiseWithRTPDecoderVideo);
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderAudio::PromiseWithRTPEncoderAudio, PromiseWithRTPEncoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderVideo::PromiseWithRTPEncoderVideo, PromiseWithRTPEncoderVideo);

      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::MediaDeviceObjectID, MediaDeviceObjectID);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);

      static IMediaEngineFactory &singleton() noexcept;

      virtual MediaEnginePtr create() noexcept;

      virtual void setLogLevel(Log::Level level) noexcept;
      virtual void ntpServerTime(const Milliseconds &value) noexcept;
      virtual void startMediaTracing() noexcept;
      virtual void stopMediaTracing() noexcept;
      virtual bool isMediaTracing() noexcept;
      virtual bool saveMediaTrace(String filename) noexcept;
      virtual bool saveMediaTrace(String host, int port) noexcept;

      virtual MediaDeviceCapturePromisePtr createMediaDeviceCapture(
                                                                    MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                    Kinds kind,
                                                                    const TrackConstraints &constraints,
                                                                    IMediaDeviceCaptureDelegatePtr delegate
                                                                    ) noexcept;

      virtual MediaDeviceRenderPromisePtr createMediaDeviceRender(
                                                                  MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                  const TrackConstraints &constraints,
                                                                  IMediaDeviceRenderDelegatePtr delegate
                                                                  ) noexcept;

      virtual PromiseWithRTPDecoderAudioPtr createRTPDecoderAudio(
                                                                  const Parameters &parameters,
                                                                  IRTPDecoderDelegatePtr delegate
                                                                  ) noexcept;
      virtual PromiseWithRTPDecoderVideoPtr createRTPDecoderVideo(
                                                                  const Parameters &parameters,
                                                                  IRTPDecoderDelegatePtr delegate
                                                                  ) noexcept;

      virtual PromiseWithRTPEncoderAudioPtr createRTPEncoderAudio(
                                                                  const Parameters &parameters,
                                                                  IRTPEncoderDelegatePtr delegate
                                                                  ) noexcept;
      virtual PromiseWithRTPEncoderVideoPtr createRTPEncoderVideo(
                                                                  const Parameters &parameters,
                                                                  IRTPEncoderDelegatePtr delegate
                                                                  ) noexcept;
    };

    class MediaEngineFactory : public IFactory<IMediaEngineFactory> {};
  }
}
