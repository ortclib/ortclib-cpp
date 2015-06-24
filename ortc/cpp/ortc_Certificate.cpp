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
    // From RFC 4572.
    const char DIGEST_MD5[]     = "md5";
    const char DIGEST_SHA_1[]   = "sha-1";
    const char DIGEST_SHA_224[] = "sha-224";
    const char DIGEST_SHA_256[] = "sha-256";
    const char DIGEST_SHA_384[] = "sha-384";
    const char DIGEST_SHA_512[] = "sha-512";

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
    #pragma mark ICertificateForDTLSTransport
    #pragma mark

    //-------------------------------------------------------------------------
    SecureByteBlockPtr ICertificateForDTLSTransport::getDigest(
                                                               const String &algorithm,
                                                               CertificateObjectType certificate
                                                               )
    {
      return Certificate::getDigest(algorithm, certificate);
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
      mPromise->setThisWeak(mPromise);
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
      pThis->mThisWeak = pThis;
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
    ICertificateTypes::FingerprintListPtr Certificate::fingerprints(const char *inAlgorithm) const
    {
      String algorithm(inAlgorithm);

      AutoRecursiveLock lock(*this);

      FingerprintListPtr result(make_shared<FingerprintList>());

      if (!mCertificate) {
        ZS_LOG_WARNING(Detail, log("no certificate found"))
        return result;
      }

      const char *algorithms[] = {
        DIGEST_MD5,
        DIGEST_SHA_1,
        DIGEST_SHA_224,
        DIGEST_SHA_256,
        DIGEST_SHA_384,
        DIGEST_SHA_512,
        NULL
      };

      for (auto loop = 0; NULL != algorithms[loop]; ++loop) {

        if (algorithm.hasData()) {
          // filter for a particular fingerprint algorithm only
          if (0 != algorithm.compareNoCase(algorithms[loop])) {
            ZS_LOG_INSANE(log("algorithm not a match") + ZS_PARAM("algorithm", algorithm) + ZS_PARAM("found", algorithms[loop]))
            continue;
          }
        }

        SecureByteBlockPtr buffer = getDigest(algorithms[loop]);
        if (!buffer) continue;

        Fingerprint fingerprint;
        fingerprint.mAlgorithm = algorithms[loop];

        String output = UseServicesHelper::convertToHex(*buffer);

        for (String::size_type pos = 0; pos < output.size(); pos += 2) {
          if (fingerprint.mValue.hasData()) {
            fingerprint.mValue += ":";
          }
          fingerprint.mValue += output.substr(pos, 2);
        }
        result->push_back(fingerprint);
      }

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
    SecureByteBlockPtr Certificate::getDigest(const String &algorithm) const
    {
      return getDigest(algorithm, mCertificate);
    }

    //-------------------------------------------------------------------------
    SecureByteBlockPtr Certificate::getDigest(
                                              const String &algorithm,
                                              CertificateObjectType certificate
                                              )
    {
      if (!certificate) return SecureByteBlockPtr();

      BYTE digest[EVP_MAX_MD_SIZE] {};

      const EVP_MD *md {};
      unsigned int n {};

      if (!Digest::GetDigestEVP(algorithm, &md)) return SecureByteBlockPtr();

      if (sizeof(digest) < static_cast<size_t>(EVP_MD_size(md))) return SecureByteBlockPtr();

      X509_digest(certificate, md, digest, &n);

      ZS_THROW_INVALID_ASSUMPTION_IF(n > sizeof(digest))

      SecureByteBlockPtr buffer(make_shared<SecureByteBlock>(n));

      memcpy(buffer->BytePtr(), digest, n);

      return buffer;
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

      KeyPairType keyPair = NULL;
      CertificateObjectType certificate = NULL;

      // scope: generate keypair outside of a lock
      {
        ZS_LOG_DEBUG(log("generating certificate"))

        keyPair = MakeKey();
        if (!keyPair) {
          ZS_LOG_ERROR(Basic, log("unable to generate key pair"))
          goto generation_done;
        }

        certificate = MakeCertificate(keyPair);
        if (!certificate) {
          ZS_LOG_ERROR(Basic, log("unable to generate certificate"))
          goto generation_done;
        }
      }

    generation_done:
      {
        AutoRecursiveLock lock(*this);

        mKeyPair = keyPair;
        mCertificate = certificate;

        promise = mPromiseWeak.lock();
        pThis = mThisWeak.lock();

        if (mCertificate) {
          ZS_LOG_DEBUG(debug("certificate generated") + toDebug())
        } else {
          ZS_LOG_WARNING(Detail, debug("certificate not generated") + toDebug())
        }

        if (!promise) {
          ZS_LOG_WARNING(Debug, log("promise is gone"))
          return;
        }

        promise->mCertificate.reset();
        goto resolve_promise;
      }

    resolve_promise:
      {
        //.......................................................................
        // WARNING: resolve the promise outside the lock
        //.......................................................................
        if (promise) {
          if ((pThis) &&
              (certificate)) {
            promise->resolve(pThis);
          } else {
            promise->reject();
          }
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
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::Certificate");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "algorithm", mAlgorithm);

      UseServicesHelper::debugAppend(resultEl, "promise", (bool)mPromise);
      UseServicesHelper::debugAppend(resultEl, "promise weak", (bool)mPromiseWeak.lock());

      UseServicesHelper::debugAppend(resultEl, "expires", mExpires);

      UseServicesHelper::debugAppend(resultEl, "certificate", (mCertificate ? true : false));

      return resultEl;
    }

    //-------------------------------------------------------------------------
    void Certificate::cancel()
    {
      //.......................................................................
      // final cleanup

      if (mCertificate) {
        X509_free(mCertificate);
        mCertificate = NULL;
      }

      if (mKeyPair) {
        EVP_PKEY_free(mKeyPair);
        mKeyPair = NULL;
      }

      if (mPromise) {
        mPromise->mCertificate.reset();
      }

      mPromise.reset();
      mPromiseWeak.reset();
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
    #pragma mark Certificate::Digest
    #pragma mark

    //-------------------------------------------------------------------------
    Certificate::Digest::Digest(const String &algorithm) {
      EVP_MD_CTX_init(&ctx_);
      if (GetDigestEVP(algorithm, &md_)) {
        EVP_DigestInit_ex(&ctx_, md_, NULL);
      } else {
        md_ = NULL;
      }
    }

    //-------------------------------------------------------------------------
    Certificate::Digest::~Digest() {
      EVP_MD_CTX_cleanup(&ctx_);
    }

    //-------------------------------------------------------------------------
    size_t Certificate::Digest::Size() const {
      if (!md_) {
        return 0;
      }
      return EVP_MD_size(md_);
    }

    //-------------------------------------------------------------------------
    void Certificate::Digest::Update(const void* buf, size_t len) {
      if (!md_) {
        return;
      }
      EVP_DigestUpdate(&ctx_, buf, len);
    }

    //-------------------------------------------------------------------------
    size_t Certificate::Digest::Finish(void* buf, size_t len) {
      if (!md_ || len < Size()) {
        return 0;
      }
      unsigned int md_len;
      EVP_DigestFinal_ex(&ctx_, static_cast<unsigned char*>(buf), &md_len);
      EVP_DigestInit_ex(&ctx_, md_, NULL);  // prepare for future Update()s
      ZS_THROW_INVALID_ASSUMPTION_IF(md_len != Size())
      return md_len;
    }

    //-------------------------------------------------------------------------
    bool Certificate::Digest::GetDigestEVP(
                                           const String &algorithm,
                                           const EVP_MD** mdp
                                           ) {
      const EVP_MD* md;
      if (algorithm == DIGEST_MD5) {
        md = EVP_md5();
      } else if (algorithm == DIGEST_SHA_1) {
        md = EVP_sha1();
      } else if (algorithm == DIGEST_SHA_224) {
        md = EVP_sha224();
      } else if (algorithm == DIGEST_SHA_256) {
        md = EVP_sha256();
      } else if (algorithm == DIGEST_SHA_384) {
        md = EVP_sha384();
      } else if (algorithm == DIGEST_SHA_512) {
        md = EVP_sha512();
      } else {
        return false;
      }

      // Can't happen
      ZS_THROW_INVALID_ASSUMPTION_IF(EVP_MD_size(md) < 16)
      *mdp = md;
      return true;
    }

    //-------------------------------------------------------------------------
    bool Certificate::Digest::GetDigestName(
                                            const EVP_MD* md,
                                            String* algorithm
                                            ) {
      ZS_THROW_INVALID_ARGUMENT_IF(NULL == md);
      ZS_THROW_INVALID_ARGUMENT_IF(NULL == algorithm);

      int md_type = EVP_MD_type(md);
      if (md_type == NID_md5) {
        *algorithm = DIGEST_MD5;
      } else if (md_type == NID_sha1) {
        *algorithm = DIGEST_SHA_1;
      } else if (md_type == NID_sha224) {
        *algorithm = DIGEST_SHA_224;
      } else if (md_type == NID_sha256) {
        *algorithm = DIGEST_SHA_256;
      } else if (md_type == NID_sha384) {
        *algorithm = DIGEST_SHA_384;
      } else if (md_type == NID_sha512) {
        *algorithm = DIGEST_SHA_512;
      } else {
        algorithm->clear();
        return false;
      }
      
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool Certificate::Digest::GetDigestSize(
                                            const String &algorithm,
                                            size_t* length
                                            )
    {
      const EVP_MD *md;
      if (!GetDigestEVP(algorithm, &md))
        return false;

      *length = EVP_MD_size(md);
      return true;
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
