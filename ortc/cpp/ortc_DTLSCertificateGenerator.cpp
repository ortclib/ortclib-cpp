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

#include <ortc/internal/ortc_DTLSCertificateGenerator.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    DTLSCertficateGenerator::CertificateHolder::CertificateHolder()
    {
    }

    //-------------------------------------------------------------------------
    DTLSCertficateGenerator::CertificateHolder::~CertificateHolder()
    {
      if (mCertificate) {
#define TODO_FREE_CERTIFICATE_HERE 1
#define TODO_FREE_CERTIFICATE_HERE 2
      }
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSCertficateGenerator
    #pragma mark
    
    //-------------------------------------------------------------------------
    DTLSCertficateGenerator::DTLSCertficateGenerator(
                                                     const make_private &,
                                                     IMessageQueuePtr queue
                                                     ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void DTLSCertficateGenerator::init()
    {
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    DTLSCertficateGenerator::~DTLSCertficateGenerator()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSCertficateGenerator => IDTLSCertficateGenerator
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr DTLSCertficateGenerator::toDebug(DTLSCertficateGeneratorPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    DTLSCertficateGeneratorPtr DTLSCertficateGenerator::create()
    {
      DTLSCertficateGeneratorPtr pThis(make_shared<DTLSCertficateGenerator>(make_private {}, IORTCForInternal::queueCertificateGeneration()));
      pThis->mThisWeak.lock();
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    DTLSCertficateGenerator::PromiseWithCertificatePtr DTLSCertficateGenerator::getCertificate()
    {
      PromiseWithCertificatePtr promise;
      CertificateHolderPtr holder;

      {
        AutoRecursiveLock lock(*this);

        promise = PromiseWithCertificate::create(IORTCForInternal::queueORTC());
        holder = mCertificate;

        if (!holder) {
          // this will resolve later
          mPendingPromises.push_back(promise);
        }
      }

      // resolve immediately
      if (holder) {
        promise->resolve(holder);
      }

      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSCertficateGenerator => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSCertficateGenerator::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      CertificateHolderPtr holder(make_shared<CertificateHolder>());

#define TODO_GENERATE_CERTIFICATE_HERE 1
#define TODO_GENERATE_CERTIFICATE_HERE 2


      // holder->mCertificate = <generated certificate pointer>

      PromiseList pendingPromises;

      // scope: resolve promises
      {
        AutoRecursiveLock lock(*this);

        mCertificate = holder;
        pendingPromises = mPendingPromises;

        mPendingPromises.clear();
      }

      for (auto iter = pendingPromises.begin(); iter != pendingPromises.end(); ++iter) {
        auto promise = (*iter);

        // promise is now resolved
        promise->resolve(holder);
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSCertficateGenerator => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params DTLSCertficateGenerator::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::DTLSCertficateGenerator");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params DTLSCertficateGenerator::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr DTLSCertficateGenerator::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::DTLSCertficateGenerator");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "pending promises", mPendingPromises.size());
      UseServicesHelper::debugAppend(resultEl, "certificate", (bool)mCertificate);

      return resultEl;
    }

  }


}
