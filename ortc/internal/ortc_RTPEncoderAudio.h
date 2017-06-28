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
    #pragma mark
    #pragma mark RTPEncoderAudio (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderAudio
    #pragma mark
    
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
                      );

    protected:
      RTPEncoderAudio(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create()),
        traceHelper_(id_)
      {}

      void init();

    public:
      virtual ~RTPEncoderAudio();

      static RTPEncoderAudioPtr create(
                                       PromiseWithRTPEncoderAudioPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPEncoderDelegatePtr delegate
                                       );

      static RTPEncoderAudioPtr convert(ForMediaEnginePtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IRTP
      #pragma mark

      virtual RTPObjectID getID() const override { return id_; }
      virtual void cancel() override;

      virtual States getState() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IRTPEncoder
      #pragma mark

      virtual void notifyAudioFrame(
                                    ImmutableMediaChannelTracePtr trace,
                                    AudioFramePtr frame
                                    ) override;
      virtual void notifyVideoFrame(
                                    ImmutableMediaChannelTracePtr trace,
                                    VideoFramePtr frame
                                    ) override {}

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IRTPEncoderAudio
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IRTPForMediaEngine
      #pragma mark

      // (duplicate) virtual RTPObjectID getID() const = 0;
      virtual void shutdown() override;

      // (duplciate) virtual States getState() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IRTPEncoderForMediaEngine
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IRTPEncoderAudioForMediaEngine
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => IRTPEncoderAysncDelegate
      #pragma mark
      
      virtual void onRTPEncoderAudioFrame(
                                          ImmutableMediaChannelTracePtr trace,
                                          AudioFramePtr frame
                                          ) override;
      virtual void onRTPEncoderVideoFrame(
                                          ImmutableMediaChannelTracePtr trace,
                                          VideoFramePtr frame
                                          ) override {} // ignored

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => (internal)
      #pragma mark

      bool isPending() const { return IRTP::State_Pending == currentState_; }
      bool isReady() const { return IRTP::State_Ready == currentState_; }
      bool isShuttingDown() const { return IRTP::State_ShuttingDown == currentState_; }
      bool isShutdown() const { return IRTP::State_Shutdown == currentState_; }

      void innerCancel();

      bool stepShutdownPendingPromise();
      bool stepShutdownCoder();

      void step();
      bool stepSetup();
      bool stepResolve();

      void setState(States state);
      void setError(PromisePtr promise);
      void setError(WORD error, const char *inReason);

      void innerNotifyRTP(
                          ImmutableMediaChannelTracePtr trace,
                          RTPPacketPtr packet
                          );
      void innerNotifyRTCP(
                           ImmutableMediaChannelTracePtr trace,
                           RTCPPacketPtr packet
                           );

    public:

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderAudio => (data)
      #pragma mark

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
    #pragma mark
    #pragma mark IRTPEncoderAudioFactory
    #pragma mark

    interaction IRTPEncoderAudioFactory
    {
      static IRTPEncoderAudioFactory &singleton();

      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderAudioPtr>, PromiseWithRTPEncoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderAudio, UseMediaEngine);

      virtual RTPEncoderAudioPtr create(
                                       PromiseWithRTPEncoderAudioPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPEncoderDelegatePtr delegate
                                       );
    };

    class RTPEncoderAudioFactory : public IFactory<IRTPEncoderAudioFactory> {};
  }
}

