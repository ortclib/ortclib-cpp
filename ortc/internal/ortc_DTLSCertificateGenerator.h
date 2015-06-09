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

#include <ortc/internal/types.h>

#include <openpeer/services/IWakeDelegate.h>

#include <zsLib/MessageQueueAssociator.h>

#include <openssl/evp.h>
#include <openssl/x509.h>

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSCertficateGenerator
    #pragma mark
    
    class DTLSCertficateGenerator : public Noop,
                                    public MessageQueueAssociator,
                                    public SharedRecursiveLock,
                                    public IWakeDelegate
    {
    protected:
      struct make_private {};

    public:
      ZS_DECLARE_STRUCT_PTR(CertificateHolder)

      typedef x509_st * CertificateObjectType; // not sure of type to use

      struct CertificateHolder : public Any
      {
        CertificateHolder();
        ~CertificateHolder();

        CertificateObjectType mCertificate {};
      };

      typedef PromiseWith<CertificateHolder> PromiseWithCertificate;
      ZS_DECLARE_PTR(PromiseWithCertificate)

      typedef std::list<PromiseWithCertificatePtr> PromiseList;

    public:
      DTLSCertficateGenerator(
                              const make_private &,
                              IMessageQueuePtr queue
                              );

    protected:
      DTLSCertficateGenerator(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~DTLSCertficateGenerator();

    public:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSCertficateGenerator
      #pragma mark

      static ElementPtr toDebug(DTLSCertficateGeneratorPtr generator);

      static DTLSCertficateGeneratorPtr create();

      virtual PUID getID() const {return mID;}

      virtual PromiseWithCertificatePtr getCertificate();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSCertficateGenerator => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSCertficateGenerator => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;
      evp_pkey_st* MakeKey();
      X509* MakeCertificate(EVP_PKEY* pkey);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSCertficateGenerator => (data)
      #pragma mark

      AutoPUID mID;
      DTLSCertficateGeneratorWeakPtr mThisWeak;

      PromiseList mPendingPromises;

      CertificateHolderPtr mCertificate;
    };

  }
}
