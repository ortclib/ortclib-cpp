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

#pragma once

#include <ortc/internal/types.h>
#include <ortc/internal/ortc_RTPSenderChannelMediaBase.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_RTPMediaEngine.h>

#include <ortc/IRTPTypes.h>
#include <ortc/IMediaStreamTrack.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>
#include <zsLib/Event.h>

#include <webrtc/transport.h>

//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_USING_PTR(zsLib, Event);

    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelMediaBaseForRTPSenderChannel);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelVideoForRTPSenderChannel);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelMediaBaseForMediaStreamTrack);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelVideoForMediaStreamTrack);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelVideoForRTPMediaEngine);

    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSenderChannelMediaBase);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSenderChannelVideo);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannelMediaBase);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannelVideo);

    ZS_DECLARE_INTERACTION_PROXY(IRTPSenderChannelVideoAsyncDelegate);


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelVideoForRTPSenderChannel
    #pragma mark

    interaction IRTPSenderChannelVideoForRTPSenderChannel : public IRTPSenderChannelMediaBaseForRTPSenderChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelVideoForRTPSenderChannel, ForRTPSenderChannel)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      ZS_DECLARE_TYPEDEF_PTR(webrtc::VideoFrame, VideoFrame);

      static RTPSenderChannelVideoPtr create(
                                             RTPSenderChannelPtr senderChannel,
                                             MediaStreamTrackPtr track,
                                             const Parameters &params
                                             );

      virtual void sendVideoFrame(VideoFramePtr videoFrame) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelVideoForMediaStreamTrack
    #pragma mark

    interaction IRTPSenderChannelVideoForMediaStreamTrack : public IRTPSenderChannelMediaBaseForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelVideoForMediaStreamTrack, ForMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelVideoForRTPMediaEngine
    #pragma mark

    interaction IRTPSenderChannelVideoForRTPMediaEngine : public IRTPSenderChannelMediaBaseForRTPMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelVideoForRTPMediaEngine, ForRTPMediaEngine)

      static ElementPtr toDebug(ForRTPMediaEnginePtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelVideoAsyncDelegate
    #pragma mark

    interaction IRTPSenderChannelVideoAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      virtual void onSecureTransportState(ISecureTransport::States state) = 0;
      virtual void onUpdate(ParametersPtr params) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo
    #pragma mark
    
    class RTPSenderChannelVideo : public Noop,
                                  public MessageQueueAssociator,
                                  public SharedRecursiveLock,
                                  public IRTPSenderChannelVideoForRTPSenderChannel,
                                  public IRTPSenderChannelVideoForMediaStreamTrack,
                                  public IRTPSenderChannelVideoForRTPMediaEngine,
                                  public IWakeDelegate,
                                  public zsLib::ITimerDelegate,
                                  public zsLib::IPromiseSettledDelegate,
                                  public IRTPSenderChannelVideoAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPSenderChannelVideo;
      friend interaction IRTPSenderChannelVideoFactory;
      friend interaction IRTPSenderChannelMediaBaseForRTPSenderChannel;
      friend interaction IRTPSenderChannelVideoForRTPSenderChannel;
      friend interaction IRTPSenderChannelMediaBaseForMediaStreamTrack;
      friend interaction IRTPSenderChannelVideoForMediaStreamTrack;
      friend interaction IRTPSenderChannelVideoForRTPMediaEngine;

      ZS_DECLARE_CLASS_PTR(Transport);
      friend class Transport;

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSenderChannelVideo, UseChannel);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelMediaBase, UseBaseMediaStreamTrack);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelVideo, UseMediaStreamTrack);
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPSenderChannelAudio, UseMediaEngine);
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineDeviceResource, UseDeviceResource);
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineVideoSenderChannelResource, UseChannelResource);

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelMediaBaseForRTPSenderChannel, ForRTPSenderChannelFromMediaBase);
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelMediaBaseForMediaStreamTrack, ForMediaStreamTrackFromMediaBase);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList);
      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);
      ZS_DECLARE_TYPEDEF_PTR(IStatsReportTypes::StatsTypeSet, StatsTypeSet)
      ZS_DECLARE_TYPEDEF_PTR(webrtc::VideoFrame, VideoFrame);

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      RTPSenderChannelVideo(
                            const make_private &,
                            IMessageQueuePtr queue,
                            UseChannelPtr senderChannel,
                            UseMediaStreamTrackPtr track,
                            const Parameters &params
                            );

    protected:
      RTPSenderChannelVideo(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPSenderChannelVideo();

      static RTPSenderChannelVideoPtr convert(ForRTPSenderChannelFromMediaBasePtr object);
      static RTPSenderChannelVideoPtr convert(ForRTPSenderChannelPtr object);
      static RTPSenderChannelVideoPtr convert(ForMediaStreamTrackFromMediaBasePtr object);
      static RTPSenderChannelVideoPtr convert(ForMediaStreamTrackPtr object);
      static RTPSenderChannelVideoPtr convert(ForRTPMediaEnginePtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => IRTPSenderChannelMediaBaseForRTPSenderChannel
      #pragma mark

      virtual PUID getID() const override {return mID;}

      virtual void onTrackChanged(UseBaseMediaStreamTrackPtr track) override;
      
      virtual void notifyTransportState(ISecureTransportTypes::States state) override;

      virtual void notifyUpdate(ParametersPtr params) override;

      virtual bool handlePacket(RTCPPacketPtr packet) override;

      virtual void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats) override;

      virtual void insertDTMF(
                              const char *tones,
                              Milliseconds duration,
                              Milliseconds interToneGap
                              ) override {}

      virtual String toneBuffer() const override { return String(); }
      virtual Milliseconds duration() const override { return Milliseconds(); }
      virtual Milliseconds interToneGap() const override { return Milliseconds(); }

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoForRTPSenderChannel
      #pragma mark

      static RTPSenderChannelVideoPtr create(
                                             RTPSenderChannelPtr senderChannel,
                                             MediaStreamTrackPtr track,
                                             const Parameters &params
                                             );

      virtual void sendVideoFrame(VideoFramePtr videoFrame) override;
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoForMediaStreamTrack
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => IRTPSenderChannelMediaBaseForRTPMediaEngine
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPMediaEnginePtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoAsyncDelegate
      #pragma mark

      virtual void onSecureTransportState(ISecureTransport::States state) override;

      virtual void onUpdate(ParametersPtr params) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => friend Transport
      #pragma mark

      virtual bool SendRtp(
                           const uint8_t* packet,
                           size_t length,
                           const webrtc::PacketOptions& options
                           );

      virtual bool SendRtcp(const uint8_t* packet, size_t length);

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo::Transport
      #pragma mark

      class Transport : public webrtc::Transport
      {
        struct make_private {};

      protected:
        void init();
        
      public:
        Transport(
                  const make_private &,
                  RTPSenderChannelVideoPtr outer
                  );
        
        ~Transport();
        
        static TransportPtr create(RTPSenderChannelVideoPtr outer);
        
      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderChannelVideo::Transport => webrtc::Transport
        #pragma mark

        virtual bool SendRtp(
                             const uint8_t* packet,
                             size_t length,
                             const webrtc::PacketOptions& options
                             ) override;

        virtual bool SendRtcp(const uint8_t* packet, size_t length) override;

      private:
        TransportWeakPtr mThisWeak;
        RTPSenderChannelVideoWeakPtr mOuter;
      };
      
    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepSetupChannel();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelVideo => (data)
      #pragma mark

      AutoPUID mID;
      RTPSenderChannelVideoWeakPtr mThisWeak;
      RTPSenderChannelVideoPtr mGracefulShutdownReference;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseChannelWeakPtr mSenderChannel;

      ParametersPtr mParameters;

      PromiseWithRTPMediaEngineChannelResourcePtr mChannelResourceLifetimeHolderPromise;
      UseChannelResourcePtr mChannelResource;

      PromisePtr mCloseChannelPromise;

      Optional<IMediaStreamTrackTypes::Kinds> mKind;
      UseMediaStreamTrackPtr mTrack;

      TransportPtr mTransport;  // allow lifetime of callback to exist separate from "this" object
      std::atomic<ISecureTransport::States> mTransportState { ISecureTransport::State_Pending };
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelVideoFactory
    #pragma mark

    interaction IRTPSenderChannelVideoFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static IRTPSenderChannelVideoFactory &singleton();

      virtual RTPSenderChannelVideoPtr create(
                                              RTPSenderChannelPtr sender,
                                              MediaStreamTrackPtr track,
                                              const Parameters &params
                                              );
    };

    class RTPSenderChannelVideoFactory : public IFactory<IRTPSenderChannelVideoFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPSenderChannelVideoAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransport::States, States)
ZS_DECLARE_PROXY_METHOD_1(onSecureTransportState, States)
ZS_DECLARE_PROXY_METHOD_1(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()
