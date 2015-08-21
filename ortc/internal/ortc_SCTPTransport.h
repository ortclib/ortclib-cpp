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

#include <ortc/ISCTPTransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>
#include <zsLib/ProxySubscriptions.h>
#include <zsLib/TearAway.h>

#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SCTPInit)
    ZS_DECLARE_CLASS_PTR(SCTPTransport)

    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSettings)
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForDataChannel)
    ZS_DECLARE_INTERACTION_PTR(IDataChannelForSCTPTransport)
    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForDataTransport)

    ZS_DECLARE_INTERACTION_PTR(IICETransportForDataTransport)

    ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportAsyncDelegate)
    ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportForDataChannelDelegate)
    
    ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISCTPTransportForDataChannelSubscription, ISCTPTransportForDataChannelDelegate)


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForSettings
    #pragma mark

    interaction ISCTPTransportForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSettings, ForSettings)

      static void applyDefaults();

      virtual ~ISCTPTransportForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForDataChannel
    #pragma mark

    interaction ISCTPTransportForDataChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel, ForDataChannel)

      static ElementPtr toDebug(ForDataChannelPtr transport);

      virtual PUID getID() const = 0;

      virtual ISCTPTransportForDataChannelSubscriptionPtr subscribe(ISCTPTransportForDataChannelDelegatePtr delegate) = 0;

      virtual bool notifySendSCTPPacket(
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportAsyncDelegate
    #pragma mark

    interaction ISCTPTransportAsyncDelegate
    {
      virtual ~ISCTPTransportAsyncDelegate() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForDataChannelDelegate
    #pragma mark

    interaction ISCTPTransportForDataChannelDelegate
    {
      virtual void onSCTPTransportReady() = 0;
      virtual void onSCTPTransportClosed() = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForDataChannelSubscription
    #pragma mark
    
    interaction ISCTPTransportForDataChannelSubscription
    {
      virtual PUID getID() const = 0;
      
      virtual void cancel() = 0;
      
      virtual void background() = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISCTPTransportAsyncDelegate)
//ZS_DECLARE_PROXY_METHOD_0(onWhatever)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISCTPTransportForDataChannelDelegate)
ZS_DECLARE_PROXY_METHOD_0(onSCTPTransportReady)
ZS_DECLARE_PROXY_METHOD_0(onSCTPTransportClosed)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::internal::ISCTPTransportForDataChannelDelegate, ortc::internal::ISCTPTransportForDataChannelSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onSCTPTransportReady)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onSCTPTransportClosed)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport
    #pragma mark
    
    class SCTPTransport : public Noop,
                          public MessageQueueAssociator,
                          public SharedRecursiveLock,
                          public ISCTPTransport,
                          public ISCTPTransportForSettings,
                          public IDataTransportForSecureTransport,
                          public ISCTPTransportForDataChannel,
                          public IWakeDelegate,
                          public zsLib::ITimerDelegate,
                          public ISCTPTransportAsyncDelegate,
                          public IICETransportDelegate,
                          public IPromiseSettledDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ISCTPTransport;
      friend interaction ISCTPTransportFactory;
      friend interaction ISCTPTransportForSettings;
      friend interaction ISCTPTransportForSecureTransport;
      friend interaction ISCTPTransportForDataChannel;
      friend interaction IDataTransportForSecureTransport;

      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, UseDataChannel)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForDataTransport, UseICETransport)

      ZS_DECLARE_STRUCT_PTR(TearAwayData)

      enum States
      {
        State_Pending,
        State_DTLSComplete,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      SCTPTransport(
                    const make_private &,
                    IMessageQueuePtr queue,
                    UseSecureTransportPtr secureTransport
                    );

    protected:
      SCTPTransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~SCTPTransport();

      static SCTPTransportPtr convert(ISCTPTransportPtr object);
      static SCTPTransportPtr convert(IDataTransportPtr object);
      static SCTPTransportPtr convert(ForSettingsPtr object);
      static SCTPTransportPtr convert(ForSecureTransportPtr object);
      static SCTPTransportPtr convert(ForDataChannelPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransport
      #pragma mark

      static ElementPtr toDebug(SCTPTransportPtr transport);

      static ISCTPTransportPtr create(
                                      ISCTPTransportDelegatePtr delegate,
                                      IDTLSTransportPtr transport
                                      );

      virtual PUID getID() const override {return mID;}

      static CapabilitiesPtr getCapabilities();

      virtual void start(const Capabilities &remoteCapabilities) override;
      virtual void stop() override;

      virtual ISCTPTransportSubscriptionPtr subscribe(ISCTPTransportDelegatePtr delegate) override;


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IDataTransportForSecureTransport
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForDTLSTransportPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      static ForSecureTransportPtr create(UseSecureTransportPtr transport);

      virtual bool handleDataPacket(
                                    const BYTE *buffer,
                                    size_t bufferLengthInBytes
                                    ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransportForDataChannel
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForDataChannelPtr transport);

      // (duplicate) virtual PUID getID() const = 0;
      virtual ISCTPTransportForDataChannelSubscriptionPtr subscribe(ISCTPTransportForDataChannelDelegatePtr delegate) override;

      virtual bool notifySendSCTPPacket(
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransportAsyncDelegate
      #pragma mark


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IICETransportDelegate
      #pragma mark

      virtual void onICETransportStateChanged(
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


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepBogusDoSomething();
      bool stepDTLSTransportReady();
      bool stepDTLSTransportError();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    public:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport::TearAwayData
      #pragma mark

      struct TearAwayData
      {
        UseSecureTransportPtr mSecureTransport;
        ISCTPTransportSubscriptionPtr mDefaultSubscription;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => (data)
      #pragma mark

      AutoPUID mID;
      SCTPTransportWeakPtr mThisWeak;
      SCTPTransportPtr mGracefulShutdownReference;

      SCTPInitPtr mSCTPInit;

      ISCTPTransportDelegateSubscriptions mSubscriptions;

      ISCTPTransportForDataChannelDelegateSubscriptions mDataChannelSubscriptions;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseSecureTransportPtr mSecureTransport;
      PromisePtr mSecureTransportReady;
//      ISecureTransportSubscriptionPtr mSecureTransportSubscription;

      UseICETransportPtr mICETransport;
      IICETransportSubscriptionPtr mICETransportSubscription;

      Capabilities mCapabilities;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportFactory
    #pragma mark

    interaction ISCTPTransportFactory
    {
      typedef ISCTPTransportTypes::CapabilitiesPtr CapabilitiesPtr;

      ZS_DECLARE_TYPEDEF_PTR(IDataTransportForSecureTransport, ForSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport)

      static ISCTPTransportFactory &singleton();

      virtual ISCTPTransportPtr create(
                                       ISCTPTransportDelegatePtr delegate,
                                       IDTLSTransportPtr transport
                                       );

      virtual ForSecureTransportPtr create(UseSecureTransportPtr transport);

      virtual CapabilitiesPtr getCapabilities();
    };

    class SCTPTransportFactory : public IFactory<ISCTPTransportFactory> {};
  }
}

ZS_DECLARE_TEAR_AWAY_BEGIN(ortc::ISCTPTransport, ortc::internal::SCTPTransport::TearAwayData)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportSubscriptionPtr, ISCTPTransportSubscriptionPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportDelegatePtr, ISCTPTransportDelegatePtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::ISCTPTransportTypes::Capabilities, Capabilities)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_TEAR_AWAY_TYPEDEF(ortc::InvalidStateError, InvalidStateError)
  // NOTE: custom tear away forward
  virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError)
  {
    return getDelegate()->getStats();
  }
ZS_DECLARE_TEAR_AWAY_METHOD_CONST_RETURN_0(getID, PUID)
ZS_DECLARE_TEAR_AWAY_METHOD_1(start, const Capabilities &)
ZS_DECLARE_TEAR_AWAY_METHOD_0(stop)
ZS_DECLARE_TEAR_AWAY_METHOD_RETURN_1(subscribe, ISCTPTransportSubscriptionPtr, ISCTPTransportDelegatePtr)
ZS_DECLARE_TEAR_AWAY_END()
