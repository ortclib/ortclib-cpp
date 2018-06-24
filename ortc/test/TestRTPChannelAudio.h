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


#if 0

#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>

#include <ortc/services/IHelper.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
#include <zsLib/ITimer.h>
#include <zsLib/Log.h>
#include <zsLib/Promise.h>
#include <zsLib/Event.h>

#include "config.h"
#include "testing.h"

namespace ortc
{
  namespace test
  {
    namespace rtpchannelaudio
    {
      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::Milliseconds;
      
      ZS_DECLARE_USING_PTR(zsLib, ITimer)
      ZS_DECLARE_USING_PTR(ortc, RTPPacket)
      ZS_DECLARE_USING_PTR(ortc, RTCPPacket)
      
      ZS_DECLARE_CLASS_PTR(FakeMediaStreamTrack)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannel)
      ZS_DECLARE_CLASS_PTR(FakeSenderChannel)
      ZS_DECLARE_CLASS_PTR(RTPChannelAudioTester)
      
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // FakeMediaStreamTrack
      //
      
      //-----------------------------------------------------------------------
      class FakeMediaStreamTrack : public ortc::internal::MediaStreamTrack
      {
      public:
        friend class RTPChannelAudioTester;
        
        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransport, ISecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelForMediaStreamTrack, UseReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelForMediaStreamTrack, UseSenderChannel)
        
      public:
        FakeMediaStreamTrack(IMessageQueuePtr queue, bool remote);
        ~FakeMediaStreamTrack();

        static FakeMediaStreamTrackPtr create(
                                              IMessageQueuePtr queue,
                                              bool remote
                                              );

        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelAudio
        //

        //virtual webrtc::AudioDeviceModule* getAudioDeviceModule() override;

        //virtual void start() override;
        virtual void stop();


        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => IMediaStreamTrackForRTPSenderChannelAudio
        //

        // (duplicate) virtual webrtc::AudioDeviceModule* getAudioDeviceModule() = 0;

        // (duplicate) virtual void start() = 0;
        // (duplicate) virtual void stop() = 0;


        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => webrtc::AudioTransport
        //

        virtual int32_t RecordedDataIsAvailable(
                                                const void* audioSamples,
                                                const size_t nSamples,
                                                const size_t nBytesPerSample,
                                                const uint8_t nChannels,
                                                const uint32_t samplesPerSec,
                                                const uint32_t totalDelayMS,
                                                const int32_t clockDrift,
                                                const uint32_t currentMicLevel,
                                                const bool keyPressed,
                                                uint32_t& newMicLevel
                                                );

        virtual int32_t NeedMorePlayData(
                                         const size_t nSamples,
                                         const size_t nBytesPerSample,
                                         const uint8_t nChannels,
                                         const uint32_t samplesPerSec,
                                         void* audioSamples,
                                         size_t& nSamplesOut,
                                         int64_t* elapsed_time_ms,
                                         int64_t* ntp_time_ms
                                         );

        
        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelAudioTesterPtr tester);

        void linkReceiverChannel(UseReceiverChannelPtr channel);

        void linkSenderChannel(UseSenderChannelPtr channel);

        void linkReceiverChannelAudio(UseReceiverChannelAudioPtr channelAudio);

        void linkSenderChannelAudio(UseSenderChannelAudioPtr channelAudio);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => (internal)
        //
        
        Log::Params log(const char *message) const;
        
        ElementPtr toDebug() const override;
        
      protected:
        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => (data)
        //
        
        FakeMediaStreamTrackWeakPtr mThisWeak;
        
        IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrackTypes::Kind_Audio};
        bool mRemote;
        
        RTPChannelAudioTesterWeakPtr mTester;
        
        UseReceiverChannelWeakPtr mReceiverChannel;
        UseSenderChannelWeakPtr mSenderChannel;
        UseReceiverChannelAudioWeakPtr mReceiverChannelAudio;
        UseSenderChannelAudioWeakPtr mSenderChannelAudio;

        webrtc::AudioDeviceModule* mAudioDeviceModule;
      };
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // FakeReceiverChannel
      //
      
      //-----------------------------------------------------------------------
      class FakeReceiverChannel : public ortc::internal::RTPReceiverChannel
      {
      public:
        friend class RTPChannelAudioTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelAudio, RTPReceiverChannelAudio)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelAudioForRTPReceiverChannel, UseReceiverChannelAudio)
        ZS_DECLARE_TYPEDEF_PTR(internal::IMediaStreamTrackForRTPReceiverChannelAudio, UseMediaStreamTrack)
        
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
        //
        // FakeReceiverChannel => IRTPReceiverChannelForRTPReceiverChannelBase
        //

        // (base handles) virtual PUID getID() const = 0;

        virtual bool sendPacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => IRTPReceiverChannelForRTPReceiverChannelAudio
        //

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => IRTPReceiverChannelForMediaStreamTrack
        //
        
        virtual ElementPtr toDebug() const override;

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => (friend RTPChannelAudioTester)
        //

        void setTransport(RTPChannelAudioTesterPtr tester);

        void linkChannelAudio(UseReceiverChannelAudioPtr channelAudio);

        void linkMediaStreamTrack(UseMediaStreamTrackPtr track);

        virtual bool handlePacket(RTPPacketPtr packet);

        virtual bool handlePacket(RTCPPacketPtr packet);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => (internal)
        //
        
        Log::Params log(const char *message) const;
        
        
      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => (data)
        //
        
        FakeReceiverChannelWeakPtr mThisWeak;
        
        IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrackTypes::Kind_Audio};
        
        RTPChannelAudioTesterWeakPtr mTester;
        
        ParametersPtr mParameters;
        
        UseReceiverChannelAudioPtr mReceiverChannelAudio;
        
        UseMediaStreamTrackPtr mTrack;
      };
      
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // FakeSenderChannel
      //
      
      //-----------------------------------------------------------------------
      class FakeSenderChannel : public ortc::internal::RTPSenderChannel
      {
      public:
        friend class RTPChannelAudioTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelAudio, RTPSenderChannelAudio)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelAudioForRTPSenderChannel, UseSenderChannelAudio)
        ZS_DECLARE_TYPEDEF_PTR(internal::IMediaStreamTrackForRTPSenderChannelAudio, UseMediaStreamTrack)
        
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
        //
        // FakeSenderChannel => IRTPSenderChannelForRTPSenderChannelBase
        //

        // (base handles) virtual PUID getID() const = 0;

        virtual bool sendPacket(RTPPacketPtr packet) override;

        virtual bool sendPacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeSenderChannel => IRTPSenderChannelForRTPSenderChannelAudio
        //

        //---------------------------------------------------------------------
        //
        // FakeSenderChannel => IRTPSenderChannelForMediaStreamTrack
        //
        
        virtual ElementPtr toDebug() const override;
        
        //---------------------------------------------------------------------
        //
        // FakeSenderChannel => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelAudioTesterPtr tester);

        void linkChannelAudio(UseSenderChannelAudioPtr channelAudio);

        void linkMediaStreamTrack(UseMediaStreamTrackPtr track);

        virtual bool handlePacket(RTCPPacketPtr packet);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSenderChannel => (internal)
        //
        
        Log::Params log(const char *message) const;
        
        
      protected:
        //---------------------------------------------------------------------
        //
        // FakeSenderChannel => (data)
        //
        
        FakeSenderChannelWeakPtr mThisWeak;
        
        IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrackTypes::Kind_Audio};

        RTPChannelAudioTesterWeakPtr mTester;
        
        ParametersPtr mParameters;
        
        UseSenderChannelAudioPtr mSenderChannelAudio;
        
        UseMediaStreamTrackPtr mTrack;
      };
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // RTPChannelAudioTester
      //
      
      //-----------------------------------------------------------------------
      class RTPChannelAudioTester : public SharedRecursiveLock,
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
        
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelAudio, RTPReceiverChannelAudio)
        
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelAudio, RTPSenderChannelAudio)
        
        ZS_DECLARE_TYPEDEF_PTR(internal::MediaStreamTrack, MediaStreamTrack)
        
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelAudioForRTPReceiverChannel, UseReceiverChannelAudioForReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelAudioForMediaStreamTrack, UseReceiverChannelAudioForTrack)
        
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelAudioForRTPSenderChannel, UseSenderChannelAudioForSenderChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelAudioForMediaStreamTrack, UseSenderChannelAudioForTrack)
        
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelAudioFactory, RTPReceiverChannelAudioFactory)
        
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelAudioFactory, RTPSenderChannelAudioFactory)
        
        ZS_DECLARE_CLASS_PTR(OverrideReceiverChannelAudioFactory)
        
        ZS_DECLARE_CLASS_PTR(OverrideSenderChannelAudioFactory)
        
        friend class OverrideReceiverChannelAudioFactory;
        
        friend class OverrideSenderChannelAudioFactory;
        
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

        typedef String ReceiverChannelAudioID;
        typedef std::map<ReceiverChannelAudioID, RTPReceiverChannelAudioPtr> ReceiverChannelAudioMap;
        
        typedef String SenderChannelAudiolID;
        typedef std::map<SenderChannelAudiolID, RTPSenderChannelAudioPtr> SenderChannelAudioMap;
        
        //---------------------------------------------------------------------
        //
        // RTPChannelAudioTester::OverrideReceiverChannelAudioFactory
        //
        
        class OverrideReceiverChannelAudioFactory : public RTPReceiverChannelAudioFactory
        {
        public:
          static OverrideReceiverChannelAudioFactoryPtr create(RTPChannelAudioTesterPtr tester);
          
          virtual RTPReceiverChannelAudioPtr create(
                                                    RTPReceiverChannelPtr receiverChannel,
                                                    MediaStreamTrackPtr track,
                                                    const Parameters &params
                                                    ) override;
          
        protected:
          RTPChannelAudioTesterWeakPtr mTester;
        };
        
        //---------------------------------------------------------------------
        //
        // RTPChannelAudioTester::OverrideSenderChannelAudioFactory
        //
        
        class OverrideSenderChannelAudioFactory : public RTPSenderChannelAudioFactory
        {
        public:
          static OverrideSenderChannelAudioFactoryPtr create(RTPChannelAudioTesterPtr tester);
          
          virtual RTPSenderChannelAudioPtr create(
                                                  RTPSenderChannelPtr senderChannel,
                                                  MediaStreamTrackPtr track,
                                                  const Parameters &params
                                                  ) override;
          
        protected:
          RTPChannelAudioTesterWeakPtr mTester;
        };
        
        
      protected:
        struct make_private {};
        
      public:
        struct Expectations {
          // listener related
          
          // general
          ULONG mReceivedPackets {0};
          
          // sender channel related
          ULONG mSenderChannelConflict {0};
          ULONG mSenderChannelError {0};
          
          bool operator==(const Expectations &op2) const;
        };
        
      public:
        //---------------------------------------------------------------------
        //
        // RTPChannelAudioTester (api)
        //
        
        static RTPChannelAudioTesterPtr create(
                                               IMessageQueuePtr queue,
                                               bool overrideFactories
                                               );
        
        RTPChannelAudioTester(
                              IMessageQueuePtr queue,
                              bool overrideFactories
                              );
        ~RTPChannelAudioTester();
        
        void init();
        
        bool matches();
        
        void close();
        void closeByReset();
        
        Expectations getExpectations() const;
        
        void connect(RTPChannelAudioTesterPtr remote);

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
        
        void createReceiverChannelAudio(
                                        const char *receiverChannelID,
                                        const char *receiverChannelAudioID,
                                        const char *mediaStreamTrackID,
                                        const char *parametersID
                                        );
        void createSenderChannelAudio(
                                      const char *senderChannelID,
                                      const char *senderChannelAudioID,
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
                    const char *receiverChannelAudioID,
                    RTPReceiverChannelAudioPtr receiverChannelAudio
                    );
        void attach(
                    const char *senderChannelAudioID,
                    RTPSenderChannelAudioPtr senderChannelAudio
                    );
        
        FakeMediaStreamTrackPtr detachMediaStreamTrack(const char *trackID);
        FakeReceiverChannelPtr detachReceiverChannel(const char *receiverChannelID);
        FakeSenderChannelPtr detachSenderChannel(const char *senderChannelID);
        RTPReceiverChannelAudioPtr detachReceiverChannelAudio(const char *receiverChannelAudioID);
        RTPSenderChannelAudioPtr detachSenderChannelAudio(const char *senderChannelAudioID);
        
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
        //
        // RTPChannelAudioTester => IRTPReceiverChannelAudioForRTPReceiverChannel
        //
        
        // simulate methods calls to IRTPReceiverChannelAudioForRTPReceiverChannel
        
        //static ElementPtr toDebug(ForRTPReceiverChannelPtr object);
        
        static RTPReceiverChannelAudioPtr create(
                                                 RTPReceiverChannelPtr receiver,
                                                 const Parameters &params
                                                 );
        
        //---------------------------------------------------------------------
        //
        // RTPChannelAudioTester => IRTPReceiverChannelAudioForMediaStreamTrack
        //
        
        // call methods calls to IRTPReceiverChannelAudioForMediaStreamTrack
        
        //---------------------------------------------------------------------
        //
        // RTPChannelAudioTester => IRTPSenderChannelAudioForRTPSenderChannel
        //
        
        // simulate methods calls to IRTPSenderChannelAudioForRTPSenderChannel
        
        //static ElementPtr toDebug(ForRTPSenderChannelPtr object);
        
        static RTPSenderChannelAudioPtr create(
                                               RTPSenderChannelPtr sender,
                                               const Parameters &params
                                               );
        
        //---------------------------------------------------------------------
        //
        // RTPChannelAudioTester => IRTPSenderChannelAudioForMediaStreamTrack
        //
        
        // call methods calls to IRTPSenderChannelAudioForMediaStreamTrack
        
      protected:
        
        //---------------------------------------------------------------------
        //
        // RTPChannelAudioTester => (friend fake media track, sender and receiver channel)
        //
        
        RTPReceiverChannelAudioPtr createReceiverChannelAudio(
                                                              RTPReceiverChannelPtr receiverChannel,
                                                              MediaStreamTrackPtr track,
                                                              const Parameters &params
                                                              );
        RTPSenderChannelAudioPtr createSenderChannelAudio(
                                                          RTPSenderChannelPtr senderChannel,
                                                          MediaStreamTrackPtr track,
                                                          const Parameters &params
                                                          );

        void sendToConnectedTester(RTPPacketPtr packet);

        void sendToConnectedTester(RTCPPacketPtr packet);

      protected:
        //---------------------------------------------------------------------
        //
        // RTPChannelAudioTester => (internal)
        //
        
        Log::Params log(const char *message) const;
        
        FakeMediaStreamTrackPtr getMediaStreamTrack(const char *trackID);
        FakeReceiverChannelPtr getReceiverChannel(const char *receiverChannelID);
        FakeSenderChannelPtr getSenderChannel(const char *senderChannelID);
        RTPReceiverChannelAudioPtr getReceiverChannelAudio(const char *receiverChannelAudioID);
        RTPSenderChannelAudioPtr getSenderChannelAudio(const char *senderChannelAudioID);
        
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
        //
        // RTPChannelAudioTester => (data)
        //
        
        AutoPUID mID;
        RTPChannelAudioTesterWeakPtr mThisWeak;
        
        bool mOverrideFactories {false};
        
        RTPChannelAudioTesterWeakPtr mConnectedTester;
        
        Expectations mExpecting;
        Expectations mExpectationsFound;
        
        SenderOrReceiverChannelMap mAttached;
        PacketMap mPackets;
        SampleMap mSamples;
        
        ParametersMap mParameters;
        
        FakeMediaStreamTrackMap mMediaStreamTracks;
        FakeReceiverChannelMap mReceiverChannels;
        FakeSenderChannelMap mSenderChannels;
        ReceiverChannelAudioMap mReceiverAudioChannels;
        SenderChannelAudioMap mSenderAudioChannels;
      };
    }
  }
}

#endif //0
