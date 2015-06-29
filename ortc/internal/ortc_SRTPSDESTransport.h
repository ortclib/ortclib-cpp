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

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(ISRTPTransportForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForSecureTransport)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPSDESTransport
    #pragma mark
    
    class SRTPSDESTransport : public Noop,
                              public MessageQueueAssociator,
                              public SharedRecursiveLock,
                              public ISRTPSDESTransport,
                              public ISecureTransportForRTPSender,
                              public ISecureTransportForICETransport,
                              public ISecureTransportForSRTP,
                              public ISecureTransportForRTPListener,
                              public IWakeDelegate,
                              public IICETransportDelegate,
                              public ISRTPTransportDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ISRTPSDESTransport;
      friend interaction ISRTPSDESTransportFactory;
      friend interaction ISecureTransportForRTPSender;
      friend interaction ISecureTransportForICETransport;
      friend interaction ISecureTransportForSRTP;
      friend interaction ISecureTransportForRTPListener;

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForSecureTransport, UseICETransport)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPTransportForSecureTransport, UseSRTPTransport)
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForSecureTransport, UseRTPListener)

      enum State
      {
      };

    public:
      SRTPSDESTransport(
                        const make_private &,
                        IMessageQueuePtr queue,
                        ISRTPSDESTransportDelegatePtr delegate,
                        IICETransportPtr iceTransport
                        );

    protected:
      SRTPSDESTransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init(
                const CryptoParameters &encryptParameters,
                const CryptoParameters &decryptParameters
                );

    public:
      virtual ~SRTPSDESTransport();

      static SRTPSDESTransportPtr convert(ISRTPSDESTransportPtr object);
      static SRTPSDESTransportPtr convert(ForRTPSenderPtr object);
      static SRTPSDESTransportPtr convert(ForICETransportPtr object);
      static SRTPSDESTransportPtr convert(ForSRTPPtr object);
      static SRTPSDESTransportPtr convert(ForRTPListenerPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => ISRTPSDESTransport
      #pragma mark

      static ElementPtr toDebug(SRTPSDESTransportPtr transport);

      static SRTPSDESTransportPtr create(
                                         ISRTPSDESTransportDelegatePtr delegate,
                                         IICETransportPtr iceTransport,
                                         const CryptoParameters &encryptParameters,
                                         const CryptoParameters &decryptParameters
                                         );

      static SRTPSDESTransportPtr convert(IRTPTransportPtr rtpTransport);

      virtual PUID getID() const override {return mID;}

      virtual ISRTPSDESTransportSubscriptionPtr subscribe(ISRTPSDESTransportDelegatePtr delegate) override;

      virtual IICETransportPtr transport() const override;
      virtual IICETransportPtr rtcpTransport() const override;

      static ParametersPtr getLocalParameters();

      virtual void stop() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => ISecureTransportForRTPSender
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPSenderPtr transport);

      // (duplicate) virtual PUID getID() const;

      virtual bool sendPacket(
                              IICETypes::Components sendOverICETransport,
                              IICETypes::Components packetType,
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => ISRTPSDESTransportForICETransport
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForICETransportPtr transport);

      // (duplicate) virtual PUID getID() const;

      virtual void handleReceivedPacket(
                                        IICETypes::Components viaTransport,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) override;
      virtual void handleReceivedSTUNPacket(
                                            IICETypes::Components viaComponent,
                                            STUNPacketPtr packet
                                            ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => ISecureTransportForSRTP
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForSRTPPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual bool sendEncryptedPacket(
                                       IICETypes::Components sendOverICETransport,
                                       IICETypes::Components packetType,
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       ) override;

      virtual bool handleReceivedDecryptedPacket(
                                                 IICETypes::Components viaTransport,
                                                 IICETypes::Components packetType,
                                                 const BYTE *buffer,
                                                 size_t bufferLengthInBytes
                                                 ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => ISecureTransportForRTPListener
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPListenerPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual RTPListenerPtr getListener() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => IICETransportDelegate
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
      #pragma mark SRTPSDESTransport => ISRTPTransportDelegate
      #pragma mark

      virtual void onSRTPTransportLifetimeRemaining(
                                                    ISRTPTransportPtr transport,
                                                    ULONG lifetimeRemaingPercentage
                                                    ) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();

      void cancel();

      void setError(WORD error, const char *reason = NULL);

      UseICETransportPtr fixRTCPTransport() const;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => (data)
      #pragma mark

      AutoPUID mID;
      SRTPSDESTransportWeakPtr mThisWeak;
      SRTPSDESTransportPtr mGracefulShutdownReference;

      std::atomic<bool> mShutdown {false}; // no lock needed to access

      ISRTPSDESTransportDelegateSubscriptions mSubscriptions;
      ISRTPSDESTransportSubscriptionPtr mDefaultSubscription;

      WORD mLastError {};
      String mLastErrorReason;

      UseICETransportPtr mICETransportRTP;
      mutable UseICETransportPtr mICETransportRTCP;

      UseSRTPTransportPtr mSRTPTransport;

      UseRTPListenerPtr mRTPListener;   // no lock needed
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISRTPSDESTransportFactory
    #pragma mark

    interaction ISRTPSDESTransportFactory
    {
      typedef ISRTPSDESTransportTypes::CryptoParameters CryptoParameters;

      static ISRTPSDESTransportFactory &singleton();

      virtual SRTPSDESTransportPtr create(
                                          ISRTPSDESTransportDelegatePtr delegate,
                                          IICETransportPtr iceTransport,
                                          const CryptoParameters &encryptParameters,
                                          const CryptoParameters &decryptParameters
                                          );
    };

    class SRTPSDESTransportFactory : public IFactory<ISRTPSDESTransportFactory> {};
  }
}
