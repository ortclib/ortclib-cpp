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

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IIdentityTypes
  #pragma mark
  
  interaction IIdentityTypes
  {
    ZS_DECLARE_STRUCT_PTR(Assertion)
    ZS_DECLARE_STRUCT_PTR(Result)
    ZS_DECLARE_STRUCT_PTR(Error)

    typedef PromiseWith<Result, Error> PromiseWithResult;
    ZS_DECLARE_PTR(PromiseWithResult)

    ZS_DECLARE_TYPEDEF_PTR(PromiseWith<Assertion>, PromiseWithAssertion)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IIdentityTypes::Assertion
    #pragma mark

    struct Assertion {
      String        mIDP;
      String        mName;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IIdentityTypes::Result
    #pragma mark

    struct Result : public Any {
      String        mAssertion;

      static ResultPtr convert(AnyPtr any);
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IIdentityTypes::Error
    #pragma mark

    struct Error : public Any {
      String        mIDP;
      String        mProtocol;
      String        mLoginURL;

      static ErrorPtr convert(AnyPtr any);
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IIdentity
  #pragma mark
  
  interaction IIdentity : public IIdentityTypes
  {
    static ElementPtr toDebug(IIdentityPtr channel);

    static IIdentityPtr create(IDTLSTransportPtr transport);

    virtual PUID getID() const = 0;

    virtual AssertionPtr peerIdentity() const = 0;

    virtual IDTLSTransportPtr transport() const = 0;

    virtual PromiseWithResultPtr getIdentityAssertion(
                                                      const char *provider,
                                                      const char *protocol = "default",
                                                      const char *username = NULL
                                                      ) throw (InvalidStateError) = 0;

    virtual PromiseWithAssertionPtr setIdentityAssertion(const String &assertion) = 0;
  };
}
