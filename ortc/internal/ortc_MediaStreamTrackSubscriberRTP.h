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

#include <ortc/internal/ortc_IMediaStreamTrackSubscriber.h>
#include <ortc/internal/ortc_IMediaStreamTrack.h>
#include <ortc/internal/ortc_IMediaStreamTrackChannel.h>

#include <ortc/internal/types.h>

#include <ortc/IRTPTypes.h>

#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStreamTrackSubscriberRTP
    //

    class MediaStreamTrackSubscriberRTP : public Noop,
                                          public MessageQueueAssociator,
                                          public SharedRecursiveLock,
                                          public IMediaStreamTrackSubscriberRTPForMediaStreamTrack,
                                          public IMediaStreamTrackSubscriberRTPForMediaStreamTrackChannel,
                                          public IMediaStreamTrackChannelMediaDelegate,
                                          public IMediaStreamTrackChannelRTPDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaStreamTrackSubscriberForMediaStreamTrack;
      friend interaction IMediaStreamTrackSubscriberRTPForMediaStreamTrack;
      friend interaction IMediaStreamTrackSubscriberForMediaStreamTrackChannel;
      friend interaction IMediaStreamTrackSubscriberRTPForMediaStreamTrackChannel;

      ZS_DECLARE_CLASS_PTR(Subscriber);

      ZS_DECLARE_TYPEDEF_PTR(MediaStreamTrackSubscriberRTP, UseOuter);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriber, UseMediaChannelBase);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriberRTP, UseMediaChannel);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::OverconstrainedError, OverconstrainedError);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

    public:
      MediaStreamTrackSubscriberRTP(
                                    IMessageQueuePtr queue
                                    ) noexcept;

      MediaStreamTrackSubscriberRTP(Noop, IMessageQueuePtr queue) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

    protected:
      void init() noexcept;

    public:
      ~MediaStreamTrackSubscriberRTP() noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => IMediaStreamTrackSubscriberForMediaStreamTrack
      //

      PUID getID() const noexcept override { return id_; }

      void shutdown() noexcept override;

      void notifyChannel(
                         UseMediaChannelBasePtr mediaChannel,
                         ImmutableMediaChannelTracePtr trace
                         ) noexcept override;

      void notifyTrackMute(bool isMuted) noexcept override;
      void notifyTrackEnded() noexcept override;
      void notifyOverConstrained(OverconstrainedErrorPtr error) noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => IMediaStreamTrackSubscriberRTPForMediaStreamTrack
      //

      static ForMediaStreamTrackPtr create(
                                           IMediaStreamTrackMediaSubscriptionPtr &outSubscription,
                                           IMediaStreamTrackRTPDelegatePtr delegate
                                           ) noexcept;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => IMediaStreamTrackSubscriberForMediaStreamTrackChannel
      //

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => IMediaStreamTrackSubscriberRTPForMediaStreamTrackChannel
      //

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => IMediaStreamTrackChannelDelegate
      //

      void notifyMediaChannelCancelled(ImmutableMediaChannelTracePtr trace) noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => IMediaStreamTrackChannelMediaDelegate
      //


      void notifyMediaChannelAudioFrame(
                                        ImmutableMediaChannelTracePtr trace,
                                        AudioFramePtr frame
                                        ) noexcept override;
      void notifyMediaChannelVideoFrame(
                                        ImmutableMediaChannelTracePtr trace,
                                        AudioFramePtr frame
                                        ) noexcept override;


      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => IMediaStreamTrackChannelRTPDelegate
      //

      
      void notifyMediaChannelRTPPacket(
                                       ImmutableMediaChannelTracePtr trace,
                                       RTPPacketPtr packet
                                       ) noexcept override;
      void notifyMediaChannelRTCPPacket(
                                        ImmutableMediaChannelTracePtr trace,
                                        RTPPacketPtr packet
                                        ) noexcept override;


      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => (for Subscriber)
      //

      void notifySubscriberCancel() noexcept;

    public:
      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP::Subscriber
      //

      class Subscriber : public IMediaStreamTrackMediaSubscription,
                         public SharedRecursiveLock
      {
      public:
        //---------------------------------------------------------------------
        //
        // MediaStreamTrackSubscriberRTP::Subscriber => (for MediaSubscriber)
        //

        static SubscriberPtr create(const SharedRecursiveLock &lock) noexcept;

        void shutdown() noexcept;

      protected:
        //---------------------------------------------------------------------
        //
        // MediaStreamTrackSubscriberRTP::Subscriber => IMediaStreamTrackMediaSubscription
        //

        virtual PUID getID() const noexcept { return id_; }

        virtual void cancel() noexcept;

        virtual void background() noexcept;

        //---------------------------------------------------------------------
        //
        // MediaStreamTrackSubscriberRTP::Subscriber => (internal)
        //

      protected:
        //---------------------------------------------------------------------
        //
        // MediaStreamTrackSubscriberRTP::Subscriber => (data)
        //

        AutoPUID id_;
        SubscriberWeakPtr thisWeak_;

        bool shutdown_ {false};

        UseOuterPtr outer_;

        SubscriberPtr backgroundReference_;
      };

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaStreamTrackSubscriberRTP => (data)
      //

      AutoPUID id_;
      MediaStreamTrackSubscriberRTPWeakPtr thisWeak_;

      bool shutdown_{ false };

      SubscriberWeakPtr subscriber_;

      IMediaStreamTrackRTPDelegatePtr delegate_;

      ParametersPtr parameters_;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackSubscriberRTPFactory
    //

    interaction IMediaStreamTrackSubscriberRTPFactory
    {
      static IMediaStreamTrackSubscriberRTPFactory &singleton() noexcept;

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberRTPForMediaStreamTrack, ForMediaStreamTrack);

      static ForMediaStreamTrackPtr create(
                                           IMediaStreamTrackMediaSubscriptionPtr &outSubscription,
                                           IMediaStreamTrackRTPDelegatePtr delegate
                                           ) noexcept;
    };

    class MediaStreamTrackSubscriberRTPFactory : public IFactory<IMediaStreamTrackSubscriberRTPFactory> {};


  } // namespace internal
} // namespace ortc
