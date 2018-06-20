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

#if 0

#include <ortc/IDataChannel.h>
#include <ortc/IRTPReceiver.h>

#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_RTPReceiver.h>
#include <ortc/internal/ortc_RTPReceiverChannel.h>
//#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
//#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
//#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
//#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
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
    namespace rtpchannel
    {
      ZS_DECLARE_INTERACTION_PROXY(IFakeICETransportAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeSecureTransportAsyncDelegate)
      ZS_DECLARE_INTERACTION_PROXY(IFakeListenerAsyncDelegate)

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
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPPacket, RTPPacket)

        virtual void onForwardBufferedPacket(RTPPacketPtr packet) = 0;
      };

    }
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpchannel::IFakeICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_METHOD(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpchannel::IFakeSecureTransportAsyncDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
//ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::test::rtpchannel::IFakeListenerAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD(onForwardBufferedPacket, RTPPacketPtr)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace rtpchannel
    {
      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::Milliseconds;

      ZS_DECLARE_USING_PTR(zsLib, ITimer)
      ZS_DECLARE_USING_PTR(ortc, RTPPacket)
      ZS_DECLARE_USING_PTR(ortc, RTCPPacket)
      ZS_DECLARE_USING_PTR(ortc, RTPReceiver)

      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(FakeListener)
      ZS_DECLARE_CLASS_PTR(FakeMediaStreamTrack)
      ZS_DECLARE_CLASS_PTR(FakeSender)
      ZS_DECLARE_CLASS_PTR(FakeReceiver)
      ZS_DECLARE_CLASS_PTR(RTPChannelTester)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannelAudio)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannelVideo)
      ZS_DECLARE_CLASS_PTR(FakeSenderChannelAudio)
      ZS_DECLARE_CLASS_PTR(FakeSenderChannelVideo)

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
        virtual ElementPtr toDebug() const override;

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
                                ) override;

        virtual States state() const override;

        virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr originalDelegate) override;

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
        friend class FakeReceiver;
        friend class RTPChannelTester;

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
        virtual ElementPtr toDebug() const override;

        //---------------------------------------------------------------------
        //
        // DTLSTransport => ISecureTransportForRTPListener
        //

        // (duplicate) static ElementPtr toDebug(ForRTPReceiverPtr transport);

        // (duplicate) virtual PUID getID() const = 0;

        virtual RTPListenerPtr getListener() const override;

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
                                  ) override;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSecureTransport => (internal)
        //

        void setState(IDTLSTransportTypes::States state);

        bool isShutdown();

        Log::Params log(const char *message) const;

        void cancel();

        virtual States state() const override;

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

        typedef String MuxID;
        typedef std::map<MuxID, UseReceiverWeakPtr> MuxIDToReceiverMap;

        typedef IRTPTypes::PayloadType PayloadType;
        typedef std::map<PayloadType, UseReceiverWeakPtr> PayloadTypeToReceiverMap;

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
        //
        // FakeListener => IRTPListenerForRTPSender
        //

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
        //
        // FakeListener => IRTPListenerForSecureTransport
        //

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
        //
        // FakeListener => Timer
        //

        virtual void onTimer(ITimerPtr timer) override;

        //---------------------------------------------------------------------
        //
        // FakeListener => IFakeListenerAsyncDelegate
        //

        virtual void onForwardBufferedPacket(RTPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeListener => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelTesterPtr tester);

        virtual IRTPListenerSubscriptionPtr subscribe(IRTPListenerDelegatePtr originalDelegate) override;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeListener => (internal)
        //

        Log::Params log(const char *message) const;

        void cancel();

      protected:
        FakeListenerWeakPtr mThisWeak;

        RTPChannelTesterWeakPtr mTester;

        IRTPListenerDelegateSubscriptions mSubscriptions;
        IRTPListenerSubscriptionPtr mDefaultSubscription;

        ITimerPtr mCleanBuffersTimer;

        BufferedRTPPacketList mBufferedRTPPackets;
        BufferedRTCPPacketList mBufferedRTCPPackets;

        UseReceiverWeakPtr mReceiver;

        SenderWeakList mSenders;

        Optional<BYTE> mMuxIDHeaderExtension;
        MuxIDToReceiverMap mMuxIDToReceivers;
        PayloadTypeToReceiverMap mPayloadTypesToReceivers;
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
        friend class RTPChannelTester;
        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransport, ISecureTransport)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverForMediaStreamTrack, UseReceiver)

      public:
        FakeMediaStreamTrack(IMessageQueuePtr queue);
        ~FakeMediaStreamTrack();

        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
        //

#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_RECEIVER_CHANNEL_CLASS_TO_FAKE_MEDIA_TRACK 1
#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_RECEIVER_CHANNEL_CLASS_TO_FAKE_MEDIA_TRACK 2

        // (handled by base) virtual PUID getID() const = 0;

        //virtual void renderVideoFrame(const webrtc::VideoFrame& videoFrame) override;

        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => IMediaStreamTrackForRTPSenderChannel
        //

#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_SENDER_CHANNEL_CLASS_TO_FAKE_MEDIA_TRACK 1
#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_SENDER_CHANNEL_CLASS_TO_FAKE_MEDIA_TRACK 2


        //---------------------------------------------------------------------
        //
        // FakeMediaStreamTrack => (friend RTPChannelTester)
        //

        static FakeMediaStreamTrackPtr create(
                                              IMessageQueuePtr queue,
                                              IMediaStreamTrackTypes::Kinds kind
                                              );

        void setTransport(RTPChannelTesterPtr tester);

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

        RTPChannelTesterWeakPtr mTester;
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
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel, RTPReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelForRTPReceiver, UseReceiverChannel)

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
        //
        // FakeReceiver => IRTPReceiverForRTPListener
        //

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
        //
        // FakeReceiver => IRTPReceiverForRTPReceiverChannel
        //

        // (duplicate) static ElementPtr toDebug(ForRTPReceiverChannelPtr object);

        // (base handles) virtual PUID getID() const = 0;

#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_RECEIVER_CHANNEL_CLASS 1
#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_RECEIVER_CHANNEL_CLASS 2

        virtual bool sendPacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeReceiver => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelTesterPtr tester);

        void linkChannel(RTPReceiverChannelPtr channel);

        virtual PromisePtr receive(const Parameters &parameters) override;
        virtual void stop() override;

        void expectData(SecureByteBlockPtr data);

        void sendPacket(SecureByteBlockPtr buffer);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiver => (internal)
        //

        Log::Params log(const char *message) const;


      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiver => (data)
        //

        FakeReceiverWeakPtr mThisWeak;

        IMediaStreamTrackTypes::Kinds mKind {IMediaStreamTrackTypes::Kind_Audio};

        RTPChannelTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;

        UseListenerPtr mListener;
        UseSecureTransportPtr mSecureTransport;

        UseReceiverChannelPtr mReceiverChannel;
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
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannel, RTPSenderChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelForRTPSender, UseSenderChannel)

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

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

        virtual bool handlePacket(
                                  IICETypes::Components viaTransport,
                                  RTCPPacketPtr packet
                                  ) override;


        //---------------------------------------------------------------------
        //
        // FakeSender => IRTPSenderForRTPSenderChannel
        //

        virtual bool sendPacket(RTPPacketPtr packet) override;
        virtual bool sendPacket(RTCPPacketPtr packet) override;

        virtual void notifyConflict(
                                    UseChannelPtr channel,
                                    IRTPTypes::SSRCType ssrc,
                                    bool selfDestruct
                                    ) override;

        //---------------------------------------------------------------------
        //
        // FakeSender => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelTesterPtr tester);

        void linkChannel(RTPSenderChannelPtr channel);

        virtual PromisePtr send(const Parameters &parameters) override;
        virtual void stop() override;

        void expectData(SecureByteBlockPtr data);

        void sendPacket(SecureByteBlockPtr buffer);

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

        RTPChannelTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;

        UseListenerPtr mListener;
        UseSecureTransportPtr mSecureTransport;

        UseSenderChannelPtr mSenderChannel;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // FakeReceiverChannelAudio
      //

      //-----------------------------------------------------------------------
      class FakeReceiverChannelAudio : public ortc::internal::RTPReceiverChannelAudio
      {
      public:
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelAudio, RTPReceiverChannelAudio)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel, RTPReceiverChannel)

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelForRTPReceiverChannelAudio, UseChannel)

        typedef std::list<SecureByteBlockPtr> BufferList;

      public:
        FakeReceiverChannelAudio(
                                 IMessageQueuePtr queue,
                                 const char *mediaID,
                                 const Parameters &params
                                 );
        ~FakeReceiverChannelAudio();

        static FakeReceiverChannelAudioPtr create(
                                                  IMessageQueuePtr queue,
                                                  const char *mediaID,
                                                  const Parameters &params
                                                  );

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelAudio => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
        //

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

        virtual bool handlePacket(RTPPacketPtr packet) override;

        virtual bool handlePacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelAudio => IRTPReceiverChannelAudioForRTPReceiverChannel
        //

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelAudio => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelTesterPtr tester);

        void create(
                    RTPReceiverChannelPtr receiverChannel,
                    const Parameters &params
                    );

        void expectData(SecureByteBlockPtr data);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelAudio => (internal)
        //

        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelAudio => (data)
        //

        FakeReceiverChannelAudioWeakPtr mThisWeak;

        String mMediaID;

        RTPChannelTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;

        UseChannelWeakPtr mReceiverChannel;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // FakeReceiverChannelVideo
      //

      //-----------------------------------------------------------------------
      class FakeReceiverChannelVideo : public ortc::internal::RTPReceiverChannelVideo
      {
      public:
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelVideo, RTPReceiverChannelVideo)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel, RTPReceiverChannel)

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelForRTPReceiverChannelVideo, UseChannel)

        typedef std::list<SecureByteBlockPtr> BufferList;

      public:
        FakeReceiverChannelVideo(
                                 IMessageQueuePtr queue,
                                 const char *mediaID,
                                 const Parameters &params
                                 );
        ~FakeReceiverChannelVideo();

        static FakeReceiverChannelVideoPtr create(
                                                  IMessageQueuePtr queue,
                                                  const char *mediaID,
                                                  const Parameters &params
                                                  );

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelVideo => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
        //

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

        virtual bool handlePacket(RTPPacketPtr packet) override;

        virtual bool handlePacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelVideo => IRTPReceiverChannelVideoForRTPReceiverChannel
        //

        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelVideo => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelTesterPtr tester);

        void create(
                    RTPReceiverChannelPtr receiverChannel,
                    const Parameters &params
                    );

        void expectData(SecureByteBlockPtr data);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelVideo => (internal)
        //

        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeReceiverChannelVideo => (data)
        //

        FakeReceiverChannelVideoWeakPtr mThisWeak;

        String mMediaID;

        RTPChannelTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;

        UseChannelWeakPtr mReceiverChannel;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // FakeSenderChannelAudio
      //

      //-----------------------------------------------------------------------
      class FakeSenderChannelAudio : public ortc::internal::RTPSenderChannelAudio
      {
      public:
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelAudio, RTPSenderChannelAudio)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannel, RTPSenderChannel)

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelForRTPSenderChannelAudio, UseChannel)

        typedef std::list<SecureByteBlockPtr> BufferList;

      public:
        FakeSenderChannelAudio(
                                 IMessageQueuePtr queue,
                                 const char *mediaID,
                                 const Parameters &params
                                 );
        ~FakeSenderChannelAudio();

        static FakeSenderChannelAudioPtr create(
                                                IMessageQueuePtr queue,
                                                const char *mediaID,
                                                const Parameters &params
                                                );

        //---------------------------------------------------------------------
        //
        // FakeSenderChannelAudio => IRTPSenderChannelMediaBaseForRTPSenderChannel
        //

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

        virtual bool handlePacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeSenderChannelAudio => IRTPSenderChannelAudioForRTPSenderChannel
        //

        //---------------------------------------------------------------------
        //
        // FakeSenderChannelAudio => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelTesterPtr tester);

        void create(
                    RTPSenderChannelPtr receiverChannel,
                    const Parameters &params
                    );

        void expectData(SecureByteBlockPtr data);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSenderChannelAudio => (internal)
        //

        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSenderChannelAudio => (data)
        //

        FakeSenderChannelAudioWeakPtr mThisWeak;

        String mMediaID;

        RTPChannelTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;

        UseChannelWeakPtr mSenderChannel;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // FakeSenderChannelVideo
      //

      //-----------------------------------------------------------------------
      class FakeSenderChannelVideo : public ortc::internal::RTPSenderChannelVideo
      {
      public:
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelVideo, RTPSenderChannelVideo)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannel, RTPSenderChannel)

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelForRTPSenderChannelVideo, UseChannel)

        typedef std::list<SecureByteBlockPtr> BufferList;

      public:
        FakeSenderChannelVideo(
                                 IMessageQueuePtr queue,
                                 const char *mediaID,
                                 const Parameters &params
                                 );
        ~FakeSenderChannelVideo();

        static FakeSenderChannelVideoPtr create(
                                                IMessageQueuePtr queue,
                                                const char *mediaID,
                                                const Parameters &params
                                                );

        //---------------------------------------------------------------------
        //
        // FakeSenderChannelVideo => IRTPSenderChannelMediaBaseForRTPSenderChannel
        //

        virtual ElementPtr toDebug() const override;

        // (duplicate) virtual PUID getID() const = 0;

        virtual bool handlePacket(RTCPPacketPtr packet) override;

        //---------------------------------------------------------------------
        //
        // FakeSenderChannelVideo => IRTPSenderChannelVideoForRTPSenderChannel
        //

        //---------------------------------------------------------------------
        //
        // FakeSenderChannelVideo => (friend RTPChannelTester)
        //

        void setTransport(RTPChannelTesterPtr tester);

        void create(
                    RTPSenderChannelPtr receiverChannel,
                    const Parameters &params
                    );

        void expectData(SecureByteBlockPtr data);

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSenderChannelVideo => (internal)
        //

        Log::Params log(const char *message) const;

      protected:
        //---------------------------------------------------------------------
        //
        // FakeSenderChannelVideo => (data)
        //

        FakeSenderChannelVideoWeakPtr mThisWeak;

        String mMediaID;

        RTPChannelTesterWeakPtr mTester;

        ParametersPtr mParameters;

        BufferList mExpectBuffers;

        UseChannelWeakPtr mSenderChannel;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // RTPChannelTester
      //

      //-----------------------------------------------------------------------
      class RTPChannelTester : public SharedRecursiveLock,
                               public zsLib::MessageQueueAssociator,
                               public IRTPListenerDelegate,
                               public IRTPReceiverDelegate
      {
      public:
        friend class FakeSender;
        friend class FakeReceiver;
        friend class FakeReceiverChannelAudio;
        friend class FakeReceiverChannelVideo;
        friend class FakeSenderChannelAudio;
        friend class FakeSenderChannelVideo;
        friend class FakeListener;
        friend class FakeMediaStreamTrack;

        ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
        ZS_DECLARE_TYPEDEF_PTR(internal::ISecureTransportTypes, ISecureTransportTypes)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel, RTPReceiverChannel)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannel, RTPSenderChannel)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelAudio, RTPReceiverChannelAudio)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelVideo, RTPReceiverChannelVideo)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelAudio, RTPSenderChannelAudio)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelVideo, RTPSenderChannelVideo)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannel::RTCPPacketList, RTCPPacketList)

        ZS_DECLARE_TYPEDEF_PTR(internal::MediaStreamTrack, MediaStreamTrack)

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelForRTPReceiver, UseReceiverChannelForReceiver)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPReceiverChannelForMediaStreamTrack, UseReceiverChannelForTrack)

        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelForRTPSender, UseSenderChannelForSender)
        ZS_DECLARE_TYPEDEF_PTR(internal::IRTPSenderChannelForMediaStreamTrack, UseSenderChannelForTrack)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelAudioFactory, RTPReceiverChannelAudioFactory)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPReceiverChannelVideoFactory, RTPReceiverChannelVideoFactory)

        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelAudioFactory, RTPSenderChannelAudioFactory)
        ZS_DECLARE_TYPEDEF_PTR(internal::RTPSenderChannelVideoFactory, RTPSenderChannelVideoFactory)

        ZS_DECLARE_CLASS_PTR(OverrideReceiverChannelAudioFactory)
        ZS_DECLARE_CLASS_PTR(OverrideReceiverChannelVideoFactory)

        ZS_DECLARE_CLASS_PTR(OverrideSenderChannelAudioFactory)
        ZS_DECLARE_CLASS_PTR(OverrideSenderChannelVideoFactory)

        friend class OverrideReceiverChannelAudioFactory;
        friend class OverrideSenderChannelVideoFactory;

        friend class OverrideSenderChannelAudioFactory;
        friend class OverrideReceiverChannelVideoFactory;

        typedef String SenderOrReceiverID;
        typedef std::pair<FakeReceiverPtr, FakeSenderPtr> FakeReceiverFakeSenderPair;
        typedef std::map<SenderOrReceiverID, FakeReceiverFakeSenderPair> SenderOrReceiverMap;

        typedef String PacketID;
        typedef std::pair<RTPPacketPtr, RTCPPacketPtr> PacketPair;
        typedef std::map<PacketID, PacketPair> PacketMap;

        typedef std::list<PacketID> PacketIDList;

        typedef String ParametersID;
        typedef std::map<ParametersID, ParametersPtr> ParametersMap;

        typedef String ReceiverChannelID;
        typedef std::map<ReceiverChannelID, RTPReceiverChannelPtr> ReceiverChannelMap;

        typedef String SenderChannelID;
        typedef std::map<SenderChannelID, RTPSenderChannelPtr> SenderChannelMap;

        typedef String MediaID;

        typedef std::pair<MediaID, FakeReceiverChannelAudioPtr> FakeReceiverChannelAudioPair;
        typedef std::list<FakeReceiverChannelAudioPair> FakeReceiverChannelAudioList;
        typedef std::map<MediaID, FakeReceiverChannelAudioWeakPtr> FakeReceiverChannelAudioMap;

        typedef std::pair<MediaID, FakeReceiverChannelVideoPtr> FakeReceiverChannelVideoPair;
        typedef std::list<FakeReceiverChannelVideoPair> FakeReceiverChannelVideoList;
        typedef std::map<MediaID, FakeReceiverChannelVideoWeakPtr> FakeReceiverChannelVideoMap;

        typedef std::pair<MediaID, FakeSenderChannelAudioPtr> FakeSenderChannelAudioPair;
        typedef std::list<FakeSenderChannelAudioPair> FakeSenderChannelAudioList;
        typedef std::map<MediaID, FakeSenderChannelAudioWeakPtr> FakeSenderChannelAudioMap;

        typedef std::pair<MediaID, FakeSenderChannelVideoPtr> FakeSenderChannelVideoPair;
        typedef std::list<FakeSenderChannelVideoPair> FakeSenderChannelVideoList;
        typedef std::map<MediaID, FakeSenderChannelVideoWeakPtr> FakeSenderChannelVideoMap;

        //---------------------------------------------------------------------
        //
        // RTPChannelTester::OverrideReceiverChannelAudioFactory
        //

        class OverrideReceiverChannelAudioFactory : public RTPReceiverChannelAudioFactory
        {
        public:
          static OverrideReceiverChannelAudioFactoryPtr create(RTPChannelTesterPtr tester);

          virtual RTPReceiverChannelAudioPtr create(
                                                    RTPReceiverChannelPtr receiverChannel,
                                                    MediaStreamTrackPtr track,
                                                    const Parameters &params
                                                    ) override;

        protected:
          RTPChannelTesterWeakPtr mTester;
        };

        //---------------------------------------------------------------------
        //
        // RTPChannelTester::OverrideReceiverChannelVideoFactory
        //

        class OverrideReceiverChannelVideoFactory : public RTPReceiverChannelVideoFactory
        {
        public:
          static OverrideReceiverChannelVideoFactoryPtr create(RTPChannelTesterPtr tester);

          virtual RTPReceiverChannelVideoPtr create(
                                                    RTPReceiverChannelPtr receiverChannel,
                                                    MediaStreamTrackPtr track,
                                                    const Parameters &params
                                                    ) override;

        protected:
          RTPChannelTesterWeakPtr mTester;
        };

        //---------------------------------------------------------------------
        //
        // RTPChannelTester::OverrideSenderChannelAudioFactory
        //

        class OverrideSenderChannelAudioFactory : public RTPSenderChannelAudioFactory
        {
        public:
          static OverrideSenderChannelAudioFactoryPtr create(RTPChannelTesterPtr tester);

          virtual RTPSenderChannelAudioPtr create(
                                                    RTPSenderChannelPtr senderChannel,
                                                    MediaStreamTrackPtr track,
                                                    const Parameters &params
                                                    ) override;

        protected:
          RTPChannelTesterWeakPtr mTester;
        };

        //---------------------------------------------------------------------
        //
        // RTPChannelTester::OverrideReceiverChannelVideoFactory
        //

        class OverrideSenderChannelVideoFactory : public RTPSenderChannelVideoFactory
        {
        public:
          static OverrideSenderChannelVideoFactoryPtr create(RTPChannelTesterPtr tester);

          virtual RTPSenderChannelVideoPtr create(
                                                  RTPSenderChannelPtr sender,
                                                  MediaStreamTrackPtr track,
                                                  const Parameters &params
                                                  ) override;

        protected:
          RTPChannelTesterWeakPtr mTester;
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

          bool operator==(const Expectations &op2) const;
        };

      public:
        //---------------------------------------------------------------------
        //
        // RTPChannelTester (api)
        //

        static RTPChannelTesterPtr create(
                                          IMessageQueuePtr queue,
                                          IMediaStreamTrackTypes::Kinds kind,
                                          bool overrideFactories,
                                          Milliseconds packetDelay = Milliseconds()
                                          );

        RTPChannelTester(
                         IMessageQueuePtr queue,
                         bool overrideFactories
                         );
        ~RTPChannelTester();

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

        void connect(RTPChannelTesterPtr remote);

        void createReceiver(const char *receiverID);
        void createSender(const char *senderID);

        void createReceiverChannelAudio(
                                        const char *receiverChannelAudioID,
                                        const char *parametersID
                                        );
        void createReceiverChannelVideo(
                                        const char *receiverChannelVideoID,
                                        const char *parametersID
                                        );
        void createSenderChannelAudio(
                                      const char *senderChannelAudioID,
                                      const char *parametersID
                                      );
        void createSenderChannelVideo(
                                      const char *senderChannelVideoID,
                                      const char *parametersID
                                      );

        void send(
                  const char *senderID,
                  const char *parametersID
                  );

        void receive(
                     const char *receiverID,
                     const char *parametersID
                     );

        void stop(const char *senderOrReceiverID);

        void attach(
                    const char *receiverChannelID,
                    FakeReceiverPtr receiver
                    );
        void attach(
                    const char *senderID,
                    FakeSenderPtr sender
                    );
        void attach(
                    const char *receiverChannelID,
                    RTPReceiverChannelPtr receiverChannel
                    );
        void attach(
                    const char *senderChannelID,
                    RTPSenderChannelPtr senderChannel
                    );
        void attach(
                    const char *receiverChannelAudioID,
                    FakeReceiverChannelAudioPtr receiverChannelAudio
                    );
        void attach(
                    const char *receiverChannelVideoID,
                    FakeReceiverChannelVideoPtr receiverChannelVideo
                    );
        void attach(
                    const char *senderChannelAudioID,
                    FakeSenderChannelAudioPtr senderChannelAudio
                    );
        void attach(
                    const char *senderChannelVideoID,
                    FakeSenderChannelVideoPtr senderChannelVideo
                    );

        FakeReceiverPtr detachReceiver(const char *receiverID);
        FakeSenderPtr detachSender(const char *senderID);
        RTPReceiverChannelPtr detachReceiverChannel(const char *receiverChannelID);
        RTPSenderChannelPtr detachSenderChannel(const char *senderChannelID);

        FakeReceiverChannelAudioPtr detachReceiverChannelAudio(const char *receiverChannelAudioID);
        FakeReceiverChannelVideoPtr detachReceiverChannelVideo(const char *receiverChannelVideoID);
        FakeSenderChannelAudioPtr detachSenderChannelAudio(const char *senderChannelAudioID);
        FakeSenderChannelVideoPtr detachSenderChannelVideo(const char *senderChannelVideoID);

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
                        const char *senderOrReceiverID,
                        const char *packetID
                        );

        void expectPacket(
                          const char *senderOrReceiverID,
                          const char *packetID
                          );


        //---------------------------------------------------------------------
        //
        // RTPChannelTester => IRTPReceiverChannelForRTPReceiver
        //

        // simulate methods calls to IRTPReceiverChannelForRTPReceiver

        void createReceiverChannel(
                                   const char *receiverChannelID,
                                   const char *linkReceiverID,
                                   const char *parametersID,
                                   const PacketIDList &packets = PacketIDList()
                                   );

        void notifyTransportState(
                                  const char *receiverChannelOrSenderChannelID,
                                  ISecureTransportTypes::States state
                                  );

        void notifyPacket(
                          const char *receiverChannelID,
                          const char *packetID
                          );

        void notifyPackets(
                           const char *receiverChannelOrSenderChannelID,
                           const PacketIDList &packets
                           );

        void notifyUpdate(
                          const char *receiverChannelOrSenderChannelID,
                          const char *parametersID
                          );

        bool handlePacket(
                          const char *receiverChannelOrSenderChannelID,
                          const char *packetID
                          );

        //---------------------------------------------------------------------
        //
        // RTPChannelTester => IRTPReceiverChannelForMediaStreamTrack
        //

        // call methods calls to IRTPReceiverChannelForMediaStreamTrack

#define MOSA_TODO_ADD_METHODS_NEEDED_TO_SIMULATE_CALLS_TO_RECEIVER_CHANNEL_FROM_MEDIA_STREAM_TRACK 1
#define MOSA_TODO_ADD_METHODS_NEEDED_TO_SIMULATE_CALLS_TO_RECEIVER_CHANNEL_FROM_MEDIA_STREAM_TRACK 2

        //---------------------------------------------------------------------
        //
        // RTPChannelTester => IRTPSenderChannelForRTPSender
        //

        // call methods calls IRTPSenderChannelForRTPSender

        void createSenderChannel(
                                 const char *senderChannelID,
                                 const char *linkSenderID,
                                 const char *parametersID
                                 );

        // (duplicate) void notifyTransportState(
        //                                       const char *senderChannelID,
        //                                       ISecureTransportTypes::States state
        //                                       );


        // (duplicate) void notifyPackets(
        //                                const char *senderChannelID,
        //                                const PacketIDList &packets = PacketIDList()
        //                                );

        // (duplicate) void notifyUpdate(
        //                               const char *senderChannelID,
        //                               const char *parametersID
        //                               );

        // (duplicate) bool handlePacket(
        //                               const char *senderChannelID,
        //                               const char *packetID
        //                               );

        //---------------------------------------------------------------------
        //
        // RTPChannelTester => IRTPReceiverChannelForMediaStreamTrack
        //

        // call methods calls IRTPReceiverChannelForMediaStreamTrack

#define MOSA_TODO_ADD_METHODS_NEEDED_TO_SIMULATE_CALLS_TO_SENDER_CHANNEL_FROM_MEDIA_STREAM_TRACK 1
#define MOSA_TODO_ADD_METHODS_NEEDED_TO_SIMULATE_CALLS_TO_SENDER_CHANNEL_FROM_MEDIA_STREAM_TRACK 2

      protected:

        //---------------------------------------------------------------------
        //
        // RTPChannelTester::IRTPListenerDelegate
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
        // RTPChannelTester::IRTPReceiverDelegate
        //

        //---------------------------------------------------------------------
        //
        // RTPChannelTester => (friend fake listener, sender, receiver channel)
        //

        FakeSecureTransportPtr getFakeSecureTransport() const;

        void notifyReceivedPacket();

        void notifySenderChannelConflict();

        FakeReceiverChannelAudioPtr createReceiverChannelAudio(
                                                               RTPReceiverChannelPtr receiverChannel,
                                                               const Parameters &params
                                                               );
        FakeReceiverChannelVideoPtr createReceiverChannelVideo(
                                                               RTPReceiverChannelPtr receiverChannel,
                                                               const Parameters &params
                                                               );
        FakeSenderChannelAudioPtr createSenderChannelAudio(
                                                           RTPSenderChannelPtr senderChannel,
                                                           const Parameters &params
                                                           );
        FakeSenderChannelVideoPtr createSenderChannelVideo(
                                                           RTPSenderChannelPtr senderChannel,
                                                           const Parameters &params
                                                           );

      protected:
        //---------------------------------------------------------------------
        //
        // RTPChannelTester => (internal)
        //

        Log::Params log(const char *message) const;

        FakeICETransportPtr getICETransport() const;

        FakeReceiverPtr getReceiver(const char *receiverID);
        FakeSenderPtr getSender(const char *senderID);
        RTPReceiverChannelPtr getReceiverChannel(const char *receiverChannelID);
        RTPSenderChannelPtr getSenderChannel(const char *senderChannelID);

        FakeReceiverChannelAudioPtr getReceiverChannelAudio(const char *receiverChannelAudioID);
        FakeReceiverChannelVideoPtr getReceiverChannelVideo(const char *receiverChannelVideoID);
        FakeSenderChannelAudioPtr getSenderChannelAudio(const char *senderChannelAudioID);
        FakeSenderChannelVideoPtr getSenderChannelVideo(const char *senderChannelVideoID);

        void expectData(
                        const char *senderOrReceiverID,
                        SecureByteBlockPtr secureBuffer
                        );
        void sendData(
                      const char *senderOrReceiverID,
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
        // RTPChannelTester => (data)
        //

        AutoPUID mID;
        RTPChannelTesterWeakPtr mThisWeak;

        bool mOverrideFactories {false};

        FakeICETransportPtr mICETransport;
        FakeSecureTransportPtr mDTLSTransport;

        FakeMediaStreamTrackPtr mMediaStreamTrack;

        RTPChannelTesterWeakPtr mConnectedTester;

        IRTPListenerSubscriptionPtr mListenerSubscription;
        IRTPReceiverSubscriptionPtr mReceiverSubscription;

        Expectations mExpecting;
        Expectations mExpectationsFound;

        SenderOrReceiverMap mAttached;
        PacketMap mPackets;

        ParametersMap mParameters;

        ReceiverChannelMap mReceiverChannels;
        SenderChannelMap mSenderChannels;

        FakeReceiverChannelAudioList mFakeReceiverChannelAudioCreationList;
        FakeReceiverChannelVideoList mFakeReceiverChannelVideoCreationList;

        FakeSenderChannelAudioList mFakeSenderChannelAudioCreationList;
        FakeSenderChannelVideoList mFakeSenderChannelVideoCreationList;

        FakeReceiverChannelAudioMap mAttachedReceiverChannelAudio;
        FakeReceiverChannelVideoMap mAttachedReceiverChannelVideo;

        FakeSenderChannelAudioMap mAttachedSenderChannelAudio;
        FakeSenderChannelVideoMap mAttachedSenderChannelVideo;
      };
    }
  }
}

#endif //0
