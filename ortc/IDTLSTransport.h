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
  #pragma mark IDTLSTransport
  #pragma mark
  
  interaction IDTLSTransport
  {
    struct Capabilities;
    struct TransportInfo;

    typedef boost::shared_ptr<Capabilities> CapabilitiesPtr;
    typedef boost::shared_ptr<TransportInfo> TransportInfoPtr;

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ConnectionStates
    #pragma mark
    
    enum ConnectionStates
    {
      ConnectionState_New,
      ConnectionState_Connecting,
      ConnectionState_Connected,
      ConnectionState_ConnectedButTransportDetached,  // either no ICE transport is attached or the ICE transport is haulted
      ConnectionState_Closed,
    };

    static const char *toString(ConnectionStates state);

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
    #pragma mark Capabilities
    #pragma mark

    struct Capabilities
    {
      typedef std::list<Options> OptionsList;

      OptionsList mOptions;

      static CapabilitiesPtr create();
      ElementPtr toDebug() const;

    protected:
      Capabilities() {}
      Capabilities(const Capabilities &) {}
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark TransportInfo
    #pragma mark
    
    struct TransportInfo
    {
      static TransportInfoPtr create();
      ElementPtr toDebug() const;

    protected:
      TransportInfo() {}
      TransportInfo(const TransportInfo &) {}
    };


    static ElementPtr toDebug(IDTLSTransportPtr transport);

    static IDTLSTransportPtr create(
                                    IDTLSTransportDelegatePtr delegate,
                                    IICETransportPtr iceTransport  // = IICETransport() - can be NULL to start disconnected
                                    );

    virtual PUID getID() const = 0;

    virtual IDTLSTransportSubscriptionPtr subscribe(IDTLSTransportDelegatePtr delegate) = 0;

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

    virtual void attach(IICETransportPtr iceTransport) = 0;

    virtual void start(TransportInfoPtr localTransportInfo) = 0;

    virtual void stop() = 0;

    virtual ConnectionStates getState(
                                      WORD *outError = NULL,
                                      String *outReason = NULL
                                      ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportDelegate
  #pragma mark

  interaction IDTLSTransportDelegate
  {
    virtual void onDTLSTransportStateChanged(
                                            IDTLSTransportPtr transport,
                                            IDTLSTransport::ConnectionStates state
                                            ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportSubscription
  #pragma mark

  interaction IDTLSTransportSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IDTLSTransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDTLSTransportPtr, IDTLSTransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDTLSTransport::ConnectionStates, ConnectionStates)
ZS_DECLARE_PROXY_METHOD_2(onDTLSTransportStateChanged, IDTLSTransportPtr, ConnectionStates)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IDTLSTransportDelegate, ortc::IDTLSTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDTLSTransportPtr, IDTLSTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDTLSTransport::ConnectionStates, ConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onDTLSTransportStateChanged, IDTLSTransportPtr, ConnectionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
