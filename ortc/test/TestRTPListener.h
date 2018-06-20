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
      //
      // IFakeICETransportAsyncDelegate
      //

      interaction IFakeICETransportAsyncDelegate
      {
        virtual void onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer) = 0;
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //
      // IFakeSecureTransportAsyncDelegate
      //

      interaction IFakeSecureTransportAsyncDelegate
      {
        virtual ~IFakeSecureTransportAsyncDelegate() {}
      };
    }
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtplistener::IFakeICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_METHOD(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtplistener::IFakeSecureTransportAsyncDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
//ZS_DECLARE_PROXY_METHOD(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
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
      ZS_DECLARE_USING_PTR(ortc, RTPPacket)
      ZS_DECLARE_USING_PTR(ortc, RTCPPacket)
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
      //
      // FakeICETransport
      //

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
        //
        // FakeICETransport => (test API)
        //

        ~FakeICETransport();

        static FakeICETransportPtr create(
                                          IMessageQueuePtr queue,
                                          Milliseconds packetDelay = Milliseconds()
                                          );

        void reliability(ULONG percentage);

        void linkTransport(FakeICETransportPtr transport);

        void set_state(IICETransport::States newState);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeICETransport => IICETransport
        //

        //---------------------------------------------------------------------
        ElementPtr toDebug() const noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeICETransport => IFakeICETransportAsyncDelegate
        //

        //---------------------------------------------------------------------
        virtual void onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer) override;

        //---------------------------------------------------------------------
        //
        // FakeICETransport => ITimerDelegate
        //

        //---------------------------------------------------------------------
        virtual void onTimer(ITimerPtr timer) override;

        //---------------------------------------------------------------------
        //
        // FakeICETransport => friend FakeSecureTransport
        //

        void attachSecure(FakeSecureTransportPtr transport);

        void detachSecure(FakeSecureTransport &transport);

        virtual bool sendPacket(
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) noexcept override;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeICETransport => (internal)
        //

        void setState(IICETransportTypes::States state);

        bool isShutdown();

        //---------------------------------------------------------------------
        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeICETransport => (data)
        //

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
      //
      // FakeSecureTransport
      //

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
        //
        // FakeSecureTransport => (testing API)
        //

        ~FakeSecureTransport();

        static FakeSecureTransportPtr create(
                                             IMessageQueuePtr queue,
                                             FakeICETransportPtr iceTransport
                                             );

        void set_state(IDTLSTransport::States newState);

        void setClientRole(bool clientRole);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => IICETransport
        //

        //---------------------------------------------------------------------
        ElementPtr toDebug() const noexcept override;

        //---------------------------------------------------------------------
        //
        // DTLSTransport => ISecureTransportForRTPListener
        //

        // (duplicate) static ElementPtr toDebug(ForRTPListenerPtr transport);

        // (duplicate) virtual PUID getID() const = 0;

        RTPListenerPtr getListener() const noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => ISecureTransportForRTPSender
        //

        PUID getID() const noexcept override;

        bool sendPacket(
                        IICETypes::Components sendOverICETransport,
                        IICETypes::Components packetType,
                        const BYTE *buffer,
                        size_t bufferLengthInBytes
                        ) noexcept override;

        IICETransportPtr getICETransport() const noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => IFakeSecureTransportAsyncDelegate
        //

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => friend FakeICETransport
        //

        //---------------------------------------------------------------------
        bool handleReceivedPacket(
                                  IICETypes::Components component,
                                  const BYTE *buffer,
                                  size_t bufferSizeInBytes
                                  ) noexcept override;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => (internal)
        //

        void setState(IDTLSTransportTypes::States state);

        bool isShutdown();

        Log::Params log(const char *message) const;

        void cancel();

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => (data)
        //

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
      //
      // FakeReceiver
      //

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
        //
        // FakeReceiver => IRTPReceiverForRTPListener
        //

        ElementPtr toDebug() const noexcept override;

        bool handlePacket(
                          IICETypes::Components viaTransport,
                          RTPPacketPtr packet
                          ) noexcept override;

        bool handlePacket(
                          IICETypes::Components viaTransport,
                          RTCPPacketPtr packet
                          ) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeReceiver => (friend RTPListenerTester)
        //

        void set_Transport(RTPListenerTesterPtr tester);

        void expectData(SecureByteBlockPtr data);

        PromisePtr receive(const Parameters &parameters) noexcept override;

        void stop() noexcept override;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiver => (internal)
        //

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
      //
      // FakeSender
      //

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
        //
        // FakeSender => IRTPSenderForRTPListener
        //

        ElementPtr toDebug() const noexcept override;

        // (duplicate) virtual PUID getID() const noexcept = 0;

        bool handlePacket(
                          IICETypes::Components viaTransport,
                          RTCPPacketPtr packet
                          ) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSender => (friend RTPListenerTester)
        //

        void set_Transport(RTPListenerTesterPtr tester);

        PromisePtr send(const Parameters &parameters) noexcept override;
        void stop() noexcept override;

        void expectData(SecureByteBlockPtr data);

        void send_Packet(SecureByteBlockPtr buffer);


      protected:
        //---------------------------------------------------------------------
        //
        // FakeSender => (internal)
        //

        Log::Params log(const char *message) const;


      protected:
        //---------------------------------------------------------------------
        //
        // RTPSender => (data)
        //

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
      //
      // RTPListenerTester
      //

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
        //
        // RTPListenerTester::UnhandledEventData
        //

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
        //
        // RTPListenerTester (api)
        //

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
        //
        // RTPListenerTester::IRTPListenerDelegate
        //

        virtual void onRTPListenerUnhandledRTP(
                                               IRTPListenerPtr listener,
                                               SSRCType ssrc,
                                               PayloadType payloadType,
                                               const char *mid,
                                               const char *rid
                                               ) override;

        //---------------------------------------------------------------------
        //
        // RTPListenerTester => (friend fake sender/receiver)
        //

        FakeSecureTransportPtr getFakeSecureTransport() const;

        void notifyReceivedPacket();

      protected:
        //---------------------------------------------------------------------
        //
        // RTPListenerTester => (internal)
        //

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
        //
        // RTPListenerTester => (data)
        //

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

