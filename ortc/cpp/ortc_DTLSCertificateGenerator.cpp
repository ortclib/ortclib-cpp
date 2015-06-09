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

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{

  // Strength of generated keys. Those are RSA.
  static const int KEY_LENGTH = 1024;

  // Random bits for certificate serial number
  static const int SERIAL_RAND_BITS = 64;

  // Certificate validity lifetime
  static const int CERTIFICATE_LIFETIME = 60 * 60 * 24 * 30;  // 30 days, arbitrarily
  // Certificate validity window.
  // This is to compensate for slightly incorrect system clocks.
  static const int CERTIFICATE_WINDOW = -60 * 60 * 24;

  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  // Print a certificate to the log, for debugging.
  static void PrintCert(X509* x509) {
    BIO* temp_memory_bio = BIO_new(BIO_s_mem());
    if (!temp_memory_bio) {
      //LOG_F(LS_ERROR) << "Failed to allocate temporary memory bio";
      return;
    }
    X509_print_ex(temp_memory_bio, x509, XN_FLAG_SEP_CPLUS_SPC, 0);
    BIO_write(temp_memory_bio, "\0", 1);
    char* buffer;
    BIO_get_mem_data(temp_memory_bio, &buffer);
    //LOG(LS_VERBOSE) << buffer;
    BIO_free(temp_memory_bio);
  }

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

    //-------------------------------------------------------------------------
    evp_pkey_st* DTLSCertficateGenerator::MakeKey()
    {
      ZS_LOG_DEBUG(log("Making key pair"))
      evp_pkey_st* pkey = EVP_PKEY_new();
      // RSA_generate_key is deprecated. Use _ex version.
      BIGNUM* exponent = BN_new();
      RSA* rsa = RSA_new();
      if (!pkey || !exponent || !rsa ||
        !BN_set_word(exponent, 0x10001) ||  // 65537 RSA exponent
        !RSA_generate_key_ex(rsa, KEY_LENGTH, exponent, NULL) ||
        !EVP_PKEY_assign_RSA(pkey, rsa)) {
        EVP_PKEY_free(pkey);
        BN_free(exponent);
        RSA_free(rsa);
        return NULL;
      }
      // ownership of rsa struct was assigned, don't free it.
      BN_free(exponent);
      ZS_LOG_DEBUG(log("Returning key pair"))
      return pkey;
    }

    //-------------------------------------------------------------------------
    // Generate a self-signed certificate, with the public key from the
    // given key pair. Caller is responsible for freeing the returned object.
    X509* DTLSCertficateGenerator::MakeCertificate(EVP_PKEY* pkey) 
    {
      ZS_LOG_DEBUG(log("Making certificate"))
      X509* x509 = NULL;
      BIGNUM* serial_number = NULL;
      X509_NAME* name = NULL;

      zsLib::String commonName = UseServicesHelper::randomString(8);

      if ((x509 = X509_new()) == NULL)
        goto error;

      if (!X509_set_pubkey(x509, pkey))
        goto error;

      // serial number
      // temporary reference to serial number inside x509 struct
      ASN1_INTEGER* asn1_serial_number;
      if ((serial_number = BN_new()) == NULL ||
        !BN_pseudo_rand(serial_number, SERIAL_RAND_BITS, 0, 0) ||
        (asn1_serial_number = X509_get_serialNumber(x509)) == NULL ||
        !BN_to_ASN1_INTEGER(serial_number, asn1_serial_number))
        goto error;

      if (!X509_set_version(x509, 0L))  // version 1
        goto error;

      // There are a lot of possible components for the name entries. In
      // our P2P SSL mode however, the certificates are pre-exchanged
      // (through the secure XMPP channel), and so the certificate
      // identification is arbitrary. It can't be empty, so we set some
      // arbitrary common_name. Note that this certificate goes out in
      // clear during SSL negotiation, so there may be a privacy issue in
      // putting anything recognizable here.
      if ((name = X509_NAME_new()) == NULL ||
        !X509_NAME_add_entry_by_NID(
        name, NID_commonName, MBSTRING_UTF8,
        (unsigned char*)commonName.c_str(), -1, -1, 0) ||
        !X509_set_subject_name(x509, name) ||
        !X509_set_issuer_name(x509, name))
        goto error;

      if (!X509_gmtime_adj(X509_get_notBefore(x509), CERTIFICATE_WINDOW) ||
        !X509_gmtime_adj(X509_get_notAfter(x509), CERTIFICATE_LIFETIME))
        goto error;

      if (!X509_sign(x509, pkey, EVP_sha1()))
        goto error;

      BN_free(serial_number);
      X509_NAME_free(name);
      ZS_LOG_DEBUG(log("Returning certificate"))
      return x509;

    error:
      BN_free(serial_number);
      X509_NAME_free(name);
      X509_free(x509);
      return NULL;
    }


  }


}
