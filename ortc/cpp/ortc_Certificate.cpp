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
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/services/IHelper.h>
#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>
#include <zsLib/ISettings.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/Integer.h>
#include <cryptopp/sha.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>

#include <sstream>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);
  ZS_DECLARE_CLASS_PTR(CertificateSettingsDefaults);

  ZS_DECLARE_USING_PTR(zsLib::XML, Document);

  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

  using CryptoPP::Integer;
  using zsLib::Numeric;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(CertificateSettingsDefaults);

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

    //-----------------------------------------------------------------------
    static Log::Params slog(const char *message)
    {
      return Log::Params(message, "ortc::Certificate");
    }

    //-------------------------------------------------------------------------
    static String toStringAlgorithm(ElementPtr keygenAlgorithm)
    {
      if (!keygenAlgorithm) return String();

      DocumentPtr doc = Document::create();

      ElementPtr clonedEl = keygenAlgorithm->clone()->toElement();

      while (clonedEl->hasChildren()) {
        auto firstChild = clonedEl->getFirstChild();
        firstChild->orphan();
        doc->adoptAsLastChild(firstChild);
      }

      auto resultJSON = doc->writeAsJSON();

      String result(resultJSON.get());

      return result;
    }

    //-------------------------------------------------------------------------
    static const char **getHashAlgorithms()
    {
      static const char *algorithms[] = {
        DIGEST_SHA_256, // put default to use first
        DIGEST_MD5,
        DIGEST_SHA_1,
        DIGEST_SHA_224,
        DIGEST_SHA_384,
        DIGEST_SHA_512,
        NULL
      };
      return algorithms;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toAlgorithmElement(const char *inAlgorithmIdentifier)
    {
      String algorithmIdentifier(inAlgorithmIdentifier);
      if (!algorithmIdentifier.isEmpty()) {
        if ('{' == *(algorithmIdentifier.c_str())) {
          // treat as object
          algorithmIdentifier = "{\"keygenAlgorithm\":" + algorithmIdentifier + "}";
          return IHelper::toJSON(algorithmIdentifier);
        }
      }

      for (size_t index = 0; ; ++index) {
        String inputKeyName(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_INPUT);
        inputKeyName += string(index);
        String outputKeyName(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_OUTPUT);
        outputKeyName += string(index);

        String input = ISettings::getString(inputKeyName);
        String output = ISettings::getString(outputKeyName);

        if ((input.isEmpty()) &&
            (output.isEmpty())) {
          goto no_algorithm_specified;
        }

        if (0 != algorithmIdentifier.compareNoCase(input)) continue;

        // found result
        output = "{\"keygenAlgorithm\":" + output + "}";
        return IHelper::toJSON(output);
      }

    no_algorithm_specified: {}

      ZS_LOG_WARNING(Detail, slog("no algorithm identifier mapping found in settings"))
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark CertificateSettingsDefaults
    #pragma mark

    class CertificateSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~CertificateSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static CertificateSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<CertificateSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static CertificateSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<CertificateSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        // which algorithm to use by default
        ISettings::setString(ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_NAME, "RSASSA-PKCS1-v1_5");

        // what named curved to use by default (for eliptical curves only)
        ISettings::setString(ORTC_SETTING_CERTIFICATE_DEFAULT_HASH, "SHA-256");

        // what named curved to use by default (for eliptical curves only)
        ISettings::setString(ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_NAMED_CURVE, "");

        // Strength of generated keys. Those are RSA.
        ISettings::setUInt(ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_LENGTH_IN_BITS, 1024);

        // Random bits for certificate serial number
        ISettings::setUInt(ORTC_SETTING_CERTIFICATE_DEFAULT_SERIAL_RANDOM_BITS, 64);

        ISettings::setString(ORTC_SETTING_CERTIFICATE_DEFAULT_PUBLIC_EXPONENT, "101b");

        // Certificate validity lifetime
        ISettings::setUInt(ORTC_SETTING_CERTIFICATE_DEFAULT_LIFETIME_IN_SECONDS, 60 * 60 * 24 * 30);  // 30 days, arbitrarily

        // Certificate validity window.
        // This is to compensate for slightly incorrect system clocks.
        ISettings::setUInt(ORTC_SETTING_CERTIFICATE_DEFAULT_NOT_BEFORE_WINDOW_IN_SECONDS, 60 * 60 * 24);  // 30 days, arbitrarily

        // various mappings to convert from string to JSON encoded version
        ISettings::setString(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_INPUT "0", "");
        ISettings::setString(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_OUTPUT "0", "{\"name\":\"RSASSA-PKCS1-v1_5\",\"modulusLength\":1024,\"hash\":\"SHA-256\"}");

        ISettings::setString(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_INPUT "1", "RSASSA-PKCS1-v1_5");
        ISettings::setString(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_OUTPUT "1", "{\"name\":\"RSASSA-PKCS1-v1_5\",\"modulusLength\":1024,\"hash\":\"SHA-256\"}");

        auto algorithms = getHashAlgorithms();

        size_t index = 2; // NOTE: must be +1 of the last manually set input/output mapping
        for (size_t loop = 0; NULL != algorithms[loop]; ++loop, ++index) {
          String inputKeyName(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_INPUT);
          inputKeyName += string(index);

          String inputKeyValue = "RSASSA-PKCS1-v1_5|";
          inputKeyValue += algorithms[loop];

          ISettings::setString(inputKeyName, inputKeyValue);

          String outputKeyName(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_OUTPUT);
          outputKeyName += string(index);

          String outputKeyValue("{\"name\":\"RSASSA-PKCS1-v1_5\",\"modulusLength\":1024,\"hash\":\"$HASH$\"}");
          outputKeyValue.replaceAll("$HASH$", algorithms[loop]);

          ISettings::setString(outputKeyName, outputKeyValue);
        }

        for (size_t loop = 0; NULL != algorithms[loop]; ++loop, ++index) {
          String inputKeyName(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_INPUT);
          inputKeyName += string(index);

          String inputKeyValue(algorithms[loop]);

          ISettings::setString(inputKeyName, inputKeyValue);

          String outputKeyName(ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_OUTPUT);
          outputKeyName += string(index);

          String outputKeyValue("{\"name\":\"RSASSA-PKCS1-v1_5\",\"modulusLength\":1024,\"hash\":\"$HASH$\"}");
          outputKeyValue.replaceAll("$HASH$", algorithms[loop]);

          ISettings::setString(outputKeyName, outputKeyValue);
        }
      }
    };

    //-------------------------------------------------------------------------
    void installCertificateSettingsDefaults()
    {
      CertificateSettingsDefaults::singleton();
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
                             ElementPtr keygenAlgorithm
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mKeygenAlgorithm(keygenAlgorithm ? keygenAlgorithm->clone()->toElement() : ElementPtr()),
      mName(ISettings::getString(ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_NAME)),
      mHash(ISettings::getString(ORTC_SETTING_CERTIFICATE_DEFAULT_HASH)),
      mNamedCurve(ISettings::getString(ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_NAMED_CURVE)),
      mKeyLength(ISettings::getUInt(ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_LENGTH_IN_BITS)),
      mRandomBits(ISettings::getUInt(ORTC_SETTING_CERTIFICATE_DEFAULT_SERIAL_RANDOM_BITS)),
      mPublicExponentLength(ISettings::getString(ORTC_SETTING_CERTIFICATE_DEFAULT_PUBLIC_EXPONENT)),
      mLifetime(Seconds(ISettings::getUInt(ORTC_SETTING_CERTIFICATE_DEFAULT_LIFETIME_IN_SECONDS))),
      mNotBeforeWindow(Seconds(ISettings::getUInt(ORTC_SETTING_CERTIFICATE_DEFAULT_NOT_BEFORE_WINDOW_IN_SECONDS))),
      mExpires(zsLib::now() + mLifetime)
    {
      if (mKeygenAlgorithm) {
        {
          ElementPtr nameEl = mKeygenAlgorithm->findFirstChildElement("name");
          if (nameEl) {
            mName = IHelper::getElementTextAndDecode(nameEl);
          }
        }
        {
          ElementPtr hashEl = mKeygenAlgorithm->findFirstChildElement("hash");
          if (hashEl) {
            mHash = IHelper::getElementTextAndDecode(hashEl);
          }
        }
        {
          ElementPtr namedCurveEl = mKeygenAlgorithm->findFirstChildElement("namedCurve");
          if (namedCurveEl) {
            mNamedCurve = IHelper::getElementTextAndDecode(namedCurveEl);
          }
        }

        {
          ElementPtr modulusLengthEl = mKeygenAlgorithm->findFirstChildElement("modulusLength");
          if (modulusLengthEl) {
            String value = IHelper::getElementText(modulusLengthEl);
            try {
              mKeyLength = Numeric<decltype(mKeyLength)>(value);
            } catch(const Numeric<decltype(mKeyLength)>::ValueOutOfRange &) {
              ZS_LOG_ERROR(Detail, log("key length value out of range") + ZS_PARAM("key length", value))
            }
          }
        }

        {
          ElementPtr saltLengthEl = mKeygenAlgorithm->findFirstChildElement("saltLength");
          if (saltLengthEl) {
            String value = IHelper::getElementText(saltLengthEl);
            try {
              mRandomBits = Numeric<decltype(mKeyLength)>(value);
            } catch(const Numeric<decltype(mRandomBits)>::ValueOutOfRange &) {
              ZS_LOG_ERROR(Detail, log("salt length value out of range") + ZS_PARAM("salt length", value))
            }
          }
        }

        {
          ElementPtr publicExponentEl = mKeygenAlgorithm->findFirstChildElement("publicExponent");
          if (publicExponentEl) {
            bool foundMoreThanOne = false;
            String finalPublicExponent;

            while (publicExponentEl) {
              finalPublicExponent += IHelper::getElementText(publicExponentEl);
              publicExponentEl = publicExponentEl->findNextSiblingElement("publicExponent");
              foundMoreThanOne = foundMoreThanOne || ((bool)publicExponentEl);
            }
            if (foundMoreThanOne) {
              if (finalPublicExponent.hasData()) {
                finalPublicExponent += "b"; // append binary suffix
              }
            }

            Integer big(finalPublicExponent);
            if (big.IsZero()) {
              ZS_LOG_ERROR(Detail, log("big integer exponent value failed to convert") + ZS_PARAM("value", finalPublicExponent))
              ZS_THROW_NOT_IMPLEMENTED("Big integer exponent value failed to convert: " + finalPublicExponent);
            }

            mPublicExponentLength = finalPublicExponent;
          }
        }
      } else {
        mKeygenAlgorithm = Element::create("keygenAlgorithm");

        if (mName.hasData()) {
          mKeygenAlgorithm->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("name", mName));
        }
        if (mNamedCurve.hasData()) {
          mKeygenAlgorithm->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("namedCurve", mNamedCurve));
        }
        if (mHash.hasData()) {
          mKeygenAlgorithm->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("hash", mHash));
        }
        if (0 != mKeyLength) {
          mKeygenAlgorithm->adoptAsLastChild(IHelper::createElementWithNumber("modulusLength", string(mKeyLength)));
        }
        if (0 != mRandomBits) {
          mKeygenAlgorithm->adoptAsLastChild(IHelper::createElementWithNumber("saltLength", string(mRandomBits)));
        }

        if (mPublicExponentLength.hasData()) {
          Integer big(mPublicExponentLength);

          // convert to big endian binary array
          auto bits = big.BitCount();
          if (bits > 0) {
            for (auto loop = bits - 1, total = bits; total != 0; --loop, --total) {
              const char *digit = "0";
              if (big.GetBit(loop)) {
                digit = "1";
              }
              mKeygenAlgorithm->adoptAsLastChild(IHelper::createElementWithNumber("publicExponent", digit));
            }
          }
        }
      }

#define TODO_SUPPORT_ELIPTICAL_CURVE 1
#define TODO_SUPPORT_ELIPTICAL_CURVE 2

      ORTC_THROW_NOT_SUPPORTED_ERROR_IF(0 != mName.compareNoCase("RSASSA-PKCS1-v1_5"))
      ORTC_THROW_NOT_SUPPORTED_ERROR_IF(mNamedCurve.hasData())  // not supported at this time (sorry)

      {
        const char **algorithms = getHashAlgorithms();

        bool foundHashAlgorithm = false;

        for (auto loop = 0; NULL != algorithms[loop]; ++loop) {
          if (0 == mHash.compareNoCase(algorithms[loop])) {
            foundHashAlgorithm = true;
            break;
          }
        }

        ORTC_THROW_NOT_SUPPORTED_ERROR_IF(!foundHashAlgorithm)
      }

      ZS_EVENTING_10(
                     x, i, Detail, CertificateCreate, ol, Certificate, Start,
                     puid, id, mID,
                     string, keygenAlgorithm, toStringAlgorithm(mKeygenAlgorithm),
                     string, name, mName,
                     string, namedCurve, mNamedCurve,
                     size_t, keyLength, mKeyLength,
                     size_t, randomBits, mRandomBits,
                     string, publicExponentLength, mPublicExponentLength,
                     duration, lifetime, mLifetime.count(),
                     duration, notBeforeWindow, mNotBeforeWindow.count(),
                     duration, expires, zsLib::timeSinceEpoch<Seconds>(mExpires).count()
                     );
      ZS_LOG_DETAIL(debug("created"));

      ORTC_THROW_INVALID_PARAMETERS_IF(!((bool)mKeygenAlgorithm))  // we do not understand any algorithm at this time
    }

    //-------------------------------------------------------------------------
    void Certificate::init()
    {
      AutoRecursiveLock lock(*this);

      mPromise = make_shared<PromiseCertificateHolder>(IORTCForInternal::queueDelegate());
      mPromise->setThisWeak(mPromise);
      mPromiseWeak = mPromise;
      mPromise->mCertificate = mThisWeak.lock();

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    Certificate::~Certificate()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"));
      mThisWeak.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, CertificateDestroy, ol, Certificate, Stop, puid, id, mID);
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
    ICertificateTypes::PromiseWithCertificatePtr Certificate::generateCertificate(ElementPtr keygenAlgorithm) throw (NotSupportedError)
    {
      CertificatePtr pThis(make_shared<Certificate>(make_private {}, IORTCForInternal::queueCertificateGeneration(), keygenAlgorithm));
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
    ICertificateTypes::FingerprintPtr Certificate::fingerprint() const
    {
      AutoRecursiveLock lock(*this);

      FingerprintPtr result(make_shared<Fingerprint>());

      if (!mCertificate) {
        ZS_LOG_WARNING(Detail, log("no certificate found"))
        return result;
      }

      const char **algorithms = getHashAlgorithms();

      for (auto loop = 0; NULL != algorithms[loop]; ++loop) {

        if (mHash.hasData()) {
          // filter for a particular fingerprint algorithm only
          if (0 != mHash.compareNoCase(algorithms[loop])) {
            ZS_LOG_INSANE(log("algorithm not a match") + ZS_PARAM("hash", mHash) + ZS_PARAM("found", algorithms[loop]))
            continue;
          }
        }

        SecureByteBlockPtr buffer = getDigest(algorithms[loop]);
        if (!buffer) continue;

        Fingerprint fingerprint;
        fingerprint.mAlgorithm = algorithms[loop];
		
        String output = IHelper::convertToHex(*buffer);
        output.toUpper();

        for (String::size_type pos = 0; pos < output.size(); pos += 2) {
          if (fingerprint.mValue.hasData()) {
            fingerprint.mValue += ":";
          }
          fingerprint.mValue += output.substr(pos, 2);
        }
        ZS_EVENTING_3(
                      x, i, Debug, CertificateFingerprint, ol, Certificate, Info,
                      puid, id, mID,
                      string, algorithm, fingerprint.mAlgorithm,
                      string, value, fingerprint.mValue
                      );

		    result->mAlgorithm = fingerprint.mAlgorithm;
		    result->mValue = fingerprint.mValue;

		    return result;
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
    #pragma mark Certificate => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    Certificate::PromiseWithStatsReportPtr Certificate::getStats(const StatsTypeSet &stats) const
    {
      if (!stats.hasStatType(IStatsReportTypes::StatsType_Certificate)) {
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }
      AutoRecursiveLock lock(*this);

      PromiseWithStatsReportPtr promise = PromiseWithStatsReport::create(IORTCForInternal::queueDelegate());
      mPendingStats.push_back(promise);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      return promise;
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
      ZS_LOG_DEBUG(log("wake"));

      if (resolveStatPromises()) return;

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
            ZS_EVENTING_2(
                          x, i, Debug, CertificateGeneratedEvent, ol, Certificate, Event,
                          puid, id, mID,
                          bool, success, true
                          );
            promise->resolve(pThis);
          } else {
            ZS_EVENTING_2(
                          x, i, Debug, CertificateGeneratedEvent, ol, Certificate, Event,
                          puid, id, mID,
                          bool, success, false
                          );
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
      IHelper::debugAppend(objectEl, "id", mID);
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

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "name", mName);
      IHelper::debugAppend(resultEl, "named curve", mNamedCurve);
      IHelper::debugAppend(resultEl, "hash", mHash);
      IHelper::debugAppend(resultEl, "key length", mKeyLength);
      IHelper::debugAppend(resultEl, "random bit", mRandomBits);
      IHelper::debugAppend(resultEl, "lifetime", mLifetime);
      IHelper::debugAppend(resultEl, "not before window", mNotBeforeWindow);

      IHelper::debugAppend(resultEl, "promise", (bool)mPromise);
      IHelper::debugAppend(resultEl, "promise weak", (bool)mPromiseWeak.lock());

      IHelper::debugAppend(resultEl, "expires", mExpires);

      IHelper::debugAppend(resultEl, "certificate", (mCertificate ? true : false));

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
    bool Certificate::resolveStatPromises()
    {
      PromiseWithStatsReportPtr promise;

      {
        AutoRecursiveLock lock(*this);
        if (mPendingStats.size() < 1) return false;

        PromiseWithStatsReportPtr promise = mPendingStats.front();
        mPendingStats.pop_front();
      }

#define TODO_DEFINE_STATS 1
#define TODO_DEFINE_STATS 2

      // no stats at this time
      promise->reject();
      return true;
    }

    //-------------------------------------------------------------------------
    evp_pkey_st* Certificate::MakeKey()
    {
      ZS_LOG_DEBUG(log("Making key pair"))
      // RSA_generate_key is deprecated. Use _ex version.
      BIGNUM* exponent = NULL;
      if (mPublicExponentLength.hasData()) {
        Integer value(mPublicExponentLength);

        std::stringstream output;

        output << value;

        String finalInBase10(output.str());

        int result = BN_dec2bn(&exponent, finalInBase10.c_str());
        if (0 == result) {
          ZS_LOG_ERROR(Detail, log("failed to convert to BN") + ZS_PARAM("input", finalInBase10))
          return NULL;
        }
        ZS_THROW_INVALID_ASSUMPTION_IF(NULL == exponent)
      } else {
       exponent = BN_new();
      }

      evp_pkey_st* pkey = EVP_PKEY_new();
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

      zsLib::String commonName = IHelper::randomString(8);

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
    ICertificateFactory::PromiseWithCertificatePtr ICertificateFactory::generateCertificate(ElementPtr keygenAlgorithm) throw (NotSupportedError)
    {
      if (this) {}
      return internal::Certificate::generateCertificate(keygenAlgorithm);
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
  ICertificateTypes::Fingerprint::Fingerprint(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::ICertificateTypes::Fingerprint", "algorithm", mAlgorithm);
    IHelper::getElementValue(elem, "ortc::ICertificateTypes::Fingerprint", "value", mValue);
  }

  //---------------------------------------------------------------------------
  ElementPtr ICertificateTypes::Fingerprint::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "algorithm", mAlgorithm, false);
    IHelper::adoptElementValue(elem, "value", mValue, false);

    if (!elem->hasChildren()) return ElementPtr();
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr ICertificateTypes::Fingerprint::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICertificateTypes::Fingerprint");

    IHelper::debugAppend(resultEl, "algorithm", mAlgorithm);
    IHelper::debugAppend(resultEl, "value", mValue);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICertificateTypes::Fingerprint::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ICertificateTypes:Fingerprint:");
    hasher->update(mAlgorithm);
    hasher->update(":");
    hasher->update(mValue);

    return hasher->finalizeAsString();
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
  ICertificatePtr ICertificate::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(ICertificate, any);
  }

  //---------------------------------------------------------------------------
  ICertificateTypes::PromiseWithCertificatePtr ICertificate::generateCertificate(ElementPtr algorithm) throw (NotSupportedError)
  {
    return internal::ICertificateFactory::singleton().generateCertificate(algorithm);
  }

  //---------------------------------------------------------------------------
  ICertificateTypes::PromiseWithCertificatePtr ICertificate::generateCertificate(const char *algorithmIdentifier) throw (NotSupportedError)
  {
    ElementPtr algorithmObjectEl = internal::toAlgorithmElement(algorithmIdentifier);
    ORTC_THROW_NOT_SUPPORTED_ERROR_IF(!((bool)algorithmObjectEl))
    return generateCertificate(algorithmObjectEl);
  }

}
