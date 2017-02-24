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
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_RTPReceiverChannelMediaBase.h>
#include <ortc/internal/ortc_RTPMediaEngine.h>

#include <ortc/IICETransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IMediaStreamTrack.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>
#include <zsLib/Event.h>

#include <webrtc/transport.h>

#include <queue>

//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_USING_PTR(zsLib, Event)

    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelVideoForSettings)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelVideoForRTPReceiverChannel)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelVideoForMediaStreamTrack)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelVideoForRTPMediaEngine)

    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForRTPReceiverChannelVideo)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiverChannelVideo)

    ZS_DECLARE_INTERACTION_PROXY(IRTPReceiverChannelVideoAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoForSettings
    #pragma mark

    interaction IRTPReceiverChannelVideoForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelVideoForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPReceiverChannelVideoForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoForRTPReceiverChannel
    #pragma mark

    interaction IRTPReceiverChannelVideoForRTPReceiverChannel : public IRTPReceiverChannelMediaBaseForRTPReceiverChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelVideoForRTPReceiverChannel, ForRTPReceiverChannel)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForRTPReceiverChannelPtr object);

      static RTPReceiverChannelVideoPtr create(
                                               RTPReceiverChannelPtr receiver,
                                               MediaStreamTrackPtr track,
                                               const Parameters &params
                                               );
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoForMediaStreamTrack
    #pragma mark

    interaction IRTPReceiverChannelVideoForMediaStreamTrack : public IRTPReceiverChannelMediaBaseForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelVideoForMediaStreamTrack, ForMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioForRTPMediaEngine
    #pragma mark

    interaction IRTPReceiverChannelVideoForRTPMediaEngine : public IRTPReceiverChannelMediaBaseForRTPMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelVideoForRTPMediaEngine, ForRTPMediaEngine)

      static ElementPtr toDebug(ForRTPMediaEnginePtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoAsyncDelegate
    #pragma mark

    interaction IRTPReceiverChannelVideoAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      virtual void onReceiverChannelVideoDeliverPackets() = 0;
      virtual void onSecureTransportState(ISecureTransport::States state) = 0;
      virtual void onUpdate(ParametersPtr params) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo
    #pragma mark
    
    class RTPReceiverChannelVideo : public Noop,
                                    public MessageQueueAssociator,
                                    public SharedRecursiveLock,
                                    public IRTPReceiverChannelVideoForSettings,
                                    public IRTPReceiverChannelVideoForRTPReceiverChannel,
                                    public IRTPReceiverChannelVideoForMediaStreamTrack,
                                    public IRTPReceiverChannelVideoForRTPMediaEngine,
                                    public IWakeDelegate,
                                    public zsLib::ITimerDelegate,
                                    public zsLib::IPromiseSettledDelegate,
                                    public IRTPReceiverChannelVideoAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPReceiverChannelVideo;
      friend interaction IRTPReceiverChannelVideoFactory;
      friend interaction IRTPReceiverChannelVideoForSettings;
      friend interaction IRTPReceiverChannelMediaBaseForRTPReceiverChannel;
      friend interaction IRTPReceiverChannelVideoForRTPReceiverChannel;
      friend interaction IRTPReceiverChannelMediaBaseForMediaStreamTrack;
      friend interaction IRTPReceiverChannelVideoForMediaStreamTrack;
      friend interaction IRTPReceiverChannelVideoForRTPMediaEngine;

      ZS_DECLARE_CLASS_PTR(Transport)
      friend class Transport;
      
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiverChannelVideo, UseChannel)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannelVideo, UseMediaStreamTrack)
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPReceiverChannelVideo, UseMediaEngine)
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineDeviceResource, UseDeviceResource)
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineVideoReceiverChannelResource, UseChannelResource)

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelMediaBaseForRTPReceiverChannel, ForReceiverChannelFromMediaBase)
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelMediaBaseForMediaStreamTrack, ForMediaStreamTrackFromMediaBase)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList)
      typedef std::queue<RTPPacketPtr> RTPPacketQueue;
      typedef std::queue<RTCPPacketPtr> RTCPPacketQueue;

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      RTPReceiverChannelVideo(
                              const make_private &,
                              IMessageQueuePtr queue,
                              UseChannelPtr receiverChannel,
                              UseMediaStreamTrackPtr track,
                              const Parameters &params
                              );

    protected:
      RTPReceiverChannelVideo(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPReceiverChannelVideo();

      static RTPReceiverChannelVideoPtr convert(ForSettingsPtr object);
      static RTPReceiverChannelVideoPtr convert(ForReceiverChannelFromMediaBasePtr object);
      static RTPReceiverChannelVideoPtr convert(ForRTPReceiverChannelPtr object);
      static RTPReceiverChannelVideoPtr convert(ForMediaStreamTrackFromMediaBasePtr object);
      static RTPReceiverChannelVideoPtr convert(ForMediaStreamTrackPtr object);
      static RTPReceiverChannelVideoPtr convert(ForRTPMediaEnginePtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
      #pragma mark

      virtual PUID getID() const override {return mID;}

      virtual void notifyTransportState(ISecureTransportTypes::States state) override;

      virtual void notifyUpdate(ParametersPtr params) override;

      virtual bool handlePacket(RTPPacketPtr packet) override;

      virtual bool handlePacket(RTCPPacketPtr packet) override;

      virtual void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelVideoForRTPReceiverChannel
      #pragma mark

      static RTPReceiverChannelVideoPtr create(
                                               RTPReceiverChannelPtr receiver,
                                               MediaStreamTrackPtr track,
                                               const Parameters &params
                                               );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelVideoForMediaStreamTrack
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelMediaBaseForRTPMediaEngine
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPMediaEnginePtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelVideoAsyncDelegate
      #pragma mark

      virtual void onReceiverChannelVideoDeliverPackets() override;

      virtual void onSecureTransportState(ISecureTransport::States state) override;

      virtual void onUpdate(ParametersPtr params) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => friend Transport
      #pragma mark

      virtual bool SendRtcp(const uint8_t* packet, size_t length);
      
    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo::Transport
      #pragma mark

      class Transport : public webrtc::Transport
      {
        struct make_private {};

      protected:
        void init();
        
      public:
        Transport(
                  const make_private &,
                  RTPReceiverChannelVideoPtr outer
                  );
        
        ~Transport();
        
        static TransportPtr create(RTPReceiverChannelVideoPtr outer);
        
      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPReceiverChannelVideo::Transport => webrtc::Transport
        #pragma mark

        virtual bool SendRtp(
                             const uint8_t* packet,
                             size_t length,
                             const webrtc::PacketOptions& options
                             ) override;

        virtual bool SendRtcp(const uint8_t* packet, size_t length) override;

      private:
        TransportWeakPtr mThisWeak;
        RTPReceiverChannelVideoWeakPtr mOuter;
      };


    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepChannelPromise();
      bool stepSetupChannel();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelVideo => (data)
      #pragma mark

      AutoPUID mID;
      RTPReceiverChannelVideoWeakPtr mThisWeak;
      RTPReceiverChannelVideoPtr mGracefulShutdownReference;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseChannelWeakPtr mReceiverChannel;

      ParametersPtr mParameters;

      PromiseWithRTPMediaEngineChannelResourcePtr mChannelResourceLifetimeHolderPromise;
      UseChannelResourcePtr mChannelResource;

      PromisePtr mCloseChannelPromise;

      Optional<IMediaStreamTrackTypes::Kinds> mKind;
      UseMediaStreamTrackPtr mTrack;

      TransportPtr mTransport;  // allow lifetime of callback to exist separate from "this" object
      std::atomic<ISecureTransport::States> mTransportState { ISecureTransport::State_Pending };

      RTPPacketQueue mQueuedRTP;
      RTCPPacketQueue mQueuedRTCP;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoFactory
    #pragma mark

    interaction IRTPReceiverChannelVideoFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static IRTPReceiverChannelVideoFactory &singleton();

      virtual RTPReceiverChannelVideoPtr create(
                                                RTPReceiverChannelPtr receiverChannel,
                                                MediaStreamTrackPtr track,
                                                const Parameters &params
                                                );
    };

    class RTPReceiverChannelVideoFactory : public IFactory<IRTPReceiverChannelVideoFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPReceiverChannelVideoAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransport::States, States)
ZS_DECLARE_PROXY_METHOD_0(onReceiverChannelVideoDeliverPackets)
ZS_DECLARE_PROXY_METHOD_1(onSecureTransportState, States)
ZS_DECLARE_PROXY_METHOD_1(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()
