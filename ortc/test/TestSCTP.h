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
#include <ortc/ISCTPTransport.h>

#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>

#include <ortc/services/IHelper.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
#include <zsLib/Promise.h>
#include <zsLib/ITimer.h>
#include <zsLib/Log.h>

#include "config.h"
#include "testing.h"

namespace ortc
{
  namespace test
  {
    namespace sctp
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

ZS_DECLARE_PROXY_BEGIN(ortc::test::sctp::IFakeICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_METHOD(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::sctp::IFakeSecureTransportAsyncDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
//ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace sctp
    {
      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::Milliseconds;

      ZS_DECLARE_USING_PTR(zsLib, ITimer)

      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(SCTPTester)

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

        void state(IICETransport::States newState);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeICETransport => IICETransport
        //

        //---------------------------------------------------------------------
        ElementPtr toDebug() const noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeICETransport => IICETransportForDataTransport
        //

        PUID getID() const noexcept override;

        IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr originalDelegate) noexcept override;

        IICETransport::States state() const noexcept override;

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

        bool sendPacket(
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

      protected:
        struct make_private {};

      public:
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IICETransportForSecureTransport::UseSecureTransport, UseSecureTransport)

        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IDataTransportForSecureTransport, UseDataTransport)

        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportTypes, ISecureTransportTypes)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransport, ISecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportDelegate, ISecureTransportDelegate)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportSubscription, ISecureTransportSubscription)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportDelegateSubscriptions, ISecureTransportDelegateSubscriptions)

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

        void state(IDTLSTransport::States newState);

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
        // FakeSecureTransport => IICETransportForDataTransport
        //

        PUID getID() const noexcept override;

        IDTLSTransportSubscriptionPtr subscribe(IDTLSTransportDelegatePtr originalDelegate) noexcept override;

        IDTLSTransport::States state() const noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => ISecureTransportForDataTransport
        //

        // (duplicate) virtual PUID getID() const;

        ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) noexcept override;

        ISecureTransportTypes::States state(ISecureTransportTypes::States ignored) const noexcept override;

        bool isClientRole() const noexcept override;

        IICETransportPtr getICETransport() const noexcept override;

        UseDataTransportPtr getDataTransport() const noexcept override;

        bool sendDataPacket(
                            const BYTE *buffer,
                            size_t bufferLengthInBytes
                            ) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => IFakeSecureTransportAsyncDelegate
        //

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => friend FakeICETransport
        //

        bool handleReceivedPacket(
                                  IICETypes::Components component,
                                  const BYTE *buffer,
                                  size_t bufferSizeInBytes
                                  ) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => friend IICETransportDelegate
        //

        virtual void onICETransportStateChange(
                                               IICETransportPtr transport,
                                               IICETransport::States state
                                               ) override;

        virtual void onICETransportCandidatePairAvailable(
                                                          IICETransportPtr transport,
                                                          CandidatePairPtr candidatePair
                                                          ) override;
        virtual void onICETransportCandidatePairGone(
                                                     IICETransportPtr transport,
                                                     CandidatePairPtr candidatePair
                                                     ) override;

        virtual void onICETransportCandidatePairChanged(
                                                        IICETransportPtr transport,
                                                        CandidatePairPtr candidatePair
                                                        ) override;
      protected:
        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => (internal)
        //

        void setState(IDTLSTransportTypes::States state);
        void setState(ISecureTransportTypes::States state);

        bool isShutdown();

        Log::Params log(const char *message) const;

        void cancel();
        void fixState();

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => (data)
        //

        FakeSecureTransportWeakPtr mThisWeak;

        FakeICETransportPtr mICETransport;
        IICETransportTypes::States mICETransportState {IICETransportTypes::State_New};
        IICETransportSubscriptionPtr mICETransportSubscription;

        IDTLSTransportTypes::States mCurrentState {IDTLSTransportTypes::State_New};

        bool mClientRole {false};

        IDTLSTransportDelegateSubscriptions mSubscriptions;
        IDTLSTransportSubscriptionPtr mDefaultSubscription;

        ISecureTransportDelegateSubscriptions mSecureTransportSubscriptions;
        std::atomic<ISecureTransportTypes::States> mSecureTransportState {State_Pending};

        UseDataTransportPtr mDataTransport;
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //
      // SCTPTester
      //

      //---------------------------------------------------------------------
      class SCTPTester : public SharedRecursiveLock,
                         public zsLib::MessageQueueAssociator,
                         public ISCTPTransportDelegate,
                         public ISCTPTransportListenerDelegate,
                         public IDataChannelDelegate
      {
      protected:
        struct make_private {};

      public:
        struct Expectations {
          // data channel related
          ULONG mIncoming {0};
          ULONG mStateConnecting {0};
          ULONG mStateOpen {0};
          ULONG mStateClosing {0};
          ULONG mStateClosed {0};

          ULONG mReceivedBinary {0};
          ULONG mReceivedText {0};

          ULONG mError {0};

          // transport
          ULONG mTransportIncoming {0};

          ULONG mTransportStateNew {0};
          ULONG mTransportStateConnecting {0};
          ULONG mTransportStateConnected {0};
          ULONG mTransportStateClosed {0};

          bool operator==(const Expectations &op2) const;
        };

        typedef std::map<String, IDataChannelPtr> DataChannelMap;

        typedef std::list<SecureByteBlockPtr> BufferList;
        typedef std::map<String, BufferList> BufferMap;

        typedef std::list<String> StringList;
        typedef std::map<String, StringList> StringMap;

      public:
        static SCTPTesterPtr create(
                                    IMessageQueuePtr queue,
                                    bool createSCTPNow = true,
                                    Optional<WORD> localPort = Optional<WORD>(),
                                    Optional<WORD> remotePort = Optional<WORD>(),
                                    Milliseconds packetDelay = Milliseconds()
                                    );

        SCTPTester(IMessageQueuePtr queue);

        ~SCTPTester();

        void init(
                  bool createSCTPNow,
                  Optional<WORD> localPort,
                  Optional<WORD> remotePort,
                  Milliseconds packetDelay
                  );

        bool matches(const Expectations &op2);

        void close();
        void closeByReset();

        Expectations getExpectations() const;

        void reliability(ULONG percentage);

        void state(IICETransport::States newState);

        void state(IDTLSTransport::States newState);

        void setClientRole(bool clientRole);

        void listen();
        void start(SCTPTesterPtr remote);

        void createChannel(const IDataChannel::Parameters &params);

        void sendData(
                      const char *channelID,
                      SecureByteBlockPtr buffer
                      );

        void sendData(
                      const char *channelID,
                      const String &message
                      );

        void closeChannel(const char *channelID);

      protected:

        //---------------------------------------------------------------------
        //
        // SCTPTester::ISCTPTransportDelegate
        //

        virtual void onSCTPTransportDataChannel(
                                                ISCTPTransportPtr transport,
                                                IDataChannelPtr channel
                                                ) override;

        virtual void onSCTPTransportStateChange(
                                                ISCTPTransportPtr transport,
                                                ISCTPTransportTypes::States state
                                                ) override;

        //---------------------------------------------------------------------
        //
        // SCTPTester::ISCTPTransportListenerDelegate
        //

        virtual void onSCTPTransport(ISCTPTransportPtr transport) override;

        //---------------------------------------------------------------------
        //
        // SCTPTester::IDataChannelDelegate
        //

        virtual void onDataChannelStateChange(
                                              IDataChannelPtr channel,
                                              IDataChannelTypes::States state
                                              ) override;

        virtual void onDataChannelError(
                                        IDataChannelPtr channel,
                                        ErrorAnyPtr error
                                        ) override;

        virtual void onDataChannelBufferedAmountLow(IDataChannelPtr channel) override;

        virtual void onDataChannelMessage(
                                          IDataChannelPtr channel,
                                          MessageEventDataPtr data
                                          ) override;

      protected:
        //---------------------------------------------------------------------
        //
        // SCTPTester => (internal)
        //

        Log::Params log(const char *message) const;

        FakeICETransportPtr getICETransport() const;

        void expectData(
                        const char *channelID,
                        SecureByteBlockPtr data
                        );
        void expectData(
                        const char *channelID,
                        const String &message
                        );

      public:
        //---------------------------------------------------------------------
        //
        // SCTPTester => (data)
        //

        AutoPUID mID;
        SCTPTesterWeakPtr mThisWeak;

        FakeICETransportPtr mICETransport;
        FakeSecureTransportPtr mDTLSTransport;
        ISCTPTransportPtr mSCTP;
        Optional<WORD> mRemotePort {};

        SCTPTesterWeakPtr mConnectedTester;

        ISCTPTransportListenerSubscriptionPtr mListenerSubscription;

        Expectations mExpectations;

        DataChannelMap mDataChannels;

        BufferMap mBuffers;
        StringMap mStrings;
      };
    }
  }
}

