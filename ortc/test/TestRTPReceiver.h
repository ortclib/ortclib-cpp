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
#include <ortc/IRTPReceiver.h>

#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>

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
    namespace rtpreceiver
    {
      ZS_DECLARE_INTERACTION_PROXY(IFakeICETransportAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeSecureTransportAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeListenerAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeReceiverChannelAsyncDelegate)

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

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //
      // IFakeListenerAsyncDelegate
      //

      interaction IFakeListenerAsyncDelegate
      {
        ZS_DECLARE_TYPEDEF_PTR(ortc::RTPPacket, RTPPacket)

        virtual void onForwardBufferedPacket(RTPPacketPtr packet) = 0;
      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //
      // IFakeReceiverChannelAsyncDelegate
      //

      interaction IFakeReceiverChannelAsyncDelegate
      {

        ZS_DECLARE_TYPEDEF_PTR(ortc::RTPPacket, RTPPacket)
        ZS_DECLARE_TYPEDEF_PTR(ortc::RTCPPacket, RTCPPacket)
        ZS_DECLARE_TYPEDEF_PTR(std::list<RTCPPacketPtr>, RTCPPacketList)
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportTypes, ISecureTransportTypes)
        ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::Parameters, Parameters)

        virtual void onState(ISecureTransportTypes::States state) = 0;
        virtual void onRTPPacket(RTPPacketPtr packet) = 0;
        virtual void onRTCPPacket(RTCPPacketPtr packet) = 0;
        virtual void onRTCPPackets(RTCPPacketListPtr packets) = 0;
        virtual void onUpdate(ParametersPtr params) = 0;
      };
    }
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpreceiver::IFakeICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_METHOD(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpreceiver::IFakeSecureTransportAsyncDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
//ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpreceiver::IFakeListenerAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD(onForwardBufferedPacket, RTPPacketPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpreceiver::IFakeReceiverChannelAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTCPPacketPtr, RTCPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransportTypes::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPTypes::Parameters, Parameters)
ZS_DECLARE_PROXY_METHOD(onState, States)
ZS_DECLARE_PROXY_METHOD(onRTPPacket, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD(onRTCPPacket, RTCPPacketPtr)
ZS_DECLARE_PROXY_METHOD(onRTCPPackets, RTCPPacketListPtr)
ZS_DECLARE_PROXY_METHOD(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace rtpreceiver
    {
      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::Milliseconds;

      ZS_DECLARE_USING_PTR(zsLib, ITimer)
      ZS_DECLARE_USING_PTR(ortc, RTPPacket)
      ZS_DECLARE_USING_PTR(ortc, RTCPPacket)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiver)
      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack)

      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(FakeListener)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannel)
      ZS_DECLARE_CLASS_PTR(FakeMediaStreamTrack)
      ZS_DECLARE_CLASS_PTR(FakeSender)
      ZS_DECLARE_CLASS_PTR(RTPReceiverTester)

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
        // FakeICETransport => IFakeICETransportAsyncDelegate
        //

        //---------------------------------------------------------------------
        void onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer) override;

        //---------------------------------------------------------------------
        //
        // FakeICETransport => ITimerDelegate
        //

        //---------------------------------------------------------------------
        void onTimer(ITimerPtr timer) override;

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

        States state() const noexcept override;

        IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr originalDelegate) noexcept override;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeICETransport => (internal)
        //

        void setState(IICETransportTypes::States state);

        bool isShutdown();

        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeICETransport => (data)
        //

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
      //
      // FakeSecureTransport
      //

      //---------------------------------------------------------------------
      class FakeSecureTransport : public ortc::internal::DTLSTransport,
                                  public IFakeSecureTransportAsyncDelegate
      {
      public:
        friend class FakeICETransport;
        friend class FakeSender;
        friend class RTPReceiverTester;

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
        // DTLSTransport => ISecureTransportForRTPListener
        //

        // (duplicate) static ElementPtr toDebug(ForRTPReceiverPtr transport);

        // (duplicate) virtual PUID getID() const = 0;

        RTPListenerPtr getListener() const noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => ISecureTransportForRTPSender
        //

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
        //
        // FakeSecureTransport => ISecureTransportForRTPReceiver
        //

        // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr transport);

        // (not needed) static void getSendingTransport(
        //                                              IRTPTransportPtr inRTPTransport,
        //                                              IRTCPTransportPtr inRTCPTransport,
        //                                              IICETypes::Components &outWhenSendingRTPUseSendOverComponent,
        //                                              IICETypes::Components &outWhenSendingRTCPUseSendOverComponent,
        //                                              ForRTPSenderPtr &outRTPSecureTransport,
        //                                              ForRTPSenderPtr &outRTCPSecureTransport
        //                                              );

        PUID getID() const noexcept override;

        ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) noexcept override;

        ISecureTransportTypes::States state(ISecureTransportTypes::States ignored) const noexcept override;

        bool sendPacket(
                        IICETypes::Components sendOverICETransport,
                        IICETypes::Components packetType,
                        const BYTE *buffer,
                        size_t bufferLengthInBytes
                        ) noexcept override;

        IICETransportPtr getICETransport() const noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => IICETransportDelegate
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

        States state() const noexcept override;

        void notifySecureState();
        ISecureTransportTypes::States getSecureState() const;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => (data)
        //

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
      //
      // FakeListener
      //

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
        //
        // FakeListener => IRTPListenerForRTPReceiver
        //

        ElementPtr toDebug() const noexcept override;

        // (not needed) static ForRTPReceiverPtr getListener(IRTPTransportPtr rtpTransport);

        // (base hanlded) virtual PUID getID() const = 0;

        void registerReceiver(
                              Optional<IMediaStreamTrackTypes::Kinds> kind,
                              UseReceiverPtr inReceiver,
                              const Parameters &inParams,
                              RTCPPacketList *outPacketList = NULL
                              ) noexcept override;

        void unregisterReceiver(UseReceiver &inReceiver) noexcept override;

        void getPackets(RTCPPacketList &outPacketList) noexcept override;

        void notifyUnhandled(
                             const String &muxID,
                             const String &rid,
                             IRTPTypes::SSRCType ssrc,
                             IRTPTypes::PayloadType payloadType
                             ) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeListener => IRTPListenerForRTPSender
        //

        // (duplciate) static ElementPtr toDebug(ForRTPSenderPtr listener);

        // (not needed) static ForRTPSenderPtr getListener(IRTPTransportPtr rtpTransport);

        // (base handled) virtual PUID getID() const = 0;

        void registerSender(
                            UseSenderPtr inSender,
                            const Parameters &inParams,
                            RTCPPacketList &outPacketList
                            ) noexcept override;

        void unregisterSender(UseSender &inSender) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeListener => IRTPListenerForSecureTransport
        //

        // (duplicate) virtual ElementPtr toDebug() override;

        // (not needed) static RTPListenerPtr create(IRTPTransportPtr transport);

        // (base handles) virtual PUID getID() const = 0;

        bool handleRTPPacket(
                             IICETypes::Components viaComponent,
                             IICETypes::Components packetType,
                             const BYTE *buffer,
                             size_t bufferLengthInBytes
                             ) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeListener => Timer
        //

        void onTimer(ITimerPtr timer) override;

        //---------------------------------------------------------------------
        //
        // FakeListener => IFakeListenerAsyncDelegate
        //

        void onForwardBufferedPacket(RTPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeListener => (friend RTPReceiverTester)
        //

        void setTransport(RTPReceiverTesterPtr tester);

        IRTPListenerSubscriptionPtr subscribe(IRTPListenerDelegatePtr originalDelegate) noexcept override;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeListener => (internal)
        //

        Log::Params log(const char *message) const;

        void cancel();

      protected:
        FakeListenerWeakPtr mThisWeak;

        RTPReceiverTesterWeakPtr mTester;

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
      //
      // FakeReceiverChannel
      //

      //-----------------------------------------------------------------------
      class FakeReceiverChannel : public ortc::internal::RTPReceiverChannel,
                                  public IFakeReceiverChannelAsyncDelegate
      {
      public:
        friend class RTPReceiverTester;

        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransportTypes, ISecureTransportTypes)
        ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
        ZS_DECLARE_TYPEDEF_PTR(IFakeReceiverChannelAsyncDelegate::RTCPPacketList, RTCPPacketList)

        typedef std::list<SecureByteBlockPtr> BufferList;

        typedef std::list<ParametersPtr> ParametersList;

        typedef std::list<ISecureTransportTypes::States> StateList;

        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransport, ISecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverForRTPReceiverChannel, UseReceiver)

      public:
        FakeReceiverChannel(
                            IMessageQueuePtr queue,
                            const char *receiverChannelID
                            );
        ~FakeReceiverChannel();

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => IRTPReceiverChannelForRTPReceiver
        //

        ElementPtr toDebug() const noexcept override;

        void create(
                    RTPReceiverPtr receiver,
                    MediaStreamTrackPtr track,
                    const Parameters &params,
                    const RTCPPacketList &packets
                    );

        // (base handles) virtual PUID getID() const = 0;

        void notifyTransportState(ISecureTransport::States state) noexcept override;

        void notifyPacket(RTPPacketPtr packet) noexcept override;

        void notifyPackets(RTCPPacketListPtr packets) noexcept override;

        void notifyUpdate(const Parameters &params) noexcept override;

        bool handlePacket(RTPPacketPtr packet) noexcept override;
        
        bool handlePacket(RTCPPacketPtr packet) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => IFakeReceiverChannelAsyncDelegate
        //

        virtual void onState(ISecureTransportTypes::States state) override;
        virtual void onRTPPacket(RTPPacketPtr packet) override;
        virtual void onRTCPPacket(RTCPPacketPtr packet) override;
        virtual void onRTCPPackets(RTCPPacketListPtr packets) override;
        virtual void onUpdate(ParametersPtr params) override;

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => (friend RTPReceiverTester)
        //

        static FakeReceiverChannelPtr create(
                                             IMessageQueuePtr queue,
                                             const char *receiverChannelID,
                                             const Parameters &expectedParams
                                             );

        void setTransport(RTPReceiverTesterPtr tester);

        bool send_Packet(RTCPPacketPtr packet);

        void expectState(ISecureTransport::States state);

        void expectUpdate(const Parameters &params);

        void expectData(SecureByteBlockPtr data);

        void stop();

      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => (internal)
        //

        Log::Params log(const char *message) const;

      protected:
        FakeReceiverChannelWeakPtr mThisWeak;

        RTPReceiverTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;
        ParametersList mExpectParameters;
        StateList mExpectStates;

        UseReceiverWeakPtr mReceiver;
        String mReceiverChannelID;
      };



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
        typedef PUID ReceiverChannelID;
        typedef std::list<ReceiverChannelID> ReceiverChannelIDList;

        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransport, ISecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel, RTPReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverForMediaStreamTrack, UseReceiver)

      public:
        FakeMediaStreamTrack(IMessageQueuePtr queue);
        ~FakeMediaStreamTrack();

        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => IMediaStreamTrackForRTPReceiver
        //

#pragma ZS_BUILD_NOTE("TODO","no more toDebug")
#if 0
        ElementPtr toDebug() const noexcept override;
#endif // 0

        void create(IMediaStreamTrackTypes::Kinds kind);

        // (base handles) virtual PUID getID() const = 0;

#pragma ZS_BUILD_NOTE("TODO","no more receiver channel")
#if 0
        void notifyActiveReceiverChannel(RTPReceiverChannelPtr channel) noexcept override;
#endif //0

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannel => (friend RTPReceiverTester)
        //

        static FakeMediaStreamTrackPtr create(
                                              IMessageQueuePtr queue,
                                              IMediaStreamTrackTypes::Kinds kind
                                              );

        void setTransport(RTPReceiverTesterPtr tester);

        void expectActiveChannel(ReceiverChannelID channelID);

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
        FakeMediaStreamTrackWeakPtr mThisWeak;

        IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrackTypes::Kind_Audio};

        RTPReceiverTesterWeakPtr mTester;

        UseReceiverWeakPtr mReceiver;

        ReceiverChannelIDList mExpectActiveChannelIDs;
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
        typedef RTPReceiver::RTCPPacketList RTCPPacketList;

      public:
        FakeSender(IMessageQueuePtr queue);
        ~FakeSender();

        static FakeSenderPtr create(IMessageQueuePtr queue);

        //---------------------------------------------------------------------
        //
        // FakeSender => IRTPSenderForRTPListener
        //

        ElementPtr toDebug() const noexcept override;

        // (duplicate) virtual PUID getID() const = 0;

        bool handlePacket(
                          IICETypes::Components viaTransport,
                          RTCPPacketPtr packet
                          ) noexcept override;

        //---------------------------------------------------------------------
        //
        // FakeSender => (friend RTPReceiverTester)
        //

        void set_Transport(RTPReceiverTesterPtr tester);

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

        RTPReceiverTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;

        UseListenerPtr mListener;
        UseSecureTransportPtr mSecureTransport;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // RTPReceiverTester
      //

      //-----------------------------------------------------------------------
      class RTPReceiverTester : public SharedRecursiveLock,
                                public zsLib::MessageQueueAssociator,
                                public IRTPListenerDelegate,
                                public IRTPReceiverDelegate
      {
      public:
        friend class FakeReceiverChannel;
        friend class FakeSender;
        friend class FakeListener;
        friend class FakeMediaStreamTrack;

        ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransportTypes, ISecureTransportTypes)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel, RTPReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelFactory, RTPReceiverChannelFactory)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel::RTCPPacketList, RTCPPacketList)

        ZS_DECLARE_TYPEDEF_PTR(internal::MediaStreamTrack, MediaStreamTrack)
        ZS_DECLARE_TYPEDEF_PTR(internal::MediaStreamTrackFactory, MediaStreamTrackFactory)

        ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPReceiverTypes::ContributingSourceList, ContributingSourceList)

        ZS_DECLARE_CLASS_PTR(OverrideReceiverChannelFactory)
        ZS_DECLARE_CLASS_PTR(OverrideMediaStreamTrackFactory)

        friend class OverrideReceiverChannelFactory;
        friend class OverrideMediaStreamTrackFactory;

        typedef String SenderOrReceiverChannelID;
        typedef std::pair<FakeReceiverChannelWeakPtr, FakeSenderPtr> FakeReceiverChannelFakeSenderPair;
        typedef std::map<SenderOrReceiverChannelID, FakeReceiverChannelFakeSenderPair> SenderOrReceiverMap;

        typedef String PacketID;
        typedef std::pair<RTPPacketPtr, RTCPPacketPtr> PacketPair;
        typedef std::map<PacketID, PacketPair> PacketMap;

        typedef String ReceiverChannelID;
        typedef std::pair<ReceiverChannelID, FakeReceiverChannelPtr> FakeReceiverChannelPair;
        typedef std::list<FakeReceiverChannelPair> FakeReceiverChannelList;

        typedef String ParametersID;
        typedef std::map<ParametersID, ParametersPtr> ParametersMap;

        typedef std::list<ReceiverChannelID> ReceiverIDList;

        //---------------------------------------------------------------------
        //
        // RTPReceiverTester::UnhandledEventData
        //

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
        //
        // RTPReceiverTester::OverrideReceiverChannelFactory
        //

        class OverrideReceiverChannelFactory : public RTPReceiverChannelFactory
        {
        public:
          static OverrideReceiverChannelFactoryPtr create(RTPReceiverTesterPtr tester);

          RTPReceiverChannelPtr create(
                                       RTPReceiverPtr receiver,
                                       MediaStreamTrackPtr track,
                                       const Parameters &params,
                                       const RTCPPacketList &packets
                                       ) noexcept override;

        protected:
          RTPReceiverTesterWeakPtr mTester;
        };

        //---------------------------------------------------------------------
        //
        // RTPReceiverTester::OverrideReceiverChannelFactory
        //

        class OverrideMediaStreamTrackFactory : public MediaStreamTrackFactory
        {
        public:
          static OverrideMediaStreamTrackFactoryPtr create(RTPReceiverTesterPtr tester);

          virtual MediaStreamTrackPtr create(
                                             IMediaStreamTrackTypes::Kinds kind,
                                             bool remote,
                                             TrackConstraintsPtr constraints
                                             );
          virtual MediaStreamTrackPtr create(IMediaStreamTrackTypes::Kinds kind);

        protected:
          RTPReceiverTesterWeakPtr mTester;
        };

      protected:
        struct make_private {};

      public:
        struct Expectations {
          // listener related
          ULONG mUnhandled {0};

          // general
          ULONG mReceivedPackets {0};

          // receiver cannel related
          ULONG mError {0};
          ULONG mChannelUpdate {0};
          ULONG mReceiverChannelOfSecureTransportState {0};

          // media stream track
          ULONG mActiveReceiverChannel {0};

          IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrack::Kind_Audio};

          bool operator==(const Expectations &op2) const;
        };

      public:
        //---------------------------------------------------------------------
        //
        // RTPReceiverTester (api)
        //

        static RTPReceiverTesterPtr create(
                                           IMessageQueuePtr queue,
                                           IMediaStreamTrackTypes::Kinds kind,
                                           bool overrideFactories,
                                           Milliseconds packetDelay = Milliseconds()
                                           );

        RTPReceiverTester(
                          IMessageQueuePtr queue,
                          bool overrideFactories
                          );
        ~RTPReceiverTester();

        void init(Milliseconds packetDelay);

        bool matches();

        void close();
        void closeByReset();

        Expectations getExpectations() const;

        void reliability(ULONG percentage);

        void state(IICETransport::States newState);

        void state(IDTLSTransport::States newState);

        void setClientRole(bool clientRole);

        void connect(RTPReceiverTesterPtr remote);

        void createReceiverChannel(
                                   const char *receiverChannelID,
                                   const char *parametersID
                                   );
        void createSender(const char *senderID);

        void send(
                  const char *senderID,
                  const char *parametersID
                  );

        void receive(const char *parametersID);

        void stop(const char *senderOrReceiverChannelID);

        void attach(
                    const char *receiverChannelID,
                    FakeReceiverChannelPtr receiver
                    );
        void attach(
                    const char *senderID,
                    FakeSenderPtr sender
                    );

        FakeReceiverChannelPtr detachReceiverChannel(const char *receiverChannelID);
        FakeSenderPtr detachSender(const char *senderID);

        void expectKind(IMediaStreamTrackTypes::Kinds kind);

        void expectingUnhandled(
                                IRTPTypes::SSRCType ssrc,
                                IRTPTypes::PayloadType payloadType,
                                const char *mid,
                                const char *rid
                                );

        void expectReceiveChannelUpdate(
                                        const char *receiverChannelID,
                                        const char *parametersID
                                        );

        void expectState(
                         const char *receiverChannelID,
                         ISecureTransportTypes::States state
                         );

        void expectActiveChannel(const char *receiverChannelID);

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
                        const char *senderOrReceiverChannelID,
                        const char *packetID
                        );

        void expectPacket(
                          const char *senderOrReceiverChannelID,
                          const char *packetID
                          );

        ContributingSourceList getContributingSources() const;

      protected:

        //---------------------------------------------------------------------
        //
        // RTPReceiverTester::IRTPListenerDelegate
        //

        virtual void onRTPListenerUnhandledRTP(
                                               IRTPListenerPtr listener,
                                               IRTPTypes::SSRCType ssrc,
                                               IRTPTypes::PayloadType payloadType,
                                               const char *mid,
                                               const char *rid
                                               ) override;

        //---------------------------------------------------------------------
        //
        // RTPReceiverTester::IRTPReceiverDelegate
        //

        //---------------------------------------------------------------------
        //
        // RTPReceiverTester => (friend fake listener, sender, receiver channel)
        //

        FakeSecureTransportPtr getFakeSecureTransport() const;

        void notifyReceivedPacket();
        void notifyReceiverChannelUpdate();
        void notifyActiveReceiverChannel();
        void notifyReceiverChannelOfSecureTransportState();

        RTPReceiverChannelPtr create(
                                     RTPReceiverPtr receiver,
                                     const Parameters &params,
                                     const RTCPPacketList &packets
                                     );

        MediaStreamTrackPtr create(IMediaStreamTrackTypes::Kinds kind);

      protected:
        //---------------------------------------------------------------------
        //
        // RTPReceiverTester => (internal)
        //

        Log::Params log(const char *message) const;

        FakeICETransportPtr getICETransport() const;

        FakeReceiverChannelPtr getReceiverChannel(const char *receiverID);
        FakeSenderPtr getSender(const char *senderID);

        void expectData(
                        const char *senderOrReceiverID,
                        SecureByteBlockPtr secureBuffer
                        );
        void sendData(
                      const char *senderOrReceiverChannelID,
                      SecureByteBlockPtr secureBuffer
                      );

      public:
        //---------------------------------------------------------------------
        //
        // RTPReceiverTester => (data)
        //

        AutoPUID mID;
        RTPReceiverTesterWeakPtr mThisWeak;

        bool mOverrideFactories {false};

        FakeICETransportPtr mICETransport;
        FakeSecureTransportPtr mDTLSTransport;

        FakeMediaStreamTrackPtr mMediaStreamTrack;
        ReceiverIDList mExpectingActiveChannelsUponMediaTrackCreation;

        IRTPReceiverPtr mReceiver;

        RTPReceiverTesterWeakPtr mConnectedTester;

        IRTPListenerSubscriptionPtr mListenerSubscription;
        IRTPReceiverSubscriptionPtr mReceiverSubscription;

        Expectations mExpecting;
        Expectations mExpectationsFound;

        SenderOrReceiverMap mAttached;
        PacketMap mPackets;

        FakeReceiverChannelList mFakeReceiverChannelCreationList;

        UnhandledEventDataList mExpectingUnhandled;

        ParametersMap mParameters;
      };
    }
  }
}

