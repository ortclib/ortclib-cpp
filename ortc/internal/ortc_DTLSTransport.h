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

#include <ortc/IDTLSTransport.h>
#include <ortc/IICETransport.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IDTLSTransportForRTPSender)
    ZS_DECLARE_INTERACTION_PTR(IDTLSTransportForICETransport)

    ZS_DECLARE_INTERACTION_PTR(IICETransportForRTPTransport)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportForRTPSender
    #pragma mark

    interaction IDTLSTransportForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportForRTPSender, ForRTPSender)

      static ElementPtr toDebug(ForRTPSenderPtr transport);

      virtual PUID getID() const = 0;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportForICETransport
    #pragma mark

    interaction IDTLSTransportForICETransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportForICETransport, ForICETransport)

      static ElementPtr toDebug(ForICETransportPtr transport);

      virtual PUID getID() const = 0;

      virtual void handleReceivedPacket(
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport
    #pragma mark
    
    class DTLSTransport : public Noop,
                          public MessageQueueAssociator,
                          public SharedRecursiveLock,
                          public IDTLSTransport,
                          public IDTLSTransportForRTPSender,
                          public IDTLSTransportForICETransport,
                          public IWakeDelegate,
                          public IICETransportDelegate
    {
    public:
      friend interaction IDTLSTransport;
      friend interaction IDTLSTransportFactory;
      friend interaction IDTLSTransportForRTPSender;
      friend interaction IDTLSTransportForICETransport;

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForRTPTransport, UseICETransport)

    protected:
      DTLSTransport(
                    IMessageQueuePtr queue,
                    IDTLSTransportDelegatePtr delegate,
                    IICETransportPtr iceTransport
                    );

      DTLSTransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~DTLSTransport();

      static DTLSTransportPtr convert(IDTLSTransportPtr object);
      static DTLSTransportPtr convert(ForRTPSenderPtr object);
      static DTLSTransportPtr convert(ForICETransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransport
      #pragma mark

      static ElementPtr toDebug(DTLSTransportPtr transport);

      static DTLSTransportPtr create(
                                      IDTLSTransportDelegatePtr delegate,
                                      IICETransportPtr iceTransport
                                      );

      virtual PUID getID() const;

      virtual IDTLSTransportSubscriptionPtr subscribe(IDTLSTransportDelegatePtr delegate);

      virtual IICETransportPtr transport() const;

      virtual States getState() const;

      virtual ParametersPtr getLocalParameters() const;
      virtual ParametersPtr getRemoteParameters() const;

      virtual SecureByteBlockListPtr getRemoteCertificates() const;

      virtual void start(const Parameters &remoteParameters) throw (
                                                                    InvalidStateError,
                                                                    InvalidParameters
                                                                    );

      virtual void stop();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransportForRTPSender
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransportForICETransport
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual void handleReceivedPacket(
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IWakeDelegate
      #pragma mark

      virtual void onWake();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IICETransportDelegate
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
      #pragma mark DTLSTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();

      void cancel();

      void setState(IDTLSTransportTypes::States state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => (data)
      #pragma mark

      AutoPUID mID;
      DTLSTransportWeakPtr mThisWeak;
      DTLSTransportPtr mGracefulShutdownReference;

      IDTLSTransportDelegateSubscriptions mSubscriptions;
      IDTLSTransportSubscriptionPtr mDefaultSubscription;

      IDTLSTransportTypes::States mCurrentState {IDTLSTransportTypes::State_New};

      WORD mLastError {};
      String mLastErrorReason;

      UseICETransportPtr mICETransport;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportFactory
    #pragma mark

    interaction IDTLSTransportFactory
    {
      static IDTLSTransportFactory &singleton();

      virtual DTLSTransportPtr create(
                                      IDTLSTransportDelegatePtr delegate,
                                      IICETransportPtr iceTransport
                                      );
    };

    class DTLSTransportFactory : public IFactory<IDTLSTransportFactory> {};
  }
}
