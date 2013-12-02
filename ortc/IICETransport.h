/*

 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
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

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport
  #pragma mark
  
  interaction IICETransport
  {
    struct Capabilities;
    struct ServerInfo;
    struct ServerList;
    struct TransportInfo;
    struct CandidateInfo;

    typedef boost::shared_ptr<Capabilities> CapabilitiesPtr;
    typedef boost::shared_ptr<ServerInfo> ServerInfoPtr;
    typedef boost::shared_ptr<ServerList> ServerListPtr;
    typedef boost::shared_ptr<TransportInfo> TransportInfoPtr;
    typedef boost::shared_ptr<CandidateInfo> CandidateInfoPtr;

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ConnectionStates
    #pragma mark
    
    enum ConnectionStates
    {
      ConnectionState_New,
      ConnectionState_Searching,
      ConnectionState_Haulted,
      ConnectionState_Connected,
      ConnectionState_Completed,
      ConnectionState_Closed,
    };
    
    static const char *toString(ConnectionStates state);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Roles
    #pragma mark
    
    enum Roles
    {
      Role_Controlling,
      Role_Controlled,
    };
    
    static const char *toString(Roles role);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Options
    #pragma mark

    enum Options
    {
      Option_Unknown,
    };

    static const char *toString(Options option);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark CandidateTypes
    #pragma mark

    enum CandidateTypes
    {
      CandidateType_Unknown,
      CandidateType_Local,
      CandidateType_ServerReflexive,
      CandidateType_PeerReflexive,
      CandidateType_Relayed,
    };

    static const char *toString(CandidateTypes type) ;

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Capabilities
    #pragma mark

    struct Capabilities
    {
      typedef std::list<Options> OptionsList;

      OptionsList mOptions;

      static CapabilitiesPtr create();
      ElementPtr toDebug() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ServerInfo
    #pragma mark

    struct ServerInfo
    {
      String mURL;
      String mCredential;

      static ServerInfoPtr create();
      ElementPtr toDebug() const;
    };
    
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ServerList
    #pragma mark

    struct ServerList : public std::list<ServerInfoPtr>
    {
      static ServerListPtr create();
      ElementPtr toDebug() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark TransportInfo
    #pragma mark
    
    struct TransportInfo
    {
      String mUsernameFrag;
      String mPassword;

      static TransportInfoPtr create();
      ElementPtr toDebug() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark CandidateInfo
    #pragma mark

    struct CandidateInfo
    {
      String         mFoundation;
      WORD           mComponent;
      String         mTransport;
      DWORD          mPriority;
      IPAddress      mConnectionAddress;

      CandidateTypes mType;
      IPAddress      mRelatedAddress;

      CandidateInfo() :
        mComponent(0),
        mPriority(0),
        mType(CandidateType_Unknown)
      {}

      static CandidateInfoPtr create();
      ElementPtr toDebug() const;
    };


    static ElementPtr toDebug(IICETransportPtr transport);

    static IICETransportPtr create(
                                   IICETransportDelegatePtr delegate,
                                   ServerListPtr servers
                                   );

    virtual PUID getID() const = 0;

    virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate) = 0;

    static CapabilitiesPtr getCapabilities();

    virtual TransportInfoPtr createParams(CapabilitiesPtr capabilities = CapabilitiesPtr()) = 0;

    static TransportInfoPtr filterParams(
                                         TransportInfoPtr params,
                                         CapabilitiesPtr capabilities
                                         );

    virtual TransportInfoPtr getLocal() = 0;
    virtual TransportInfoPtr getRemote() = 0;

    virtual void setLocal(TransportInfoPtr info) = 0;
    virtual void setRemote(TransportInfoPtr info) = 0;

    virtual void start(
                       TransportInfoPtr localTransportInfo,
                       Roles role
                       ) = 0;
    virtual void stop() = 0;

    virtual ConnectionStates getState(
                                      WORD *outError = NULL,
                                      String *outReason = NULL
                                      ) = 0;

    virtual Roles getRole() = 0;

    virtual void addRemoteCandidate(CandidateInfoPtr candidate) = 0;
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
    virtual void onICETransportCandidatesChangeDetected(IICETransportPtr transport) = 0;

    virtual void onICETransportCandidate(
                                         IICETransportPtr transport,
                                         IICETransport::CandidateInfoPtr candidate
                                         ) = 0;

    virtual void onICETransportEndOfCandidates(IICETransportPtr trannsport) = 0;

    virtual void onICETransportActiveCandidate(
                                               IICETransportPtr transport,
                                               IICETransport::CandidateInfoPtr localCandidate,
                                               IICETransport::CandidateInfoPtr remoteCandidate
                                               ) = 0;

    virtual void onICETransportStateChanged(
                                            IICETransportPtr transport,
                                            IICETransport::ConnectionStates state
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
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransport::CandidateInfoPtr, CandidateInfoPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETransport::ConnectionStates, ConnectionStates)
ZS_DECLARE_PROXY_METHOD_1(onICETransportCandidatesChangeDetected, IICETransportPtr)
ZS_DECLARE_PROXY_METHOD_2(onICETransportCandidate, IICETransportPtr, CandidateInfoPtr)
ZS_DECLARE_PROXY_METHOD_1(onICETransportEndOfCandidates, IICETransportPtr)
ZS_DECLARE_PROXY_METHOD_3(onICETransportActiveCandidate, IICETransportPtr, CandidateInfoPtr, CandidateInfoPtr)
ZS_DECLARE_PROXY_METHOD_2(onICETransportStateChanged, IICETransportPtr, ConnectionStates)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IICETransportDelegate, ortc::IICETransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransportPtr, IICETransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransport::CandidateInfoPtr, CandidateInfoPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICETransport::ConnectionStates, ConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onICETransportCandidatesChangeDetected, IICETransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICETransportCandidate, IICETransportPtr, CandidateInfoPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onICETransportEndOfCandidates, IICETransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_3(onICETransportActiveCandidate, IICETransportPtr, CandidateInfoPtr, CandidateInfoPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICETransportStateChanged, IICETransportPtr, ConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
