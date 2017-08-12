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
#include <ortc/internal/ortc_IRTP.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPDecoderForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IRTPDecoderAudioForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IRTPDecoderVideoForMediaEngine);

    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForRTPDecoderAudio);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForRTPDecoderVideo);

    ZS_DECLARE_INTERACTION_PROXY(IRTPDecoderAysncDelegate);

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoder
    #pragma mark

    interaction IRTPDecoder : public IRTP
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);

      virtual void notifyRTPPacket(
                                   ImmutableMediaChannelTracePtr trace,
                                   RTPPacketPtr packet
                                   ) = 0;
      virtual void notifyRTCPPacket(
                                    ImmutableMediaChannelTracePtr trace,
                                    RTCPPacketPtr packet
                                    ) = 0;

      virtual ~IRTPDecoder() {}
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderAudio
    #pragma mark

    interaction IRTPDecoderAudio : public IRTPDecoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderAudioPtr>, PromiseWithRTPDecoderAudio);

      static PromiseWithRTPDecoderAudioPtr create(
                                                  const Parameters &parameters,
                                                  IRTPDecoderDelegatePtr delegate
                                                  );

      virtual ~IRTPDecoderAudio() {}
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderVideo
    #pragma mark

    interaction IRTPDecoderVideo : public IRTPDecoder
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderVideoPtr>, PromiseWithRTPDecoderVideo);

      static PromiseWithRTPDecoderVideoPtr create(
                                                  const Parameters &parameters,
                                                  IRTPDecoderDelegatePtr delegate
                                                  );
    
      virtual ~IRTPDecoderVideo() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderDelegate
    #pragma mark

    interaction IRTPDecoderDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IRTP::States, States);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);

      virtual void notifyRTPDecoderAudioFrame(
                                              ImmutableMediaChannelTracePtr trace,
                                              AudioFramePtr frame
                                              ) = 0;
      virtual void notifyRTPDecoderVideoFrame(
                                              ImmutableMediaChannelTracePtr trace,
                                              VideoFramePtr frame
                                              ) = 0;

      virtual void onRTPDecoderStateChanged(
                                            IRTPDecoderPtr decoder,
                                            States state
                                            ) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderSubscription
    #pragma mark

    interaction IRTPDecoderSubscription
    {
      virtual PUID getID() const = 0;

      virtual void cancel() = 0;

      virtual void background() = 0;
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderForMediaEngine
    #pragma mark

    interaction IRTPDecoderForMediaEngine : public IRTPForMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPDecoderForMediaEngine, ForMediaEngine);

      virtual ~IRTPDecoderForMediaEngine() {}
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderAudioForMediaEngine
    #pragma mark

    interaction IRTPDecoderAudioForMediaEngine : public IRTPDecoderForMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPDecoderAudioForMediaEngine, ForMediaEngine);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderAudio, UseMediaEngine);

      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderAudioPtr>, PromiseWithRTPDecoderAudio);

      static ForMediaEnginePtr create(
                                      PromiseWithRTPDecoderAudioPtr promise,
                                      UseMediaEnginePtr mediaEngine,
                                      ParametersPtr parameters,
                                      IRTPDecoderDelegatePtr delegate
                                      );

      virtual ~IRTPDecoderAudioForMediaEngine() {}
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderVideoForMediaEngine
    #pragma mark

    interaction IRTPDecoderVideoForMediaEngine : public IRTPDecoderForMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPDecoderVideoForMediaEngine, ForMediaEngine);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForRTPDecoderVideo, UseMediaEngine);

      ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWithHolderPtr<IRTPDecoderVideoPtr>, PromiseWithRTPDecoderVideo);

      static ForMediaEnginePtr create(
                                      PromiseWithRTPDecoderVideoPtr promise,
                                      UseMediaEnginePtr mediaEngine,
                                      ParametersPtr parameters,
                                      IRTPDecoderDelegatePtr delegate
                                      );

      virtual ~IRTPDecoderVideoForMediaEngine() {}
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPDecoderAsyncDelegate
    #pragma mark

    interaction IRTPDecoderAysncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);

      virtual void onRTPDecoderRTPPacket(
                                         ImmutableMediaChannelTracePtr trace,
                                         RTPPacketPtr packet
                                         ) = 0;
      virtual void onRTPDecoderRTCPPacket(
                                          ImmutableMediaChannelTracePtr trace,
                                          RTCPPacketPtr packet
                                          ) = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPDecoderDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::AudioFramePtr, AudioFramePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::VideoFramePtr, VideoFramePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPDecoderPtr, IRTPDecoderPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTP::States, States)
ZS_DECLARE_PROXY_METHOD_SYNC_2(notifyRTPDecoderAudioFrame, ImmutableMediaChannelTracePtr, AudioFramePtr)
ZS_DECLARE_PROXY_METHOD_SYNC_2(notifyRTPDecoderVideoFrame, ImmutableMediaChannelTracePtr, VideoFramePtr)
ZS_DECLARE_PROXY_METHOD_2(onRTPDecoderStateChanged, IRTPDecoderPtr, States)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::internal::IRTPDecoderDelegate, ortc::internal::IRTPDecoderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::AudioFramePtr, AudioFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::VideoFramePtr, VideoFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::internal::IRTPDecoderPtr, IRTPDecoderPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::internal::IRTP::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_SYNC_2(notifyRTPDecoderAudioFrame, ImmutableMediaChannelTracePtr, AudioFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_SYNC_2(notifyRTPDecoderVideoFrame, ImmutableMediaChannelTracePtr, VideoFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onRTPDecoderStateChanged, IRTPDecoderPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()


ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPDecoderAysncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTCPPacketPtr, RTCPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_METHOD_2(onRTPDecoderRTPPacket, ImmutableMediaChannelTracePtr, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD_2(onRTPDecoderRTCPPacket, ImmutableMediaChannelTracePtr, RTCPPacketPtr)
ZS_DECLARE_PROXY_END()
