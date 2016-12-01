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



#include <ortc/IDataChannel.h>
#include <ortc/IRTPListener.h>

#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_RTPSender.h>

#include <ortc/services/IHelper.h>

#include <zsLib/ISettings.h>
#include <zsLib/IMessageQueueThread.h>
#include <zsLib/Promise.h>
#include <zsLib/ITimer.h>
#include <zsLib/Log.h>

#include "config.h"
#include "testing.h"

namespace ortc
{
  namespace test
  {
    namespace rtplistener
    {
      ZS_DECLARE_INTERACTION_PROXY(IFakeICETransportAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeSecureTransportAsyncDelegate)

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeICETransportAsyncDelegate
      #pragma mark

      interaction IFakeICETransportAsyncDelegate
      {
        virtual void onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer) = 0;
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeSecureTransportAsyncDelegate
      #pragma mark

      interaction IFakeSecureTransportAsyncDelegate
      {
        virtual ~IFakeSecureTransportAsyncDelegate() {}
      };
    }
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtplistener::IFakeICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtplistener::IFakeSecureTransportAsyncDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
//ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace rtplistener
    {
      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::Milliseconds;

      ZS_DECLARE_USING_PTR(zsLib, ITimer)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPPacket)
      ZS_DECLARE_USING_PTR(ortc::internal, RTCPPacket)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPListener)

      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(FakeReceiver)
      ZS_DECLARE_CLASS_PTR(FakeSender)
      ZS_DECLARE_CLASS_PTR(RTPListenerTester)

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport
      #pragma mark

      //---------------------------------------------------------------------
      class FakeICETransport : public ortc::internal::ICETransport,
                               public IFakeICETransportAsyncDelegate
      {
      public:
        friend class FakeSecureTransport;

      protected:
        struct make_private {};

        typedef std::pair<Time, SecureByteBlockPtr> DelayedBufferPair;
        typedef std::list<DelayedBufferPair> DelayedBufferList;

      public:
        //---------------------------------------------------------------------
        FakeICETransport(
                         const make_private &,
                         IMessageQueuePtr queue,
                         Milliseconds packetDelay
                         );

      protected:
        //---------------------------------------------------------------------
        void init();

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => (test API)
        #pragma mark

        ~FakeICETransport();

        static FakeICETransportPtr create(
                                          IMessageQueuePtr queue,
                                          Milliseconds packetDelay = Milliseconds()
                                          );

        void reliability(ULONG percentage);

        void linkTransport(FakeICETransportPtr transport);

        void state(IICETransport::States newState);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => IICETransport
        #pragma mark

        //---------------------------------------------------------------------
        virtual ElementPtr toDebug() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => IFakeICETransportAsyncDelegate
        #pragma mark

        //---------------------------------------------------------------------
        virtual void onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => ITimerDelegate
        #pragma mark

        //---------------------------------------------------------------------
        virtual void onTimer(ITimerPtr timer) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => friend FakeSecureTransport
        #pragma mark

        void attachSecure(FakeSecureTransportPtr transport);

        void detachSecure(FakeSecureTransport &transport);

        virtual bool sendPacket(
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => (internal)
        #pragma mark

        void setState(IICETransportTypes::States state);

        bool isShutdown();

        //---------------------------------------------------------------------
        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => (data)
        #pragma mark

        FakeICETransportWeakPtr mThisWeak;

        IICETypes::Components mComponent {IICETypes::Component_RTP};

        IICETransportTypes::States mCurrentState {IICETransportTypes::State_New};

        PUID mSecureTransportID {0};
        FakeSecureTransportWeakPtr mSecureTransport;

        FakeICETransportWeakPtr mLinkedTransport;

        IICETransportDelegateSubscriptions mSubscriptions;
        IICETransportSubscriptionPtr mDefaultSubscription;

        ULONG mReliability {100};
        Milliseconds mPacketDelay {};

        DelayedBufferList mDelayedBuffers;

        ITimerPtr mTimer;
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport
      #pragma mark

      //---------------------------------------------------------------------
      class FakeSecureTransport : public ortc::internal::DTLSTransport,
                                  public IFakeSecureTransportAsyncDelegate
      {
      public:
        friend class FakeICETransport;
        friend class FakeReceiver;
        friend class FakeSender;
        friend class RTPListenerTester;

      protected:
        struct make_private {};

      public:
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IICETransportForSecureTransport::UseSecureTransport, UseSecureTransport)

        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IRTPListenerForSecureTransport, UseListener)

      public:
        //---------------------------------------------------------------------
        FakeSecureTransport(
                            const make_private &,
                            IMessageQueuePtr queue,
                            FakeICETransportPtr iceTransport
                            );

      protected:
        //---------------------------------------------------------------------
        void init();

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => (testing API)
        #pragma mark

        ~FakeSecureTransport();

        static FakeSecureTransportPtr create(
                                             IMessageQueuePtr queue,
                                             FakeICETransportPtr iceTransport
                                             );

        void state(IDTLSTransport::States newState);

        void setClientRole(bool clientRole);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => IICETransport
        #pragma mark

        //---------------------------------------------------------------------
        virtual ElementPtr toDebug() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark DTLSTransport => ISecureTransportForRTPListener
        #pragma mark

        // (duplicate) static ElementPtr toDebug(ForRTPListenerPtr transport);

        // (duplicate) virtual PUID getID() const = 0;

        virtual RTPListenerPtr getListener() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => ISecureTransportForRTPSender
        #pragma mark

        virtual PUID getID() const override;

        virtual bool sendPacket(
                                IICETypes::Components sendOverICETransport,
                                IICETypes::Components packetType,
                                const BYTE *buffer,
                                size_t bufferLengthInBytes
                                ) override;

        virtual IICETransportPtr getICETransport() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => IFakeSecureTransportAsyncDelegate
        #pragma mark

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => friend FakeICETransport
        #pragma mark

        //---------------------------------------------------------------------
        bool handleReceivedPacket(
                                  IICETypes::Components component,
                                  const BYTE *buffer,
                                  size_t bufferSizeInBytes
                                  ) override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => (internal)
        #pragma mark

        void setState(IDTLSTransportTypes::States state);

        bool isShutdown();

        Log::Params log(const char *message) const;

        void cancel();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => (data)
        #pragma mark

        FakeSecureTransportWeakPtr mThisWeak;

        FakeICETransportPtr mICETransport;

        IDTLSTransportTypes::States mCurrentState {IDTLSTransportTypes::State_New};

        bool mClientRole {false};

        UseListenerPtr mListener;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeReceiver : public ortc::internal::RTPReceiver
      {
      public:
        typedef std::list<SecureByteBlockPtr> BufferList;
        typedef RTPListener::RTCPPacketList RTCPPacketList;

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPListenerForRTPReceiver, UseListener)

      public:
        FakeReceiver(IMediaStreamTrackTypes::Kinds kind);
        ~FakeReceiver();

        static FakeReceiverPtr create(IMediaStreamTrackTypes::Kinds kind);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiver => IRTPReceiverForRTPListener
        #pragma mark

        virtual ElementPtr toDebug() const override;

        virtual bool handlePacket(
                                  IICETypes::Components viaTransport,
                                  RTPPacketPtr packet
                                  ) override;

        virtual bool handlePacket(
                                  IICETypes::Components viaTransport,
                                  RTCPPacketPtr packet
                                  ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiver => (friend RTPListenerTester)
        #pragma mark

        void setTransport(RTPListenerTesterPtr tester);

        void expectData(SecureByteBlockPtr data);

        virtual PromisePtr receive(const Parameters &parameters) override;

        virtual void stop() override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiver => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

      protected:
        FakeReceiverWeakPtr mThisWeak;

        RTPListenerTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mBuffers;

        IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrackTypes::Kind_First};
        UseListenerPtr mListener;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeSender : public ortc::internal::RTPSender
      {
      public:
        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransportForRTPSender, UseSecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPListenerForRTPSender, UseListener)

        typedef std::list<SecureByteBlockPtr> BufferList;
        typedef RTPListener::RTCPPacketList RTCPPacketList;

      public:
        FakeSender();
        ~FakeSender();

        static FakeSenderPtr create();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSender => IRTPSenderForRTPListener
        #pragma mark

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

        virtual bool handlePacket(
                                  IICETypes::Components viaTransport,
                                  RTCPPacketPtr packet
                                  ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSender => (friend RTPListenerTester)
        #pragma mark

        void setTransport(RTPListenerTesterPtr tester);

        virtual PromisePtr send(const Parameters &parameters) override;
        virtual void stop() override;

        void expectData(SecureByteBlockPtr data);

        void sendPacket(SecureByteBlockPtr buffer);


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

        RTPListenerTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mBuffers;

        UseListenerPtr mListener;
        UseSecureTransportPtr mSecureTransport;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListenerTester
      #pragma mark

      //-----------------------------------------------------------------------
      class RTPListenerTester : public SharedRecursiveLock,
                                public zsLib::MessageQueueAssociator,
                                public IRTPListenerDelegate
      {
      public:
        friend class FakeReceiver;
        friend class FakeSender;

        ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

        typedef String SenderOrReceiverID;
        typedef std::pair<FakeReceiverPtr, FakeSenderPtr> FakePair;
        typedef std::map<SenderOrReceiverID, FakePair> SenderOrReceiverMap;

        typedef String PacketID;
        typedef std::pair<RTPPacketPtr, RTCPPacketPtr> PacketPair;
        typedef std::map<PacketID, PacketPair> PacketMap;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPListenerTester::UnhandledEventData
        #pragma mark

        struct UnhandledEventData
        {
          UnhandledEventData(
                             DWORD ssrc,
                             BYTE pt,
                             const char *mid
                             );

          bool operator==(const UnhandledEventData &op2) const;

          DWORD mSSRC {};
          BYTE mPT {};
          String mMID;
        };

        typedef std::list<UnhandledEventData> UnhandledEventDataList;

      protected:
        struct make_private {};

      public:
        struct Expectations {
          // data channel related
          ULONG mStateConnecting {0};
          ULONG mStateOpen {0};
          ULONG mStateClosing {0};
          ULONG mStateClosed {0};

          ULONG mUnhandled {0};
          ULONG mReceivedPackets {0};

          ULONG mError {0};

          bool operator==(const Expectations &op2) const;
        };

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPListenerTester (api)
        #pragma mark

        static RTPListenerTesterPtr create(
                                           IMessageQueuePtr queue,
                                           Milliseconds packetDelay = Milliseconds()
                                           );

        RTPListenerTester(IMessageQueuePtr queue);

        ~RTPListenerTester();

        void init(Milliseconds packetDelay);

        bool matches(const Expectations &op2);

        void close();
        void closeByReset();

        Expectations getExpectations() const;

        void reliability(ULONG percentage);

        void state(IICETransport::States newState);

        void state(IDTLSTransport::States newState);

        void setClientRole(bool clientRole);

        void connect(RTPListenerTesterPtr remote);

        void createReceiver(const char *receiverID);
        void createSender(const char *senderID);

        void send(
                  const char *senderID,
                  const Parameters &params
                  );

        void receive(
                     const char *receiverID,
                     const Parameters &params
                     );

        void stop(const char *senderOrReceiverID);

        void attach(
                    const char *receiverID,
                    FakeReceiverPtr receiver
                    );
        void attach(
                    const char *senderID,
                    FakeSenderPtr sender
                    );

        FakeReceiverPtr detachReceiver(const char *receiverID);
        FakeSenderPtr detachSender(const char *senderID);

        void expectingUnhandled(
                                SSRCType ssrc,
                                PayloadType payloadType,
                                const char *mid
                                );

        void store(
                   const char *packetID,
                   RTPPacketPtr packet
                   );
        void store(
                   const char *packetID,
                   RTCPPacketPtr packet
                   );
        RTPPacketPtr getRTPPacket(const char *packetID);
        RTCPPacketPtr getRTCPPacket(const char *packetID);

        void sendPacket(
                        const char *viaSenderID,
                        const char *packetID
                        );
        void expectPacket(
                          const char *senderOrReceiverID,
                          const char *packetID
                          );

      protected:

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPListenerTester::IRTPListenerDelegate
        #pragma mark

        virtual void onRTPListenerUnhandledRTP(
                                               IRTPListenerPtr listener,
                                               SSRCType ssrc,
                                               PayloadType payloadType,
                                               const char *mid,
                                               const char *rid
                                               ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPListenerTester => (friend fake sender/receiver)
        #pragma mark

        FakeSecureTransportPtr getFakeSecureTransport() const;

        void notifyReceivedPacket();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPListenerTester => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

        FakeICETransportPtr getICETransport() const;

        FakeReceiverPtr getReceiver(const char *receiverID);
        FakeSenderPtr getSender(const char *senderID);

        void expectData(
                        const char *senderOrReceiverID,
                        SecureByteBlockPtr secureBuffer
                        );
        void sendData(
                      const char *senderID,
                      SecureByteBlockPtr secureBuffer
                      );

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPListenerTester => (data)
        #pragma mark

        AutoPUID mID;
        RTPListenerTesterWeakPtr mThisWeak;

        FakeICETransportPtr mICETransport;
        FakeSecureTransportPtr mDTLSTransport;

        RTPListenerTesterWeakPtr mConnectedTester;

        IRTPListenerSubscriptionPtr mListenerSubscription;

        Expectations mExpectations;

        SenderOrReceiverMap mAttached;
        PacketMap mPackets;

        UnhandledEventDataList mExpectingUnhandled;
      };
    }
  }
}

