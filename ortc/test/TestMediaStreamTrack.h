/*

 Copyright (c) 2015, Hookflash Inc.
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


#include <ortc/internal/ortc_MediaDevices.h>
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>

#include <ortc/services/IHelper.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
#include <zsLib/Promise.h>
#include <zsLib/Event.h>
#include <zsLib/Log.h>

#include "config.h"
#include "testing.h"

namespace ortc
{
  namespace test
  {
    namespace mediastreamtrack
    {
      ZS_DECLARE_INTERACTION_PROXY(IFakeReceiverAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeReceiverChannelAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeSenderAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeSenderChannelAsyncDelegate)

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeReceiverAsyncDelegate
      #pragma mark

      interaction IFakeReceiverAsyncDelegate
      {
        virtual ~IFakeReceiverAsyncDelegate() {}
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeReceiverChannelAsyncDelegate
      #pragma mark

      interaction IFakeReceiverChannelAsyncDelegate
      {
        virtual ~IFakeReceiverChannelAsyncDelegate() {}
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeSenderAsyncDelegate
      #pragma mark

      interaction IFakeSenderAsyncDelegate
      {
        virtual ~IFakeSenderAsyncDelegate() {}
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeSenderChannelAsyncDelegate
      #pragma mark

      interaction IFakeSenderChannelAsyncDelegate
      {
        virtual void onNotifyLocalVideoTrackEvent() = 0;
      };
    }
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::test::mediastreamtrack::IFakeReceiverAsyncDelegate)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::mediastreamtrack::IFakeReceiverChannelAsyncDelegate)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::mediastreamtrack::IFakeSenderAsyncDelegate)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::mediastreamtrack::IFakeSenderChannelAsyncDelegate)
ZS_DECLARE_PROXY_METHOD_0(onNotifyLocalVideoTrackEvent)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace mediastreamtrack
    {
      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::Milliseconds;

      ZS_DECLARE_USING_PTR(zsLib, ITimer)
      ZS_DECLARE_USING_PTR(zsLib, Event)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiver)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSender)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(ortc::IMediaDevicesTypes::PromiseWithDeviceList, PromiseWithDeviceList)
      ZS_DECLARE_TYPEDEF_PTR(ortc::IMediaDevicesTypes::PromiseWithMediaStreamTrackList, PromiseWithMediaStreamTrackList)

      ZS_DECLARE_CLASS_PTR(FakeReceiver)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannel)
      ZS_DECLARE_CLASS_PTR(FakeSender)
      ZS_DECLARE_CLASS_PTR(FakeSenderChannel)
      ZS_DECLARE_CLASS_PTR(MediaStreamTrackTester)
      ZS_DECLARE_CLASS_PTR(PromiseWithCertificateCallback)
      ZS_DECLARE_CLASS_PTR(PromiseWithMediaStreamTrackListCallback)
      ZS_DECLARE_CLASS_PTR(PromiseWithDeviceListCallback)

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver
      #pragma mark

      //---------------------------------------------------------------------
      class FakeReceiver : public ortc::internal::RTPReceiver,
                           public IFakeReceiverAsyncDelegate
      {
      protected:
        struct make_private {};

      public:
        friend class MediaStreamTrackTester;

      public:
        FakeReceiver(
                     const make_private &,
                     MediaStreamTrackTesterPtr tester,
                     IMediaStreamTrackTypes::Kinds kind,
                     IMessageQueuePtr queue = IMessageQueuePtr()
                     );
        ~FakeReceiver();

        static FakeReceiverPtr create(
                                      MediaStreamTrackTesterPtr tester,
                                      IMediaStreamTrackTypes::Kinds kind
                                      );

      protected:
        void init();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiver => IRTPReceverForMediaStreamTrack
        #pragma mark

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiver => IFakeReceiverAsyncDelegate
        #pragma mark

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiver => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiver => (data)
        #pragma mark

        FakeReceiverWeakPtr mThisWeak;

        MediaStreamTrackTesterWeakPtr mTester;

        UseChannelPtr mChannel;
      };


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeReceiverChannel : public ortc::internal::RTPReceiverChannel,
                                  public IFakeReceiverChannelAsyncDelegate
      {
      protected:
        struct make_private {};

      public:
        friend class MediaStreamTrackTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::IMediaStreamTrackForRTPReceiverChannel, UseMediaStreamTrack)

      public:
        FakeReceiverChannel(
                            const make_private &,
                            MediaStreamTrackTesterPtr tester,
                            UseReceiverPtr receiver,
                            IMediaStreamTrackPtr track,
                            IMessageQueuePtr queue = IMessageQueuePtr()
                            );
        ~FakeReceiverChannel();

        static FakeReceiverChannelPtr create(
                                             MediaStreamTrackTesterPtr tester,
                                             UseReceiverPtr receiver,
                                             IMediaStreamTrackPtr track
                                             );

      protected:
        void init();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => IRTPReceiverChannelForMediaStreamTrack
        #pragma mark

        virtual ElementPtr toDebug() const override;

        virtual int32_t getAudioSamples(
                                        const size_t numberOfSamples,
                                        const uint8_t numberOfChannels,
                                        void* audioSamples,
                                        size_t& numberOfSamplesOut
                                        ) override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => IFakeReceiverChannelAsyncDelegate
        #pragma mark

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => ITimerDelegate
        #pragma mark

        virtual void onTimer(ITimerPtr timer) override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

      protected:
        FakeReceiverChannelWeakPtr mThisWeak;

        MediaStreamTrackTesterWeakPtr mTester;

        UseMediaStreamTrackPtr mTrack;

        ITimerPtr mTimer;

        ULONG mSentVideoFrames;
        ULONG mSentAudioSamples;
      };


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeSender : public ortc::internal::RTPSender,
                         public IFakeSenderAsyncDelegate
      {
      protected:
        struct make_private {};

      public:
        friend class MediaStreamTrackTester;

      public:
        FakeSender(
                   const make_private &,
                   MediaStreamTrackTesterPtr tester,
                   IMediaStreamTrackPtr track,
                   IMessageQueuePtr queue = IMessageQueuePtr()
                   );
        ~FakeSender();

        static FakeSenderPtr create(
                                    MediaStreamTrackTesterPtr tester,
                                    IMediaStreamTrackPtr track
                                    );

      protected:
        void init();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSender => IRTPSenderForMediaStreamTrack
        #pragma mark

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSender => IFakeSenderAsyncDelegate
        #pragma mark

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSender => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSender => (data)
        #pragma mark

        FakeSenderWeakPtr mThisWeak;

        MediaStreamTrackTesterWeakPtr mTester;

        UseChannelPtr mChannel;
      };


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeSenderChannel : public ortc::internal::RTPSenderChannel,
                                public IFakeSenderChannelAsyncDelegate
      {
      protected:
        struct make_private {};

      public:
        friend class MediaStreamTrackTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::IMediaStreamTrackForRTPSenderChannel, UseMediaStreamTrack)

      public:
        FakeSenderChannel(
                          const make_private &,
                          MediaStreamTrackTesterPtr tester,
                          UseSenderPtr sender,
                          IMediaStreamTrackPtr track,
                          IMessageQueuePtr queue = IMessageQueuePtr()
                          );
        ~FakeSenderChannel();

        static FakeSenderChannelPtr create(
                                           MediaStreamTrackTesterPtr tester,
                                           UseSenderPtr sender,
                                           IMediaStreamTrackPtr track
                                           );

      protected:
        void init();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => IRTPSenderChannelForMediaStreamTrack
        #pragma mark

        virtual ElementPtr toDebug() const override;

        //virtual void sendVideoFrame(
        //                            const uint8_t* videoFrame,
        //                            const size_t videoFrameSize
        //                            ) override;

        //virtual void sendAudioSamples(
        //                              const void* audioSamples,
        //                              const size_t numberOfSamples,
        //                              const uint8_t numberOfChannels
        //                              ) override;
      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => IFakeSenderChannelAsyncDelegate
        #pragma mark
          
        virtual void onNotifyLocalVideoTrackEvent() override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

      protected:
        FakeSenderChannelWeakPtr mThisWeak;

        MediaStreamTrackTesterWeakPtr mTester;

        UseMediaStreamTrackPtr mTrack;

        ULONG mReceivedVideoFrames;
        ULONG mReceivedAudioSamples;
      };


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester
      #pragma mark

      //-----------------------------------------------------------------------
      class MediaStreamTrackTester : public SharedRecursiveLock,
                                     public zsLib::MessageQueueAssociator,
                                     public IMediaStreamTrackDelegate
      {
      public:
        friend class FakeReceiver;
        friend class FakeReceiverChannel;
        friend class FakeSender;
        friend class FakeSenderChannel;
        friend class PromiseWithCertificateCallback;
        friend class PromiseWithMediaStreamTrackListCallback;
        friend class PromiseWithDeviceListCallback;

        ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::Parameters, Parameters)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTCPPacket, RTCPPacket)
        typedef std::list<RTCPPacketPtr> RTCPPacketList;

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiver, RTPReceiver)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverFactory, RTPReceiverFactory)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel, RTPReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelFactory, RTPReceiverChannelFactory)

        ZS_DECLARE_CLASS_PTR(OverrideReceiverFactory)
        ZS_DECLARE_CLASS_PTR(OverrideReceiverChannelFactory)

        friend class OverrideReceiverFactory;
        friend class OverrideReceiverChannelFactory;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester::OverrideReceiverFactory
        #pragma mark

        class OverrideReceiverFactory : public RTPReceiverFactory
        {
        public:
          static OverrideReceiverFactoryPtr create(MediaStreamTrackTesterPtr tester);

          virtual RTPReceiverPtr create(
                                        IRTPReceiverDelegatePtr delegate,
                                        IMediaStreamTrackTypes::Kinds kind,
                                        IRTPTransportPtr transport,
                                        IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                        ) override;

        protected:
          MediaStreamTrackTesterWeakPtr mTester;
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester::OverrideReceiverChannelFactory
        #pragma mark

        class OverrideReceiverChannelFactory : public RTPReceiverChannelFactory
        {
        public:
          static OverrideReceiverChannelFactoryPtr create(MediaStreamTrackTesterPtr tester);

          virtual RTPReceiverChannelPtr create(
                                               RTPReceiverPtr receiver,
                                               MediaStreamTrackPtr track,
                                               const Parameters &params,
                                               const RTCPPacketList &packets
                                               ) override;

        protected:
          MediaStreamTrackTesterWeakPtr mTester;
        };

      protected:
        struct make_private {};

      public:
        struct Expectations {
          // general

          // receiver related

          // receiver cannel related

          ULONG mSentVideoFrames;
          ULONG mSentAudioSamples;

          // sender related

          // sender channel related

          ULONG mReceivedVideoFrames;
          ULONG mReceivedAudioSamples;

          bool operator==(const Expectations &op2) const;
        };

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester (api)
        #pragma mark

        static MediaStreamTrackTesterPtr create(
                                                IMessageQueuePtr queue,
                                                bool overrideFactories,
                                                void* videoSurface
                                                );

        MediaStreamTrackTester(
                               IMessageQueuePtr queue,
                               bool overrideFactories,
                               void* videoSurface
                               );
        ~MediaStreamTrackTester();

        void init();

        bool matches(const Expectations &op2);

        void close();
        void closeByReset();

        void startLocalVideoTrack();
        void startRemoteVideoTrack();
        void startLocalAudioTrack();
        void startRemoteAudioTrack();

        Expectations getExpectations() const;

        void createReceiverChannel(
                                   const char *receiverChannelID,
                                   const char *parametersID
                                   );

        void stop(const char *senderOrReceiverChannelID);

        void attach(
                    const char *receiverChannelID,
                    FakeReceiverChannelPtr receiver
                    );

        FakeReceiverChannelPtr detachReceiverChannel(const char *receiverChannelID);

      protected:

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester::IMediaStreamTrackDelegate
        #pragma mark

        virtual void onMediaStreamTrackMute(
                                            IMediaStreamTrackPtr track,
                                            bool isMuted
                                            ) override;

        virtual void onMediaStreamTrackEnded(IMediaStreamTrackPtr track) override;
        virtual void onMediaStreamTrackOverConstrained(
                                                       IMediaStreamTrackPtr track,
                                                       OverconstrainedErrorPtr error
                                                       ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester => (friend fake receiver)
        #pragma mark

        RTPReceiverPtr create(
                              IRTPReceiverDelegatePtr delegate,
                              IMediaStreamTrackTypes::Kinds kind,
                              IRTPTransportPtr transport,
                              IRTCPTransportPtr rtcpTransport
                              );
       
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester => (friend fake receiver channel)
        #pragma mark

        RTPReceiverChannelPtr create(
                                     RTPReceiverPtr receiver,
                                     MediaStreamTrackPtr track,
                                     const Parameters &params,
                                     const RTCPPacketList &packets
                                     );

        void notifySentVideoFrame();
        void notifyRemoteVideoTrackEvent();
        void notifySentAudioSamples(int numberOfSamples);
        void notifyRemoteAudioTrackEvent();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester => (friend fake sender)
        #pragma mark

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester => (friend fake sender channel)
        #pragma mark

        void notifyReceivedVideoFrame();
        void notifyLocalVideoTrackEvent();
        void notifyReceivedAudioSamples(int numberOfSamples);
        void notifyLocalAudioTrackEvent();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

        FakeReceiverPtr getReceiver(const char *receiverID);
        FakeReceiverChannelPtr getReceiverChannel(const char *receiverID);

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrackTester => (data)
        #pragma mark

        AutoPUID mID;
        MediaStreamTrackTesterWeakPtr mThisWeak;

        bool mOverrideFactories{ false };

        void* mVideoSurface;

        EventPtr mLocalVideoTrackEvent;
        EventPtr mRemoteVideoTrackEvent;
        EventPtr mLocalAudioTrackEvent;
        EventPtr mRemoteAudioTrackEvent;

        PromiseWithMediaStreamTrackListPtr mVideoPromiseWithMediaStreamTrackList;
        PromiseWithDeviceListPtr mVideoPromiseWithDeviceList;
        PromiseWithMediaStreamTrackListPtr mAudioPromiseWithMediaStreamTrackList;
        PromiseWithDeviceListPtr mAudioPromiseWithDeviceList;

        MediaStreamTrackPtr mLocalVideoMediaStreamTrack;
        MediaStreamTrackPtr mRemoteVideoMediaStreamTrack;
        MediaStreamTrackPtr mLocalAudioMediaStreamTrack;
        MediaStreamTrackPtr mRemoteAudioMediaStreamTrack;

        FakeSenderPtr mVideoSender;
        FakeReceiverPtr mVideoReceiver;
        FakeSenderPtr mAudioSender;
        FakeReceiverPtr mAudioReceiver;

        Expectations mExpectations;
      };
    }
  }
}
