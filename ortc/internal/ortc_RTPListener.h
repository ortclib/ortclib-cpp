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
#include <ortc/internal/ortc_ISecureTransport.h>

#include <ortc/IRTPListener.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>
#include <zsLib/TearAway.h>

#define ORTC_SETTING_RTP_LISTENER_MAX_RTCP_PACKETS_IN_BUFFER "ortc/rtp-listener/max-rtcp-packets-in-buffer"
#define ORTC_SETTING_RTP_LISTENER_MAX_AGE_RTCP_PACKETS_IN_SECONDS "ortc/rtp-listener/max-age-rtcp-packets-in-seconds"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForRTPListener)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForRTPListener)

    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForSettings)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForRTPReceiver)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForRTPSender)

    ZS_DECLARE_INTERACTION_PROXY(IRTPListenerAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForSettings
    #pragma mark

    interaction IRTPListenerForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPListenerForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForSecureTransport
    #pragma mark

    interaction IRTPListenerForSecureTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForSecureTransport, ForSecureTransport)

      static ElementPtr toDebug(ForSecureTransportPtr listener);

      static RTPListenerPtr create(IRTPTransportPtr transport);

      virtual PUID getID() const = 0;

      virtual bool handleRTPPacket(
                                   IICETypes::Components viaComponent,
                                   IICETypes::Components packetType,
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForRTPReceiver
    #pragma mark

    interaction IRTPListenerForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForRTPReceiver, ForRTPReceiver)

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPListener, UseReceiver)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<SecureByteBlockPtr> BufferList;

      static ElementPtr toDebug(ForRTPReceiverPtr listener);

      static ForRTPReceiverPtr getListener(IRTPTransportPtr rtpTransport);

      virtual PUID getID() const = 0;

      virtual void registerReceiver(
                                    UseReceiverPtr inReceiver,
                                    const Parameters &inParams,
                                    BufferList &outBufferList
                                    ) = 0;

      virtual void unregisterReceiver(UseReceiver &inReceiver) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerForRTPSender
    #pragma mark

    interaction IRTPListenerForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForRTPSender, ForRTPSender)

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPListener, UseSender)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<SecureByteBlockPtr> BufferList;

      static ElementPtr toDebug(ForRTPSenderPtr listener);

      static ForRTPSenderPtr getListener(IRTPTransportPtr rtpTransport);

      virtual PUID getID() const = 0;

      virtual void registerSender(
                                  UseSenderPtr inSender,
                                  const Parameters &inParams,
                                  BufferList &outBufferList
                                  ) = 0;

      virtual void unregisterSender(UseSender &inSender) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerAsyncDelegate
    #pragma mark

    interaction IRTPListenerAsyncDelegate
    {
      virtual ~IRTPListenerAsyncDelegate() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPListener
    #pragma mark

    class RTPListener : public Noop,
                        public MessageQueueAssociator,
                        public SharedRecursiveLock,
                        public IRTPListener,
                        public IRTPListenerForSettings,
                        public IRTPListenerForSecureTransport,
                        public IRTPListenerForRTPReceiver,
                        public IRTPListenerForRTPSender,
                        public IWakeDelegate,
                        public zsLib::ITimerDelegate,
                        public IRTPListenerAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPListener;
      friend interaction IRTPListenerFactory;
      friend interaction IRTPListenerForSettings;
      friend interaction IRTPListenerForSecureTransport;
      friend interaction IRTPListenerForRTPReceiver;
      friend interaction IRTPListenerForRTPSender;

      ZS_DECLARE_STRUCT_PTR(TearAwayData)

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPListener, UseRTPReceiver)
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPListener, UseRTPSender)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTransport, UseRTPTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPListener, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      typedef std::list<SecureByteBlockPtr> BufferList;

      typedef std::pair<Time, SecureByteBlockPtr> TimeBufferPair;
      typedef std::list<TimeBufferPair> BufferedRTCPPacketList;

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      RTPListener(
                  const make_private &,
                  IMessageQueuePtr queue,
                  IRTPListenerDelegatePtr delegate,
                  IRTPTransportPtr transport
                  );

    protected:
      RTPListener(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPListener();

      static RTPListenerPtr convert(IRTPListenerPtr object);
      static RTPListenerPtr convert(ForSettingsPtr object);
      static RTPListenerPtr convert(ForSecureTransportPtr object);
      static RTPListenerPtr convert(ForRTPReceiverPtr object);
      static RTPListenerPtr convert(ForRTPSenderPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListener
      #pragma mark

      static ElementPtr toDebug(RTPListenerPtr listener);

      static IRTPListenerPtr create(
                                    IRTPListenerDelegatePtr delegate,
                                    IRTPTransportPtr transport,
                                    Optional<HeaderExtensionParametersList> headerExtensions
                                    );

      virtual PUID getID() const {return mID;}

      virtual IRTPListenerSubscriptionPtr subscribe(IRTPListenerDelegatePtr delegate) override;

      virtual IRTPTransportPtr transport() const override;

      virtual void setHeaderExtensions(const HeaderExtensionParametersList &headerExtensions);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListenerForSecureTransport
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForSecureTransportPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      static RTPListenerPtr create(IRTPTransportPtr transport);

      virtual bool handleRTPPacket(
                                   IICETypes::Components viaComponent,
                                   IICETypes::Components packetType,
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListenerForRTPReceiver
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPReceiverPtr listener);

      // (duplicate) virtual PUID getID() const = 0;

      virtual void registerReceiver(
                                    UseReceiverPtr inReceiver,
                                    const Parameters &inParams,
                                    BufferList &outBufferList
                                    );

      virtual void unregisterReceiver(UseReceiver &inReceiver);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListenerForRTPSender
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr listener);

      // (duplicate) virtual PUID getID() const = 0;

      virtual void registerSender(
                                  UseSenderPtr inSender,
                                  const Parameters &inParams,
                                  BufferList &outBufferList
                                  );

      virtual void unregisterSender(UseSender &inReceiver);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => IRTPListenerAsyncDelegate
      #pragma mark

    public:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener::TearAwayData
      #pragma mark

      struct TearAwayData
      {
        UseRTPTransportPtr mRTPTransport;
        IRTPListenerSubscriptionPtr mDefaultSubscription;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => (internal)
      #pragma mark

      static Log::Params slog(const char *message);
      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepBogusDoSomething();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

      void expireRTCPPackets();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListener => (data)
      #pragma mark

      AutoPUID mID;
      RTPListenerWeakPtr mThisWeak;
      RTPListenerPtr mGracefulShutdownReference;

      IRTPListenerDelegateSubscriptions mSubscriptions;
      IRTPListenerSubscriptionPtr mDefaultSubscription;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseRTPTransportWeakPtr mRTPTransport;

      size_t mMaxBufferedRTCPPackets {};
      Seconds mMaxRTCPPacketAge {};

      BufferedRTCPPacketList mBufferedRTCPPackets;

      PUID mReceiverID {};
      UseRTPReceiverWeakPtr mReceiver;

      PUID mSenderID {};
      UseRTPSenderWeakPtr mSender;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPListenerFactory
    #pragma mark

    interaction IRTPListenerFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::HeaderExtensionParametersList, HeaderExtensionParametersList)

      static IRTPListenerFactory &singleton();

      virtual IRTPListenerPtr create(
                                     IRTPListenerDelegatePtr delegate,
                                     IRTPTransportPtr transport,
                                     Optional<HeaderExtensionParametersList> headerExtensions = Optional<HeaderExtensionParametersList>()
                                     );

      virtual RTPListenerPtr create(IRTPTransportPtr transport);
    };

    class RTPListenerFactory : public IFactory<IRTPListenerFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPListenerAsyncDelegate)
//ZS_DECLARE_PROXY_METHOD_0(onWhatever)
ZS_DECLARE_PROXY_END()


ZS_DECLARE_TEAR_AWAY_BEGIN(ortc::IRTPListener, ortc::internal::RTPListener::TearAwayData)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IRTPListenerSubscriptionPtr, IRTPListenerSubscriptionPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IRTPListenerDelegatePtr, IRTPListenerDelegatePtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IRTPTransportPtr, IRTPTransportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IRTPTypes::HeaderExtensionParametersList, HeaderExtensionParametersList)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(getID, PUID)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_1(subscribe, IRTPListenerSubscriptionPtr, IRTPListenerDelegatePtr)
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(transport, IRTPTransportPtr)
ZS_DECLARE_TEAR_AWAY_METHOD_1(setHeaderExtensions, const HeaderExtensionParametersList &)
ZS_DECLARE_TEAR_AWAY_END()
