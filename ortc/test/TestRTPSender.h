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
#include <ortc/IRTPSender.h>

#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>

#include <ortc/services/IHelper.h>

#include <zsLib/ISettings.h>
#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ITimer.h>
#include <zsLib/Promise.h>
#include <zsLib/Log.h>

#include "config.h"
#include "testing.h"

namespace ortc
{
  namespace test
  {
    namespace rtpsender
    {
      ZS_DECLARE_INTERACTION_PROXY(IFakeICETransportAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeSecureTransportAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeListenerAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeSenderChannelAsyncDelegate)

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

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeListenerAsyncDelegate
      #pragma mark

      interaction IFakeListenerAsyncDelegate
      {
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPPacket, RTPPacket)

        virtual void onForwardBufferedPacket(RTPPacketPtr packet) = 0;
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
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPPacket, RTPPacket)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTCPPacket, RTCPPacket)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportTypes, ISecureTransportTypes)
        ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::Parameters, Parameters)
        ZS_DECLARE_TYPEDEF_PTR(std::list<RTCPPacketPtr>, RTCPPacketList)

        virtual void onState(ISecureTransportTypes::States state) = 0;
        virtual void onRTCPPackets(RTCPPacketListPtr packets) = 0;
        virtual void onUpdate(ParametersPtr params) = 0;
      };
    }
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpsender::IFakeICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpsender::IFakeSecureTransportAsyncDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
//ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpsender::IFakeListenerAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD_1(onForwardBufferedPacket, RTPPacketPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpsender::IFakeSenderChannelAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::RTCPPacketPtr, RTCPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransportTypes::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPTypes::Parameters, Parameters)
ZS_DECLARE_PROXY_METHOD_1(onState, States)
ZS_DECLARE_PROXY_METHOD_1(onRTCPPackets, RTCPPacketListPtr)
ZS_DECLARE_PROXY_METHOD_1(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace rtpsender
    {
      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::Milliseconds;

      ZS_DECLARE_USING_PTR(zsLib, ITimer)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPPacket)
      ZS_DECLARE_USING_PTR(ortc::internal, RTCPPacket)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSender)
      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack)

      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(FakeListener)
      ZS_DECLARE_CLASS_PTR(FakeSenderChannel)
      ZS_DECLARE_CLASS_PTR(FakeMediaStreamTrack)
      ZS_DECLARE_CLASS_PTR(FakeReceiver)
      ZS_DECLARE_CLASS_PTR(RTPSenderTester)

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

        virtual States state() const override;

        virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr originalDelegate) override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => (internal)
        #pragma mark

        void setState(IICETransportTypes::States state);

        bool isShutdown();

        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => (data)
        #pragma mark

        FakeICETransportWeakPtr mThisWeak;

        IICETypes::Components mComponent {IICETypes::Component_RTP};

        std::atomic<IICETransportTypes::States> mCurrentState {IICETransportTypes::State_New};

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
        friend class RTPSenderTester;

      protected:
        struct make_private {};

      public:
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPListener, RTPListener)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IICETransportForSecureTransport::UseSecureTransport, UseSecureTransport)

        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IRTPListenerForSecureTransport, UseListener)

        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportSubscription, ISecureTransportSubscription)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportDelegate, ISecureTransportDelegate)
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

        // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr transport);

        // (duplicate) virtual PUID getID() const = 0;

        virtual RTPListenerPtr getListener() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => ISecureTransportForRTPSender
        #pragma mark

        // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr transport);

        // (not needed) static void getSendingTransport(
        //                                              IRTPTransportPtr inRTPTransport,
        //                                              IRTCPTransportPtr inRTCPTransport,
        //                                              IICETypes::Components &outWhenSendingRTPUseSendOverComponent,
        //                                              IICETypes::Components &outWhenSendingRTCPUseSendOverComponent,
        //                                              ForRTPSenderPtr &outRTPSecureTransport,
        //                                              ForRTPSenderPtr &outRTCPSecureTransport
        //                                              );

        // (duplicate) virtual PUID getID() const = 0;

        // (duplicate) virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) = 0;

        // (duplicate) virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored = ISecureTransportTypes::States()) const = 0;

        // (duplicate) virtual bool sendPacket(
        //                                     IICETypes::Components sendOverICETransport,
        //                                     IICETypes::Components packetType,
        //                                     const BYTE *buffer,
        //                                     size_t bufferLengthInBytes
        //                                     ) = 0;

        // (duplicate) virtual IICETransportPtr getICETransport() const = 0;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => ISecureTransportForRTPSender
        #pragma mark

        // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr transport);

        // (not needed) static void getSendingTransport(
        //                                              IRTPTransportPtr inRTPTransport,
        //                                              IRTCPTransportPtr inRTCPTransport,
        //                                              IICETypes::Components &outWhenSendingRTPUseSendOverComponent,
        //                                              IICETypes::Components &outWhenSendingRTCPUseSendOverComponent,
        //                                              ForRTPSenderPtr &outRTPSecureTransport,
        //                                              ForRTPSenderPtr &outRTCPSecureTransport
        //                                              );

        virtual PUID getID() const override;

        virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) override;

        virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored) const override;

        virtual bool sendPacket(
                                IICETypes::Components sendOverICETransport,
                                IICETypes::Components packetType,
                                const BYTE *buffer,
                                size_t bufferLengthInBytes
                                ) override;

        virtual IICETransportPtr getICETransport() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => IICETransportDelegate
        #pragma mark

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

        virtual States state() const override;

        void notifySecureState();
        ISecureTransportTypes::States getSecureState() const;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => (data)
        #pragma mark

        FakeSecureTransportWeakPtr mThisWeak;

        FakeICETransportPtr mICETransport;
        IICETransportSubscriptionPtr mICESubscription;

        std::atomic<IDTLSTransportTypes::States> mCurrentState {IDTLSTransportTypes::State_New};

        bool mClientRole {false};

        UseListenerPtr mListener;

        ISecureTransportDelegateSubscriptions mSubscriptions;
        ISecureTransportSubscriptionPtr mDefaultSubscription;

        ISecureTransportTypes::States mLastReportedState {ISecureTransportTypes::State_Pending};
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeListener : public ortc::internal::RTPListener,
                           public IFakeListenerAsyncDelegate
      {
      public:
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverForRTPListener, UseReceiver)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderForRTPListener, UseSender)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPListener, RTPListener)

        struct UnhandledData {
          String mMuxID;
          String mRID;
          IRTPTypes::SSRCType mSSRC {};
          IRTPTypes::PayloadType mPayloadType {};
        };

        typedef std::list<UnhandledData> UnhandledDataList;

        typedef std::list<UseSenderPtr> SenderList;
        typedef std::list<UseSenderWeakPtr> SenderWeakList;

      protected:
        void init();

      public:
        FakeListener(IMessageQueuePtr queue);
        ~FakeListener();

        static FakeListenerPtr create(IMessageQueuePtr queue);
        static FakeListenerPtr convert(RTPListenerPtr listener);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeListener => IRTPListenerForRTPReceiver
        #pragma mark

        virtual ElementPtr toDebug() const override;

        // (not needed) static ForRTPReceiverPtr getListener(IRTPTransportPtr rtpTransport);

        // (base hanlded) virtual PUID getID() const = 0;

        virtual void registerReceiver(
                                      Optional<IMediaStreamTrackTypes::Kinds> kind,
                                      UseReceiverPtr inReceiver,
                                      const Parameters &inParams,
                                      RTCPPacketList *outPacketList = NULL
                                      ) override;

        virtual void unregisterReceiver(UseReceiver &inReceiver) override;

        virtual void getPackets(RTCPPacketList &outPacketList) override;

        virtual void notifyUnhandled(
                                     const String &muxID,
                                     const String &rid,
                                     IRTPTypes::SSRCType ssrc,
                                     IRTPTypes::PayloadType payloadType
                                     ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeListener => IRTPListenerForRTPSender
        #pragma mark

        // (duplciate) static ElementPtr toDebug(ForRTPSenderPtr listener);

        // (not needed) static ForRTPSenderPtr getListener(IRTPTransportPtr rtpTransport);

        // (base handled) virtual PUID getID() const = 0;

        virtual void registerSender(
                                    UseSenderPtr inSender,
                                    const Parameters &inParams,
                                    RTCPPacketList &outPacketList
                                    ) override;

        virtual void unregisterSender(UseSender &inSender) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeListener => IRTPListenerForSecureTransport
        #pragma mark

        // (duplicate) virtual ElementPtr toDebug() override;

        // (not needed) static RTPListenerPtr create(IRTPTransportPtr transport);

        // (base handles) virtual PUID getID() const = 0;

        virtual bool handleRTPPacket(
                                     IICETypes::Components viaComponent,
                                     IICETypes::Components packetType,
                                     const BYTE *buffer,
                                     size_t bufferLengthInBytes
                                     ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeListener => Timer
        #pragma mark

        virtual void onTimer(ITimerPtr timer) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeListener => IFakeListenerAsyncDelegate
        #pragma mark

        virtual void onForwardBufferedPacket(RTPPacketPtr packet) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeListener => (friend RTPSenderTester)
        #pragma mark

        void setTransport(RTPSenderTesterPtr tester);

        virtual IRTPListenerSubscriptionPtr subscribe(IRTPListenerDelegatePtr originalDelegate) override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeListener => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

        void cancel();

      protected:
        FakeListenerWeakPtr mThisWeak;

        RTPSenderTesterWeakPtr mTester;

        IRTPListenerDelegateSubscriptions mSubscriptions;
        IRTPListenerSubscriptionPtr mDefaultSubscription;

        ITimerPtr mCleanBuffersTimer;

        BufferedRTPPacketList mBufferedRTPPackets;
        BufferedRTCPPacketList mBufferedRTCPPackets;

        UseReceiverWeakPtr mReceiver;

        UnhandledDataList mUnhandled;

        SenderWeakList mSenders;
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
      public:
        friend class RTPSenderTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransportTypes, ISecureTransportTypes)
        ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
        ZS_DECLARE_TYPEDEF_PTR(IFakeSenderChannelAsyncDelegate::RTCPPacketList, RTCPPacketList)

        typedef std::list<SecureByteBlockPtr> BufferList;

        typedef std::list<ParametersPtr> ParametersList;

        typedef std::list<ISecureTransportTypes::States> StateList;

        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransport, ISecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderForRTPSenderChannel, UseSender)

      public:
        FakeSenderChannel(
                          IMessageQueuePtr queue,
                          const char *senderChannelID
                          );
        ~FakeSenderChannel();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => IRTPSenderChannelForRTPSender
        #pragma mark

        virtual ElementPtr toDebug() const override;

        // (base handles) virtual PUID getID() const = 0;

        void create(
                    RTPSenderPtr sender,
                    MediaStreamTrackPtr track,
                    const Parameters &params
                    );

        virtual PUID getID() const override {return mID;}

        virtual void notifyTransportState(ISecureTransport::States state) override;

        virtual void notifyPackets(RTCPPacketListPtr packets) override;

        virtual void notifyUpdate(const Parameters &params) override;

        virtual bool handlePacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => IFakeSenderChannelAsyncDelegate
        #pragma mark

        virtual void onState(ISecureTransportTypes::States state) override;
        virtual void onRTCPPackets(RTCPPacketListPtr packets) override;
        virtual void onUpdate(ParametersPtr params) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => (friend RTPSenderTester)
        #pragma mark

        static FakeSenderChannelPtr create(
                                             IMessageQueuePtr queue,
                                             const char *senderChannelID,
                                             const Parameters &expectedParams
                                             );

        void setTransport(RTPSenderTesterPtr tester);

        bool sendPacket(RTPPacketPtr packet);
        bool sendPacket(RTCPPacketPtr packet);

        void expectState(ISecureTransport::States state);

        void expectUpdate(const Parameters &params);

        void expectData(SecureByteBlockPtr data);

        void stop();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

      protected:
        FakeSenderChannelWeakPtr mThisWeak;

        RTPSenderTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;
        ParametersList mExpectParameters;
        StateList mExpectStates;

        UseSenderWeakPtr mSender;
        String mSenderChannelID;
      };



      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeMediaStreamTrack : public ortc::internal::MediaStreamTrack
      {
      public:
        typedef PUID SenderChannelID;
        typedef std::list<SenderChannelID> SenderChannelIDList;

        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransport, ISecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannel, RTPSenderChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderForMediaStreamTrack, UseSender)

      public:
        FakeMediaStreamTrack(IMessageQueuePtr queue);
        ~FakeMediaStreamTrack();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPSender
        #pragma mark

        virtual ElementPtr toDebug() const override;

        void create(IMediaStreamTrackTypes::Kinds kind);

        // (base handles) virtual PUID getID() const = 0;

        virtual Kinds kind() const override;

        virtual void notifyAttachSenderChannel(RTPSenderChannelPtr channel) override;
        virtual void notifyDetachSenderChannel(RTPSenderChannelPtr channel) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSenderChannel => (friend RTPSenderTester)
        #pragma mark

        static FakeMediaStreamTrackPtr create(
                                              IMessageQueuePtr queue,
                                              IMediaStreamTrackTypes::Kinds kind
                                              );

        void setTransport(RTPSenderTesterPtr tester);

        void expectAttachChannel(SenderChannelID channelID);
        void expectDetachChannel(SenderChannelID channelID);

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
        FakeMediaStreamTrackWeakPtr mThisWeak;

        IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrackTypes::Kind_Audio};

        RTPSenderTesterWeakPtr mTester;

        UseSenderWeakPtr mSender;

        SenderChannelIDList mExpectAttachChannelIDs;
        SenderChannelIDList mExpectDetachChannelIDs;
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
        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransportForRTPReceiver, UseSecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPListenerForRTPReceiver, UseListener)

        typedef std::list<SecureByteBlockPtr> BufferList;

      public:
        FakeReceiver(
                     IMessageQueuePtr queue,
                     IMediaStreamTrackTypes::Kinds kind
                     );
        ~FakeReceiver();

        static FakeReceiverPtr create(
                                      IMessageQueuePtr queue,
                                      IMediaStreamTrackTypes::Kinds kind
                                      );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeReceiver => IRTPReceiverForRTPListener
        #pragma mark

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

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
        #pragma mark FakeReceiver => (friend RTPSenderTester)
        #pragma mark

        void setTransport(RTPSenderTesterPtr tester);

        virtual PromisePtr receive(const Parameters &parameters) override;
        virtual void stop() override;

        void expectData(SecureByteBlockPtr data);

        void sendPacket(SecureByteBlockPtr buffer);

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

        IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrackTypes::Kind_Audio};

        RTPSenderTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;

        UseListenerPtr mListener;
        UseSecureTransportPtr mSecureTransport;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester
      #pragma mark

      //-----------------------------------------------------------------------
      class RTPSenderTester : public SharedRecursiveLock,
                              public zsLib::MessageQueueAssociator,
                              public IRTPListenerDelegate,
                              public IRTPSenderDelegate
      {
      public:
        friend class FakeSenderChannel;
        friend class FakeReceiver;
        friend class FakeListener;
        friend class FakeMediaStreamTrack;

        ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransportTypes, ISecureTransportTypes)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannel, RTPSenderChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelFactory, RTPSenderChannelFactory)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannel::RTCPPacketList, RTCPPacketList)

        ZS_DECLARE_TYPEDEF_PTR(internal::MediaStreamTrack, MediaStreamTrack)
        ZS_DECLARE_TYPEDEF_PTR(internal::MediaStreamTrackFactory, MediaStreamTrackFactory)

        ZS_DECLARE_CLASS_PTR(OverrideSenderChannelFactory)
        ZS_DECLARE_CLASS_PTR(OverrideMediaStreamTrackFactory)

        friend class OverrideSenderChannelFactory;
        friend class OverrideMediaStreamTrackFactory;

        typedef String SenderOrReceiverChannelID;
        typedef std::pair<FakeSenderChannelWeakPtr, FakeReceiverPtr> FakeSenderChannelFakeReceiverPair;
        typedef std::map<SenderOrReceiverChannelID, FakeSenderChannelFakeReceiverPair> SenderOrReceiverMap;

        typedef String PacketID;
        typedef std::pair<RTPPacketPtr, RTCPPacketPtr> PacketPair;
        typedef std::map<PacketID, PacketPair> PacketMap;

        typedef String SenderChannelID;
        typedef std::pair<SenderChannelID, FakeSenderChannelPtr> FakeSenderChannelPair;
        typedef std::list<FakeSenderChannelPair> FakeSenderChannelList;

        typedef String ParametersID;
        typedef std::map<ParametersID, ParametersPtr> ParametersMap;

        typedef std::list<SenderChannelID> SenderIDList;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester::UnhandledEventData
        #pragma mark

        struct UnhandledEventData
        {
          UnhandledEventData(
                             DWORD ssrc,
                             BYTE pt,
                             const char *mid,
                             const char *rid
                             );

          bool operator==(const UnhandledEventData &op2) const;

          DWORD mSSRC {};
          BYTE mPT {};
          String mMID;
          String mRID;
        };

        typedef std::list<UnhandledEventData> UnhandledEventDataList;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester::OverrideSenderChannelFactory
        #pragma mark

        class OverrideSenderChannelFactory : public RTPSenderChannelFactory
        {
        public:
          static OverrideSenderChannelFactoryPtr create(RTPSenderTesterPtr tester);

          virtual RTPSenderChannelPtr create(
                                             RTPSenderPtr sender,
                                             MediaStreamTrackPtr track,
                                             const Parameters &params
                                             ) override;

        protected:
          RTPSenderTesterWeakPtr mTester;
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester::OverrideSenderChannelFactory
        #pragma mark

        class OverrideMediaStreamTrackFactory : public MediaStreamTrackFactory
        {
        public:
          static OverrideMediaStreamTrackFactoryPtr create(RTPSenderTesterPtr tester);

          virtual MediaStreamTrackPtr create(
                                             IMediaStreamTrackTypes::Kinds kind,
                                             bool remote,
                                             TrackConstraintsPtr constraints
                                             );
          virtual MediaStreamTrackPtr create(IMediaStreamTrackTypes::Kinds kind);

        protected:
          RTPSenderTesterWeakPtr mTester;
        };

      protected:
        struct make_private {};

      public:
        struct Expectations {
          // listener related
          ULONG mUnhandled {0};

          // general
          ULONG mReceivedPackets {0};

          // sender cannel related
          ULONG mError {0};
          ULONG mSSRCconflict {0};
          ULONG mChannelUpdate {0};
          ULONG mSenderChannelOfSecureTransportState {0};

          // media stream track
          ULONG mAttachSenderChannel {0};
          ULONG mDetachSenderChannel {0};

          IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrack::Kind_Audio};

          bool operator==(const Expectations &op2) const;
        };

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester (api)
        #pragma mark

        static RTPSenderTesterPtr create(
                                         IMessageQueuePtr queue,
                                         IMediaStreamTrackTypes::Kinds kind,
                                         bool overrideFactories,
                                         Milliseconds packetDelay = Milliseconds()
                                         );

        RTPSenderTester(
                        IMessageQueuePtr queue,
                        bool overrideFactories
                        );
        ~RTPSenderTester();

        void init(
                  IMediaStreamTrackTypes::Kinds kind,
                  Milliseconds packetDelay
                  );

        bool matches();

        void close();
        void closeByReset();

        Expectations getExpectations() const;

        void reliability(ULONG percentage);

        void state(IICETransport::States newState);

        void state(IDTLSTransport::States newState);

        void setClientRole(bool clientRole);

        void connect(RTPSenderTesterPtr remote);

        void createSenderChannel(
                                 const char *senderChannelID,
                                 const char *parametersID
                                 );
        void createReceiver(const char *senderID);

        void receive(
                     const char *receiverID,
                     const char *parametersID
                     );

        void send(const char *parametersID);

        void stop(const char *senderOrSenderChannelID);

        void attach(
                    const char *senderChannelID,
                    FakeSenderChannelPtr sender
                    );
        void attach(
                    const char *receiverID,
                    FakeReceiverPtr receiver
                    );

        FakeSenderChannelPtr detachSenderChannel(const char *senderChannelID);
        FakeReceiverPtr detachReceiver(const char *receiverID);

        void expectKind(IMediaStreamTrackTypes::Kinds kind);

        void expectingUnhandled(
                                IRTPTypes::SSRCType ssrc,
                                IRTPTypes::PayloadType payloadType,
                                const char *mid,
                                const char *rid
                                );

        void expectReceiveChannelUpdate(
                                        const char *senderChannelID,
                                        const char *parametersID
                                        );

        void expectState(
                         const char *senderChannelID,
                         ISecureTransportTypes::States state
                         );

        void expectAttachChannel(const char *senderChannelID);
        void expectDetachChannel(const char *senderChannelID);

        void store(
                   const char *packetID,
                   RTPPacketPtr packet
                   );
        void store(
                   const char *packetID,
                   RTCPPacketPtr packet
                   );
        void store(
                   const char *parametersID,
                   const Parameters &params
                   );

        RTPPacketPtr getRTPPacket(const char *packetID);
        RTCPPacketPtr getRTCPPacket(const char *packetID);
        ParametersPtr getParameters(const char *parametersID);

        void sendPacket(
                        const char *receiverOrSenderChannelID,
                        const char *packetID
                        );

        void expectPacket(
                          const char *receiverOrSenderChannelID,
                          const char *packetID
                          );

      protected:

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester::IRTPListenerDelegate
        #pragma mark

        virtual void onRTPListenerUnhandledRTP(
                                               IRTPListenerPtr listener,
                                               IRTPTypes::SSRCType ssrc,
                                               IRTPTypes::PayloadType payloadType,
                                               const char *mid,
                                               const char *rid
                                               ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester::IRTPSenderDelegate
        #pragma mark

        virtual void onRTPSenderSSRCConflict(
                                             IRTPSenderPtr sender,
                                             IRTPTypes::SSRCType ssrc
                                             ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester => (friend fake listener, sender, sender channel)
        #pragma mark

        FakeSecureTransportPtr getFakeSecureTransport() const;

        void notifyReceivedPacket();
        void notifyReceivedChannelUpdate();
        void notifyAttachSenderChannel();
        void notifyDetachSenderChannel();
        void notifySenderChannelOfSecureTransportState();

        RTPSenderChannelPtr create(
                                   RTPSenderPtr sender,
                                   const Parameters &params
                                   );

        MediaStreamTrackPtr create(IMediaStreamTrackTypes::Kinds kind);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

        FakeICETransportPtr getICETransport() const;

        FakeSenderChannelPtr getSenderChannel(const char *senderChannelID);
        FakeReceiverPtr getReceiver(const char *receiverID);

        void expectData(
                        const char *senderOrReceiverID,
                        SecureByteBlockPtr secureBuffer
                        );
        void sendData(
                      const char *receiverOrSenderChannelID,
                      SecureByteBlockPtr secureBuffer
                      );

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderTester => (data)
        #pragma mark

        AutoPUID mID;
        RTPSenderTesterWeakPtr mThisWeak;

        bool mOverrideFactories {false};

        FakeICETransportPtr mICETransport;
        FakeSecureTransportPtr mDTLSTransport;

        FakeMediaStreamTrackPtr mMediaStreamTrack;

        IRTPSenderPtr mSender;

        RTPSenderTesterWeakPtr mConnectedTester;

        IRTPListenerSubscriptionPtr mListenerSubscription;
        IRTPSenderSubscriptionPtr mSenderSubscription;

        Expectations mExpecting;
        Expectations mExpectationsFound;

        SenderOrReceiverMap mAttached;
        PacketMap mPackets;

        FakeSenderChannelList mFakeSenderChannelCreationList;

        UnhandledEventDataList mExpectingUnhandled;

        ParametersMap mParameters;
      };
    }
  }
}

