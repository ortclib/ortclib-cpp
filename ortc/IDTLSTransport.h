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
#include <ortc/IStatsProvider.h>
#include <ortc/ICertificate.h>

#include <zsLib/Exception.h>

#include <list>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportTypes
  #pragma mark
  
  interaction IDTLSTransportTypes
  {
    ZS_DECLARE_STRUCT_PTR(Parameters)
    ZS_DECLARE_TYPEDEF_PTR(std::list<SecureByteBlock>, SecureByteBlockList)
    ZS_DECLARE_TYPEDEF_PTR(ICertificateTypes::FingerprintList, FingerprintList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<ICertificatePtr>, CertificateList)

    typedef PromiseWith<Parameters> PromiseWithParameters;
    ZS_DECLARE_PTR(PromiseWithParameters)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark TransportStates
    #pragma mark
    
    enum States
    {
      State_First,

      State_New         = State_First,
      State_Connecting,
      State_Connected,
      State_Closed,
      State_Failed,

      State_Last        = State_Failed,
    };

    static const char *toString(States state);
    static States toState(const char *state) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Roles
    #pragma mark

    enum Roles
    {
      Role_First,

      Role_Auto     = Role_First,
      Role_Client,
      Role_Server,

      Role_Last     = Role_Server,
    };

    static const char *toString(Roles role);
    static Roles toRole(const char *role) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Roles
    #pragma mark

    struct Parameters : public Any
    {
      Roles mRole {Role_Auto};
      FingerprintList mFingerprints;

      Parameters() {}
      Parameters(const Parameters &op2) {(*this) = op2;}
      Parameters(ElementPtr rootEl);

      static ParametersPtr create(ElementPtr rootEl) { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
      ElementPtr createElement(const char *objectName) const;

      static ParametersPtr convert(AnyPtr any);
      ElementPtr toDebug() const;
      String hash() const;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransport
  #pragma mark

  interaction IDTLSTransport : public IDTLSTransportTypes,
                               public IRTPTransport,
                               public IRTCPTransport,
                               public IStatsProvider
  {
    static ElementPtr toDebug(IDTLSTransportPtr transport);

    static IDTLSTransportPtr convert(IRTPTransportPtr object);
    static IDTLSTransportPtr convert(IRTCPTransportPtr object);

    static IDTLSTransportPtr create(
                                    IDTLSTransportDelegatePtr delegate,
                                    IICETransportPtr iceTransport,
                                    const CertificateList &certificates
                                    );

    virtual PUID getID() const = 0;

    virtual IDTLSTransportSubscriptionPtr subscribe(IDTLSTransportDelegatePtr delegate) = 0;

    virtual CertificateListPtr certificates() const = 0;
    virtual IICETransportPtr transport() const = 0;

    virtual States state() const = 0;

    virtual ParametersPtr getLocalParameters() const = 0;
    virtual ParametersPtr getRemoteParameters() const = 0;

    virtual SecureByteBlockListPtr getRemoteCertificates() const = 0;

    virtual void start(const Parameters &remoteParameters) throw (
                                                                  InvalidStateError,
                                                                  InvalidParameters
                                                                  ) = 0;

    virtual void stop() = 0;
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
    virtual void onDTLSTransportStateChange(
                                            IDTLSTransportPtr transport,
                                            IDTLSTransport::States state
                                            ) = 0;

    virtual void onDTLSTransportError(
                                      IDTLSTransportPtr transport,
                                      ErrorAnyPtr error
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
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDTLSTransport::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_METHOD_2(onDTLSTransportStateChange, IDTLSTransportPtr, States)
ZS_DECLARE_PROXY_METHOD_2(onDTLSTransportError, IDTLSTransportPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IDTLSTransportDelegate, ortc::IDTLSTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDTLSTransportPtr, IDTLSTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDTLSTransport::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onDTLSTransportStateChange, IDTLSTransportPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onDTLSTransportError, IDTLSTransportPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
