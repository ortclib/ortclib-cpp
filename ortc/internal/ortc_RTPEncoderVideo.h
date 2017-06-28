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
    #pragma mark RTPEncoderVideo (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPEncoderVideo
    #pragma mark
    
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
                      );

    protected:
      RTPEncoderVideo(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create()),
        traceHelper_(id_)
      {}

      void init();

    public:
      virtual ~RTPEncoderVideo();

      static RTPEncoderVideoPtr create(
                                       PromiseWithRTPEncoderVideoPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPEncoderDelegatePtr delegate
                                       );

      static RTPEncoderVideoPtr convert(ForMediaEnginePtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IRTP
      #pragma mark

      virtual RTPObjectID getID() const override { return id_; }
      virtual void cancel() override;

      virtual States getState() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IRTPEncoder
      #pragma mark

      virtual void notifyAudioFrame(
                                    ImmutableMediaChannelTracePtr trace,
                                    AudioFramePtr frame
                                    ) override {}
      virtual void notifyVideoFrame(
                                    ImmutableMediaChannelTracePtr trace,
                                    VideoFramePtr frame
                                    ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IRTPEncoderVideo
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IRTPForMediaEngine
      #pragma mark

      // (duplicate) virtual RTPObjectID getID() const = 0;
      virtual void shutdown() override;

      // (duplciate) virtual States getState() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IRTPEncoderForMediaEngine
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IRTPEncoderVideoForMediaEngine
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => IRTPEncoderAysncDelegate
      #pragma mark
      
      virtual void onRTPEncoderAudioFrame(
                                          ImmutableMediaChannelTracePtr trace,
                                          AudioFramePtr frame
                                          ) override {} // ignored
      virtual void onRTPEncoderVideoFrame(
                                          ImmutableMediaChannelTracePtr trace,
                                          VideoFramePtr frame
                                          ) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPEncoderVideo => (internal)
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
      #pragma mark RTPEncoderVideo => (data)
      #pragma mark

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
    #pragma mark
    #pragma mark IRTPEncoderVideoFactory
    #pragma mark

    interaction IRTPEncoderVideoFactory
    {
      static IRTPEncoderVideoFactory &singleton();

      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPEncoderVideoPtr>, PromiseWithRTPEncoderVideo);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPEncoderVideo, UseMediaEngine);

      virtual RTPEncoderVideoPtr create(
                                       PromiseWithRTPEncoderVideoPtr promise,
                                       UseMediaEnginePtr mediaEngine,
                                       ParametersPtr parameters,
                                       IRTPEncoderDelegatePtr delegate
                                       );
    };

    class RTPEncoderVideoFactory : public IFactory<IRTPEncoderVideoFactory> {};
  }
}

