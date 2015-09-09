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


#include <zsLib/MessageQueueThread.h>

#include <ortc/IDataChannel.h>
#include <ortc/ISCTPTransport.h>
#include <ortc/ISettings.h>

#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Promise.h>
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

ZS_DECLARE_PROXY_BEGIN(ortc::test::sctp::IFakeICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(openpeer::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::sctp::IFakeSecureTransportAsyncDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(openpeer::services::SecureByteBlockPtr, SecureByteBlockPtr)
//ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace sctp
    {
      using zsLib::Log;
      using zsLib::IPromiseSettledDelegate;
      using zsLib::AutoPUID;

      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(SCTPTester)

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

      public:
        //---------------------------------------------------------------------
        FakeICETransport(
                         const make_private &,
                         IMessageQueuePtr queue
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

        static FakeICETransportPtr create(IMessageQueuePtr queue);

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
        #pragma mark FakeICETransport => IICETransportForDataTransport
        #pragma mark

        virtual PUID getID() const override;

        virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr originalDelegate) override;

        virtual IICETransport::States state() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => IFakeICETransportAsyncDelegate
        #pragma mark

        //---------------------------------------------------------------------
        virtual void onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer);

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

      protected:
        struct make_private {};

      public:
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IICETransportForSecureTransport::UseSecureTransport, UseSecureTransport)

        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IDataTransportForSecureTransport, UseDataTransport)

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
        #pragma mark FakeSecureTransport => IICETransportForDataTransport
        #pragma mark

        virtual PUID getID() const override;

        virtual IDTLSTransportSubscriptionPtr subscribe(IDTLSTransportDelegatePtr originalDelegate) override;

        virtual IDTLSTransport::States state() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => ISecureTransportForDataTransport
        #pragma mark

        // (duplicate) virtual PUID getID() const;

        virtual PromisePtr notifyWhenReady();
        virtual PromisePtr notifyWhenClosed();

        virtual bool isClientRole() const;

        virtual IICETransportPtr getICETransport() const;

        virtual UseDataTransportPtr getDataTransport() const;

        virtual bool sendDataPacket(
                                    const BYTE *buffer,
                                    size_t bufferLengthInBytes
                                    );

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
                                  );

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

        IDTLSTransportDelegateSubscriptions mSubscriptions;
        IDTLSTransportSubscriptionPtr mDefaultSubscription;

        std::list<PromisePtr> mNotifyReadyPromises;
        std::list<PromisePtr> mNotifyClosedPromises;

        UseDataTransportPtr mDataTransport;
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTester
      #pragma mark

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
          ULONG mStateConnecting {0};
          ULONG mStateOpen {0};
          ULONG mStateClosing {0};
          ULONG mStateClosed {0};

          ULONG mError {0};

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
                                    Optional<WORD> removePort = Optional<WORD>()
                                    );

        SCTPTester(IMessageQueuePtr queue);

        ~SCTPTester();

        void init(
                  bool createSCTPNow,
                  Optional<WORD> localPort,
                  Optional<WORD> removePort
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

      protected:

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark SCTPTester::ISCTPTransportDelegate
        #pragma mark

        virtual void onSCTPTransportDataChannel(
                                                ISCTPTransportPtr transport,
                                                IDataChannelPtr channel
                                                ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark SCTPTester::ISCTPTransportListenerDelegate
        #pragma mark

        virtual void onSCTPTransport(ISCTPTransportPtr transport);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark SCTPTester::IDataChannelDelegate
        #pragma mark

        virtual void onDataChannelStateChanged(
                                               IDataChannelPtr channel,
                                               States state
                                               );

        virtual void onDataChannelError(
                                        IDataChannelPtr channel,
                                        ErrorCode errorCode,
                                        String errorReason
                                        );

        virtual void onDataChannelMessage(
                                          IDataChannelPtr channel,
                                          MessageEventDataPtr data
                                          );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark SCTPTester => (internal)
        #pragma mark

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
        #pragma mark
        #pragma mark SCTPTester => (data)
        #pragma mark

        AutoPUID mID;
        SCTPTesterWeakPtr mThisWeak;

        FakeICETransportPtr mICETransport;
        FakeSecureTransportPtr mDTLSTransport;
        ISCTPTransportPtr mSCTP;

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

