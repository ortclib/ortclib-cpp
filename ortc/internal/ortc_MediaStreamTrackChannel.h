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

#include <ortc/internal/ortc_IMediaStreamTrackChannel.h>
#include <ortc/internal/ortc_IMediaStreamTrackSubscriber.h>
#include <ortc/internal/ortc_IMediaStreamTrack.h>

#include <ortc/internal/types.h>

#include <ortc/IMediaStreamTrack.h>

#include <zsLib/MessageQueueAssociator.h>

#include <queue>

namespace ortc
{
  namespace internal
  {

    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrackChannel
    #pragma mark

    class MediaStreamTrackChannel : public Noop,
                                    public MessageQueueAssociator,
                                    public SharedRecursiveLock,
                                    public IMediaStreamTrackChannelForMediaStreamTrack,
                                    public IMediaStreamTrackChannelForMediaStreamTrackSubscriberMedia,
                                    public IMediaStreamTrackChannelForMediaStreamTrackSubscriberRTP
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaStreamTrackChannelForMediaStreamTrack;
      friend interaction IMediaStreamTrackChannelForMediaStreamTrackSubscriber;
      friend interaction IMediaStreamTrackChannelForMediaStreamTrackSubscriberMedia;
      friend interaction IMediaStreamTrackChannelForMediaStreamTrackSubscriberRTP;

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriber, ForMediaStreamTrackSubscriber);

      typedef PUID SubscriberObjectID;

      typedef std::map<SubscriberObjectID, IMediaStreamTrackChannelMediaDelegatePtr> MediaDelegateMap;
      typedef std::map<SubscriberObjectID, IMediaStreamTrackChannelRTPDelegatePtr> RTPDelegateMap;

      typedef std::queue<RTPPacketPtr> RTPPacketQueue;
      typedef std::queue<RTCPPacketPtr> RTCPPacketQueue;
      typedef std::queue<AudioFramePtr> AudioFrameQueue;
      typedef std::queue<VideoFramePtr> VideoFrameQueue;

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::ImmutableParameters, ImmutableParameters);

      ZS_DECLARE_PTR(MediaDelegateMap);
      ZS_DECLARE_PTR(RTPDelegateMap);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaStreamTrackChannel, UseMediaStreamTrack);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberForMediaStreamTrackChannel, UseMediaStreamTrackSubscriber);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberMediaForMediaStreamTrackChannel, UseMediaStreamTrackSubscriberMedia);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberRTPForMediaStreamTrackChannel, UseMediaStreamTrackSubscriberRTP);

    public:
      MediaStreamTrackChannel(
                              const make_private &,
                              IMessageQueuePtr queue
                              );

      MediaStreamTrackChannel(Noop, IMessageQueuePtr queue) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

    protected:
      void init();

    public:
      ~MediaStreamTrackChannel();

      MediaStreamTrackChannelPtr convert(ForMediaStreamTrackPtr object);
      MediaStreamTrackChannelPtr convert(ForMediaStreamTrackSubscriberPtr object);
      MediaStreamTrackChannelPtr convert(ForMediaStreamTrackSubscriberMediaPtr object);
      MediaStreamTrackChannelPtr convert(ForMediaStreamTrackSubscriberRTPPtr object);

    protected:

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaChannel (for MediaStreamTrack)
      #pragma mark

      static ForMediaStreamTrackPtr create(
                                           size_t timeIndex,
                                           ImmutableMediaChannelTracePtr trace
                                           );

      PUID getID() const override { return id_; }

      void shutdown() override;

      void notifyAudioFrame(
                            size_t timeIndex,
                            ImmutableMediaChannelTracePtr trace,
                            AudioFramePtr frame
                            ) override;
      void notifyVideoFrame(
                            size_t timeIndex,
                            ImmutableMediaChannelTracePtr trace,
                            VideoFramePtr frame
                            ) override;

      void notifyRTPPacket(
                           size_t timeIndex,
                           ImmutableMediaChannelTracePtr trace,
                           RTPPacketPtr packet
                           ) override;
      void notifyRTCPPacket(
                            size_t timeIndex,
                            ImmutableMediaChannelTracePtr trace,
                            RTPPacketPtr packet
                            ) override;

      void subscribeComplete() override;

      bool shouldCancel(size_t cleanOlderThanTimeIndex) override;

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaChannel (ForMediaStreamTrackSubscriber)
      #pragma mark

      virtual void notifyCancelled(SubscriberObjectID subscriberID) = 0;

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaChannel (ForMediaStreamTrackSubscriberMedia)
      #pragma mark

      virtual void subscribeMedia(
                                 SubscriberObjectID subscriberID,
                                 IMediaStreamTrackChannelMediaDelegatePtr delegate
                                 ) override;

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaChannel (ForMediaStreamTrackSubscriberRTP)
      #pragma mark

      virtual void subscribeRTP(
                                SubscriberObjectID subscriberID,
                                ImmutableParametersPtr params,
                                IMediaStreamTrackChannelRTPDelegatePtr delegate
                                ) override;

    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack::RTPSubscriber => (data)
      #pragma mark

      AutoPUID id_;
      MediaStreamTrackChannelWeakPtr thisWeak_;

      bool shutdown_{ false };

      ImmutableMediaChannelTracePtr trace_;

      size_t lastUsageTimeIndex_ {};

      MediaDelegateMap mediaSubscribers_;
      RTPDelegateMap rtpSubscribers_;

      RTPPacketQueue rtpQueue_;
      RTCPPacketQueue rtcpQueue_;
      AudioFrameQueue audioFrames_;
      VideoFrameQueue videoFrames_;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackChannelFactory
    #pragma mark

    interaction IMediaStreamTrackChannelFactory
    {
      static IMediaStreamTrackChannelFactory &singleton();

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackChannelForMediaStreamTrack, ForMediaStreamTrack);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);

      virtual ForMediaStreamTrackPtr create(
                                            size_t timeIndex,
                                            ImmutableMediaChannelTracePtr trace
                                            );
    };

    class MediaStreamTrackChannelFactory : public IFactory<IMediaStreamTrackChannelFactory> {};

  } // namespace internal
} // namespace ortc
