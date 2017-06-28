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

#include <ortc/internal/ortc_IRTPDecoder.h>
#include <ortc/internal/ortc_MediaChannelTraceHelper.h>

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
    #pragma mark RTPDecoderAudio (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPDecoderAudio
    #pragma mark
    
    class RTPDecoderAudio : public Any,
                            public Noop,
                            public MessageQueueAssociator,
                            public SharedRecursiveLock,
                            public IRTPDecoderAudio,
                            public IRTPDecoderAudioForMediaEngine,
                            public zsLib::IWakeDelegate,
                            public zsLib::IPromiseSettledDelegate,
                            public IRTPDecoderAysncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPDecoderAudioForMediaEngine;
      friend interaction IRTPDecoderAysncDelegate;

      typedef IRTP::RTPObjectID RTPObjectID;
      typedef IRTP::States States;
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderAudioPtr>, PromiseWithRTPDecoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderAudio, UseMediaEngine);

    public:
      RTPDecoderAudio(
                      const make_private &,
                      IMessageQueuePtr queue,
                      PromiseWithRTPDecoderAudioPtr promise,
                      UseMediaEnginePtr mediaEngine,
                      ParametersPtr parameters,
                      IRTPDecoderDelegatePtr delegate
                      );

    protected:
      RTPDecoderAudio(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create()),
        traceHelper_(id_)
      {}

      void init();

    public:
      virtual ~RTPDecoderAudio();

      static RTPDecoderAudioPtr create(
                                       PromiseWithRTPDecoderAudioPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPDecoderDelegatePtr delegate
                                       );

      static RTPDecoderAudioPtr convert(ForMediaEnginePtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IRTP
      #pragma mark

      virtual RTPObjectID getID() const override { return id_; }
      virtual void cancel() override;

      virtual States getState() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IRTPDecoder
      #pragma mark

      virtual void notifyRTPPacket(
                                   ImmutableMediaChannelTracePtr trace,
                                   RTPPacketPtr packet
                                   ) override;
      virtual void notifyRTCPPacket(
                                    ImmutableMediaChannelTracePtr trace,
                                    RTCPPacketPtr packet
                                    ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IRTPDecoderAudio
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IRTPForMediaEngine
      #pragma mark

      // (duplicate) virtual RTPObjectID getID() const = 0;
      virtual void shutdown() override;

      // (duplciate) virtual States getState() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IRTPDecoderForMediaEngine
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IRTPDecoderAudioForMediaEngine
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => IRTPDecoderAysncDelegate
      #pragma mark
      
      virtual void onRTPDecoderRTPPacket(
                                         ImmutableMediaChannelTracePtr trace,
                                         RTPPacketPtr packet
                                         ) override;
      virtual void onRTPDecoderRTCPPacket(
                                          ImmutableMediaChannelTracePtr trace,
                                          RTCPPacketPtr packet
                                          ) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => (internal)
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

      void innerNotifyAudioFrame(
                                 ImmutableMediaChannelTracePtr trace,
                                 AudioFramePtr frame
                                 );
      void innerNotifyVideoFrame(
                                 ImmutableMediaChannelTracePtr trace,
                                 VideoFramePtr frame
                                 ) {} // ignored

    public:

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPDecoderAudio => (data)
      #pragma mark

      AutoPUID id_;
      RTPDecoderAudioWeakPtr thisWeak_;

      RTPDecoderAudioPtr gracefulShutdownReference_;

      UseMediaEngineWeakPtr mediaEngine_;

      States currentState_ {IRTP::State_Pending};
      WORD lastError_ {};
      String lastErrorReason_;

      PromiseWithRTPDecoderAudioPtr promise_;
      ParametersPtr parameters_;
      IRTPDecoderDelegateWeakPtr notifyDelegate_;
      IRTPDecoderDelegatePtr delegate_;

      IRTPDecoderAysncDelegatePtr asyncThisDelegate_;

      MediaChannelTraceHelper traceHelper_;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderAudioFactory
    #pragma mark

    interaction IRTPDecoderAudioFactory
    {
      static IRTPDecoderAudioFactory &singleton();

      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderAudioPtr>, PromiseWithRTPDecoderAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderAudio, UseMediaEngine);

      virtual RTPDecoderAudioPtr create(
                                       PromiseWithRTPDecoderAudioPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPDecoderDelegatePtr delegate
                                       );
    };

    class RTPDecoderAudioFactory : public IFactory<IRTPDecoderAudioFactory> {};
  }
}

