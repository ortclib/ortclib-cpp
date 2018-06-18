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
#include <ortc/internal/ortc_IMediaStreamTrack.h>

#include <ortc/IICETransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IMediaStreamTrack.h>
#include <ortc/IStatsProvider.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>


//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForRTPReceiver);
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForMediaStreamTrack);

    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForRTPReceiverChannel);

    ZS_DECLARE_INTERACTION_PROXY(IRTPReceiverChannelAsyncDelegate);

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverChannelForRTPReceiver
    //

    interaction IRTPReceiverChannelForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiver, ForRTPReceiver);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList);
      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);
      ZS_DECLARE_TYPEDEF_PTR(IStatsReportTypes::StatsTypeSet, StatsTypeSet)

      static ElementPtr toDebug(ForRTPReceiverPtr object) noexcept;

      static RTPReceiverChannelPtr create(
                                          RTPReceiverPtr receiver,
                                          MediaStreamTrackPtr track,
                                          const Parameters &params,
                                          const RTCPPacketList &packets
                                          ) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual void notifyTransportState(ISecureTransportTypes::States state) noexcept = 0;

      virtual void notifyPacket(RTPPacketPtr packet) noexcept = 0;

      virtual void notifyPackets(RTCPPacketListPtr packets) noexcept = 0;

      virtual void notifyUpdate(const Parameters &params) noexcept = 0;

      virtual bool handlePacket(RTPPacketPtr packet) noexcept = 0;

      virtual bool handlePacket(RTCPPacketPtr packet) noexcept = 0;

      virtual void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverChannelForMediaStreamTrack
    //

    interaction IRTPReceiverChannelForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForMediaStreamTrack, ForMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForMediaStreamTrackPtr object) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual int32_t getAudioSamples(
                                      const size_t numberOfSamples,
                                      const uint8_t numberOfChannels,
                                      void* audioSamples,
                                      size_t& numberOfSamplesOut
                                      ) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverChannelAsyncDelegate
    //

    interaction IRTPReceiverChannelAsyncDelegate
    {
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      virtual void onSecureTransportState(ISecureTransport::States state) = 0;

      virtual void onNotifyPacket(RTPPacketPtr packet) = 0;

      virtual void onNotifyPackets(RTCPPacketListPtr packets) = 0;

      virtual void onUpdate(ParametersPtr params) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverChannelForRTPReceiverChannelMediaBase
    //

    interaction IRTPReceiverChannelForRTPReceiverChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiverChannelMediaBase, ForRTPReceiverChannelMediaBase)

      virtual PUID getID() const noexcept = 0;

      virtual bool sendPacket(RTCPPacketPtr packet) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverChannelForRTPReceiverChannelAudio
    //

    interaction IRTPReceiverChannelForRTPReceiverChannelAudio : public IRTPReceiverChannelForRTPReceiverChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiverChannelAudio, ForRTPReceiverChannelAudio)
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverChannelForRTPReceiverChannelVideo
    //

    interaction IRTPReceiverChannelForRTPReceiverChannelVideo : public IRTPReceiverChannelForRTPReceiverChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiverChannelVideo, ForRTPReceiverChannelVideo)
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPReceiverChannel
    //
    
    class RTPReceiverChannel : public Noop,
                               public MessageQueueAssociator,
                               public SharedRecursiveLock,
                               public IRTPReceiverChannelForRTPReceiver,
                               public IRTPReceiverChannelForMediaStreamTrack,
                               public IRTPReceiverChannelForRTPReceiverChannelAudio,
                               public IRTPReceiverChannelForRTPReceiverChannelVideo,
                               public IWakeDelegate,
                               public zsLib::ITimerDelegate,
                               public IRTPReceiverChannelAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPReceiverChannel;
      friend interaction IRTPReceiverChannelFactory;
      friend interaction IRTPReceiverChannelForRTPReceiver;
      friend interaction IRTPReceiverChannelForRTPReceiverChannelMediaBase;
      friend interaction IRTPReceiverChannelForRTPReceiverChannelAudio;
      friend interaction IRTPReceiverChannelForRTPReceiverChannelVideo;
      friend interaction IRTPReceiverChannelForMediaStreamTrack;

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPReceiverChannel, UseReceiver)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannel, UseMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList)

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state) noexcept;

    public:
      RTPReceiverChannel(
                         const make_private &,
                         IMessageQueuePtr queue,
                         UseReceiverPtr receiver,
                         UseMediaStreamTrackPtr track,
                         const Parameters &params
                         ) noexcept;

    protected:
      RTPReceiverChannel(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init(const RTCPPacketList &packets) noexcept;

    public:
      virtual ~RTPReceiverChannel() noexcept;

      static RTPReceiverChannelPtr convert(ForRTPReceiverPtr object) noexcept;
      static RTPReceiverChannelPtr convert(ForRTPReceiverChannelMediaBasePtr object) noexcept;
      static RTPReceiverChannelPtr convert(ForRTPReceiverChannelAudioPtr object) noexcept;
      static RTPReceiverChannelPtr convert(ForRTPReceiverChannelVideoPtr object) noexcept;
      static RTPReceiverChannelPtr convert(ForMediaStreamTrackPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => IRTPReceiverChannelForRTPReceiver
      //

      static ElementPtr toDebug(RTPReceiverChannelPtr object) noexcept;

      static RTPReceiverChannelPtr create(
                                          RTPReceiverPtr receiver,
                                          MediaStreamTrackPtr track,
                                          const Parameters &params,
                                          const RTCPPacketList &packets
                                          ) noexcept;

      PUID getID() const noexcept override {return mID;}

      void notifyTransportState(ISecureTransportTypes::States state) noexcept override;

      void notifyPacket(RTPPacketPtr packet) noexcept override;

      void notifyPackets(RTCPPacketListPtr packets) noexcept override;

      void notifyUpdate(const Parameters &params) noexcept override;

      bool handlePacket(RTPPacketPtr packet) noexcept override;

      bool handlePacket(RTCPPacketPtr packet) noexcept override;

      void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => IRTPReceiverChannelForMediaStreamTrack
      //

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      // (duplicate) virtual PUID getID() const = 0;

      int32_t getAudioSamples(
                              const size_t numberOfSamples,
                              const uint8_t numberOfChannels,
                              void *audioSamples,
                              size_t& numberOfSamplesOut
                              ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => IRTPReceiverChannelForRTPReceiverChannelMediaBase
      //

      // (duplicate) virtual PUID getID() const = 0;

      bool sendPacket(RTCPPacketPtr packet) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => IRTPReceiverChannelForRTPReceiverChannelAudio
      //

      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => IRTPReceiverChannelForRTPReceiverChannelVideo
      //

      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => IRTPReceiverChannelAsyncDelegate
      //

      void onSecureTransportState(ISecureTransport::States state) override;

      void onNotifyPacket(RTPPacketPtr packet) override;

      void onNotifyPackets(RTCPPacketListPtr packets) override;

      void onUpdate(ParametersPtr params) override;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;

      void cancel() noexcept;

      void setState(States state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;


    protected:
      //-----------------------------------------------------------------------
      //
      // RTPReceiverChannel => (data)
      //

      AutoPUID mID;
      RTPReceiverChannelWeakPtr mThisWeak;
      RTPReceiverChannelPtr mGracefulShutdownReference;


      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      ISecureTransport::States mSecureTransportState {ISecureTransport::State_Pending};

      UseReceiverWeakPtr mReceiver;

      ParametersPtr mParameters;

      Optional<IMediaStreamTrackTypes::Kinds> mKind;
      UseMediaStreamTrackPtr mTrack;

      // NO lock is needed:
#if 0
      UseMediaBasePtr mMediaBase; // valid
      UseAudioPtr mAudio; // either
      UseVideoPtr mVideo; // or valid
#endif //0
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPReceiverChannelFactory
    //

    interaction IRTPReceiverChannelFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;

      static IRTPReceiverChannelFactory &singleton() noexcept;

      virtual RTPReceiverChannelPtr create(
                                           RTPReceiverPtr receiver,
                                           MediaStreamTrackPtr track,
                                           const Parameters &params,
                                           const RTCPPacketList &packets
                                           ) noexcept;
    };

    class RTPReceiverChannelFactory : public IFactory<IRTPReceiverChannelFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPReceiverChannelAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransport::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPReceiverChannelAsyncDelegate::RTCPPacketListPtr, RTCPPacketListPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPReceiverChannelAsyncDelegate::ParametersPtr, ParametersPtr)
ZS_DECLARE_PROXY_METHOD(onSecureTransportState, States)
ZS_DECLARE_PROXY_METHOD(onNotifyPacket, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD(onNotifyPackets, RTCPPacketListPtr)
ZS_DECLARE_PROXY_METHOD(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()

