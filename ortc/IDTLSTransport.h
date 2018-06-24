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
  //
  // IDTLSTransportTypes
  //
  
  interaction IDTLSTransportTypes
  {
    ZS_DECLARE_STRUCT_PTR(Parameters);
    ZS_DECLARE_TYPEDEF_PTR(std::list<SecureByteBlock>, SecureByteBlockList);
    ZS_DECLARE_TYPEDEF_PTR(ICertificateTypes::FingerprintList, FingerprintList);
    ZS_DECLARE_TYPEDEF_PTR(std::list<ICertificatePtr>, CertificateList);

    typedef PromiseWith<Parameters> PromiseWithParameters;
    ZS_DECLARE_PTR(PromiseWithParameters);

    //-------------------------------------------------------------------------
    //
    // TransportStates
    //
    
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

    static const char *toString(States state) noexcept;
    static States toState(const char *state) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    //
    // Roles
    //

    enum Roles
    {
      Role_First,

      Role_Auto     = Role_First,
      Role_Client,
      Role_Server,

      Role_Last     = Role_Server,
    };

    static const char *toString(Roles role) noexcept;
    static Roles toRole(const char *role) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    //
    // Roles
    //

    struct Parameters : public Any
    {
      Roles mRole {Role_Auto};
      FingerprintList mFingerprints;

      Parameters() noexcept {}
      Parameters(const Parameters &op2) noexcept {(*this) = op2;}
      Parameters(ElementPtr rootEl) noexcept;

      static ParametersPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
      ElementPtr createElement(const char *objectName) const noexcept;

      static ParametersPtr convert(AnyPtr any) noexcept;
      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IDTLSTransport
  //

  interaction IDTLSTransport : public IDTLSTransportTypes,
                               public IRTPTransport,
                               public IRTCPTransport,
                               public IStatsProvider
  {
    static ElementPtr toDebug(IDTLSTransportPtr transport) noexcept;

    static IDTLSTransportPtr convert(IRTPTransportPtr object) noexcept;
    static IDTLSTransportPtr convert(IRTCPTransportPtr object) noexcept;

    static IDTLSTransportPtr create(
                                    IDTLSTransportDelegatePtr delegate,
                                    IICETransportPtr iceTransport,
                                    const CertificateList &certificates
                                    ) noexcept(false); // throws InvalidParameters

    virtual PUID getID() const noexcept = 0;

    virtual IDTLSTransportSubscriptionPtr subscribe(IDTLSTransportDelegatePtr delegate) noexcept = 0;

    virtual CertificateListPtr certificates() const noexcept = 0;
    virtual IICETransportPtr transport() const noexcept = 0;

    virtual States state() const noexcept = 0;

    virtual ParametersPtr getLocalParameters() const noexcept = 0;
    virtual ParametersPtr getRemoteParameters() const noexcept = 0;

    virtual SecureByteBlockListPtr getRemoteCertificates() const noexcept = 0;

    virtual void start(const Parameters &remoteParameters) noexcept(false) = 0; // throws InvalidStateError, InvalidParameters

    virtual void stop() noexcept = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IDTLSTransportDelegate
  //

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
  //
  // IDTLSTransportSubscription
  //

  interaction IDTLSTransportSubscription
  {
    virtual PUID getID() const noexcept = 0;

    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IDTLSTransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDTLSTransportPtr, IDTLSTransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDTLSTransport::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_METHOD(onDTLSTransportStateChange, IDTLSTransportPtr, States)
ZS_DECLARE_PROXY_METHOD(onDTLSTransportError, IDTLSTransportPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IDTLSTransportDelegate, ortc::IDTLSTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDTLSTransportPtr, IDTLSTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDTLSTransport::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onDTLSTransportStateChange, IDTLSTransportPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onDTLSTransportError, IDTLSTransportPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
