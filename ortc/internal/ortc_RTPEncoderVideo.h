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

#include <ortc/internal/ortc_MediaChannelTraceHelper.h>
#include <ortc/internal/ortc_IRTPEncoder.h>

#include <ortc/RTPPacket.h>
#include <ortc/RTCPPacket.h>

#include <ortc/IMediaStreamTrack.h>

#include <zsLib/IWakeDelegate.h>

namespace ortc
{
  namespace internal
  {

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //
    // RTPEncoderVideo (helpers)
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderVideo
    //
    
    class RTPEncoderVideo : public Any,
                            public Noop,
                            public MessageQueueAssociator,
                            public SharedRecursiveLock,
                            public IRTPEncoderVideo,
                            public IRTPEncoderVideoForMediaEngine,
                            public zsLib::IWakeDelegate,
                            public zsLib::IPromiseSettledDelegate,
                            public IRTPEncoderAysncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPEncoderVideoForMediaEngine;
      friend interaction IRTPEncoderAysncDelegate;

      typedef IRTP::RTPObjectID RTPObjectID;
      typedef IRTP::States States;
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderVideoPtr>, PromiseWithRTPEncoderVideo);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderVideo, UseMediaEngine);

    public:
      RTPEncoderVideo(
                      const make_private &,
                      IMessageQueuePtr queue,
                      PromiseWithRTPEncoderVideoPtr promise,
                      UseMediaEnginePtr mediaEngine,
                      ParametersPtr parameters,
                      IRTPEncoderDelegatePtr delegate
                      ) noexcept;

    protected:
      RTPEncoderVideo(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create()),
        traceHelper_(id_)
      {}

      void init() noexcept;

    public:
      virtual ~RTPEncoderVideo() noexcept;

      static RTPEncoderVideoPtr create(
                                       PromiseWithRTPEncoderVideoPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPEncoderDelegatePtr delegate
                                       ) noexcept;

      static RTPEncoderVideoPtr convert(ForMediaEnginePtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IRTP
      //

      RTPObjectID getID() const noexcept override { return id_; }
      void cancel() noexcept override;

      States getState() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IRTPEncoder
      //

      void notifyAudioFrame(
                            ImmutableMediaChannelTracePtr trace,
                            AudioFramePtr frame
                            ) noexcept override {}
      void notifyVideoFrame(
                            ImmutableMediaChannelTracePtr trace,
                            VideoFramePtr frame
                            ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IRTPEncoderVideo
      //

      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IRTPForMediaEngine
      //

      // (duplicate) virtual RTPObjectID getID() const = 0;
      void shutdown() noexcept override;

      // (duplciate) virtual States getState() const override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IRTPEncoderForMediaEngine
      //

      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IRTPEncoderVideoForMediaEngine
      //

      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IPromiseSettledDelegate
      //

      void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => IRTPEncoderAysncDelegate
      //
      
      void onRTPEncoderAudioFrame(
                                  ImmutableMediaChannelTracePtr trace,
                                  AudioFramePtr frame
                                  ) override {} // ignored
      void onRTPEncoderVideoFrame(
                                  ImmutableMediaChannelTracePtr trace,
                                  VideoFramePtr frame
                                  ) override;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => (internal)
      //

      bool isPending() const noexcept { return IRTP::State_Pending == currentState_; }
      bool isReady() const noexcept { return IRTP::State_Ready == currentState_; }
      bool isShuttingDown() const noexcept { return IRTP::State_ShuttingDown == currentState_; }
      bool isShutdown() const noexcept { return IRTP::State_Shutdown == currentState_; }

      void innerCancel() noexcept;

      bool stepShutdownPendingPromise() noexcept;
      bool stepShutdownCoder() noexcept;

      void step() noexcept;
      bool stepSetup() noexcept;
      bool stepResolve() noexcept;

      void setState(States state) noexcept;
      void setError(PromisePtr promise) noexcept;
      void setError(WORD error, const char *inReason) noexcept;

      void innerNotifyRTP(
                          ImmutableMediaChannelTracePtr trace,
                          RTPPacketPtr packet
                          ) noexcept;
      void innerNotifyRTCP(
                           ImmutableMediaChannelTracePtr trace,
                           RTCPPacketPtr packet
                           ) noexcept;

    public:

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPEncoderVideo => (data)
      //

      AutoPUID id_;
      RTPEncoderVideoWeakPtr thisWeak_;

      RTPEncoderVideoPtr gracefulShutdownReference_;

      UseMediaEngineWeakPtr mediaEngine_;

      States currentState_ {IRTP::State_Pending};
      WORD lastError_ {};
      String lastErrorReason_;

      PromiseWithRTPEncoderVideoPtr promise_;
      ParametersPtr parameters_;
      IRTPEncoderDelegateWeakPtr notifyDelegate_;
      IRTPEncoderDelegatePtr delegate_;

      IRTPEncoderAysncDelegatePtr asyncThisDelegate_;
      MediaChannelTraceHelper traceHelper_;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPEncoderVideoFactory
    //

    interaction IRTPEncoderVideoFactory
    {
      static IRTPEncoderVideoFactory &singleton() noexcept;

      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderVideoPtr>, PromiseWithRTPEncoderVideo);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderVideo, UseMediaEngine);

      virtual RTPEncoderVideoPtr create(
                                       PromiseWithRTPEncoderVideoPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPEncoderDelegatePtr delegate
                                       ) noexcept;
    };

    class RTPEncoderVideoFactory : public IFactory<IRTPEncoderVideoFactory> {};
  }
}

