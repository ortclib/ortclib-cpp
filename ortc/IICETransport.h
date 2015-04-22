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

#include <ortc/types.h>
#include <ortc/IICETypes.h>
#include <ortc/IStatsProvider.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportTypes
  #pragma mark
  
  interaction IICETransportTypes : public IICETypes
  {
    ZS_DECLARE_STRUCT_PTR(CandidatePair)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportTypes::States
    #pragma mark

    enum States
    {
      State_New,
      State_Checking,
      State_Connected,
      State_Completed,
      State_Disconnected,
      State_Closed,
    };

    static const char *toString(States state);
    static States toState(const char *state);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportTypes::States
    #pragma mark

    struct CandidatePair
    {
      Candidate mLocal;
      Candidate mRemote;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport
  #pragma mark
  
  interaction IICETransport : public IICETransportTypes,
                              public IRTCPTransport,
                              public IStatsProvider
  {
    static ElementPtr toDebug(IICETransportPtr transport);

    static IICETransportPtr create(
                                   IICETransportDelegatePtr delegate,
                                   IICEGathererPtr gatherer = IICEGathererPtr() // option to specify a gatherer
                                   );

    virtual PUID getID() const = 0;

    virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) = 0;

    virtual IICEGathererPtr iceGatherer() const = 0;

    virtual Roles role() const = 0;
    virtual Components component() const = 0;
    virtual States state() const = 0;

    virtual CandidateListPtr getRemoteCandidates() const = 0;

    virtual CandidatePairPtr getNominatedCandidatePair() const = 0;

    virtual void start(
                       IICEGathererPtr gatherer,
                       Parameters remoteParameters,
                       Optional<Roles> role = Optional<Roles>()
                       ) throw (InvalidParameters) = 0;

    virtual void stop() = 0;

    virtual ParametersPtr getRemoteParameters() const = 0;

    virtual IICETransportPtr createAssociatedTransport() throw (InvalidStateError) = 0;

    virtual void addRemoteCandidate(const GatherCandidate &remoteCandidate) = 0;
    virtual void setRemoteCandidates(const CandidateList &remoteCandidates) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportDelegate
  #pragma mark

  interaction IICETransportDelegate
  {
    ZS_DECLARE_TYPEDEF_PTR(IICETransportTypes::CandidatePair, CandidatePair)

    virtual void onICETransportStateChanged(
                                            IICETransportPtr transport,
                                            IICETransport::States state
                                            ) = 0;

    virtual void onICETransportCandidatePairChanged(
                                                    IICETransportPtr transport,
                                                    CandidatePairPtr candidatePair
                                                    ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportSubscription
  #pragma mark

  interaction IICETransportSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IICETransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransportPtr, IICETransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransport::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransport::CandidatePairPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_METHOD_2(onICETransportStateChanged, IICETransportPtr, States)
ZS_DECLARE_PROXY_METHOD_2(onICETransportCandidatePairChanged, IICETransportPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IICETransportDelegate, ortc::IICETransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransportPtr, IICETransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransport::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransport::CandidatePairPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICETransportStateChanged, IICETransportPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICETransportCandidatePairChanged, IICETransportPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
