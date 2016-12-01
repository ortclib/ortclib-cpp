/*
 
 Copyright (c) 2016, Hookflash Inc.
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




#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>

#include <ortc/services/IHelper.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
#include <zsLib/ITimer.h>
#include <zsLib/Log.h>

#include "config.h"
#include "testing.h"

namespace ortc
{
  namespace test
  {
    namespace rtpchannelvideo
    {
      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::Milliseconds;

      ZS_DECLARE_USING_PTR(zsLib, ITimer)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPPacket)
      ZS_DECLARE_USING_PTR(ortc::internal, RTCPPacket)

      ZS_DECLARE_CLASS_PTR(FakeMediaStreamTrack)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannel)
      ZS_DECLARE_CLASS_PTR(FakeSenderChannel)
      ZS_DECLARE_CLASS_PTR(RTPChannelVideoTester)


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack
      #pragma mark

        //-----------------------------------------------------------------------
      class FakeMediaStreamTrack : public ortc::internal::MediaStreamTrack,
                                   public webrtc::VideoCaptureDataCallback
      {
      public:
        friend class RTPChannelVideoTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransport, ISecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelForMediaStreamTrack, UseReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelForMediaStreamTrack, UseSenderChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelVideoForMediaStreamTrack, UseReceiverChannelVideo)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelVideoForMediaStreamTrack, UseSenderChannelVideo)

      public:
        FakeMediaStreamTrack(IMessageQueuePtr queue, bool remote);
        ~FakeMediaStreamTrack();

        static FakeMediaStreamTrackPtr create(
                                              IMessageQueuePtr queue,
                                              bool remote
                                              );


        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeMediaStreamTrack => IMediaStreamTrack
        #pragma mark

        virtual void stop() override;

        virtual void setVideoRenderCallback(IMediaStreamTrackRenderCallbackPtr callback) override;

        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
        #pragma mark

        // (duplicate) virtual PUID getID() const = 0;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelVideo
        #pragma mark

        //virtual void renderVideoFrame(const webrtc::VideoFrame& videoFrame) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPSenderChannelVideo
        #pragma mark

        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeMediaStreamTrack => webrtc::VideoCaptureDataCallback
        #pragma mark

        virtual void OnIncomingCapturedFrame(const int32_t id, const webrtc::VideoFrame& videoFrame) override;

        virtual void OnCaptureDelayChanged(const int32_t id, const int32_t delay) override;


        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeMediaStreamTrack => (friend RTPChannelTester)
        #pragma mark

        void setTransport(RTPChannelVideoTesterPtr tester);

        void linkReceiverChannel(UseReceiverChannelPtr channel);

        void linkSenderChannel(UseSenderChannelPtr channel);

        void linkReceiverChannelVideo(UseReceiverChannelVideoPtr channelVideo);

        void linkSenderChannelVideo(UseSenderChannelVideoPtr channelVideo);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeMediaStreamTrack => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

        ElementPtr toDebug() const override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeMediaStreamTrack => (data)
        #pragma mark

        FakeMediaStreamTrackWeakPtr mThisWeak;

        IMediaStreamTrackTypes::Kinds mKind{ IMediaStreamTrackTypes::Kind_Video };
        bool mRemote;

        RTPChannelVideoTesterWeakPtr mTester;

        UseReceiverChannelWeakPtr mReceiverChannel;
        UseSenderChannelWeakPtr mSenderChannel;
        UseReceiverChannelVideoWeakPtr mReceiverChannelVideo;
        UseSenderChannelVideoWeakPtr mSenderChannelVideo;

        webrtc::VideoCaptureModule* mVideoCaptureModule;
        //webrtc::VideoRender* mVideoRenderModule;
        webrtc::VideoRenderCallback* mVideoRendererCallback;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeReceiverChannel : public ortc::internal::RTPReceiverChannel
      {
      public:
        friend class RTPChannelVideoTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelVideo, RTPReceiverChannelVideo)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelVideoForRTPReceiverChannel, UseReceiverChannelVideo)
        ZS_DECLARE_TYPEDEF_PTR(internal::IMediaStreamTrackForRTPReceiverChannelVideo, UseMediaStreamTrack)

      public:
        FakeReceiverChannel(
                            IMessageQueuePtr queue,
                            const Parameters &params,
                            UseMediaStreamTrackPtr track
                            );
        ~FakeReceiverChannel();

        static FakeReceiverChannelPtr create(
                                             IMessageQueuePtr queue,
                                             const Parameters &params,
                                             UseMediaStreamTrackPtr track
                                             );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => IRTPReceiverChannelForRTPReceiverChannelBase
        #pragma mark

        // (base handles) virtual PUID getID() const = 0;

        virtual bool sendPacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => IRTPReceiverChannelForRTPReceiverChannelVideo
        #pragma mark

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

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => (friend RTPChannelVideoTester)
        #pragma mark

        void setTransport(RTPChannelVideoTesterPtr tester);

        void linkChannelVideo(UseReceiverChannelVideoPtr channelVideo);

        void linkMediaStreamTrack(UseMediaStreamTrackPtr track);

        virtual bool handlePacket(RTPPacketPtr packet);

        virtual bool handlePacket(RTCPPacketPtr packet);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => (internal)
        #pragma mark

        Log::Params log(const char *message) const;


      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiverChannel => (data)
        #pragma mark

        FakeReceiverChannelWeakPtr mThisWeak;

        IMediaStreamTrackTypes::Kinds mKind{ IMediaStreamTrackTypes::Kind_Video };

        RTPChannelVideoTesterWeakPtr mTester;

        ParametersPtr mParameters;

        UseReceiverChannelVideoPtr mReceiverChannelVideo;

        UseMediaStreamTrackPtr mTrack;
      };


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeSenderChannel : public ortc::internal::RTPSenderChannel
      {
      public:
        friend class RTPChannelVideoTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelVideo, RTPSenderChannelVideo)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelVideoForRTPSenderChannel, UseSenderChannelVideo)
        ZS_DECLARE_TYPEDEF_PTR(internal::IMediaStreamTrackForRTPSenderChannelVideo, UseMediaStreamTrack)

      public:
        FakeSenderChannel(
                          IMessageQueuePtr queue,
                          const Parameters &params,
                          UseMediaStreamTrackPtr track
                          );
        ~FakeSenderChannel();

        static FakeSenderChannelPtr create(
                                           IMessageQueuePtr queue,
                                           const Parameters &params,
                                           UseMediaStreamTrackPtr track
                                           );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => IRTPSenderChannelForRTPSenderChannelBase
        #pragma mark

        // (base handles) virtual PUID getID() const = 0;

        virtual bool sendPacket(RTPPacketPtr packet) override;

        virtual bool sendPacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => IRTPSenderChannelForRTPSenderChannelVideo
        #pragma mark


        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => IRTPSenderChannelForMediaStreamTrack
        #pragma mark

        virtual ElementPtr toDebug() const override;

        //virtual void sendVideoFrame(const webrtc::VideoFrame& videoFrame) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => (friend RTPChannelTester)
        #pragma mark

        void setTransport(RTPChannelVideoTesterPtr tester);

        void linkChannelVideo(UseSenderChannelVideoPtr channelVideo);

        void linkMediaStreamTrack(UseMediaStreamTrackPtr track);

        virtual bool handlePacket(RTCPPacketPtr packet);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => (internal)
        #pragma mark

        Log::Params log(const char *message) const;


      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => (data)
        #pragma mark

        FakeSenderChannelWeakPtr mThisWeak;

        IMediaStreamTrackTypes::Kinds mKind{ IMediaStreamTrackTypes::Kind_Video };

        RTPChannelVideoTesterWeakPtr mTester;

        ParametersPtr mParameters;

        UseSenderChannelVideoPtr mSenderChannelVideo;

        UseMediaStreamTrackPtr mTrack;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelVideoTester
      #pragma mark

      //-----------------------------------------------------------------------
      class RTPChannelVideoTester : public SharedRecursiveLock,
                                    public zsLib::MessageQueueAssociator
      {
      public:
        friend class FakeMediaStreamTrack;
        friend class FakeReceiverChannel;
        friend class FakeSenderChannel;

        struct RTPSample {
          USHORT *mData;
          ULONG mDataLength;
        };

        ZS_DECLARE_TYPEDEF_PTR(RTPSample, RTPSample)

        ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel, RTPReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannel, RTPSenderChannel)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelVideo, RTPReceiverChannelVideo)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelVideo, RTPSenderChannelVideo)

        ZS_DECLARE_TYPEDEF_PTR(internal::MediaStreamTrack, MediaStreamTrack)

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelVideoForRTPReceiverChannel, UseReceiverChannelVideoForReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelVideoForMediaStreamTrack, UseReceiverChannelVideoForTrack)

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelVideoForRTPSenderChannel, UseSenderChannelVideoForSenderChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelVideoForMediaStreamTrack, UseSenderChannelVideoForTrack)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelVideoFactory, RTPReceiverChannelVideoFactory)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelVideoFactory, RTPSenderChannelVideoFactory)

        ZS_DECLARE_CLASS_PTR(OverrideReceiverChannelVideoFactory)

        ZS_DECLARE_CLASS_PTR(OverrideSenderChannelVideoFactory)

        friend class OverrideReceiverChannelVideoFactory;

        friend class OverrideSenderChannelVideoFactory;

        typedef String PacketID;
        typedef std::pair<RTPPacketPtr, RTCPPacketPtr> PacketPair;
        typedef std::map<PacketID, PacketPair> PacketMap;

        typedef std::list<PacketID> PacketIDList;

        typedef String SampleID;
        typedef std::pair<RTPSamplePtr, RTPSamplePtr> SamplePair;
        typedef std::map<SampleID, SamplePair> SampleMap;

        typedef std::list<SampleID> SampleIDList;

        typedef String ParametersID;
        typedef std::map<ParametersID, ParametersPtr> ParametersMap;

        typedef String TrackID;
        typedef std::map<TrackID, FakeMediaStreamTrackPtr> FakeMediaStreamTrackMap;

        typedef String SenderOrReceiverChannelID;
        typedef std::pair<FakeReceiverChannelPtr, FakeSenderChannelPtr> FakeReceiverFakeSenderChannelPair;
        typedef std::map<SenderOrReceiverChannelID, FakeReceiverFakeSenderChannelPair> SenderOrReceiverChannelMap;

        typedef String ReceiverChannelID;
        typedef std::map<ReceiverChannelID, FakeReceiverChannelPtr> FakeReceiverChannelMap;

        typedef String SenderChannelID;
        typedef std::map<SenderChannelID, FakeSenderChannelPtr> FakeSenderChannelMap;

        typedef String ReceiverChannelVideoID;
        typedef std::map<ReceiverChannelVideoID, RTPReceiverChannelVideoPtr> ReceiverChannelVideoMap;

        typedef String SenderChannelVideolID;
        typedef std::map<SenderChannelVideolID, RTPSenderChannelVideoPtr> SenderChannelVideoMap;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester::OverrideReceiverChannelVideoFactory
        #pragma mark

        class OverrideReceiverChannelVideoFactory : public RTPReceiverChannelVideoFactory
        {
        public:
          static OverrideReceiverChannelVideoFactoryPtr create(RTPChannelVideoTesterPtr tester);

          virtual RTPReceiverChannelVideoPtr create(
                                                    RTPReceiverChannelPtr receiverChannel,
                                                    MediaStreamTrackPtr track,
                                                    const Parameters &params
                                                    ) override;

        protected:
          RTPChannelVideoTesterWeakPtr mTester;
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester::OverrideSenderChannelVideoFactory
        #pragma mark

        class OverrideSenderChannelVideoFactory : public RTPSenderChannelVideoFactory
        {
        public:
          static OverrideSenderChannelVideoFactoryPtr create(RTPChannelVideoTesterPtr tester);

          virtual RTPSenderChannelVideoPtr create(
                                                  RTPSenderChannelPtr senderChannel,
                                                  MediaStreamTrackPtr track,
                                                  const Parameters &params
                                                  ) override;

        protected:
          RTPChannelVideoTesterWeakPtr mTester;
        };


      protected:
        struct make_private {};

      public:
        struct Expectations {
          // listener related

          // general
          ULONG mReceivedPackets{ 0 };

          // sender channel related
          ULONG mSenderChannelConflict{ 0 };
          ULONG mSenderChannelError{ 0 };

          bool operator==(const Expectations &op2) const;
        };

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester (api)
        #pragma mark

        static RTPChannelVideoTesterPtr create(
                                               IMessageQueuePtr queue,
                                               bool overrideFactories,
                                               void* videoSurface
                                               );

        RTPChannelVideoTester(
                              IMessageQueuePtr queue,
                              bool overrideFactories,
                              void* videoSurface
                              );
        ~RTPChannelVideoTester();

        void init();

        bool matches();

        void close();
        void closeByReset();

        Expectations getExpectations() const;

        void connect(RTPChannelVideoTesterPtr remote);

        void createMediaStreamTrack(
                                   const char *trackID,
                                   bool remote
                                   );

        void createReceiverChannel(
                                   const char *receiverChannelID,
                                   const char *mediaStreamTrackID,
                                   const char *parametersID
                                   );
        void createSenderChannel(
                                 const char *senderChannelID,
                                 const char *mediaStreamTrackID,
                                 const char *parametersID
                                 );

        void createReceiverChannelVideo(
                                        const char *receiverChannelID,
                                        const char *receiverChannelVideoID,
                                        const char *mediaStreamTrackID,
                                        const char *parametersID
                                        );
        void createSenderChannelVideo(
                                      const char *senderChannelID,
                                      const char *senderChannelVideoID,
                                      const char *mediaStreamTrackID,
                                      const char *parametersID
                                      );

        void send(
                  const char *senderChannelID,
                  const char *parametersID
                  );

        void receive(
                     const char *receiverChannelID,
                     const char *parametersID
                     );

        void stop(const char *senderOrReceiverChannelID);

        void attach(
                    const char *trackID,
                    FakeMediaStreamTrackPtr mediaStreamTrack
                    );

        void attach(
                    const char *receiverChannelID,
                    FakeReceiverChannelPtr receiverChannel
                    );
        void attach(
                    const char *senderChannelID,
                    FakeSenderChannelPtr senderChannel
                    );
        void attach(
                    const char *receiverChannelVideoID,
                    RTPReceiverChannelVideoPtr receiverChannelVideo
                    );
        void attach(
                    const char *senderChannelVideoID,
                    RTPSenderChannelVideoPtr senderChannelVideo
                    );

        FakeMediaStreamTrackPtr detachMediaStreamTrack(const char *trackID);
        FakeReceiverChannelPtr detachReceiverChannel(const char *receiverChannelID);
        FakeSenderChannelPtr detachSenderChannel(const char *senderChannelID);
        RTPReceiverChannelVideoPtr detachReceiverChannelVideo(const char *receiverChannelVideoID);
        RTPSenderChannelVideoPtr detachSenderChannelVideo(const char *senderChannelVideoID);

        void store(
                   const char *packetID,
                   RTPPacketPtr packet
                   );
        void store(
                   const char *packetID,
                   RTCPPacketPtr packet
                   );
        void store(
                   const char *sampleID,
                   RTPSamplePtr sample
                   );
        void store(
                   const char *parametersID,
                   const Parameters &params
                   );

        RTPPacketPtr getRTPPacket(const char *packetID);
        RTCPPacketPtr getRTCPPacket(const char *packetID);
        RTPSamplePtr getSample(const char *sampleID);
        ParametersPtr getParameters(const char *parametersID);

        void sendPacket(
                        const char *senderOrReceiverChannelID,
                        const char *packetID
                        );

        void expectPacket(
                          const char *senderOrReceiverChannelID,
                          const char *packetID
                          );


        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester => IRTPReceiverChannelVideoForRTPReceiverChannel
        #pragma mark

        // simulate methods calls to IRTPReceiverChannelVideoForRTPReceiverChannel

        //static ElementPtr toDebug(ForRTPReceiverChannelPtr object);

        static RTPReceiverChannelVideoPtr create(
                                                 RTPReceiverChannelPtr receiver,
                                                 const Parameters &params
                                                 );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester => IRTPReceiverChannelVideoForMediaStreamTrack
        #pragma mark

        // call methods calls to IRTPReceiverChannelVideoForMediaStreamTrack

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester => IRTPSenderChannelVideoForRTPSenderChannel
        #pragma mark

        // simulate methods calls to IRTPSenderChannelVideoForRTPSenderChannel

        //static ElementPtr toDebug(ForRTPSenderChannelPtr object);

        static RTPSenderChannelVideoPtr create(
                                               RTPSenderChannelPtr sender,
                                               const Parameters &params
                                               );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester => IRTPSenderChannelVideoForMediaStreamTrack
        #pragma mark

        // call methods calls to IRTPSenderChannelVideoForMediaStreamTrack

      protected:

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester => (friend fake media track, sender and receiver channel)
        #pragma mark

        RTPReceiverChannelVideoPtr createReceiverChannelVideo(
                                                              RTPReceiverChannelPtr receiverChannel,
                                                              MediaStreamTrackPtr track,
                                                              const Parameters &params
                                                              );
        RTPSenderChannelVideoPtr createSenderChannelVideo(
                                                          RTPSenderChannelPtr senderChannel,
                                                          MediaStreamTrackPtr track,
                                                          const Parameters &params
                                                          );

        void sendToConnectedTester(RTPPacketPtr packet);

        void sendToConnectedTester(RTCPPacketPtr packet);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

        FakeMediaStreamTrackPtr getMediaStreamTrack(const char *trackID);
        FakeReceiverChannelPtr getReceiverChannel(const char *receiverChannelID);
        FakeSenderChannelPtr getSenderChannel(const char *senderChannelID);
        RTPReceiverChannelVideoPtr getReceiverChannelVideo(const char *receiverChannelVideoID);
        RTPSenderChannelVideoPtr getSenderChannelVideo(const char *senderChannelVideoID);

        void expectData(
                        const char *senderOrReceiverChannelID,
                        SecureByteBlockPtr secureBuffer
                        );
        void sendData(
                      const char *senderOrReceiverChannelID,
                      SecureByteBlockPtr secureBuffer
                      );

        void getPackets(
                        const char *packetID,
                        RTPPacketPtr &outRTP,
                        RTCPPacketPtr &outRTCP
                        );

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPChannelVideoTester => (data)
        #pragma mark

        AutoPUID mID;
        RTPChannelVideoTesterWeakPtr mThisWeak;

        bool mOverrideFactories{ false };

        void* mVideoSurface;

        RTPChannelVideoTesterWeakPtr mConnectedTester;

        Expectations mExpecting;
        Expectations mExpectationsFound;

        SenderOrReceiverChannelMap mAttached;
        PacketMap mPackets;
        SampleMap mSamples;

        ParametersMap mParameters;

        FakeMediaStreamTrackMap mMediaStreamTracks;
        FakeReceiverChannelMap mReceiverChannels;
        FakeSenderChannelMap mSenderChannels;
        ReceiverChannelVideoMap mReceiverVideoChannels;
        SenderChannelVideoMap mSenderVideoChannels;
      };
    }
  }
}
