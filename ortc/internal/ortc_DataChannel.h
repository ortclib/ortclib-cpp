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
    //
    // IDataChannelForSCTPTransport
    //

    interaction IDataChannelForSCTPTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, ForDataTransport);

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, UseDataTransport);

      static ElementPtr toDebug(ForDataTransportPtr dataChannel) noexcept;

      static ForDataTransportPtr create(
                                        UseDataTransportPtr transport,
                                        WORD sessionID
                                        ) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual bool isIncoming() const noexcept = 0;

      virtual bool handleSCTPPacket(SCTPPacketIncomingPtr packet) noexcept = 0;

      virtual void requestShutdown() noexcept = 0;
      virtual void notifyClosed() noexcept = 0;

      virtual void notifyRemapFailure() noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IDataChannelAsyncDelegate
    //

    interaction IDataChannelAsyncDelegate
    {
      virtual void onRequestShutdown() = 0;
      virtual void onNotifiedClosed() = 0;
      virtual void onNotifiedRemapFailure() = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IDataChannelAsyncDelegate)
ZS_DECLARE_PROXY_METHOD(onRequestShutdown)
ZS_DECLARE_PROXY_METHOD(onNotifiedClosed)
ZS_DECLARE_PROXY_METHOD(onNotifiedRemapFailure)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // DataChannel
    //

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
                  ) noexcept;

    protected:
      DataChannel(Noop) noexcept :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

    public:
      virtual ~DataChannel() noexcept;

      static DataChannelPtr convert(IDataChannelPtr object) noexcept;
      static DataChannelPtr convert(ForDataTransportPtr object) noexcept;

    protected:

      //-----------------------------------------------------------------------
      //
      // DataChannel => IDataChannel
      //

      static ElementPtr toDebug(DataChannelPtr transport) noexcept;

      static DataChannelPtr create(
                                   IDataChannelDelegatePtr delegate,
                                   IDataTransportPtr transport,
                                   const Parameters &params
                                   ) noexcept(false);  // throws InvalidParameters

      PUID getID() const noexcept override {return mID;}

      IDataChannelSubscriptionPtr subscribe(IDataChannelDelegatePtr delegate) noexcept override;

      IDataTransportPtr transport() const noexcept override;

      ParametersPtr parameters() const noexcept override;

      States readyState() const noexcept override;

      size_t bufferedAmount() const noexcept override;
      size_t bufferedAmountLowThreshold() const noexcept override;
      void bufferedAmountLowThreshold(size_t value) noexcept override;

      String binaryType() const noexcept override;
      void binaryType(const char *str) noexcept override;

      void close() noexcept override;

      void send(const String &data) noexcept override;
      void send(const SecureByteBlock &data) noexcept override;
      void send(
                const BYTE *buffer,
                size_t bufferSizeInBytes
                ) noexcept(false) override; // throws InvalidParameters

      //-----------------------------------------------------------------------
      //
      // DataChannel => IStatsProvider
      //

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // DataChannel => IDataChannelForSCTPTransport
      //

      // (duplicate) static ElementPtr toDebug(DataChannelPtr transport);

      static ForDataTransportPtr create(
                                        UseDataTransportPtr transport,
                                        WORD sessionID
                                        ) noexcept;

      // (duplicate) virtual PUID getID() const = 0;

      bool isIncoming() const noexcept override {return mIncoming;}

      bool handleSCTPPacket(SCTPPacketIncomingPtr packet) noexcept override;

      void requestShutdown() noexcept override;
      void notifyClosed() noexcept override;

      void notifyRemapFailure() noexcept override;

      //-----------------------------------------------------------------------
      //
      // DataChannel => ISCTPTransportForDataChannelDelegate
      //
      
      void onSCTPTransportStateChanged() override;

      //-----------------------------------------------------------------------
      //
      // DataChannel => IDataChannelAsyncDelegate
      //

      void onRequestShutdown() override;
      void onNotifiedClosed() override;
      void onNotifiedRemapFailure() override;

      //-----------------------------------------------------------------------
      //
      // DataChannel => IWakeDelegate
      //

      void onWake() override;
      
      //-----------------------------------------------------------------------
      //
      // DataChannel => IPromiseSettledDelegate
      //

      void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      //
      // DataChannel => IDataChannelAsyncDelegate
      //

    protected:
      //-----------------------------------------------------------------------
      //
      // DataChannel => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      static Log::Params slog(const char *message) noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isOpen() const noexcept;
      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;
      bool stepSCTPTransport() noexcept;
      bool stepIssueConnect() noexcept;
      bool stepWaitConnectAck() noexcept;
      bool stepOpen() noexcept;
      bool stepSendData(bool onlyControlPackets = false) noexcept;
      bool stepDeliveryIncomingPacket() noexcept;

      void cancel() noexcept;

      void setState(States state) noexcept;
      void setError(WORD error, const char *reason = NULL) noexcept;

      bool send(
                SCTPPayloadProtocolIdentifier ppid,
                const BYTE *buffer,
                size_t bufferSizeInBytes
                ) noexcept;

      void sendControlOpen() noexcept;
      void sendControlAck() noexcept;

      bool deliverOutgoing(
                           SCTPPacketOutgoingPtr packet,
                           bool fixPacket = true
                           ) noexcept;

      bool handleOpenPacket(SecureByteBlock &buffer) noexcept;
      bool handleAckPacket(SecureByteBlock &buffer) noexcept;
      void forwardDataPacketAsEvent(const SCTPPacketIncoming &packet) noexcept;

      void outgoingPacketAdded(SCTPPacketOutgoingPtr packet) noexcept;
      void outgoingPacketRemoved(SCTPPacketOutgoingPtr packet) noexcept;

    public:
      //-----------------------------------------------------------------------
      //
      // DataChannel::TearAwayData
      //

      struct TearAwayData
      {
        IDataChannelSubscriptionPtr mDefaultSubscription;
      };

    protected:
      //-----------------------------------------------------------------------
      //
      // DataChannel => (data)
      //

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
    //
    // IDataChannelFactory
    //

    interaction IDataChannelFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IDataChannelTypes::Parameters, Parameters);

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, ForDataTransport);

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, UseDataTransport);

      static IDataChannelFactory &singleton() noexcept;

      virtual DataChannelPtr create(
                                    IDataChannelDelegatePtr delegate,
                                    IDataTransportPtr transport,
                                    const Parameters &params
                                    ) noexcept(false); // throws InvalidParameters, InvalidStateError

      virtual ForDataTransportPtr create(
                                         UseDataTransportPtr transport,
                                         WORD sessionID
                                         ) noexcept;
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
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(getStats, PromiseWithStatsReportPtr, const StatsTypeSet &)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(getID, PUID)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN(subscribe, IDataChannelSubscriptionPtr, IDataChannelDelegatePtr)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(transport, IDataTransportPtr)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(parameters, ParametersPtr)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(readyState, States)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(binaryType, String)
ZS_DECLARE_TEAR_AWAY_METHOD_SYNC(binaryType, const char *)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(bufferedAmount, size_t)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_CONST(bufferedAmountLowThreshold, size_t)
ZS_DECLARE_TEAR_AWAY_METHOD_SYNC(bufferedAmountLowThreshold, size_t)
ZS_DECLARE_TEAR_AWAY_METHOD_SYNC(close)
ZS_DECLARE_TEAR_AWAY_METHOD_SYNC(send, const String &)
ZS_DECLARE_TEAR_AWAY_METHOD_SYNC(send, const SecureByteBlock &)
ZS_DECLARE_TEAR_AWAY_METHOD_SYNC_THROWS(send, const BYTE *, size_t)
ZS_DECLARE_TEAR_AWAY_END()
