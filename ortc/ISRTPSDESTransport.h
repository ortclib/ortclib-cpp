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
  #pragma mark
  #pragma mark ISRTPSDESTransportTypes
  #pragma mark
  
  interaction ISRTPSDESTransportTypes
  {
    typedef String SessionParam;

    ZS_DECLARE_STRUCT_PTR(Parameters)
    ZS_DECLARE_STRUCT_PTR(CryptoParameters)
    ZS_DECLARE_STRUCT_PTR(KeyParameters)

    ZS_DECLARE_TYPEDEF_PTR(std::list<CryptoParameters>, CryptoParametersList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<KeyParameters>, KeyParametersList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<SessionParam>, SessionParamList)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark CryptoParameters
    #pragma mark

    struct CryptoParameters { // see RFC 4568 sect 9.2
      WORD              mTag {};
      String            mCryptoSuite;
      KeyParametersList mKeyParams;
      SessionParamList  mSessionParams;

      CryptoParameters() {}
      CryptoParameters(const CryptoParameters &op2) {(*this) = op2;}
      CryptoParameters(ElementPtr elem);

      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Parameters
    #pragma mark

    struct Parameters {
      CryptoParametersList mCryptoParams;

      Parameters() {}
      Parameters(const Parameters &op2) {(*this) = op2;}
      Parameters(ElementPtr rootEl);

      static ParametersPtr create(ElementPtr rootEl) { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark KeyParameters
    #pragma mark

    struct KeyParameters {    // see RFC 4568 sect 9.2
      String  mKeyMethod;     // must be "inline"
      String  mKeySalt;       // key + salt, base 64 encoded e.g. base64(16 bytes + 14 bytes)
      String  mLifetime;      // must be "2^n" where n is the max number of packets to flow throw the transport
      String  mMKIValue;      // base 10 expressed value of Master Key Identifier (MKI) converted to string
      WORD    mMKILength {};  // number of bytes allocated on each SRTP packet for Master Key Identifier (MKI) [max = 128 bytes]

      KeyParameters() {}
      KeyParameters(const KeyParameters &op2) {(*this) = op2;}
      KeyParameters(ElementPtr elem);

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
  #pragma mark ISRTPSDESTransport
  #pragma mark

  interaction ISRTPSDESTransport : public ISRTPSDESTransportTypes,
                                   public IRTPTransport,
                                   public IStatsProvider
  {
    static ElementPtr toDebug(ISRTPSDESTransportPtr transport);

    static ISRTPSDESTransportPtr convert(IRTPTransportPtr rtpTransport);

    static ISRTPSDESTransportPtr create(
                                        ISRTPSDESTransportDelegatePtr delegate,
                                        IICETransportPtr iceTransport,
                                        const CryptoParameters &encryptParameters,
                                        const CryptoParameters &decryptParameters
                                        );

    virtual PUID getID() const = 0;

    virtual ISRTPSDESTransportSubscriptionPtr subscribe(ISRTPSDESTransportDelegatePtr delegate) = 0;

    virtual IICETransportPtr transport() const = 0;
    virtual IICETransportPtr rtcpTransport() const = 0;

    static ParametersPtr getLocalParameters();

    virtual void stop() = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISRTPSDESTransportDelegate
  #pragma mark

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
  #pragma mark
  #pragma mark ISRTPSDESTransportSubscription
  #pragma mark

  interaction ISRTPSDESTransportSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::ISRTPSDESTransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ISRTPSDESTransportPtr, ISRTPSDESTransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_METHOD_3(onSRTPSDESTransportLifetimeRemaining, ISRTPSDESTransportPtr, ULONG, ULONG)
ZS_DECLARE_PROXY_METHOD_2(onSRTPSDESTransportError, ISRTPSDESTransportPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::ISRTPSDESTransportDelegate, ortc::ISRTPSDESTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ISRTPSDESTransportPtr, ISRTPSDESTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_3(onSRTPSDESTransportLifetimeRemaining, ISRTPSDESTransportPtr, ULONG, ULONG)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onSRTPSDESTransportError, ISRTPSDESTransportPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
