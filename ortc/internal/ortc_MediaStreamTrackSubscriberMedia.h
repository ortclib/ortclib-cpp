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
    #pragma mark
    #pragma mark MediaStreamTrackSubscriberMedia
    #pragma mark

    class MediaStreamTrackSubscriberMedia : public Noop,
                                            public MessageQueueAssociator,
                                            public SharedRecursiveLock,
                                            public IMediaStreamTrackSubscriberMediaForMediaStreamTrack,
                                            public IMediaStreamTrackSubscriberMediaForMediaStreamTrackChannel,
                                            public IMediaStreamTrackChannelMediaDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaStreamTrackSubscriberForMediaStreamTrack;
      friend interaction IMediaStreamTrackSubscriberMediaForMediaStreamTrack;
      friend interaction IMediaStreamTrackSubscriberForMediaStreamTrackChannel;
      friend interaction IMediaStreamTrackSubscriberMediaForMediaStreamTrackChannel;

      ZS_DECLARE_CLASS_PTR(Subscriber);


      ZS_DECLARE_TYPEDEF_PTR(MediaStreamTrackSubscriberMedia, UseOuter);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriber, UseMediaChannelBase);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriberMedia, UseMediaChannel);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::OverconstrainedError, OverconstrainedError);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);


    public:
      MediaStreamTrackSubscriberMedia(
                                      IMessageQueuePtr queue
                                      );

      MediaStreamTrackSubscriberMedia(Noop, IMessageQueuePtr queue) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

    protected:
      void init();

    public:
      ~MediaStreamTrackSubscriberMedia();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberMedia => IMediaStreamTrackSubscriberForMediaStreamTrack
      #pragma mark

      PUID getID() const override { return id_; }

      void shutdown() override;

      void notifyChannel(
                         UseMediaChannelBasePtr mediaChannel,
                         ImmutableMediaChannelTracePtr trace
                         ) override;

      void notifyTrackMute(bool isMuted) override;
      void notifyTrackEnded() override;
      void notifyOverConstrained(OverconstrainedErrorPtr error) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberMedia => IMediaStreamTrackSubscriberMediaForMediaStreamTrack
      #pragma mark

      static ForMediaStreamTrackPtr create(
                                           IMediaStreamTrackMediaSubscriptionPtr &outSubscription,
                                           IMediaStreamTrackSyncMediaDelegatePtr syncDelegate,
                                           IMediaStreamTrackAsyncMediaDelegatePtr asyncDelegate
                                           );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberMedia => IMediaStreamTrackSubscriberForMediaStreamTrackChannel
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberMedia => IMediaStreamTrackSubscriberMediaForMediaStreamTrackChannel
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackChannelDelegate
      #pragma mark

      void notifyMediaChannelCancelled(ImmutableMediaChannelTracePtr trace) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackChannelMediaDelegate
      #pragma mark


      void notifyMediaChannelAudioFrame(
                                        ImmutableMediaChannelTracePtr trace,
                                        AudioFramePtr frame
                                        ) override;
      void notifyMediaChannelVideoFrame(
                                        ImmutableMediaChannelTracePtr trace,
                                        AudioFramePtr frame
                                        ) override;
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberMedia (for Subscriber)
      #pragma mark

      void notifySubscriberCancel();

    public:


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberMedia::Subscriber
      #pragma mark

      class Subscriber : public IMediaStreamTrackMediaSubscription,
                         public SharedRecursiveLock
      {
      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackSubscriberMedia::Subscriber => (for MediaSubscriber)
        #pragma mark

        static SubscriberPtr create(const SharedRecursiveLock &lock);

        void shutdown();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackSubscriberMedia::Subscriber => IMediaStreamTrackMediaSubscription
        #pragma mark

        virtual PUID getID() const { return id_; }

        virtual void cancel();

        virtual void background();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackSubscriberMedia::Subscriber => (internal)
        #pragma mark

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackSubscriberMedia::Subscriber => (data)
        #pragma mark

        AutoPUID id_;
        SubscriberWeakPtr thisWeak_;

        bool shutdown_ {false};

        UseOuterPtr outer_;

        SubscriberPtr backgroundReference_;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberMedia => (data)
      #pragma mark

      AutoPUID id_;
      MediaStreamTrackSubscriberMediaWeakPtr thisWeak_;

      bool shutdown_{ false };

      SubscriberWeakPtr subscriber_;

      IMediaStreamTrackSyncMediaDelegatePtr syncDelegate_;
      IMediaStreamTrackAsyncMediaDelegatePtr asyncDelegate_;
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackSubscriberMediaFactory
    #pragma mark

    interaction IMediaStreamTrackSubscriberMediaFactory
    {
      static IMediaStreamTrackSubscriberMediaFactory &singleton();

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberMediaForMediaStreamTrack, ForMediaStreamTrack);

      static ForMediaStreamTrackPtr create(
                                           IMediaStreamTrackMediaSubscriptionPtr &outSubscription,
                                           IMediaStreamTrackSyncMediaDelegatePtr syncDelegate,
                                           IMediaStreamTrackAsyncMediaDelegatePtr asyncDelegate
                                           );    
    };

    class MediaStreamTrackSubscriberMediaFactory : public IFactory<IMediaStreamTrackSubscriberMediaFactory> {};

  } // namespace internal
} // namespace ortc
