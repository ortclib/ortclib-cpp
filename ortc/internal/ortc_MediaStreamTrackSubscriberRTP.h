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
    #pragma mark MediaStreamTrackSubscriberRTP
    #pragma mark

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
                                    );

      MediaStreamTrackSubscriberRTP(Noop, IMessageQueuePtr queue) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

    protected:
      void init();

    public:
      ~MediaStreamTrackSubscriberRTP();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackSubscriberForMediaStreamTrack
      #pragma mark

      virtual PUID getID() const override { return id_; }

      virtual void shutdown() override;

      virtual void notifyChannel(
                                 UseMediaChannelBasePtr mediaChannel,
                                 ImmutableMediaChannelTracePtr trace
                                 ) override;

      virtual void notifyTrackMute(bool isMuted) override;
      virtual void notifyTrackEnded() override;
      virtual void notifyOverConstrained(OverconstrainedErrorPtr error) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackSubscriberRTPForMediaStreamTrack
      #pragma mark

      static ForMediaStreamTrackPtr create(
                                           IMediaStreamTrackMediaSubscriptionPtr &outSubscription,
                                           IMediaStreamTrackRTPDelegatePtr delegate
                                           );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackSubscriberForMediaStreamTrackChannel
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackSubscriberRTPForMediaStreamTrackChannel
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackChannelDelegate
      #pragma mark

      virtual void notifyMediaChannelCancelled(ImmutableMediaChannelTracePtr trace) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackChannelMediaDelegate
      #pragma mark


      virtual void notifyMediaChannelAudioFrame(
                                                ImmutableMediaChannelTracePtr trace,
                                                AudioFramePtr frame
                                                ) override;
      virtual void notifyMediaChannelVideoFrame(
                                                ImmutableMediaChannelTracePtr trace,
                                                AudioFramePtr frame
                                                ) override;
      

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => IMediaStreamTrackChannelRTPDelegate
      #pragma mark

      
      virtual void notifyMediaChannelRTPPacket(
                                               ImmutableMediaChannelTracePtr trace,
                                               RTPPacketPtr packet
                                               ) override;
      virtual void notifyMediaChannelRTCPPacket(
                                                ImmutableMediaChannelTracePtr trace,
                                                RTPPacketPtr packet
                                                ) override;


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP => (for Subscriber)
      #pragma mark

      void notifySubscriberCancel();

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackSubscriberRTP::Subscriber
      #pragma mark

      class Subscriber : public IMediaStreamTrackMediaSubscription,
                         public SharedRecursiveLock
      {
      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackSubscriberRTP::Subscriber => (for MediaSubscriber)
        #pragma mark

        static SubscriberPtr create(const SharedRecursiveLock &lock);

        void shutdown();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackSubscriberRTP::Subscriber => IMediaStreamTrackMediaSubscription
        #pragma mark

        virtual PUID getID() const { return id_; }

        virtual void cancel();

        virtual void background();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackSubscriberRTP::Subscriber => (internal)
        #pragma mark

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackSubscriberRTP::Subscriber => (data)
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
      #pragma mark MediaStreamTrackSubscriberRTP => (data)
      #pragma mark

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
    #pragma mark
    #pragma mark IMediaStreamTrackSubscriberRTPFactory
    #pragma mark

    interaction IMediaStreamTrackSubscriberRTPFactory
    {
      static IMediaStreamTrackSubscriberRTPFactory &singleton();

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberRTPForMediaStreamTrack, ForMediaStreamTrack);

      static ForMediaStreamTrackPtr create(
                                           IMediaStreamTrackMediaSubscriptionPtr &outSubscription,
                                           IMediaStreamTrackRTPDelegatePtr delegate
                                           );    
    };

    class MediaStreamTrackSubscriberRTPFactory : public IFactory<IMediaStreamTrackSubscriberRTPFactory> {};


  } // namespace internal
} // namespace ortc
