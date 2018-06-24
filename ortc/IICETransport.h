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
  //
  // IICETransportTypes
  //
  
  interaction IICETransportTypes : public IICETypes
  {
    ZS_DECLARE_STRUCT_PTR(CandidatePair);
    ZS_DECLARE_STRUCT_PTR(Options);

    //-------------------------------------------------------------------------
    //
    // IICETransportTypes::States
    //

    enum States
    {
      State_First,

      State_New     = State_First,
      State_Checking,
      State_Connected,
      State_Completed,
      State_Disconnected,
      State_Failed,
      State_Closed,

      State_Last    = State_Closed,
    };

    static const char *toString(States state) noexcept;
    static States toState(const char *state) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    //
    // IICETransportTypes::States
    //

    struct CandidatePair
    {
      CandidatePtr mLocal;
      CandidatePtr mRemote;

      CandidatePair() noexcept {}
      CandidatePair(const CandidatePair &op2) noexcept;
      CandidatePair(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "candidatePair") const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash(bool includePriorities = true) const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IICETransportTypes::Options
    //

    struct Options
    {
      bool mAggressiveICE {false};
      IICETypes::Roles mRole {IICETypes::Role_Controlled};

      Options() noexcept {}
      Options(const Options &op2) noexcept {(*this) = op2;}
      Options(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IICETransport
  //
  
  interaction IICETransport : public IICETransportTypes,
                              public IRTCPTransport,
                              public IStatsProvider
  {
    static ElementPtr toDebug(IICETransportPtr transport) noexcept;

    static IICETransportPtr convert(IRTCPTransportPtr object) noexcept;

    static IICETransportPtr create(
                                   IICETransportDelegatePtr delegate,
                                   IICEGathererPtr gatherer = IICEGathererPtr() // option to specify a gatherer
                                   ) noexcept(false);

    virtual PUID getID() const noexcept = 0;

    virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) noexcept = 0;

    virtual IICEGathererPtr iceGatherer() const noexcept = 0;

    virtual Roles role() const noexcept = 0;
    virtual Components component() const noexcept = 0;
    virtual States state() const noexcept = 0;

    virtual CandidateListPtr getRemoteCandidates() const noexcept = 0;

    virtual CandidatePairPtr getSelectedCandidatePair() const noexcept = 0;

    virtual void start(
                       IICEGathererPtr gatherer,
                       const Parameters &remoteParameters,
                       Optional<Options> options = Optional<Options>()
                       ) noexcept(false) = 0; // throws InvalidParameters

    virtual void stop() noexcept = 0;

    virtual ParametersPtr getRemoteParameters() const noexcept = 0;

    virtual IICETransportPtr createAssociatedTransport(IICETransportDelegatePtr delegate) noexcept(false) = 0; // throws InvalidStateError

    virtual void addRemoteCandidate(const GatherCandidate &remoteCandidate) noexcept(false) = 0; // throws InvalidStateError, InvalidParameters
    virtual void setRemoteCandidates(const CandidateList &remoteCandidates) noexcept(false) = 0; // throws InvalidStateError, InvalidParameters
    virtual void removeRemoteCandidate(const GatherCandidate &remoteCandidate) noexcept(false) = 0; // throws InvalidStateError, InvalidParameters

    virtual void keepWarm(
                          const CandidatePair &candidatePair,
                          bool keepWarm = true
                          ) noexcept(false) = 0; // throws InvalidStateError
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IICETransportDelegate
  //

  interaction IICETransportDelegate
  {
    ZS_DECLARE_TYPEDEF_PTR(IICETransportTypes::CandidatePair, CandidatePair);

    virtual void onICETransportStateChange(
                                           IICETransportPtr transport,
                                           IICETransport::States state
                                           ) = 0;

    virtual void onICETransportCandidatePairAvailable(
                                                      IICETransportPtr transport,
                                                      CandidatePairPtr candidatePair
                                                      ) = 0;
    virtual void onICETransportCandidatePairGone(
                                                 IICETransportPtr transport,
                                                 CandidatePairPtr candidatePair
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
  //
  // IICETransportSubscription
  //

  interaction IICETransportSubscription
  {
    virtual PUID getID() const noexcept = 0;

    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IICETransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransportPtr, IICETransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransport::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransport::CandidatePairPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_METHOD(onICETransportStateChange, IICETransportPtr, States)
ZS_DECLARE_PROXY_METHOD(onICETransportCandidatePairAvailable, IICETransportPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_METHOD(onICETransportCandidatePairGone, IICETransportPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_METHOD(onICETransportCandidatePairChanged, IICETransportPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IICETransportDelegate, ortc::IICETransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransportPtr, IICETransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransport::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransport::CandidatePairPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onICETransportStateChange, IICETransportPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onICETransportCandidatePairAvailable, IICETransportPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onICETransportCandidatePairGone, IICETransportPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onICETransportCandidatePairChanged, IICETransportPtr, CandidatePairPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
