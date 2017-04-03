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
  #pragma mark IICEGathererTypes
  #pragma mark
  
  interaction IICEGathererTypes : public IICETypes
  {
    ZS_DECLARE_STRUCT_PTR(Options)
    ZS_DECLARE_STRUCT_PTR(Server)
    ZS_DECLARE_STRUCT_PTR(InterfacePolicy)
    ZS_DECLARE_STRUCT_PTR(ErrorEvent)

    ZS_DECLARE_TYPEDEF_PTR(std::list<String>, StringList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<Server>, ServerList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<InterfacePolicy>, InterfacePolicyList)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererTypes::States
    #pragma mark

    enum States {
      State_First,

      State_New       = State_First,
      State_Gathering,
      State_Complete,
      State_Closed,

      State_Last      = State_Closed,
    };

    static const char *toString(States state);
    static States toState(const char *state) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererTypes::FilterPolicies
    #pragma mark

    enum FilterPolicies {
      FilterPolicy_None                = 0,
      FilterPolicy_NoIPv4Host          = 0x00000001,
      FilterPolicy_NoIPv4Srflx         = 0x00000002,
      FilterPolicy_NoIPv4Prflx         = 0x00000004,
      FilterPolicy_NoIPv4Relay         = 0x00000008,
      FilterPolicy_NoIPv4Private       = 0x00000010,
      FilterPolicy_NoIPv4              = 0x000000FF,
      FilterPolicy_NoIPv6Host          = 0x00000100,
      FilterPolicy_NoIPv6Srflx         = 0x00000200,
      FilterPolicy_NoIPv6Prflx         = 0x00000400,
      FilterPolicy_NoIPv6Relay         = 0x00000800,
      FilterPolicy_NoIPv6Private       = 0x00001000,
      FilterPolicy_NoIPv6Tunnel        = 0x00002000,
      FilterPolicy_NoIPv6Permanent     = 0x00004000,
      FilterPolicy_NoIPv6              = 0x0000FF00,
      FilterPolicy_NoHost              = (FilterPolicy_NoIPv4Host | FilterPolicy_NoIPv6Host),
      FilterPolicy_NoSrflx             = (FilterPolicy_NoIPv4Srflx | FilterPolicy_NoIPv6Srflx),
      FilterPolicy_NoPrflx             = (FilterPolicy_NoIPv4Prflx | FilterPolicy_NoIPv6Prflx),
      FilterPolicy_NoRelay             = (FilterPolicy_NoIPv4Relay | FilterPolicy_NoIPv6Relay),
      FilterPolicy_NoPrivate           = (FilterPolicy_NoIPv4Private | FilterPolicy_NoIPv6Private),
      FilterPolicy_RelayOnly           = (FilterPolicy_NoHost | FilterPolicy_NoSrflx | FilterPolicy_NoPrflx),
      FilterPolicy_NoCandidates        = (0x7FFFFFFF)
    };

    static String toString(FilterPolicies policies);
    static FilterPolicies toPolicy(const char *filters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererTypes::CredentialType
    #pragma mark

    enum CredentialTypes {
      CredentialType_First,

      CredentialType_Password   = CredentialType_First,
      CredentialType_Token,

      CredentialType_Last       = CredentialType_Token,
    };

    static const char *toString(CredentialTypes type);
    static CredentialTypes toCredentialType(const char *type) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererTypes::Options
    #pragma mark

    struct Options {
      bool                mContinuousGathering {true};
      InterfacePolicyList mInterfacePolicies;
      ServerList          mICEServers;

      Options() {}
      Options(const Options &op2) {(*this) = op2;}
      Options(ElementPtr elem);

      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererTypes::Server
    #pragma mark

    struct Server {
      StringList      mURLs;
      String          mUserName;
      String          mCredential;
      CredentialTypes mCredentialType {CredentialType_Password};

      Server() {}
      Server(const Server &op2) {(*this) = op2;}
      Server(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "iceServer") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererTypes::InterfacePolicy
    #pragma mark

    struct InterfacePolicy {
      String          mInterfaceType;
      FilterPolicies  mGatherPolicy {FilterPolicy_None};

      InterfacePolicy() {}
      InterfacePolicy(const InterfacePolicy &op2) {(*this) = op2;}
      InterfacePolicy(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "interfacePolicy") const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererTypes::ErrorEvent
    #pragma mark

    struct ErrorEvent {
      typedef WORD ErrorCode;

      CandidatePtr    mHostCandidate;
      String          mURL;
      ErrorCode       mErrorCode;
      String          mErrorText;

      ErrorEvent() {}
      ErrorEvent(const ErrorEvent &op2) { (*this) = op2; }
      ErrorEvent(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "errorEvent") const;

      ElementPtr toDebug() const;
      String hash() const;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGatherer
  #pragma mark
  
  interaction IICEGatherer : public IICEGathererTypes,
                             public IStatsProvider
  {
    static ElementPtr toDebug(IICEGathererPtr gatherer);

    static IICEGathererPtr create(
                                  IICEGathererDelegatePtr delegate,
                                  const Options &options
                                  );

    virtual PUID getID() const = 0;

    virtual IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate) = 0;

    virtual Components component() const = 0;
    virtual States state() const = 0;

    virtual ParametersPtr getLocalParameters() const = 0;
    virtual CandidateListPtr getLocalCandidates() const = 0;

    virtual IICEGathererPtr createAssociatedGatherer(IICEGathererDelegatePtr delegate) throw(InvalidStateError) = 0;

    virtual void gather(const Optional<Options> &options = Optional<Options>()) = 0;

    virtual void close() = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererDelegate
  #pragma mark

  interaction IICEGathererDelegate
  {
    ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate)
    ZS_DECLARE_TYPEDEF_PTR(IICETypes::CandidateComplete, CandidateComplete)
    ZS_DECLARE_TYPEDEF_PTR(IICEGathererTypes::ErrorEvent, ErrorEvent)

    virtual void onICEGathererStateChange(
                                          IICEGathererPtr gatherer,
                                          IICEGatherer::States state
                                          ) = 0;

    virtual void onICEGathererLocalCandidate(
                                             IICEGathererPtr gatherer,
                                             CandidatePtr candidate
                                             ) = 0;

    virtual void onICEGathererLocalCandidateComplete(
                                                     IICEGathererPtr gatherer,
                                                     CandidateCompletePtr candidate
                                                     ) = 0;

    virtual void onICEGathererLocalCandidateGone(
                                                 IICEGathererPtr gatherer,
                                                 CandidatePtr candidate
                                                 ) = 0;

    virtual void onICEGathererError(
                                     IICEGathererPtr gatherer,
                                     ErrorEventPtr errorEvent
                                     ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererSubscription
  #pragma mark

  interaction IICEGathererSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IICEGathererDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGathererPtr, IICEGathererPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGatherer::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGathererDelegate::CandidatePtr, CandidatePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGathererDelegate::CandidateCompletePtr, CandidateCompletePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGathererTypes::ErrorEventPtr, ErrorEventPtr)
ZS_DECLARE_PROXY_METHOD_2(onICEGathererStateChange, IICEGathererPtr, States)
ZS_DECLARE_PROXY_METHOD_2(onICEGathererLocalCandidate, IICEGathererPtr, CandidatePtr)
ZS_DECLARE_PROXY_METHOD_2(onICEGathererLocalCandidateComplete, IICEGathererPtr, CandidateCompletePtr)
ZS_DECLARE_PROXY_METHOD_2(onICEGathererLocalCandidateGone, IICEGathererPtr, CandidatePtr)
ZS_DECLARE_PROXY_METHOD_2(onICEGathererError, IICEGathererPtr, ErrorEventPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IICEGathererDelegate, ortc::IICEGathererSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGathererPtr, IICEGathererPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGatherer::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGathererDelegate::CandidatePtr, CandidatePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGathererDelegate::CandidateCompletePtr, CandidateCompletePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGathererTypes::ErrorEventPtr, ErrorEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICEGathererStateChange, IICEGathererPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICEGathererLocalCandidate, IICEGathererPtr, CandidatePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICEGathererLocalCandidateComplete, IICEGathererPtr, CandidateCompletePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICEGathererLocalCandidateGone, IICEGathererPtr, CandidatePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICEGathererError, IICEGathererPtr, ErrorEventPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
