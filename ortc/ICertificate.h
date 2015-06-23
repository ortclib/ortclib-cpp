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

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportTypes
  #pragma mark
  
  interaction ICertificateTypes : public Any
  {
    ZS_DECLARE_STRUCT_PTR(Fingerprint)
    ZS_DECLARE_TYPEDEF_PTR(std::list<Fingerprint>, FingerprintList)
    ZS_DECLARE_TYPEDEF_PTR(String, AlgorithmIdentifier)

    typedef PromiseWith<ICertificate> PromiseWithCertificate;
    ZS_DECLARE_PTR(PromiseWithCertificate)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Fingerprint
    #pragma mark

    struct Fingerprint
    {
      String mAlgorithm;
      String mValue;

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

  interaction ICertificate : public ICertificateTypes
  {
    static ElementPtr toDebug(ICertificatePtr certificate);

    static PromiseWithCertificatePtr generateCertificate(AlgorithmIdentifier algorithm);

    virtual PUID getID() const = 0;

    virtual Time expires() const = 0;

    virtual FingerprintListPtr fingerprints() const = 0;
  };

}
