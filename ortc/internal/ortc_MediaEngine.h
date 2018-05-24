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

#include <rtc_base/logging.h>
//#include <system_wrappers/include/trace.h>
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
    #pragma mark
    #pragma mark IMediaEngineForORTC
    #pragma mark

    interaction IMediaEngineForORTC
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForORTC, ForORTC);
    
      static void setLogLevel(Log::Level level);
      static void ntpServerTime(const Milliseconds &value);
      static void startMediaTracing();
      static void stopMediaTracing();
      static bool isMediaTracing();
      static bool saveMediaTrace(String filename);
      static bool saveMediaTrace(String host, int port);

      virtual ~IMediaEngineForORTC() {}
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDevice
    #pragma mark

    interaction IMediaEngineForMediaDevice
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDevice, ForDevice);

      virtual void notifyDeviceIsIdleOrShutdownStateChanged() = 0;

      virtual ~IMediaEngineForMediaDevice() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDeviceCapture
    #pragma mark

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
                                                                   );

      virtual ~IMediaEngineForMediaDeviceCapture() {}
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDeviceCaptureAudio
    #pragma mark

    interaction IMediaEngineForMediaDeviceCaptureAudio : public IMediaEngineForMediaDeviceCapture
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceCaptureAudio, ForMediaDeviceCaptureAudio);

      virtual ~IMediaEngineForMediaDeviceCaptureAudio() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDeviceCaptureVideo
    #pragma mark

    interaction IMediaEngineForMediaDeviceCaptureVideo : public IMediaEngineForMediaDeviceCapture
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceCaptureVideo, ForMediaDeviceCaptureVideo);

      virtual ~IMediaEngineForMediaDeviceCaptureVideo() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDeviceRender
    #pragma mark

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
                                                                 );

      virtual ~IMediaEngineForMediaDeviceRender() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForMediaDeviceRenderAudio : public IMediaEngineForMediaDeviceRender
    #pragma mark

    interaction IMediaEngineForMediaDeviceRenderAudio : public IMediaEngineForMediaDeviceRender
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceRenderAudio, ForMediaDeviceRenderAudio);

      virtual ~IMediaEngineForMediaDeviceRenderAudio() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTP
    #pragma mark

    interaction IMediaEngineForRTP
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTP, ForRTP);

    virtual void notifyRTPShutdownStateChanged() = 0;

      virtual ~IMediaEngineForRTP() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPDecoder
    #pragma mark

    interaction IMediaEngineForRTPDecoder : public IMediaEngineForRTP
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoder, ForRTPDecode);

      virtual ~IMediaEngineForRTPDecoder() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPDecoderAudio
    #pragma mark

    interaction IMediaEngineForRTPDecoderAudio : public IMediaEngineForRTPDecoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderAudio, ForRTPDecodeAudio);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderAudioPtr>, PromiseWithRTPDecoderAudio);

      static PromiseWithRTPDecoderAudioPtr createRTPDecoderAudio(
                                                                 const Parameters &parameters,
                                                                 IRTPDecoderDelegatePtr delegate
                                                                 );

      virtual ~IMediaEngineForRTPDecoderAudio() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPDecoderVideo
    #pragma mark

    interaction IMediaEngineForRTPDecoderVideo : public IMediaEngineForRTPDecoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderVideo, ForRTPDecodeVideo);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderVideoPtr>, PromiseWithRTPDecoderVideo);

      static PromiseWithRTPDecoderVideoPtr createRTPDecoderVideo(
                                                                 const Parameters &parameters,
                                                                 IRTPDecoderDelegatePtr delegate
                                                                 );

      virtual ~IMediaEngineForRTPDecoderVideo() {}
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPEncoder
    #pragma mark

    interaction IMediaEngineForRTPEncoder : public IMediaEngineForRTP
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoder, ForRTPEncode);

      virtual ~IMediaEngineForRTPEncoder() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPEncoderAudio
    #pragma mark

    interaction IMediaEngineForRTPEncoderAudio : public IMediaEngineForRTPEncoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderAudio, ForRTPEncodeAudio);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderAudioPtr>, PromiseWithRTPEncoderAudio);

      static PromiseWithRTPEncoderAudioPtr createRTPEncoderAudio(
                                                                 const Parameters &parameters,
                                                                 IRTPEncoderDelegatePtr delegate
                                                                 );

      virtual ~IMediaEngineForRTPEncoderAudio() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineForRTPEncoderVideo
    #pragma mark

    interaction IMediaEngineForRTPEncoderVideo : public IMediaEngineForRTPEncoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderVideo, ForRTPEncodeVideo);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderVideoPtr>, PromiseWithRTPEncoderVideo);

      static PromiseWithRTPEncoderVideoPtr createRTPEncoderVideo(
                                                                 const Parameters &parameters,
                                                                 IRTPEncoderDelegatePtr delegate
                                                                 );

      virtual ~IMediaEngineForRTPEncoderVideo() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine
    #pragma mark

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

      static const char *toString(States state);

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
                  );

    protected:
      MediaEngine(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~MediaEngine();

      static MediaEnginePtr singleton();
      static MediaEnginePtr create();

      static MediaEnginePtr convert(ForORTCPtr object);
      static MediaEnginePtr convert(ForMediaDeviceCapturePtr object);
      static MediaEnginePtr convert(ForMediaDeviceRenderPtr object);
      static MediaEnginePtr convert(ForMediaDeviceRenderAudioPtr object);
      static MediaEnginePtr convert(ForMediaDeviceCaptureAudioPtr object);
      static MediaEnginePtr convert(ForMediaDeviceCaptureVideoPtr object);
      static MediaEnginePtr convert(ForRTPPtr object);
      static MediaEnginePtr convert(ForRTPDecodePtr object);
      static MediaEnginePtr convert(ForRTPDecodeAudioPtr object);
      static MediaEnginePtr convert(ForRTPDecodeVideoPtr object);
      static MediaEnginePtr convert(ForRTPEncodePtr object);
      static MediaEnginePtr convert(ForRTPEncodeAudioPtr object);
      static MediaEnginePtr convert(ForRTPEncodeVideoPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForORTC
      #pragma mark

      static void setLogLevel(Log::Level level)                   { auto engine = singleton(); if (!engine) return; engine->internalSetLogLevel(level); }
      static void ntpServerTime(const Milliseconds &value)        { auto engine = singleton(); if (!engine) return; engine->internalNTPServerTime(value); }
      static void startMediaTracing()                             { auto engine = singleton(); if (!engine) return; engine->internalStartMediaTracing(); }
      static void stopMediaTracing()                              { auto engine = singleton(); if (!engine) return; engine->internalStopMediaTracing(); }
      static bool isMediaTracing()                                { auto engine = singleton(); if (!engine) return false; return engine->internalIsMediaTracing(); }
      static bool saveMediaTrace(String filename)                 { auto engine = singleton(); if (!engine) return false; return engine->internalSaveMediaTrace(filename); }
      static bool saveMediaTrace(String host, int port)           { auto engine = singleton(); if (!engine) return false; return engine->internalSaveMediaTrace(host, port); }

      virtual void internalSetLogLevel(Log::Level level);
      virtual void internalNTPServerTime(const Milliseconds &value);
      virtual void internalStartMediaTracing();
      virtual void internalStopMediaTracing();
      virtual bool internalIsMediaTracing();
      virtual bool internalSaveMediaTrace(String filename);
      virtual bool internalSaveMediaTrace(String host, int port);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForMediaDevice
      #pragma mark

      void notifyDeviceIsIdleOrShutdownStateChanged() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForMediaDeviceCapture
      #pragma mark

      static MediaDeviceCapturePromisePtr createMediaDeviceCapture(
                                                                   MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                   Kinds kind,
                                                                   const TrackConstraints &constraints,
                                                                   IMediaDeviceCaptureDelegatePtr delegate
                                                                   )                                             { auto engine = singleton(); if (!engine) return MediaDeviceCapturePromise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateMediaDeviceCapture(repaceExistingDeviceObjectID, kind, constraints, delegate); }

      virtual MediaDeviceCapturePromisePtr innerCreateMediaDeviceCapture(
                                                                         MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                         Kinds kind,
                                                                         const TrackConstraints &constraints,
                                                                         IMediaDeviceCaptureDelegatePtr delegate
                                                                         );


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForMediaDeviceCaptureAudio
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForMediaDeviceCaptureVideo
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForMediaDeviceRender
      #pragma mark

      
      static MediaDeviceRenderPromisePtr createMediaDeviceRender(
                                                                 MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                 const TrackConstraints &constraints,
                                                                 IMediaDeviceRenderDelegatePtr delegate
                                                                 )                                             { auto engine = singleton(); if (!engine) return MediaDeviceRenderPromise::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateMediaDeviceRender(repaceExistingDeviceObjectID, constraints, delegate); }

      virtual MediaDeviceRenderPromisePtr innerCreateMediaDeviceRender(
                                                                       MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                       const TrackConstraints &constraints,
                                                                       IMediaDeviceRenderDelegatePtr delegate
                                                                       );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForMediaDeviceRenderAudio
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForRTP
      #pragma mark

      void notifyRTPShutdownStateChanged() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForRTPDecoder
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForRTPDecoderAudio
      #pragma mark

      static PromiseWithRTPDecoderAudioPtr createRTPDecoderAudio(
                                                                 const Parameters &parameters,
                                                                 IRTPDecoderDelegatePtr delegate
                                                                 )                                                { auto engine = singleton(); if (!engine) return PromiseWithRTPDecoderAudio::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateRTPDecoderAudio(parameters, delegate); }

      virtual PromiseWithRTPDecoderAudioPtr innerCreateRTPDecoderAudio(
                                                                       const Parameters &parameters,
                                                                       IRTPDecoderDelegatePtr delegate
                                                                       );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForRTPDecoderVideo
      #pragma mark

      static PromiseWithRTPDecoderVideoPtr createRTPDecoderVideo(
                                                                 const Parameters &parameters,
                                                                 IRTPDecoderDelegatePtr delegate
                                                                 )                                                { auto engine = singleton(); if (!engine) return PromiseWithRTPDecoderVideo::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateRTPDecoderVideo(parameters, delegate); }

      virtual PromiseWithRTPDecoderVideoPtr innerCreateRTPDecoderVideo(
                                                                       const Parameters &parameters,
                                                                       IRTPDecoderDelegatePtr delegate
                                                                       );


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForRTPEncoder
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForRTPEncoderAudio
      #pragma mark

      static PromiseWithRTPEncoderAudioPtr createRTPEncoderAudio(
                                                                 const Parameters &parameters,
                                                                 IRTPEncoderDelegatePtr delegate
                                                                 )                                                { auto engine = singleton(); if (!engine) return PromiseWithRTPEncoderAudio::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->createRTPEncoderAudio(parameters, delegate); }

      virtual PromiseWithRTPEncoderAudioPtr innerCreateRTPEncoderAudio(
                                                                       const Parameters &parameters,
                                                                       IRTPEncoderDelegatePtr delegate
                                                                       );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaEngineForRTPEncoderVideo
      #pragma mark

      static PromiseWithRTPEncoderVideoPtr createRTPEncoderVideo(
                                                                 const Parameters &parameters,
                                                                 IRTPEncoderDelegatePtr delegate
                                                                 )                                                { auto engine = singleton(); if (!engine) return PromiseWithRTPEncoderVideo::createRejected(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "singleton gone"), IORTCForInternal::queueDelegate()); return engine->innerCreateRTPEncoderVideo(parameters, delegate); }

      virtual PromiseWithRTPEncoderVideoPtr innerCreateRTPEncoderVideo(
                                                                       const Parameters &parameters,
                                                                       IRTPEncoderDelegatePtr delegate
                                                                       );

    protected:
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IMediaDevicesDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void onMediaDevicesChanged() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IWakeDelegate
      #pragma mark

      void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => ITimerDelegate
      #pragma mark

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => IPromiseSettledDelegate
      #pragma mark

      void onPromiseSettled(PromisePtr promise) override;

    protected:
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine::WebRtcTraceCallback
      #pragma mark

      class WebRtcTraceCallback : public webrtc::TraceCallback
      {
      public:
        void Print(webrtc::TraceLevel level, const char* message, int length) override;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine::WebRtcTraceCallback
      #pragma mark

      class WebRtcLogSink : public rtc::LogSink
      {
      public:
        void OnLogMessage(const std::string& message) override;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => webrtc::SetupEventTracer
      #pragma mark

#if defined(WINRT)
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
#endif //defined(WINRT)

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (internal)
      #pragma mark

#if defined(WINRT)
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
                                         );
#endif //defined(WINRT)

      bool isPending() const { return State_Pending == currentState_; }
      bool isWakingUp() const { return State_WakingUp == currentState_; }
      bool isReady() const { return State_Ready == currentState_; }
      bool isGoingToSleep() const { return State_GoingToSleep == currentState_; }
      bool isSleeping() const { return State_Sleeping == currentState_; }
      bool isShuttingDown() const { return State_ShuttingDown == currentState_; }
      bool isShutdown() const { return State_Shutdown == currentState_; }

      void cancel();
      bool stepShutdownPendingRequests();
      bool stepShutdownDevicesAndRTP();

      void step();

      bool stepPendingPromises();

      bool stepRefreshDeviceList();

      void stepFixState();

      bool stepWakeUp1();
      void onStepWakeUp1(PromisePtr promise);

      bool stepGoingToSleep1();
      void onStepGoingToSleep1(PromisePtr promise);

      bool stepShutdownDevices();
      bool stepShutdownRTP();
      bool stepStartPendingDevices();
      bool stepStartPendingRTP();
      bool stepReady1();
      void onReady1(PromisePtr promise);

      void performAsync(std::function<void(PromisePtr)> func);

      template <class Closure>
      PromisePtr performAsyncClosure(const Closure &closure)                 { auto promise = Promise::create(); pendingPromises_[promise->getID()] = promise; postClosure([promise, closure] { closure(); promise->resolve(); }); return promise; }

      template <class Closure>
      void performAsyncClosure(PromisePtr promise, const Closure &closure)   { pendingPromises_[promise->getID()] = promise; postClosure([closure] { closure(); }); }

      void setState(States state);
      void setError(PromisePtr promise);
      void setError(WORD errorCode, const char *inReason);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (data)
      #pragma mark

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
    #pragma mark
    #pragma mark IMediaEngineFactory
    #pragma mark

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

      static IMediaEngineFactory &singleton();

      virtual MediaEnginePtr create();

      virtual void setLogLevel(Log::Level level);
      virtual void ntpServerTime(const Milliseconds &value);
      virtual void startMediaTracing();
      virtual void stopMediaTracing();
      virtual bool isMediaTracing();
      virtual bool saveMediaTrace(String filename);
      virtual bool saveMediaTrace(String host, int port);

      virtual MediaDeviceCapturePromisePtr createMediaDeviceCapture(
                                                                    MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                    Kinds kind,
                                                                    const TrackConstraints &constraints,
                                                                    IMediaDeviceCaptureDelegatePtr delegate
                                                                    );

      virtual MediaDeviceRenderPromisePtr createMediaDeviceRender(
                                                                  MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                  const TrackConstraints &constraints,
                                                                  IMediaDeviceRenderDelegatePtr delegate
                                                                  );

      virtual PromiseWithRTPDecoderAudioPtr createRTPDecoderAudio(
                                                                  const Parameters &parameters,
                                                                  IRTPDecoderDelegatePtr delegate
                                                                  );
      virtual PromiseWithRTPDecoderVideoPtr createRTPDecoderVideo(
                                                                  const Parameters &parameters,
                                                                  IRTPDecoderDelegatePtr delegate
                                                                  );

      virtual PromiseWithRTPEncoderAudioPtr createRTPEncoderAudio(
                                                                  const Parameters &parameters,
                                                                  IRTPEncoderDelegatePtr delegate
                                                                  );
      virtual PromiseWithRTPEncoderVideoPtr createRTPEncoderVideo(
                                                                  const Parameters &parameters,
                                                                  IRTPEncoderDelegatePtr delegate
                                                                  );
    };

    class MediaEngineFactory : public IFactory<IMediaEngineFactory> {};
  }
}
