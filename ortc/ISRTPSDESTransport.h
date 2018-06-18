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

#include <zsLib/Exception.h>

#include <list>
#include <zsLib/internal/types.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISRTPSDESTransportTypes
  //
  
  interaction ISRTPSDESTransportTypes
  {
    typedef String SessionParam;

    ZS_DECLARE_STRUCT_PTR(Parameters);
    ZS_DECLARE_STRUCT_PTR(CryptoParameters);
    ZS_DECLARE_STRUCT_PTR(KeyParameters);

    ZS_DECLARE_TYPEDEF_PTR(std::list<CryptoParameters>, CryptoParametersList);
    ZS_DECLARE_TYPEDEF_PTR(std::list<KeyParameters>, KeyParametersList);
    ZS_DECLARE_TYPEDEF_PTR(std::list<SessionParam>, SessionParamList);

    //-------------------------------------------------------------------------
    //
    // CryptoParameters
    //

    struct CryptoParameters { // see RFC 4568 sect 9.2
      WORD              mTag {};
      String            mCryptoSuite;
      KeyParametersList mKeyParams;
      SessionParamList  mSessionParams;

      CryptoParameters() noexcept {}
      CryptoParameters(const CryptoParameters &op2) noexcept {(*this) = op2;}
      CryptoParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // Parameters
    //

    struct Parameters {
      CryptoParametersList mCryptoParams;

      Parameters() noexcept {}
      Parameters(const Parameters &op2) noexcept {(*this) = op2;}
      Parameters(ElementPtr rootEl) noexcept;

      static ParametersPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // KeyParameters
    //

    struct KeyParameters {    // see RFC 4568 sect 9.2
      String  mKeyMethod;     // must be "inline"
      String  mKeySalt;       // key + salt, base 64 encoded e.g. base64(16 bytes + 14 bytes)
      String  mLifetime;      // must be "2^n" where n is the max number of packets to flow throw the transport
      String  mMKIValue;      // base 10 expressed value of Master Key Identifier (MKI) converted to string
      WORD    mMKILength {};  // number of bytes allocated on each SRTP packet for Master Key Identifier (MKI) [max = 128 bytes]

      KeyParameters() noexcept {}
      KeyParameters(const KeyParameters &op2) noexcept {(*this) = op2;}
      KeyParameters(ElementPtr elem) noexcept;

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
  // ISRTPSDESTransport
  //

  interaction ISRTPSDESTransport : public ISRTPSDESTransportTypes,
                                   public IRTPTransport,
                                   public IStatsProvider
  {
    static ElementPtr toDebug(ISRTPSDESTransportPtr transport) noexcept;

    static ISRTPSDESTransportPtr convert(IRTPTransportPtr rtpTransport) noexcept;

    static ISRTPSDESTransportPtr create(
                                        ISRTPSDESTransportDelegatePtr delegate,
                                        IICETransportPtr iceTransport,
                                        const CryptoParameters &encryptParameters,
                                        const CryptoParameters &decryptParameters
                                        ) noexcept;

    virtual PUID getID() const noexcept = 0;

    virtual ISRTPSDESTransportSubscriptionPtr subscribe(ISRTPSDESTransportDelegatePtr delegate) noexcept = 0;

    virtual IICETransportPtr transport() const noexcept = 0;
    virtual IICETransportPtr rtcpTransport() const noexcept = 0;

    static ParametersPtr getLocalParameters() noexcept;

    virtual void stop() noexcept = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISRTPSDESTransportDelegate
  //

  interaction ISRTPSDESTransportDelegate
  {
    typedef WORD ErrorCode;

    virtual void onSRTPSDESTransportLifetimeRemaining(
                                                      ISRTPSDESTransportPtr transport,
                                                      ULONG leastLifetimeRemainingPercentageForAllKeys,
                                                      ULONG overallLifetimeRemainingPercentage
                                                      ) = 0;

    virtual void onSRTPSDESTransportError(
                                          ISRTPSDESTransportPtr transport,
                                          ErrorAnyPtr error
                                          ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISRTPSDESTransportSubscription
  //

  interaction ISRTPSDESTransportSubscription
  {
    virtual PUID getID() const noexcept = 0;

    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::ISRTPSDESTransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ISRTPSDESTransportPtr, ISRTPSDESTransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_METHOD(onSRTPSDESTransportLifetimeRemaining, ISRTPSDESTransportPtr, ULONG, ULONG)
ZS_DECLARE_PROXY_METHOD(onSRTPSDESTransportError, ISRTPSDESTransportPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::ISRTPSDESTransportDelegate, ortc::ISRTPSDESTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ISRTPSDESTransportPtr, ISRTPSDESTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onSRTPSDESTransportLifetimeRemaining, ISRTPSDESTransportPtr, ULONG, ULONG)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onSRTPSDESTransportError, ISRTPSDESTransportPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
