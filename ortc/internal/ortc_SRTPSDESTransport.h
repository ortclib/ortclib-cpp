/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/ISRTPSDESTransport.h>
#include <ortc/IICETransport.h>

#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_ISRTPTransport.h>

#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForSecureTransport);
    ZS_DECLARE_INTERACTION_PTR(ISRTPTransportForSecureTransport);
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForSecureTransport);

    ZS_DECLARE_INTERACTION_PTR(ISRTPSDESTransportForSettings);
    ZS_DECLARE_INTERACTION_PROXY(ISRTPSDESTransportAsyncDelegate);



    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISRTPSDESTransportAsyncDelegate
    //

    interaction ISRTPSDESTransportAsyncDelegate
    {
      virtual void onAttachRTCP() = 0;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SRTPSDESTransport
    //
    
    class SRTPSDESTransport : public Noop,
                              public MessageQueueAssociator,
                              public SharedRecursiveLock,
                              public ISRTPSDESTransport,
                              public ISecureTransport,
                              public ISecureTransportForRTPSender,
                              public ISecureTransportForRTPReceiver,
                              public ISecureTransportForICETransport,
                              public ISecureTransportForSRTPTransport,
                              public ISecureTransportForRTPListener,
                              public ISRTPSDESTransportAsyncDelegate,
                              public ISRTPTransportDelegate,
                              public IICETransportDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ISRTPSDESTransport;
      friend interaction ISRTPSDESTransportFactory;
      friend interaction ISecureTransportForRTPSender;
      friend interaction ISecureTransportForRTPReceiver;
      friend interaction ISecureTransportForICETransport;
      friend interaction ISecureTransportForSRTPTransport;
      friend interaction ISecureTransportForRTPListener;

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForSecureTransport, UseICETransport);
      ZS_DECLARE_TYPEDEF_PTR(ISRTPTransportForSecureTransport, UseSRTPTransport);
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForSecureTransport, UseRTPListener);

    public:
      SRTPSDESTransport(
                        const make_private &,
                        IMessageQueuePtr queue,
                        ISRTPSDESTransportDelegatePtr delegate,
                        IICETransportPtr iceTransport
                        ) noexcept(false); // throws InvalidParameters

    protected:
      SRTPSDESTransport(Noop) noexcept :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init(
                const CryptoParameters &encryptParameters,
                const CryptoParameters &decryptParameters
                ) noexcept;

    public:
      virtual ~SRTPSDESTransport() noexcept;

      static SRTPSDESTransportPtr convert(ISRTPSDESTransportPtr object) noexcept;
      static SRTPSDESTransportPtr convert(ForRTPSenderPtr object) noexcept;
      static SRTPSDESTransportPtr convert(ForRTPReceiverPtr object) noexcept;
      static SRTPSDESTransportPtr convert(ForICETransportPtr object) noexcept;
      static SRTPSDESTransportPtr convert(ForSRTPPtr object) noexcept;
      static SRTPSDESTransportPtr convert(ForRTPListenerPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => IStatsProvider
      //

      PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISRTPSDESTransport
      //

      static ElementPtr toDebug(SRTPSDESTransportPtr transport) noexcept;

      static SRTPSDESTransportPtr create(
                                         ISRTPSDESTransportDelegatePtr delegate,
                                         IICETransportPtr iceTransport,
                                         const CryptoParameters &encryptParameters,
                                         const CryptoParameters &decryptParameters
                                         ) noexcept(false); // throws InvalidParameters

      static SRTPSDESTransportPtr convert(IRTPTransportPtr rtpTransport) noexcept;

      PUID getID() const noexcept override {return mID;}

      ISRTPSDESTransportSubscriptionPtr subscribe(ISRTPSDESTransportDelegatePtr delegate) noexcept override;

      IICETransportPtr transport() const noexcept override;
      IICETransportPtr rtcpTransport() const noexcept override;

      static ParametersPtr getLocalParameters() noexcept;

      void stop() noexcept override;

      //-----------------------------------------------------------------------
      //
      // DTLSTransport => ISecureTransport
      //

      // (duplicate) virtual PUID getID() const;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISecureTransportForRTPSender
      //

      // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr transport);

      // (duplicate) virtual PUID getID() const;

      ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) noexcept override;

      ISecureTransportTypes::States state(ISecureTransportTypes::States ignored) const noexcept override;

      virtual bool sendPacket(
                              IICETypes::Components sendOverICETransport,
                              IICETypes::Components packetType,
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) noexcept override;

      IICETransportPtr getICETransport() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISecureTransportForRTPReceiver
      //

      // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr transport) noexcept;

      // (duplicate) virtual PUID getID() const noexcept;

      // (duplicate) virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) noexcept = 0;
      // (duplicate) virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored = ISecureTransportTypes::States()) const noexcept = 0;

      // (duplicate) virtual bool sendPacket(
      //                                     IICETypes::Components sendOverICETransport,
      //                                     IICETypes::Components packetType,
      //                                     const BYTE *buffer,
      //                                     size_t bufferLengthInBytes
      //                                     ) noexcept override;

      // (duplicate) virtual IICETransportPtr getICETransport() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISRTPSDESTransportForICETransport
      //

      // (duplicate) static ElementPtr toDebug(ForICETransportPtr transport) noexcept;

      // (duplicate) virtual PUID getID() const noexcept;

      void notifyAssociateTransportCreated(
                                           IICETypes::Components associatedComponent,
                                           ICETransportPtr assoicated
                                           ) noexcept override;

      bool handleReceivedPacket(
                                IICETypes::Components viaTransport,
                                const BYTE *buffer,
                                size_t bufferLengthInBytes
                                ) noexcept override;
      void handleReceivedSTUNPacket(
                                    IICETypes::Components viaComponent,
                                    STUNPacketPtr packet
                                    ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISecureTransportForSRTPTransport
      //

      // (duplicate) static ElementPtr toDebug(ForSRTPPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      bool sendEncryptedPacket(
                               IICETypes::Components sendOverICETransport,
                               IICETypes::Components packetType,
                               const BYTE *buffer,
                               size_t bufferLengthInBytes
                               ) noexcept override;

      bool handleReceivedDecryptedPacket(
                                         IICETypes::Components viaTransport,
                                         IICETypes::Components packetType,
                                         const BYTE *buffer,
                                         size_t bufferLengthInBytes
                                         ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISecureTransportForRTPListener
      //

      // (duplicate) static ElementPtr toDebug(ForRTPListenerPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      RTPListenerPtr getListener() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISRTPSDESTransportForSettings
      //

      // (duplicate) static void applyDefaults() noexcept;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISRTPSDESTransportAsyncDelegate
      //

      void onAttachRTCP() override;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => ISRTPTransportDelegate
      //

      void onSRTPTransportLifetimeRemaining(
                                            ISRTPTransportPtr transport,
                                            ULONG leastLifetimeRemainingPercentageForAllKeys,
                                            ULONG overallLifetimeRemainingPercentage
                                            ) override;

      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => IICETransportDelegate
      //

      void onICETransportStateChange(
                                     IICETransportPtr transport,
                                     IICETransport::States state
                                     ) override;

      void onICETransportCandidatePairAvailable(
                                                IICETransportPtr transport,
                                                CandidatePairPtr candidatePair
                                                ) override;
      void onICETransportCandidatePairGone(
                                           IICETransportPtr transport,
                                           CandidatePairPtr candidatePair
                                           ) override;

      void onICETransportCandidatePairChanged(
                                              IICETransportPtr transport,
                                              CandidatePairPtr candidatePair
                                              ) override;

    protected:
      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      Log::Params debug(const char *message) const noexcept;
      ElementPtr toDebug() const noexcept;

      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;
      void cancel() noexcept;

      bool stepIceState() noexcept;

      void setError(WORD error, const char *reason = NULL) noexcept;
      void setState(ISecureTransportTypes::States state) noexcept;

      UseICETransportPtr fixRTCPTransport() const noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // SRTPSDESTransport => (data)
      //

      AutoPUID mID;
      SRTPSDESTransportWeakPtr mThisWeak;
      SRTPSDESTransportPtr mGracefulShutdownReference;

      std::atomic<bool> mShutdown {false}; // no lock needed to access

      ISRTPSDESTransportDelegateSubscriptions mSubscriptions;
      ISRTPSDESTransportSubscriptionPtr mDefaultSubscription;

      ISecureTransportDelegateSubscriptions mSecureTransportSubscriptions;
      std::atomic<ISecureTransport::States> mSecureTransportState {ISecureTransport::State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseICETransportPtr mICETransportRTP;
      IICETransportSubscriptionPtr mICETransportSubscription;
      bool mAttachedRTCP {false};
      mutable UseICETransportPtr mICETransportRTCP;

      UseSRTPTransportPtr mSRTPTransport;

      UseRTPListenerPtr mRTPListener;   // no lock needed
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISRTPSDESTransportFactory
    //

    interaction ISRTPSDESTransportFactory
    {
      typedef ISRTPSDESTransportTypes::CryptoParameters CryptoParameters;
      typedef ISRTPSDESTransportTypes::ParametersPtr ParametersPtr;

      static ISRTPSDESTransportFactory &singleton() noexcept;

      virtual SRTPSDESTransportPtr create(
                                          ISRTPSDESTransportDelegatePtr delegate,
                                          IICETransportPtr iceTransport,
                                          const CryptoParameters &encryptParameters,
                                          const CryptoParameters &decryptParameters
                                          ) noexcept;

      virtual ParametersPtr getLocalParameters() noexcept;
    };

    class SRTPSDESTransportFactory : public IFactory<ISRTPSDESTransportFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISRTPSDESTransportAsyncDelegate)
ZS_DECLARE_PROXY_METHOD(onAttachRTCP)
ZS_DECLARE_PROXY_END()
