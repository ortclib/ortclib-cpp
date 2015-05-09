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

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForRTPTransport)

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
                          public IWakeDelegate,
                          public IICETransportDelegate
    {
    public:
      friend interaction ISRTPSDESTransport;
      friend interaction ISRTPSDESTransportFactory;
      friend interaction ISecureTransportForRTPSender;
      friend interaction ISecureTransportForICETransport;

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForRTPTransport, UseICETransport)

      enum State
      {
      };

    protected:
      SRTPSDESTransport(
                        IMessageQueuePtr queue,
                        ISRTPSDESTransportDelegatePtr delegate,
                        IICETransportPtr iceTransport,
                        const CryptoParameters &encryptParameters,
                        const CryptoParameters &decryptParameters
                        );

      SRTPSDESTransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~SRTPSDESTransport();

      static SRTPSDESTransportPtr convert(ISRTPSDESTransportPtr object);
      static SRTPSDESTransportPtr convert(ForRTPSenderPtr object);
      static SRTPSDESTransportPtr convert(ForICETransportPtr object);

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

      virtual PUID getID() const;

      virtual ISRTPSDESTransportSubscriptionPtr subscribe(ISRTPSDESTransportDelegatePtr delegate);

      virtual IICETransportPtr transport() const;

      static ParametersPtr getLocalParameters();

      virtual void stop();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => ISRTPSDESTransportForRTPSender
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => ISRTPSDESTransportForICETransport
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual void handleReceivedPacket(
                                        IICETypes::Components viaComponent,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) override;

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

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPSDESTransport => (data)
      #pragma mark

      AutoPUID mID;
      SRTPSDESTransportWeakPtr mThisWeak;
      SRTPSDESTransportPtr mGracefulShutdownReference;

      ISRTPSDESTransportDelegateSubscriptions mSubscriptions;
      ISRTPSDESTransportSubscriptionPtr mDefaultSubscription;

      WORD mLastError {};
      String mLastErrorReason;

      UseICETransportPtr mICETransport;
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
