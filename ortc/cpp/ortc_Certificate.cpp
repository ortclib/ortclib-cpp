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

#include <ortc/internal/ortc_Certificate.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
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
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    // Strength of generated keys. Those are RSA.
//    static const int KEY_LENGTH = 1024;

    // Random bits for certificate serial number
//    static const int SERIAL_RAND_BITS = 64;

    // Certificate validity lifetime
//    static const int CERTIFICATE_LIFETIME = 60 * 60 * 24 * 30;  // 30 days, arbitrarily
    // Certificate validity window.
    // This is to compensate for slightly incorrect system clocks.
//    static const int CERTIFICATE_WINDOW = -60 * 60 * 24;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void ICertificateForSettings::applyDefaults()
    {

      // Strength of generated keys. Those are RSA.
      UseSettings::setUInt(ORTC_SETTING_CERTIFICATE_KEY_LENGTH_IN_BITS, 1024);

      // Random bits for certificate serial number
      UseSettings::setUInt(ORTC_SETTING_CERTIFICATE_SERIAL_RANDOM_BITS, 64);

      // Certificate validity lifetime
      UseSettings::setUInt(ORTC_SETTING_CERTIFICATE_LIFETIME_IN_SECONDS, 60 * 60 * 24 * 30);  // 30 days, arbitrarily

      // Certificate validity window.
      // This is to compensate for slightly incorrect system clocks.
      UseSettings::setUInt(ORTC_SETTING_CERTIFICATE_NOT_BEFORE_WINDOW_IN_SECONDS, 60 * 60 * 24);  // 30 days, arbitrarily
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICertificateForDTLSTransport::toDebug(ForDTLSTransportPtr certificate)
    {
      if (!certificate) return ElementPtr();
      return (ZS_DYNAMIC_PTR_CAST(Certificate, certificate))->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Certificate
    #pragma mark
    
    //-------------------------------------------------------------------------
    Certificate::Certificate(
                             const make_private &,
                             IMessageQueuePtr queue,
                             AlgorithmIdentifier algorithm
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mAlgorithm(algorithm),
      mKeyLength(UseSettings::getUInt(ORTC_SETTING_CERTIFICATE_KEY_LENGTH_IN_BITS)),
      mRandomBits(UseSettings::getUInt(ORTC_SETTING_CERTIFICATE_SERIAL_RANDOM_BITS)),
      mLifetime(Seconds(UseSettings::getUInt(ORTC_SETTING_CERTIFICATE_LIFETIME_IN_SECONDS))),
      mNotBeforeWindow(Seconds(UseSettings::getUInt(ORTC_SETTING_CERTIFICATE_NOT_BEFORE_WINDOW_IN_SECONDS))),
      mExpires(zsLib::now() + mLifetime)
    {
      ZS_LOG_DETAIL(debug("created"))

      ORTC_THROW_INVALID_PARAMETERS_IF(mAlgorithm.hasData())  // we do not understand any algorithm at this time
    }

    //-------------------------------------------------------------------------
    void Certificate::init()
    {
      AutoRecursiveLock lock(*this);

      mPromise = PromiseCertificateHolderPtr(make_shared<PromiseCertificateHolder>(IORTCForInternal::queueDelegate()));
      mPromiseWeak = mPromise;
      mPromise->mCertificate = mThisWeak.lock();

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    Certificate::~Certificate()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    CertificatePtr Certificate::convert(ICertificatePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(Certificate, object);
    }

    //-------------------------------------------------------------------------
    CertificatePtr Certificate::convert(ForDTLSTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(Certificate, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Certificate => ICertificate
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr Certificate::toDebug(CertificatePtr certificate)
    {
      if (!certificate) return ElementPtr();
      return certificate->toDebug();
    }

    //-------------------------------------------------------------------------
    ICertificateTypes::PromiseWithCertificatePtr Certificate::generateCertificate(AlgorithmIdentifier algorithm)
    {
      CertificatePtr pThis(make_shared<Certificate>(make_private {}, IORTCForInternal::queueCertificateGeneration(), algorithm));
      pThis->mThisWeak.lock();
      pThis->init();

      AutoRecursiveLock lock(*pThis);
      auto promise = pThis->mPromise;
      pThis->mPromise.reset();
      return promise;
    }

    //-------------------------------------------------------------------------
    Time Certificate::expires() const
    {
      return mExpires;
    }

    //-------------------------------------------------------------------------
    ICertificateTypes::FingerprintListPtr Certificate::fingerprints() const
    {
      AutoRecursiveLock lock(*this);

      FingerprintListPtr result(make_shared<FingerprintList>());

      if (!mCertificate) {
        ZS_LOG_WARNING(Detail, log("no certificate found"))
        return result;
      }

      Fingerprint fingerprint;

#define TODO_IMPLEMENT_THIS 1
#define TODO_IMPLEMENT_THIS 2

      //fingerprint.mAlgorithm = ;
      //fingerprint.mValue = ;

      result->push_back(fingerprint);

      return result;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Certificate => ICertificateForDTLSTransport
    #pragma mark

    //-------------------------------------------------------------------------
    Certificate::KeyPairType Certificate::getKeyPair() const
    {
      AutoRecursiveLock lock(*this);
      return mKeyPair;
    }

    //-------------------------------------------------------------------------
    Certificate::CertificateObjectType Certificate::getCertificate() const
    {
      AutoRecursiveLock lock(*this);
      return mCertificate;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Certificate => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void Certificate::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      PromiseCertificateHolderPtr promise;
      CertificatePtr pThis;

      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_DEBUG(debug("generating certificate") + toDebug())

#define TODO_GENERATE_CERTIFICATE_HERE 1
#define TODO_GENERATE_CERTIFICATE_HERE 2

        ZS_LOG_DEBUG(debug("certificate generated") + toDebug())

        promise = mPromiseWeak.lock();
        pThis = mThisWeak.lock();

        if (!promise) {
          ZS_LOG_WARNING(Debug, log("promise is gone"))
          return;
        }

        promise->mCertificate.reset();
      }

      //.......................................................................
      // WARNING: resolve the promise outside the lock
      //.......................................................................
      if (promise) {
        if (pThis) {
          promise->resolve(pThis);
        } else {
          promise->reject();
        }
      }

    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Certificate => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params Certificate::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::Certificate");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params Certificate::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr Certificate::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::Certificate");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "algorithm", mAlgorithm);

      UseServicesHelper::debugAppend(resultEl, "promise", (bool)mPromise);
      UseServicesHelper::debugAppend(resultEl, "promise weak", (bool)mPromiseWeak.lock());

      UseServicesHelper::debugAppend(resultEl, "expires", mExpires);

      UseServicesHelper::debugAppend(resultEl, "certificate", (bool)mCertificate);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    void Certificate::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_CERTIFICATE_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_CERTIFICATE_IS_SHUTDOWN 2
      }

      //.......................................................................
      // final cleanup

      if (mCertificate) {
#define TODO_CLEAN_UP_GENERATED_CERTIFICATE 1
#define TODO_CLEAN_UP_GENERATED_CERTIFICATE 2

        X509_free(mCertificate);
        mCertificate = NULL;
      }

      if (mKeyPair) {
#define TODO_CLEAN_UP_GENERATED_KEY_PAIR 1
#define TODO_CLEAN_UP_GENERATED_KEY_PAIR 2
        EVP_PKEY_free(mKeyPair);
        mKeyPair = NULL;
      }

      if (mPromise) {
        mPromise->mCertificate.reset();
      }

      mPromise.reset();
      mPromiseWeak.reset();

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    evp_pkey_st* Certificate::MakeKey()
    {
      ZS_LOG_DEBUG(log("Making key pair"))
      evp_pkey_st* pkey = EVP_PKEY_new();
      // RSA_generate_key is deprecated. Use _ex version.
      BIGNUM* exponent = BN_new();
      RSA* rsa = RSA_new();
      if (!pkey || !exponent || !rsa ||
          !BN_set_word(exponent, 0x10001) ||  // 65537 RSA exponent
          !RSA_generate_key_ex(rsa, (int)mKeyLength, exponent, NULL) ||
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
    X509* Certificate::MakeCertificate(EVP_PKEY* pkey)
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
          !BN_pseudo_rand(serial_number, (int)mRandomBits, 0, 0) ||
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

      if (!X509_gmtime_adj(X509_get_notBefore(x509), (-1 * ((long)(mNotBeforeWindow.count())))) ||
          !X509_gmtime_adj(X509_get_notAfter(x509), (long)(mLifetime.count())))
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

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICertificateFactory
    #pragma mark

    //-------------------------------------------------------------------------
    ICertificateFactory &ICertificateFactory::singleton()
    {
      return CertificateFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ICertificateFactory::PromiseWithCertificatePtr ICertificateFactory::generateCertificate(AlgorithmIdentifier algorithm)
    {
      if (this) {}
      return internal::Certificate::generateCertificate(algorithm);
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICertificateTypes
  #pragma mark

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICertificateTypes::Fingerprint
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ICertificateTypes::Fingerprint::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICertificateTypes::Fingerprint");

    UseServicesHelper::debugAppend(resultEl, "algorithm", mAlgorithm);
    UseServicesHelper::debugAppend(resultEl, "value", mValue);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICertificateTypes::Fingerprint::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ICertificateTypes:Fingerprint:");
    hasher.update(mAlgorithm);
    hasher.update(":");
    hasher.update(mValue);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICertificate
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ICertificate::toDebug(ICertificatePtr transport)
  {
    return internal::Certificate::toDebug(internal::Certificate::convert(transport));
  }

  //---------------------------------------------------------------------------
  ICertificateTypes::PromiseWithCertificatePtr ICertificate::generateCertificate(AlgorithmIdentifier algorithm)
  {
    return internal::ICertificateFactory::singleton().generateCertificate(algorithm);
  }

}
