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
    // RTPEncoderAudio (helpers)
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPEncoderAudio
    //
    
    class RTPEncoderAudio : public Any,
                            public Noop,
                            public MessageQueueAssociator,
                            public SharedRecursiveLock,
                            public IRTPEncoderAudio,
                            public IRTPEncoderAudioForMediaEngine,
                            public zsLib::IWakeDelegate,
                            public zsLib::IPromiseSettledDelegate,
                            public IRTPEncoderAysncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPEncoderAudioForMediaEngine;
      friend interaction IRTPEncoderAysncDelegate;

      typedef IRTP::RTPObjectID RTPObjectID;
      typedef IRTP::States States;
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderAudioPtr>, PromiseWithRTPEncoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderAudio, UseMediaEngine);

    public:
      RTPEncoderAudio(
                      const make_private &,
                      IMessageQueuePtr queue,
                      PromiseWithRTPEncoderAudioPtr promise,
                      UseMediaEnginePtr mediaEngine,
                      ParametersPtr parameters,
                      IRTPEncoderDelegatePtr delegate
                      ) noexcept;

    protected:
      RTPEncoderAudio(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create()),
        traceHelper_(id_)
      {}

      void init() noexcept;

    public:
      virtual ~RTPEncoderAudio() noexcept;

      static RTPEncoderAudioPtr create(
                                       PromiseWithRTPEncoderAudioPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPEncoderDelegatePtr delegate
                                       ) noexcept;

      static RTPEncoderAudioPtr convert(ForMediaEnginePtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IRTP
      //

      RTPObjectID getID() const noexcept override { return id_; }
      void cancel() noexcept override;

      States getState() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IRTPEncoder
      //

      void notifyAudioFrame(
                            ImmutableMediaChannelTracePtr trace,
                            AudioFramePtr frame
                            ) noexcept override;
      void notifyVideoFrame(
                            ImmutableMediaChannelTracePtr trace,
                            VideoFramePtr frame
                            ) noexcept override {}

      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IRTPEncoderAudio
      //

      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IRTPForMediaEngine
      //

      // (duplicate) virtual RTPObjectID getID() const = 0;
      void shutdown() noexcept override;

      // (duplciate) virtual States getState() const override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IRTPEncoderForMediaEngine
      //

      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IRTPEncoderAudioForMediaEngine
      //

      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IPromiseSettledDelegate
      //

      void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => IRTPEncoderAysncDelegate
      //
      
      void onRTPEncoderAudioFrame(
                                  ImmutableMediaChannelTracePtr trace,
                                  AudioFramePtr frame
                                  ) override;
      void onRTPEncoderVideoFrame(
                                  ImmutableMediaChannelTracePtr trace,
                                  VideoFramePtr frame
                                  ) override {} // ignored

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPEncoderAudio => (internal)
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
      // RTPEncoderAudio => (data)
      //

      AutoPUID id_;
      RTPEncoderAudioWeakPtr thisWeak_;

      RTPEncoderAudioPtr gracefulShutdownReference_;

      UseMediaEngineWeakPtr mediaEngine_;

      States currentState_ {IRTP::State_Pending};
      WORD lastError_ {};
      String lastErrorReason_;

      PromiseWithRTPEncoderAudioPtr promise_;
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
    // IRTPEncoderAudioFactory
    //

    interaction IRTPEncoderAudioFactory
    {
      static IRTPEncoderAudioFactory &singleton() noexcept;

      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderAudioPtr>, PromiseWithRTPEncoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderAudio, UseMediaEngine);

      virtual RTPEncoderAudioPtr create(
                                       PromiseWithRTPEncoderAudioPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPEncoderDelegatePtr delegate
                                       ) noexcept;
    };

    class RTPEncoderAudioFactory : public IFactory<IRTPEncoderAudioFactory> {};
  }
}

