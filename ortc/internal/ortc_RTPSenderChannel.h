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

#include <ortc/IDTLSTransport.h>
#include <ortc/IICETransport.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IMediaStreamTrack.h>
#include <ortc/IStatsProvider.h>
#include <ortc/RTPPacket.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>

#include <webrtc/api/video/video_frame.h>

#define ORTC_SETTING_RTP_SENDER_CHANNEL_RETAG_RTP_PACKETS_AFTER_SSRC_NOT_SENT_IN_SECONDS "ortc/rtp-sender-channel/retag-rtp-packets-after-ssrc-not-sent-in-seconds"
#define ORTC_SETTING_RTP_SENDER_CHANNEL_TAG_MID_RID_IN_RTCP_SDES "ortc/rtp-sender-channel/tag-mid-rid-in-rtcp-sdes"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSender);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSenderChannelMediaBase);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSenderChannelAudio);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSenderChannelVideo);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForMediaStreamTrack);

    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForRTPSenderChannel);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelMediaBaseForRTPSenderChannel);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelAudioForRTPSenderChannel);
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelVideoForRTPSenderChannel);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannel);

    ZS_DECLARE_INTERACTION_PROXY(IRTPSenderChannelAsyncDelegate);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderChannelForRTPSender
    //

    interaction IRTPSenderChannelForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSender, ForRTPSender);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList);

      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);
      ZS_DECLARE_TYPEDEF_PTR(IStatsReportTypes::StatsTypeSet, StatsTypeSet);

      static ElementPtr toDebug(ForRTPSenderPtr object) noexcept;

      static RTPSenderChannelPtr create(
                                        RTPSenderPtr sender,
                                        MediaStreamTrackPtr track,
                                        const Parameters &params
                                        ) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual void notifyTrackChanged(MediaStreamTrackPtr track) noexcept = 0;

      virtual void notifyTransportState(ISecureTransportTypes::States state) noexcept = 0;

      virtual void notifyPackets(RTCPPacketListPtr packets) noexcept = 0;

      virtual void notifyUpdate(const Parameters &params) noexcept = 0;

      virtual bool handlePacket(RTCPPacketPtr packet) noexcept = 0;

      virtual void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats) noexcept = 0;

      virtual void insertDTMF(
                              const char *tones,
                              Milliseconds duration,
                              Milliseconds interToneGap
                              ) noexcept = 0;

      virtual String toneBuffer() const noexcept = 0;
      virtual Milliseconds duration() const noexcept = 0;
      virtual Milliseconds interToneGap() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderChannelForRTPSenderChannelMediaBase
    //

    interaction IRTPSenderChannelForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSenderChannelMediaBase, ForRTPSenderChannelMediaBase)

      virtual PUID getID() const noexcept = 0;

      virtual bool sendPacket(RTPPacketPtr packet) noexcept = 0;

      virtual bool sendPacket(RTCPPacketPtr packet) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderChannelForRTPSenderChannelAudio
    //

    interaction IRTPSenderChannelForRTPSenderChannelAudio : public IRTPSenderChannelForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSenderChannelAudio, ForRTPSenderChannelAudio)

      virtual void notifyDTMFSenderToneChanged(const char *tone) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderChannelForRTPSenderChannelVideo
    //

    interaction IRTPSenderChannelForRTPSenderChannelVideo : public IRTPSenderChannelForRTPSenderChannelMediaBase
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSenderChannelVideo, ForRTPSenderChannelVideo)
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderChannelForMediaStreamTrack
    //

    interaction IRTPSenderChannelForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForMediaStreamTrack, ForMediaStreamTrack);
      ZS_DECLARE_TYPEDEF_PTR(webrtc::VideoFrame, VideoFrame);

      static ElementPtr toDebug(ForMediaStreamTrackPtr object) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual int32_t sendAudioSamples(
                                       const void* audioSamples,
                                       const size_t numberOfSamples,
                                       const uint8_t numberOfChannels
                                       ) noexcept = 0;
      
      virtual void sendVideoFrame(VideoFramePtr videoFrame) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderChannelAsyncDelegate
    //

    interaction IRTPSenderChannelAsyncDelegate
    {
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannel, UseMediaStreamTrack);

      virtual void onTrackChanged(UseMediaStreamTrackPtr track) = 0;

      virtual void onSecureTransportState(ISecureTransport::States state) = 0;

      virtual void onNotifyPackets(RTCPPacketListPtr packets) = 0;

      virtual void onUpdate(ParametersPtr params) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // RTPSenderChannel
    //
    
    class RTPSenderChannel : public Noop,
                             public MessageQueueAssociator,
                             public SharedRecursiveLock,
                             public IRTPSenderChannelForRTPSender,
                             public IRTPSenderChannelForRTPSenderChannelAudio,
                             public IRTPSenderChannelForRTPSenderChannelVideo,
                             public IRTPSenderChannelForMediaStreamTrack,
                             public IWakeDelegate,
                             public zsLib::ITimerDelegate,
                             public IRTPSenderChannelAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPSenderChannel;
      friend interaction IRTPSenderChannelFactory;
      friend interaction IRTPSenderChannelForRTPSender;
      friend interaction IRTPSenderChannelForRTPSenderChannelMediaBase;
      friend interaction IRTPSenderChannelForRTPSenderChannelAudio;
      friend interaction IRTPSenderChannelForRTPSenderChannelVideo;
      friend interaction IRTPSenderChannelForMediaStreamTrack;

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPSenderChannel, UseSender);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannel, UseMediaStreamTrack);
      ZS_DECLARE_TYPEDEF_PTR(webrtc::VideoFrame, VideoFrame);

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelMediaBaseForRTPSenderChannel, UseMediaBase);
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelAudioForRTPSenderChannel, UseAudio);
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelVideoForRTPSenderChannel, UseVideo);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList);

      typedef IRTPTypes::SSRCType SSRCType;
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtensionParameters, HeaderExtensionParameters)

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state) noexcept;

      ZS_DECLARE_STRUCT_PTR(TaggingInfo);

      struct TaggingInfo
      {
        SSRCType mSSRC {};

        Time mLastSentPacket {};

        bool mReceiverAck {};
        WORD mSequenceNumberFirst {};
        WORD mSequenceNumberLast {};
      };

      typedef std::map<SSRCType, TaggingInfoPtr> TaggingMap;

    public:
      RTPSenderChannel(
                       const make_private &,
                       IMessageQueuePtr queue,
                       UseSenderPtr sender,
                       UseMediaStreamTrackPtr track,
                       const Parameters &params
                       ) noexcept;

    protected:
      RTPSenderChannel(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

    public:
      virtual ~RTPSenderChannel() noexcept;

      static RTPSenderChannelPtr convert(ForRTPSenderPtr object) noexcept;
      static RTPSenderChannelPtr convert(ForRTPSenderChannelMediaBasePtr object) noexcept;
      static RTPSenderChannelPtr convert(ForRTPSenderChannelAudioPtr object) noexcept;
      static RTPSenderChannelPtr convert(ForRTPSenderChannelVideoPtr object) noexcept;
      static RTPSenderChannelPtr convert(ForMediaStreamTrackPtr object) noexcept;


    protected:
      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => IRTPSenderChannelForRTPSender
      //

      static ElementPtr toDebug(RTPSenderChannelPtr receiver) noexcept;

      static RTPSenderChannelPtr create(
                                        RTPSenderPtr sender,
                                        MediaStreamTrackPtr track,
                                        const Parameters &params
                                        ) noexcept;

      PUID getID() const noexcept override {return mID;}

      void notifyTrackChanged(MediaStreamTrackPtr track) noexcept override;

      void notifyTransportState(ISecureTransportTypes::States state) noexcept override;

      void notifyPackets(RTCPPacketListPtr packets) noexcept override;

      void notifyUpdate(const Parameters &params) noexcept override;

      bool handlePacket(RTCPPacketPtr packet) noexcept override;

      void requestStats(PromiseWithStatsReportPtr promise, const StatsTypeSet &stats) noexcept override;

      virtual void insertDTMF(
                              const char *tones,
                              Milliseconds duration,
                              Milliseconds interToneGap
                              ) noexcept override;

      String toneBuffer() const noexcept override;
      Milliseconds duration() const noexcept override;
      Milliseconds interToneGap() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => IRTPSenderChannelForRTPSenderChannelMediaBase
      //

      // (duplicate) virtual PUID getID() const noexcept = 0;

      bool sendPacket(RTPPacketPtr packet) noexcept override;

      bool sendPacket(RTCPPacketPtr packet) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => IRTPSenderChannelForRTPSenderChannelAudio
      //

      void notifyDTMFSenderToneChanged(const char *tone) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => IRTPSenderChannelForRTPSenderChannelVideo
      //

      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => IRTPSenderChannelForMediaStreamTrack
      //

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr receiver) noexcept;

      // (duplicate) virtual PUID getID() const noexcept= 0;

      int32_t sendAudioSamples(
                               const void* audioSamples,
                               const size_t numberOfSamples,
                               const uint8_t numberOfChannels
                               ) noexcept override;

      void sendVideoFrame(VideoFramePtr videoFrame) noexcept override;

      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => ITimerDelegate
      //

      void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => IRTPSenderChannelAsyncDelegate
      //

      void onTrackChanged(UseMediaStreamTrackPtr track) override;

      void onSecureTransportState(ISecureTransport::States state) override;

      void onNotifyPackets(RTCPPacketListPtr packets) override;

      void onUpdate(ParametersPtr params) override;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => (internal)
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

      void setupTagging() noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // RTPSenderChannel => (data)
      //

      AutoPUID mID;
      RTPSenderChannelWeakPtr mThisWeak;
      RTPSenderChannelPtr mGracefulShutdownReference;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      ISecureTransport::States mSecureTransportState {ISecureTransport::State_Pending};

      UseSenderWeakPtr mSender;

      ParametersPtr mParameters;

      // tagging options
      HeaderExtensionParametersPtr mMuxHeader;
      HeaderExtensionParametersPtr mRIDHeader;
      std::atomic<bool> mIsTagging {false};
      String mMuxID;
      String mRID;
      String mHeaderHash;

      Seconds mRetagAfterInSeconds {};
      bool mTagSDES {false};
      TaggingMap mTaggings;

      Optional<IMediaStreamTrackTypes::Kinds> mKind;
      UseMediaStreamTrackPtr mTrack;

      // NO lock is needed:
      UseMediaBasePtr mMediaBase; // valid
      UseAudioPtr mAudio; // either
      UseVideoPtr mVideo; // or valid
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IRTPSenderChannelFactory
    //

    interaction IRTPSenderChannelFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      static IRTPSenderChannelFactory &singleton() noexcept;

      virtual RTPSenderChannelPtr create(
                                         RTPSenderPtr sender,
                                         MediaStreamTrackPtr track,
                                         const Parameters &params
                                         ) noexcept;
    };

    class RTPSenderChannelFactory : public IFactory<IRTPSenderChannelFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPSenderChannelAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransport::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPSenderChannelAsyncDelegate::RTCPPacketListPtr, RTCPPacketListPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPSenderChannelAsyncDelegate::ParametersPtr, ParametersPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaStreamTrackForRTPSenderChannel, UseMediaStreamTrack)
ZS_DECLARE_PROXY_METHOD(onTrackChanged, UseMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD(onSecureTransportState, States)
ZS_DECLARE_PROXY_METHOD(onNotifyPackets, RTCPPacketListPtr)
ZS_DECLARE_PROXY_METHOD(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()
