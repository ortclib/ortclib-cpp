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
#include <ortc/IDataTransport.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportTypes
  #pragma mark
  
  interaction ISCTPTransportTypes
  {
    ZS_DECLARE_STRUCT_PTR(Capabilities)

    enum States
    {
      State_First,

      State_New           = State_First,
      State_Connecting,
      State_Connected,
      State_Closed,

      State_Last          = State_Closed,
    };

    static const char *toString(States state);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportTypes::Capabilities
    #pragma mark

    struct Capabilities {
      size_t mMaxMessageSize {};
      WORD mMinPort {};
      WORD mMaxPort {};
      WORD mMaxUsablePorts {};
      WORD mMaxSessionsPerPort {};

      Capabilities() {}
      Capabilities(const Capabilities &op2) {(*this) = op2;}
      Capabilities(ElementPtr rootEl);

      static CapabilitiesPtr create(ElementPtr rootEl) { if (!rootEl) return CapabilitiesPtr(); return make_shared<Capabilities>(rootEl); }
      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransport
  #pragma mark
  
  interaction ISCTPTransport : public ISCTPTransportTypes,
                               public IDataTransport
  {
    static ElementPtr toDebug(ISCTPTransportPtr transport);

    static ISCTPTransportPtr convert(IDataTransportPtr object);

    static ISCTPTransportPtr create(
                                    ISCTPTransportDelegatePtr delegate,
                                    IDTLSTransportPtr transport,
                                    WORD localPort = 0    // 0 = port automatically chosen
                                    ) throw (InvalidParameters, InvalidStateError);

    static ISCTPTransportListenerSubscriptionPtr listen(
                                                        ISCTPTransportListenerDelegatePtr delegate,
                                                        IDTLSTransportPtr transport,
                                                        const Capabilities &remoteCapabilities
                                                        );

    virtual PUID getID() const = 0;

    static CapabilitiesPtr getCapabilities();

    virtual IDTLSTransportPtr transport() const = 0;
    virtual States state() const = 0;

    virtual WORD port() const = 0;

    virtual WORD localPort() const = 0;
    virtual Optional<WORD> remotePort() const = 0;

    virtual void start(
                       const Capabilities &remoteCapabilities,
                       WORD remotePort = 0
                       ) throw (InvalidStateError, InvalidParameters) = 0;
    virtual void stop() = 0;

    virtual ISCTPTransportSubscriptionPtr subscribe(ISCTPTransportDelegatePtr delegate) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportDelegate
  #pragma mark

  interaction ISCTPTransportDelegate
  {
    typedef ISCTPTransportTypes::States States;

    virtual void onSCTPTransportStateChange(
                                            ISCTPTransportPtr transport,
                                            States state
                                            ) = 0;
    virtual void onSCTPTransportDataChannel(
                                            ISCTPTransportPtr transport,
                                            IDataChannelPtr channel
                                            ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportSubscription
  #pragma mark

  interaction ISCTPTransportSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportListenerDelegate
  #pragma mark

  interaction ISCTPTransportListenerDelegate
  {
    virtual void onSCTPTransport(ISCTPTransportPtr transport) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportListenerSubscription
  #pragma mark

  interaction ISCTPTransportListenerSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };

}


ZS_DECLARE_PROXY_BEGIN(ortc::ISCTPTransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ISCTPTransportPtr, ISCTPTransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDataChannelPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_METHOD_2(onSCTPTransportStateChange, ISCTPTransportPtr, States)
ZS_DECLARE_PROXY_METHOD_2(onSCTPTransportDataChannel, ISCTPTransportPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::ISCTPTransportDelegate, ortc::ISCTPTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ISCTPTransportPtr, ISCTPTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDataChannelPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onSCTPTransportStateChange, ISCTPTransportPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onSCTPTransportDataChannel, ISCTPTransportPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

ZS_DECLARE_PROXY_BEGIN(ortc::ISCTPTransportListenerDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ISCTPTransportPtr, ISCTPTransportPtr)
ZS_DECLARE_PROXY_METHOD_1(onSCTPTransport, ISCTPTransportPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::ISCTPTransportListenerDelegate, ortc::ISCTPTransportListenerSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ISCTPTransportPtr, ISCTPTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onSCTPTransport, ISCTPTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
