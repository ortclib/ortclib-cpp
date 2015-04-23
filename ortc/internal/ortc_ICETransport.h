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

#include <ortc/IICETransport.h>
#include <openpeer/services/IWakeDelegate.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForICEGatherer)
    ZS_DECLARE_INTERACTION_PTR(IICETransportForRTPTransport)

    ZS_DECLARE_INTERACTION_PTR(IICEGathererForICETransport)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForICEGatherer
    #pragma mark

    interaction IICETransportForICEGatherer
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICEGatherer, ForICEGatherer)

      virtual PUID getID() const = 0;

      virtual void notifyRouteAdded(
                                    PUID routeID,
                                    IICETypes::CandidatePtr localCandidate,
                                    const IPAddress &fromIP
                                    ) = 0;
      virtual void notifyRouteRemoved(PUID routeID) = 0;

      virtual void notifyPacket(
                                PUID routeID,
                                STUNPacketPtr packet
                                ) = 0;
      virtual void notifyPacket(
                                PUID routeID,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) = 0;

      virtual bool needsMoreCandidates() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForRTPTransport
    #pragma mark

    interaction IICETransportForRTPTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForRTPTransport, ForRTPTransport)

      virtual PUID getID() const = 0;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              ) = 0;
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport
    #pragma mark

    class ICETransport : public Noop,
                         public MessageQueueAssociator,
                         public SharedRecursiveLock,
                         public IICETransport,
                         public IICETransportForICEGatherer,
                         public IICETransportForRTPTransport,
                         public IWakeDelegate
    {
    public:
      friend interaction IICETransport;
      friend interaction IICETransportFactory;
      friend interaction IICETransportForICEGatherer;

    protected:
      ICETransport(
                   IMessageQueuePtr queue,
                   IICETransportDelegatePtr delegate,
                   IICEGathererPtr gatherer
                   );

      ICETransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create()) {}

      void init();

    public:
      virtual ~ICETransport();

      static ICETransportPtr convert(IICETransportPtr object);
      static ICETransportPtr convert(ForICEGathererPtr object);
      static ICETransportPtr convert(ForRTPTransportPtr object);

      ZS_DECLARE_TYPEDEF_PTR(IICEGathererForICETransport, UseICEGatherer)

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransport
      #pragma mark

      static ElementPtr toDebug(ICETransportPtr transport);

      static ICETransportPtr create(
                                    IICETransportDelegatePtr delegate,
                                    IICEGathererPtr gatherer
                                    );

      virtual PUID getID() const;

      virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate);

      virtual IICEGathererPtr iceGatherer() const;

      virtual Roles role() const;
      virtual Components component() const;
      virtual States state() const;

      virtual CandidateListPtr getRemoteCandidates() const;

      virtual CandidatePairPtr getNominatedCandidatePair() const;

      virtual void start(
                         IICEGathererPtr gatherer,
                         Parameters remoteParameters,
                         Optional<Roles> role = Optional<Roles>()
                         ) throw (InvalidParameters);

      virtual void stop();

      virtual ParametersPtr getRemoteParameters() const;

      virtual IICETransportPtr createAssociatedTransport() throw (InvalidStateError);

      virtual void addRemoteCandidate(const GatherCandidate &remoteCandidate);
      virtual void setRemoteCandidates(const CandidateList &remoteCandidates);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForICEGatherer
      #pragma mark

      virtual void notifyRouteAdded(
                                    PUID routeID,
                                    IICETypes::CandidatePtr localCandidate,
                                    const IPAddress &fromIP
                                    );
      virtual void notifyRouteRemoved(PUID routeID);

      virtual void notifyPacket(
                                PUID routeID,
                                STUNPacketPtr packet
                                );
      virtual void notifyPacket(
                                PUID routeID,
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                );

      virtual bool needsMoreCandidates() const;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForRTPTransport
      #pragma mark

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferSizeInBytes
                              );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IWakeDelegate
      #pragma mark

      virtual void onWake();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();

      void cancel();

      void setState(IICETransportTypes::States state);
      void setError(WORD error, const char *reason = NULL);

    private:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGatherer => (data)
      #pragma mark
      
      AutoPUID mID;

      ICETransportWeakPtr mThisWeak;
      ICETransportPtr mGracefulShutdownReference;

      IICETransportDelegateSubscriptions mSubscriptions;
      IICETransportSubscriptionPtr mDefaultSubscription;

      Components mComponent {Component_RTP};

      IICETransportTypes::States mCurrentState {IICETransportTypes::State_New};

      WORD mLastError {};
      String mLastErrorReason;

      UseICEGathererPtr mGatherer;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportFactory
    #pragma mark

    interaction IICETransportFactory
    {
      static IICETransportFactory &singleton();

      virtual ICETransportPtr create(
                                     IICETransportDelegatePtr delegate,
                                     IICEGathererPtr gatherer
                                     );
    };

    class ICETransportFactory : public IFactory<IICETransportFactory> {};
  }
}
