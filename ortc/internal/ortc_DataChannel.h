/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#pragma once

#include <ortc/internal/types.h>
#include <ortc/internal/ortc_SCTPTransport.h>

#include <ortc/IDataChannel.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/TearAway.h>

//#define ORTC_SETTING_SRTP_TRANSPORT_WARN_OF_KEY_LIFETIME_EXHAUGSTION_WHEN_REACH_PERCENTAGE_USSED "ortc/srtp/warm-key-lifetime-exhaustion-when-reach-percentage-used"

#define ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID 0xFFFF


namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForDataChannel);

    ZS_DECLARE_INTERACTION_PTR(IDataChannelForSCTPTransport);
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForDataChannelSubscription);

    ZS_DECLARE_INTERACTION_PROXY(IDataChannelAsyncDelegate);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDataChannelForSCTPTransport
    #pragma mark

    interaction IDataChannelForSCTPTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, ForDataTransport)

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelTypes::Parameters, Parameters)

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, UseDataTransport)

      static ElementPtr toDebug(ForDataTransportPtr dataChannel);

      static ForDataTransportPtr create(
                                        UseDataTransportPtr transport,
                                        WORD sessionID
                                        );

      virtual PUID getID() const = 0;

      virtual bool isIncoming() const = 0;

      virtual bool handleSCTPPacket(SCTPPacketIncomingPtr packet) = 0;

      virtual void requestShutdown() = 0;
      virtual void notifyClosed() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDataChannelAsyncDelegate
    #pragma mark

    interaction IDataChannelAsyncDelegate
    {
      virtual void onRequestShutdown() = 0;
      virtual void onNotifiedClosed() = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IDataChannelAsyncDelegate)
ZS_DECLARE_PROXY_METHOD_0(onRequestShutdown)
ZS_DECLARE_PROXY_METHOD_0(onNotifiedClosed)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannel
    #pragma mark

    class DataChannel : public Noop,
                        public MessageQueueAssociator,
                        public SharedRecursiveLock,
                        public IDataChannel,
                        public IDataChannelForSCTPTransport,
                        public ISCTPTransportForDataChannelDelegate,
                        public IDataChannelAsyncDelegate,
                        public IWakeDelegate,
                        public zsLib::IPromiseSettledDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IDataChannel;
      friend interaction IDataChannelFactory;
      friend interaction IDataChannelForSCTPTransport;

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, UseDataTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannelSubscription, UseDataTransportSubscription)

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelTypes::Parameters, Parameters)

      ZS_DECLARE_STRUCT_PTR(TearAwayData)

      typedef std::list<SCTPPacketIncomingPtr> BufferIncomingList;
      typedef std::list<SCTPPacketOutgoingPtr> BufferOutgoingList;

    public:
      DataChannel(
                  const make_private &,
                  IMessageQueuePtr queue,
                  IDataChannelDelegatePtr delegate,
                  UseDataTransportPtr transport,
                  ParametersPtr params,
                  WORD sessionID = ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID
                  );

    protected:
      DataChannel(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~DataChannel();

      static DataChannelPtr convert(IDataChannelPtr object);
      static DataChannelPtr convert(ForDataTransportPtr object);

    protected:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => IDataChannel
      #pragma mark

      static ElementPtr toDebug(DataChannelPtr transport);

      static DataChannelPtr create(
                                   IDataChannelDelegatePtr delegate,
                                   IDataTransportPtr transport,
                                   const Parameters &params
                                   );

      virtual PUID getID() const override {return mID;}

      virtual IDataChannelSubscriptionPtr subscribe(IDataChannelDelegatePtr delegate) override;

      virtual IDataTransportPtr transport() const override;

      virtual ParametersPtr parameters() const override;

      virtual States readyState() const override;

      virtual size_t bufferedAmount() const override;
      virtual size_t bufferedAmountLowThreshold() const override;
      virtual void bufferedAmountLowThreshold(size_t value) override;

      virtual String binaryType() const override;
      virtual void binaryType(const char *str) override;

      virtual void close() override;

      virtual void send(const String &data) override;
      virtual void send(const SecureByteBlock &data) override;
      virtual void send(
                        const BYTE *buffer,
                        size_t bufferSizeInBytes
                        ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => IDataChannelForSCTPTransport
      #pragma mark

      // (duplicate) static ElementPtr toDebug(DataChannelPtr transport);

      static ForDataTransportPtr create(
                                        UseDataTransportPtr transport,
                                        WORD sessionID
                                        );

      // (duplicate) virtual PUID getID() const = 0;

      virtual bool isIncoming() const override {return mIncoming;}

      virtual bool handleSCTPPacket(SCTPPacketIncomingPtr packet) override;

      virtual void requestShutdown() override;
      virtual void notifyClosed() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => ISCTPTransportForDataChannelDelegate
      #pragma mark
      
      virtual void onSCTPTransportStateChanged() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => IDataChannelAsyncDelegate
      #pragma mark

      virtual void onRequestShutdown() override;
      virtual void onNotifiedClosed() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => IWakeDelegate
      #pragma mark

      virtual void onWake() override;
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => IDataChannelAsyncDelegate
      #pragma mark

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isOpen() const;
      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepSCTPTransport();
      bool stepIssueConnect();
      bool stepWaitConnectAck();
      bool stepOpen();
      bool stepSendData(bool onlyControlPackets = false);
      bool stepDeliveryIncomingPacket();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

      bool send(
                SCTPPayloadProtocolIdentifier ppid,
                const BYTE *buffer,
                size_t bufferSizeInBytes
                );

      void sendControlOpen();
      void sendControlAck();

      bool deliverOutgoing(
                           SCTPPacketOutgoingPtr packet,
                           bool fixPacket = true
                           );

      bool handleOpenPacket(SecureByteBlock &buffer);
      bool handleAckPacket(SecureByteBlock &buffer);
      void forwardDataPacketAsEvent(const SCTPPacketIncoming &packet);

      void outgoingPacketAdded(SCTPPacketOutgoingPtr packet);
      void outgoingPacketRemoved(SCTPPacketOutgoingPtr packet);

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel::TearAwayData
      #pragma mark

      struct TearAwayData
      {
        IDataChannelSubscriptionPtr mDefaultSubscription;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DataChannel => (data)
      #pragma mark

      AutoPUID mID;
      DataChannelWeakPtr mThisWeak;
      DataChannelPtr mGracefulShutdownReference;

      IDataChannelDelegateSubscriptions mSubscriptions;
      IDataChannelSubscriptionPtr mDefaultSubscription;

      UseDataTransportWeakPtr mDataTransport;
      UseDataTransportSubscriptionPtr mDataTransportSubscription;

      States mCurrentState {State_Connecting};

      bool mIncoming {};
      bool mIssuedOpen {};
      WORD mSessionID {};

      bool mRequestedSCTPShutdown {};
      bool mNotifiedClosed {};

      WORD mLastError {};
      String mLastErrorReason;

      String mBinaryType;
      ParametersPtr mParameters;

      BufferIncomingList mIncomingData;
      BufferOutgoingList mOutgoingData;
      size_t mOutgoingBufferFillSize {};
      size_t mBufferedAmountLowThreshold {};
      bool mBufferedAmountLowThresholdFired {};

      PromisePtr mSendReady;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDataChannelFactory
    #pragma mark

    interaction IDataChannelFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IDataChannelTypes::Parameters, Parameters)

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, ForDataTransport)

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, UseDataTransport)

      static IDataChannelFactory &singleton();

      virtual DataChannelPtr create(
                                    IDataChannelDelegatePtr delegate,
                                    IDataTransportPtr transport,
                                    const Parameters &params
                                    );

      virtual ForDataTransportPtr create(
                                         UseDataTransportPtr transport,
                                         WORD sessionID
                                         );
    };

    class DataChannelFactory : public IFactory<IDataChannelFactory> {};
  }
}


ZS_DECLARE_TEAR_AWAY_BEGIN(ortc::IDataChannel, ortc::internal::DataChannel::TearAwayData)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IStatsProvider::StatsTypeSet, StatsTypeSet)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::InvalidStateError, InvalidStateError)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IDataChannelSubscriptionPtr, IDataChannelSubscriptionPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IDataChannelDelegatePtr, IDataChannelDelegatePtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IDataTransportPtr, IDataTransportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IDataChannelTypes::ParametersPtr, ParametersPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IDataChannelTypes::States, States)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::SecureByteBlock, SecureByteBlock)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(zsLib::String, String)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(zsLib::BYTE, BYTE)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_1(getStats, PromiseWithStatsReportPtr, const StatsTypeSet &)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(getID, PUID)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_1(subscribe, IDataChannelSubscriptionPtr, IDataChannelDelegatePtr)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(transport, IDataTransportPtr)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(parameters, ParametersPtr)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(readyState, States)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(binaryType, String)
ZS_DECLARE_TEAR_AWAY_METHOD_1(binaryType, const char *)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(bufferedAmount, size_t)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(bufferedAmountLowThreshold, size_t)
ZS_DECLARE_TEAR_AWAY_METHOD_1(bufferedAmountLowThreshold, size_t)
ZS_DECLARE_TEAR_AWAY_METHOD_0(close)
ZS_DECLARE_TEAR_AWAY_METHOD_1(send, const String &)
ZS_DECLARE_TEAR_AWAY_METHOD_1(send, const SecureByteBlock &)
ZS_DECLARE_TEAR_AWAY_METHOD_2(send, const BYTE *, size_t)
ZS_DECLARE_TEAR_AWAY_END()
