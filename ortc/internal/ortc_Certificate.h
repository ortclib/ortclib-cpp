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

#include <ortc/ICertificate.h>

#include <zsLib/MessageQueueAssociator.h>

#include <openssl/evp.h>
#include <openssl/x509.h>

#define ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_NAME "ortc/certificate/default-key-name"
#define ORTC_SETTING_CERTIFICATE_DEFAULT_HASH "ortc/certificate/default-hash"
#define ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_NAMED_CURVE "ortc/certificate/default-key-named-curve"
#define ORTC_SETTING_CERTIFICATE_DEFAULT_KEY_LENGTH_IN_BITS "ortc/certificate/default-key-length-in-bits"
#define ORTC_SETTING_CERTIFICATE_DEFAULT_SERIAL_RANDOM_BITS "ortc/certificate/default-salt-length-in-bits"
#define ORTC_SETTING_CERTIFICATE_DEFAULT_PUBLIC_EXPONENT "ortc/certificate/default-public-exponenet"
#define ORTC_SETTING_CERTIFICATE_DEFAULT_LIFETIME_IN_SECONDS  "ortc/certificate/default-lifetime-in-seconds"
#define ORTC_SETTING_CERTIFICATE_DEFAULT_NOT_BEFORE_WINDOW_IN_SECONDS "ortc/certificate/default-not-before-window-in-seconds"

#define ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_INPUT "ortc/certificate/map-algorithm-identifier-input-"
#define ORTC_SETTING_CERTIFICATE_MAP_ALGORITHM_IDENTIFIER_OUTPUT "ortc/certificate/map-algorithm-identifier-output-"

namespace ortc
{
  namespace internal
  {
    extern const char DIGEST_MD5[];
    extern const char DIGEST_SHA_1[];
    extern const char DIGEST_SHA_224[];
    extern const char DIGEST_SHA_256[];
    extern const char DIGEST_SHA_384[];
    extern const char DIGEST_SHA_512[];

    ZS_DECLARE_INTERACTION_PTR(ICertificateForSettings);
    ZS_DECLARE_INTERACTION_PTR(ICertificateForDTLSTransport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICertificateForDTLSTransport
    #pragma mark

    interaction ICertificateForDTLSTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(ICertificateForDTLSTransport, ForDTLSTransport)
      ZS_DECLARE_TYPEDEF_PTR(ICertificateTypes::Fingerprint, Fingerprint)

      static ElementPtr toDebug(ForDTLSTransportPtr certificate);

      typedef EVP_PKEY * KeyPairType;
      typedef X509 * CertificateObjectType; // not sure of type to use

      virtual PUID getID() const = 0;

      virtual KeyPairType getKeyPair() const = 0;
      virtual CertificateObjectType getCertificate() const = 0;

      virtual SecureByteBlockPtr getDigest(const String &algorithm) const = 0;

      virtual FingerprintPtr fingerprint() const = 0;

      static SecureByteBlockPtr getDigest(
                                          const String &algorithm,
                                          CertificateObjectType certificate
                                          );
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Certificate
    #pragma mark
    
    class Certificate : public Noop,
                        public MessageQueueAssociator,
                        public SharedRecursiveLock,
                        public ICertificate,
                        public ICertificateForDTLSTransport,
                        public IWakeDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ICertificate;
      friend interaction ICertificateFactory;
      friend interaction ICertificateForDTLSTransport;

      ZS_DECLARE_STRUCT_PTR(PromiseCertificateHolder);
      ZS_DECLARE_CLASS_PTR(Digest);

      ZS_DECLARE_TYPEDEF_PTR(ICertificateTypes::Fingerprint, Fingerprint);

      ZS_DECLARE_TYPEDEF_PTR(std::list<PromiseWithStatsReportPtr>, PromiseWithStatsReportList);

      typedef EVP_PKEY * KeyPairType;
      typedef X509 * CertificateObjectType; // not sure of type to use

    public:
      Certificate(
                  const make_private &,
                  IMessageQueuePtr queue,
                  ElementPtr keygenAlgorithm
                  );

    protected:
      Certificate(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~Certificate();

      static CertificatePtr convert(ICertificatePtr object);
      static CertificatePtr convert(ForDTLSTransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate => ICertificate
      #pragma mark

      static ElementPtr toDebug(CertificatePtr certificate);

      static PromiseWithCertificatePtr generateCertificate(ElementPtr keygenAlgorithm) throw (NotSupportedError);

      virtual PUID getID() const override {return mID;}

      virtual Time expires() const override;

      virtual FingerprintPtr fingerprint() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate => ICertificateForDTLSTransport
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual KeyPairType getKeyPair() const override;
      virtual CertificateObjectType getCertificate() const override;

      virtual SecureByteBlockPtr getDigest(const String &algorithm) const override;

      // (duplicate) virtual FingerprintPtr fingerprint() const override;

      static SecureByteBlockPtr getDigest(
                                          const String &algorithm,
                                          CertificateObjectType certificate
                                          );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate::PromiseCertificateHolder
      #pragma mark

      struct PromiseCertificateHolder : public PromiseWithCertificate
      {
        PromiseCertificateHolder(IMessageQueuePtr queue = IMessageQueuePtr()) :
          PromiseWithCertificate(Promise::make_private{}, queue)
        {}

        ~PromiseCertificateHolder() {}

        void setThisWeak(PromisePtr promise) {mThisWeak = promise;}

        CertificatePtr mCertificate;
      };

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate::Digest
      #pragma mark

      class Digest
      {
      public:
        Digest(const String &algorithm);
        ~Digest();

        // Returns the digest output size (e.g. 16 bytes for MD5).
        size_t Size() const;

        // Updates the digest with |len| bytes from |buf|.
        void Update(const void* buf, size_t len);

        // Outputs the digest value to |buf| with length |len|.
        size_t Finish(void* buf, size_t len);

        // Helper function to look up a digest's EVP by name.
        static bool GetDigestEVP(
                                 const String &algorithm,
                                 const EVP_MD** md
                                 );

        // Helper function to look up a digest's name by EVP.
        static bool GetDigestName(
                                  const EVP_MD* md,
                                  String* algorithm
                                  );

        // Helper function to get the length of a digest.
        static bool GetDigestSize(
                                  const String &algorithm,
                                  size_t* len
                                  );

      private:
        EVP_MD_CTX ctx_;
        const EVP_MD* md_;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      void cancel();
      bool resolveStatPromises();

      evp_pkey_st* MakeKey();
      X509* MakeCertificate(EVP_PKEY* pkey);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate => (data)
      #pragma mark

      AutoPUID mID;
      CertificateWeakPtr mThisWeak;

      ElementPtr mKeygenAlgorithm;

      PromiseCertificateHolderPtr mPromise;
      PromiseCertificateHolderWeakPtr mPromiseWeak;

      String mName;
      String mNamedCurve;
      String mHash;
      size_t mKeyLength {};
      size_t mRandomBits {};
      String mPublicExponentLength;
      Seconds mLifetime {};
      Seconds mNotBeforeWindow {};

      Time mExpires;

      KeyPairType mKeyPair;
      CertificateObjectType mCertificate;

      mutable PromiseWithStatsReportList mPendingStats;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICertificateFactory
    #pragma mark

    interaction ICertificateFactory
    {
      static ICertificateFactory &singleton();

      ZS_DECLARE_TYPEDEF_PTR(ICertificateTypes::PromiseWithCertificate, PromiseWithCertificate)

      virtual PromiseWithCertificatePtr generateCertificate(ElementPtr keygenAlgorithm) throw (NotSupportedError);;
    };

    class CertificateFactory : public IFactory<ICertificateFactory> {};
  }
}
