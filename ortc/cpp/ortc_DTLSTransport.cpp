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

#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_Certificate.h>
#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_SRTPTransport.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/ISRTPSDESTransport.h>
#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/SafeInt.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/tls1.h>
#include <openssl/x509v3.h>

//#include <webrtc/base/safe_conversions.h>
//#include <webrtc/base/common.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_dtlstransport) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

#ifndef RTC_UNUSED
#define RTC_UNUSED(x) RtcUnused(static_cast<const void*>(&x))
#endif //RTC_UNUSED

  using zsLib::Numeric;
  using zsLib::Log;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(DTLSTransportSettingsDefaults);

    typedef DTLSTransport::StreamResult StreamResult;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    const int SRTP_MASTER_KEY_KEY_LEN = 16;
    const int SRTP_MASTER_KEY_SALT_LEN = 14;

    // RFC 5705 exporter using the RFC 5764 parameters
    static const char kDtlsSrtpExporterLabel[] = "EXTRACTOR-dtls_srtp";

    // We don't pull the RTP constants from rtputils.h, to avoid a layer violation.
    static const size_t kDtlsRecordHeaderLen = 13;
    static const size_t kMaxDtlsPacketLen = 2048;
    static const size_t kMinRtpPacketLen = 12;

    // Maximum number of pending packets in the queue. Packets are read immediately
    // after they have been written, so a capacity of "1" is sufficient.
    static const size_t kMaxPendingPackets = 1;

    static bool isDtlsPacket(const BYTE *data, size_t len) {
      return (len >= kDtlsRecordHeaderLen && (data[0] > 19 && data[0] < 64));
    }
    static bool isRtpPacket(const BYTE *data, size_t len) {
      return (len >= kMinRtpPacketLen && (data[0] & 0xC0) == 0x80);
    }


#if (OPENSSL_VERSION_NUMBER >= 0x10001000L)
#define HAVE_DTLS_SRTP
#endif

    // SRTP cipher suite table
    struct SrtpCipherMapEntry {
      const char* external_name;
      const char* internal_name;
    };

    // This isn't elegant, but it's better than an external reference
    static SrtpCipherMapEntry SrtpCipherMap[] = {
      {"AES_CM_128_HMAC_SHA1_80", "SRTP_AES128_CM_SHA1_80"},
      {"AES_CM_128_HMAC_SHA1_32", "SRTP_AES128_CM_SHA1_32"},
      {NULL, NULL}
    };

    // Default cipher used between OpenSSL/BoringSSL stream adapters.
    // This needs to be updated when the default of the SSL library changes.
    static const char kDefaultSslCipher10[] =
    "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA";

    static const char kDefaultSslCipher12[] =
    "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256";
    // Fallback cipher for DTLS 1.2 if hardware-accelerated AES-GCM is unavailable.
    static const char kDefaultSslCipher12NoAesGcm[] =
    "TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256";

    //////////////////////////////////////////////////////////////////////
    // StreamBIO
    //////////////////////////////////////////////////////////////////////

    static int stream_write(BIO* h, const char* buf, int num);
    static int stream_read(BIO* h, char* buf, int size);
    static int stream_puts(BIO* h, const char* str);
    static long stream_ctrl(BIO* h, int cmd, long arg1, void* arg2);
    static int stream_new(BIO* h);
    static int stream_free(BIO* data);
    
    // TODO(davidben): This should be const once BoringSSL is assumed.
    static BIO_METHOD methods_stream = {
      BIO_TYPE_BIO,
      "stream",
      stream_write,
      stream_read,
      stream_puts,
      0,
      stream_ctrl,
      stream_new,
      stream_free,
      NULL,
    };

    //-------------------------------------------------------------------------
    static BIO_METHOD* BIO_s_stream() { return(&methods_stream); }

    //-------------------------------------------------------------------------
    static BIO* BIO_new_stream(DTLSTransportPtr transport) {
      BIO* ret = BIO_new(BIO_s_stream());
      if (ret == NULL)
        return NULL;

      DTLSTransportWeakPtr *weakTransport(new DTLSTransportWeakPtr(transport));
      ret->ptr = weakTransport;
      return ret;
    }

    // bio methods return 1 (or at least non-zero) on success and 0 on failure.

    //-------------------------------------------------------------------------
    static int stream_new(BIO* b) {
      b->shutdown = 0;
      b->init = 1;
      b->num = 0;  // 1 means end-of-stream
      b->ptr = 0;
      return 1;
    }

    //-------------------------------------------------------------------------
    static int stream_free(BIO* b) {
      if (b == NULL)
        return 0;

      DTLSTransportWeakPtr *weakTransport = static_cast<DTLSTransportWeakPtr *>(b->ptr);
      if (weakTransport) {
        delete weakTransport;
        b->ptr = NULL;
      }

      return 1;
    }

    //-------------------------------------------------------------------------
    static int stream_read(BIO* b, char* out, int outl) {
      if (!out)
        return -1;

      BIO_clear_retry_flags(b);

      DTLSTransportWeakPtr *weakTransport = static_cast<DTLSTransportWeakPtr *>(b->ptr);
      if (!weakTransport) return -1;

      DTLSTransportPtr transport = weakTransport->lock();
      if (!transport) return -1;

      size_t read {};
      int error {};

      StreamResult result = transport->bioRead(out, outl, &read, &error);
      if (result == DTLSTransport::SR_SUCCESS) {
        return SafeInt<decltype(read)>(read);
      } else if (result == DTLSTransport::SR_EOS) {
        b->num = 1;
      } else if (result == DTLSTransport::SR_BLOCK) {
        BIO_set_retry_read(b);
      }
      return -1;
    }

    //-------------------------------------------------------------------------
    static int stream_write(BIO* b, const char* in, int inl) {
      if (!in)
        return -1;

      BIO_clear_retry_flags(b);

      DTLSTransportWeakPtr *weakTransport = static_cast<DTLSTransportWeakPtr *>(b->ptr);
      if (!weakTransport) return -1;

      DTLSTransportPtr transport = weakTransport->lock();
      if (!transport) return -1;

      size_t written;
      int error;
      StreamResult result = transport->bioWrite(in, inl, &written, &error);
      if (result == DTLSTransport::SR_SUCCESS) {
        return SafeInt<decltype(written)>(written);
      } else if (result == DTLSTransport::SR_BLOCK) {
        BIO_set_retry_write(b);
      }
      return -1;
    }

    //-------------------------------------------------------------------------
    static int stream_puts(BIO* b, const char* str) {
      return stream_write(b, str, SafeInt<size_t>(strlen(str)));
    }

    //-------------------------------------------------------------------------
    static long stream_ctrl(BIO* b, int cmd, long num, void* ptr) {
      //RTC_UNUSED(num);
     // RTC_UNUSED(ptr);

      switch (cmd) {
        case BIO_CTRL_RESET:
          return 0;
        case BIO_CTRL_EOF:
          return b->num;
        case BIO_CTRL_WPENDING:
        case BIO_CTRL_PENDING:
          return 0;
        case BIO_CTRL_FLUSH:
          return 1;
        case BIO_CTRL_DGRAM_QUERY_MTU:
          // openssl defaults to mtu=256 unless we return something here.
          // The handshake doesn't actually need to send packets above 1k,
          // so this seems like a sensible value that should work in most cases.
          // Webrtc uses the same value for video packets.
          return 1200;
        default:
          return 0;
      }
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransportSettingsDefaults
    #pragma mark

    class DTLSTransportSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~DTLSTransportSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static DTLSTransportSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<DTLSTransportSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static DTLSTransportSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<DTLSTransportSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        ISettings::setUInt(ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_DTLS_BUFFER, kMaxDtlsPacketLen * 4);

        ISettings::setUInt(ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_RTP_PACKETS, 50);
      }
      
    };

    //-------------------------------------------------------------------------
    void installDTLSTransportSettingsDefaults()
    {
      DTLSTransportSettingsDefaults::singleton();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr DTLSTransport::getStats(const StatsTypeSet &stats) const
    {
      if (!stats.hasStatType(IStatsReportTypes::StatsType_DTLSTransport)) {
        return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
      }
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    DTLSTransport::DTLSTransport(
                                 const make_private &,
                                 IMessageQueuePtr queue,
                                 IDTLSTransportDelegatePtr originalDelegate,
                                 IICETransportPtr iceTransport,
                                 const CertificateList &certificates
                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mICETransport(ICETransport::convert(iceTransport)),
      mComponent(mICETransport->component()),
      mMaxPendingDTLSBuffer(ISettings::getUInt(ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_DTLS_BUFFER)),
      mMaxPendingRTPPackets(ISettings::getUInt(ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_RTP_PACKETS))
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!mICETransport);

      ZS_EVENTING_6(
                    x, i, Detail, DtlsTransportCreate, ol, DtlsTransport, Start,
                    puid, id, mID,
                    puid, iceTransportId, mICETransport->getID(),
                    string, component, IICETypes::toString(mComponent),
                    size_t, maxPendingDtlsBuffer, mMaxPendingDTLSBuffer,
                    size_t, maxPendingRtpPackets, mMaxPendingRTPPackets,
                    size_t, certificates, certificates.size()
                    );

      ZS_LOG_DETAIL(debug("created"));

      ORTC_THROW_INVALID_PARAMETERS_IF(certificates.size() <  1)

      for (auto iter = certificates.begin(); iter != certificates.end(); ++iter) {
        auto &cert = (*iter);
        ORTC_THROW_INVALID_PARAMETERS_IF(!((bool)cert))

        mCertificates.push_back(Certificate::convert(cert));
      }

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::init()
    {
      UseICETransportPtr transport;

      // scope: setup generator
      {
        AutoRecursiveLock lock(*this);

        switch (mComponent) {
          case IICETypes::Component_RTP: {
            mRTPListener = UseRTPListener::create(mThisWeak.lock());
            break;
          }
          case IICETypes::Component_RTCP: {
            UseICETransportPtr rtpIceTransport = mICETransport->getRTPTransport();
            ORTC_THROW_INVALID_STATE_IF(!rtpIceTransport)

            auto useSecureTransport = DTLSTransport::convert(rtpIceTransport->getSecureTransport());
            ORTC_THROW_INVALID_STATE_IF(!useSecureTransport)

            mRTPListener = useSecureTransport->getListener();
            break;
          }
        }

        mDataTransport = UseDataTransport::create(mThisWeak.lock());

        mAdapter = make_shared<Adapter>(mThisWeak.lock());
        mAdapter->setIdentity(mCertificates.front());

        std::vector<String> ciphers;
        for (SrtpCipherMapEntry *entry = SrtpCipherMap; entry->internal_name; ++entry) {
          ZS_EVENTING_2(
                        x, i, Detail, DtlsTransportInitializationInstallCipher, ol, DtlsTransport, Initialization,
                        puid, id, mID,
                        string, srtpCipherMapEntry, entry->external_name
                        );

          ciphers.push_back(entry->external_name);
        }

        mAdapter->setDtlsSrtpCiphers(ciphers);

        transport = mICETransport;

        mICETransportSubscription = transport->subscribe(mThisWeak.lock());

        FingerprintList fingerprints;

        for (auto iter = mCertificates.begin(); iter != mCertificates.end(); ++iter)
        {
          auto &cert = (*iter);
          auto fingerprint = cert->fingerprint();
          if (!fingerprint) continue;

          ZS_EVENTING_4(
                        x, i, Detail, DtlsTransportInitializationInstallFingerprint, ol, DtlsTransport, Initialization,
                        puid, id, mID,
                        string, algorithm, fingerprint->mAlgorithm,
                        string, fingerprint, fingerprint->mValue,
                        bool, isLocal, true
                        );

          fingerprints.push_back(*fingerprint);
        }

        mLocalParams.mFingerprints = fingerprints;

        ZS_EVENTING_5(
                      x, i, Detail, DtlsTransportInitialization, ol, DtlsTransport, Initialization,
                      puid, id, mID,
                      puid, iceTransportId, transport->getID(),
                      puid, rtpListenerId, mRTPListener->getID(),
                      size_t, ciphers, ciphers.size(),
                      size_t, fingerprints, fingerprints.size()
                      );
      }

      transport->notifyAttached(mID, mThisWeak.lock());

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    DTLSTransport::~DTLSTransport()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, DtlsTransportDestroy, ol, DtlsTransport, Stop, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(IDTLSTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTLSTransport, object);
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(ForDataTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTLSTransport, object);
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(ForRTPSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTLSTransport, object);
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(ForRTPReceiverPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTLSTransport, object);
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(ForICETransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTLSTransport, object);
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(ForSRTPPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTLSTransport, object);
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(ForRTPListenerPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTLSTransport, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IDTLSTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr DTLSTransport::toDebug(DTLSTransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::create(
                                           IDTLSTransportDelegatePtr delegate,
                                           IICETransportPtr iceTransport,
                                           const CertificateList &certificates
                                           )
    {
      DTLSTransportPtr pThis(make_shared<DTLSTransport>(make_private {}, IORTCForInternal::queueORTC(), delegate, iceTransport, certificates));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(IRTPTransportPtr rtpTransport)
    {
      auto transport = ZS_DYNAMIC_PTR_CAST(DTLSTransport, rtpTransport);
      if (!transport) return transport;

      auto component = transport->component();
      if (IICETypes::Component_RTP != component) {
        ZS_LOG_WARNING(Detail, transport->log("component not RTP") + ZS_PARAM("component", IICETypes::toString(component)))
        return DTLSTransportPtr();
      }
      return transport;
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr DTLSTransport::convert(IRTCPTransportPtr rtcpTransport)
    {
      auto transport = ZS_DYNAMIC_PTR_CAST(DTLSTransport, rtcpTransport);
      if (!transport) return transport;

      auto component = transport->component();
      if (IICETypes::Component_RTCP != component) {
        ZS_LOG_WARNING(Detail, transport->log("component not RTCP") + ZS_PARAM("component", IICETypes::toString(component)))
        return DTLSTransportPtr();
      }
      return transport;
    }

    //-------------------------------------------------------------------------
    IDTLSTransportSubscriptionPtr DTLSTransport::subscribe(IDTLSTransportDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IDTLSTransportSubscriptionPtr subscription = mSubscriptions.subscribe(IDTLSTransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));

      IDTLSTransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        DTLSTransportPtr pThis = mThisWeak.lock();

        if (IDTLSTransportTypes::State_New != mCurrentState) {
          delegate->onDTLSTransportStateChange(pThis, mCurrentState);
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    DTLSTransport::CertificateListPtr DTLSTransport::certificates() const
    {
      AutoRecursiveLock lock(*this);
      CertificateListPtr result(make_shared<CertificateList>());

      for (auto iter = mCertificates.begin(); iter != mCertificates.end(); ++iter) {
        auto &cert = (*iter);

        result->push_back(Certificate::convert(cert));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    IICETransportPtr DTLSTransport::transport() const
    {
      AutoRecursiveLock lock(*this);
      return ICETransport::convert(mICETransport);
    }

    //-------------------------------------------------------------------------
    IDTLSTransportTypes::States DTLSTransport::state() const
    {
      AutoRecursiveLock lock(*this);
      return mCurrentState;
    }

    //-------------------------------------------------------------------------
    IDTLSTransportTypes::ParametersPtr DTLSTransport::getLocalParameters() const
    {
      AutoRecursiveLock lock(*this);

      ParametersPtr result(make_shared<Parameters>(mLocalParams));
      return result;
    }

    //-------------------------------------------------------------------------
    IDTLSTransportTypes::ParametersPtr DTLSTransport::getRemoteParameters() const
    {
      AutoRecursiveLock lock(*this);

      ParametersPtr result(make_shared<Parameters>(mRemoteParams));
      return result;
    }

    //-------------------------------------------------------------------------
    IDTLSTransportTypes::SecureByteBlockListPtr DTLSTransport::getRemoteCertificates() const
    {
      AutoRecursiveLock lock(*this);
#define TODO_COMPLETE_ONCE_ORTC_SPEC_DEFINES_THIS_PROPERLY 1
#define TODO_COMPLETE_ONCE_ORTC_SPEC_DEFINES_THIS_PROPERLY 2
      return SecureByteBlockListPtr();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::start(const Parameters &remoteParameters) throw (
                                                                         InvalidStateError,
                                                                         InvalidParameters
                                                                         )
    {
      ZS_EVENTING_3(
                    x, i, Detail, DtlsTransportStart, ol, DtlsTransport, Start,
                    puid, id, mID,
                    string, role, IDTLSTransportTypes::toString(remoteParameters.mRole),
                    size_t, remoteFingerprints, remoteParameters.mFingerprints.size()
                    );

      AutoRecursiveLock lock(*this);

      if ((isShuttingDown()) ||
          (isShutdown())) {
#define WARNING_WHAT_IF_DTLS_ERROR_CAUSED_CLOSE 1
#define WARNING_WHAT_IF_DTLS_ERROR_CAUSED_CLOSE 2
        ORTC_THROW_INVALID_STATE("already shutting down")
      }

      ORTC_THROW_INVALID_STATE_IF(mRemoteParams.mFingerprints.size() > 0)
      ORTC_THROW_INVALID_PARAMETERS_IF(remoteParameters.mFingerprints.size() < 1)

      mRemoteParams = remoteParameters;

      if (!mFixedRole) {
        switch (mRemoteParams.mRole) {
          case IDTLSTransportTypes::Role_Auto:  break;
          case IDTLSTransportTypes::Role_Client: {
            ZS_EVENTING_2(
                          x, i, Detail, DtlsTransportRoleSet, ol, DtlsTransport, Info,
                          puid, id, mID,
                          string, role, IDTLSTransportTypes::toString(IDTLSTransportTypes::Role_Server)
                          );
            mFixedRole = true;
            mAdapter->setServerRole();
            mAdapter->startSSLWithPeer();
            break;
          }
          case IDTLSTransportTypes::Role_Server: {
            ZS_EVENTING_2(
                          x, i, Detail, DtlsTransportRoleSet, ol, DtlsTransport, Info,
                          puid, id, mID,
                          string, role, IDTLSTransportTypes::toString(IDTLSTransportTypes::Role_Client)
                          );
            mFixedRole = true;
            mAdapter->startSSLWithPeer();
            break;
          }
        }
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::stop()
    {
      ZS_EVENTING_1(x, i, Detail, DtlsTransportStop, ol, DtlsTransport, Stop, puid, id, mID);
      ZS_LOG_DEBUG(log("stop called"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => ISecureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => ISecureTransportForRTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    bool DTLSTransport::sendPacket(
                                   IICETypes::Components sendOverICETransport,
                                   IICETypes::Components packetType,
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   )
    {
      ZS_EVENTING_5(
                    x, i, Trace, DtlsTransportSendRtpPacket, ol, DtlsTransport, Send,
                    puid, id, mID,
                    enum, sendOverComponent, zsLib::to_underlying(sendOverICETransport),
                    enum, packetType, zsLib::to_underlying(packetType),
                    buffer, packet, buffer,
                    size, size, bufferLengthInBytes
                    );

      ZS_LOG_TRACE(log("sending rtp packet") + ZS_PARAM("length", bufferLengthInBytes))

      UseSRTPTransportPtr transport;

      {
        AutoRecursiveLock lock(*this);
        if (!mSRTPTransport) {
          ZS_LOG_WARNING(Debug, log("srtp transport is not ready"))
          return false;
        }

        transport = mSRTPTransport;

        if ((isShutdown()) ||
            (isShuttingDown())) {
          ZS_LOG_WARNING(Debug, log("cannot send rtp packet while shutdown/shutting down") + ZS_PARAM("packet length", bufferLengthInBytes))
          return false;
        }

        if (!isValidated()) {
          ZS_LOG_WARNING(Debug, log("cannot send rtp packets while stream is not validated") + ZS_PARAM("packet length", bufferLengthInBytes))
          return false;
        }

      }

      // WARNING: Best to not send packet to srtp transport inside an object lock
      return transport->sendPacket(sendOverICETransport, packetType, buffer, bufferLengthInBytes);
    }


    //-------------------------------------------------------------------------
    IICETransportPtr DTLSTransport::getICETransport() const
    {
      AutoRecursiveLock lock(*this);
      return ICETransport::convert(mICETransport);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => ISecureTransportForICETransport
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::notifyAssociateTransportCreated(
                                                        IICETypes::Components associatedComponent,
                                                        ICETransportPtr assoicated
                                                        )
    {
      // ignored since RTP vs RTCP are treated as independent transports
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::handleReceivedPacket(
                                             IICETypes::Components viaTransport,
                                             const BYTE *buffer,
                                             size_t bufferLengthInBytes
                                             )
    {
      bool isDTLSPacket = isDtlsPacket(buffer, bufferLengthInBytes);

      StreamResult streamResult {};
      int streamError {};

      ZS_EVENTING_5(
                    x, i, Trace, DtlsTransportReceivedPacket, ol, DtlsTransport, Receive,
                    puid, id, mID,
                    enum, viaComponent, zsLib::to_underlying(viaTransport),
                    bool, isDtlsPacket, isDTLSPacket, 
                    buffer, packet, buffer,
                    size, size, bufferLengthInBytes
                    );

      ZS_LOG_TRACE(log("handle receive packet") + ZS_PARAM("length", bufferLengthInBytes));

      PacketQueue decryptedPackets;
      UseSRTPTransportPtr srtpTransport;

      ASSERT(viaTransport == component());  // must be identical

      // scope: pre-validation check
      {
        AutoRecursiveLock lock(*this);

        if ((isShutdown())) {
          ZS_LOG_WARNING(Debug, log("received packet after already shutdown (thus discarding)") + ZS_PARAM("buffer length", bufferLengthInBytes))
          return false;
        }

        if (isShuttingDown()) {
          if (isRtpPacket(buffer, bufferLengthInBytes)) {
            ZS_LOG_WARNING(Debug, log("received RTP packet after shutting down (thus discarding)") + ZS_PARAM("buffer length", bufferLengthInBytes))
            return false;
          }
        }

        ZS_THROW_BAD_STATE_IF(!mAdapter)

        if (isDTLSPacket) {
          // Sanity check we're not passing junk that
          // just looks like DTLS.
          const BYTE * tmp_data = buffer;
          size_t tmp_size = bufferLengthInBytes;
          while (tmp_size > 0) {
            if (tmp_size < kDtlsRecordHeaderLen) {
              ZS_LOG_WARNING(Trace, log("too short for the DTLS header") + ZS_PARAM("buffer length", bufferLengthInBytes) + ZS_PARAM("tmp size", tmp_size))
              return false;
            }

            size_t record_len = (tmp_data[11] << 8) | (tmp_data[12]);
            if ((record_len + kDtlsRecordHeaderLen) > tmp_size) {
              ZS_LOG_WARNING(Trace, log("DTLS body is too short") + ZS_PARAM("buffer length", bufferLengthInBytes) + ZS_PARAM("record len", record_len) + ZS_PARAM("tmp size", tmp_size))
              return false;
            }

            tmp_data += record_len + kDtlsRecordHeaderLen;
            tmp_size -= record_len + kDtlsRecordHeaderLen;
          }

          if (mPendingIncomingDTLS.CurrentSize() < mMaxPendingDTLSBuffer) {
            mPendingIncomingDTLS.Put(buffer, bufferLengthInBytes);
          } else {
            ZS_LOG_WARNING(Debug, log("too many pending dtls packets (thus ignoring incoming dtls packet)"))
          }

          if (!mFixedRole) {
            ZS_EVENTING_2(
                          x, i, Detail, DtlsTransportRoleSet, ol, DtlsTransport, Info,
                          puid, id, mID,
                          string, role, IDTLSTransportTypes::toString(IDTLSTransportTypes::Role_Server)
                          );

            mFixedRole = true;
            mAdapter->setServerRole();
            mAdapter->startSSLWithPeer();
          }

          BYTE extractedBuffer[kMaxDtlsPacketLen] {};

          while (true) {
            size_t read = 0;
            streamResult = mAdapter->read(extractedBuffer, sizeof(extractedBuffer), &read, &streamError);

            switch (streamResult) {
              case SR_SUCCESS: {
                decryptedPackets.push(make_shared<SecureByteBlock>(extractedBuffer, read));
                break;
              }
              case SR_BLOCK: 
              case SR_EOS:  
              case SR_ERROR:  goto handle_data_packet;
            }
          }

          goto handle_data_packet;
        }

        if (!isRtpPacket(buffer, bufferLengthInBytes)) {
           ZS_LOG_WARNING(Debug, log("received non DTLS nor RTP packet (thus discarding)") + ZS_PARAM("buffer length", bufferLengthInBytes))
          return false;
        }

        if (mPutIncomingRTPIntoPendingQueue) {
          ZS_LOG_TRACE(log("transport not verified thus pushing RTP packet onto pending queue") + ZS_PARAM("buffer length", bufferLengthInBytes))
          mPendingIncomingRTP.push(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes));
          if (mPendingIncomingRTP.size() > mMaxPendingRTPPackets) {
            ZS_LOG_WARNING(Debug, log("too many pending rtp packets (thus popping first packet)"))
            mPendingIncomingRTP.pop();
          }
          return true;
        }

        srtpTransport = mSRTPTransport;
        if (!srtpTransport) {
          ZS_LOG_WARNING(Debug, log("srtp is not yet setup (thus discarding packet)") + ZS_PARAM("buffer length", bufferLengthInBytes))
        }
        goto handle_rtp;
      }

      // WARNING: Forward packet to data channel or RTP listener outside of object lock
    handle_rtp:
      {
#define WARNING_HANDLE_SRTP_KEY_LIFETIME_EXHAUSTION 1
#define WARNING_HANDLE_SRTP_KEY_LIFETIME_EXHAUSTION 2

        ZS_EVENTING_5(
                      x, i, Trace, DtlsTransportForwardingEncryptedPacketToSrtpTransport, ol, DtlsTransport, Deliver,
                      puid, id, mID,
                      puid, srtpTransportId, srtpTransport->getID(),
                      enum, viaTransportId, zsLib::to_underlying(viaTransport),
                      buffer, packet, buffer,
                      size, size, bufferLengthInBytes
                      );

        ZS_LOG_INSANE(log("forwarding packet to SRTP transport") + ZS_PARAM("srtp transport id", srtpTransport->getID()) + ZS_PARAM("via", IICETypes::toString(viaTransport)) + ZS_PARAM("buffer length", bufferLengthInBytes))
        return srtpTransport->handleReceivedPacket(viaTransport, buffer, bufferLengthInBytes);
      }

    handle_data_packet:
      {
        wakeUpIfNeeded();

        bool returnResult {true};

        while (decryptedPackets.size() > 0) {
          SecureByteBlockPtr decryptedPacket = decryptedPackets.front();
          decryptedPackets.pop();

          ZS_EVENTING_5(
                        x, i, Trace, DtlsTransportForwardingPacketToDataTransport, ol, DtlsTransport, Deliver,
                        puid, id, mID,
                        puid, dataTransportId, mDataTransport->getID(),
                        enum, viaTransport, zsLib::to_underlying(viaTransport),
                        buffer, packet, decryptedPacket->BytePtr(),
                        size, size, decryptedPacket->SizeInBytes()
                        );

          auto result = mDataTransport->handleDataPacket(decryptedPacket->BytePtr(), decryptedPacket->SizeInBytes());
          if (!result) returnResult = false;
        }

        switch (streamResult) {
          case SR_SUCCESS:  break;
          case SR_BLOCK: {
            ZS_LOG_TRACE(log("dtls packet consumed"));
            break;
          }
          case SR_EOS: {
            ZS_LOG_DEBUG(log("end of stream reached (thus shutting down)"));
            cancel();
            break;
          }
          case SR_ERROR: {
            ZS_LOG_ERROR(Debug, log("read error found (thus shutting down)") + ZS_PARAM("error code", streamError));
            cancel();
            break;
          }
        }

        return returnResult;
      }

      ASSERT(false); // cannot reach this point
      return false;
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::handleReceivedSTUNPacket(
                                                 IICETypes::Components viaComponent,
                                                 STUNPacketPtr packet
                                                 )
    {
      ZS_EVENTING_2(
                    x, i, Trace, DtlsTransportReceivedStunPacket, ol, DtlsTransport, Receive,
                    puid, id, mID,
                    enum, viaComponent, zsLib::to_underlying(viaComponent)
                    );
      packet->trace(__func__);

      ZS_LOG_TRACE(log("handle receive STUN packet") + packet->toDebug())

      {
        AutoRecursiveLock lock(*this);
      }

      // WARNING: Forward packet to data channel or RTP listener outside of object lock
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => ISecureTransportForSRTPTransport
    #pragma mark

    //-------------------------------------------------------------------------
    bool DTLSTransport::sendEncryptedPacket(
                                            IICETypes::Components sendOverICETransport,
                                            IICETypes::Components packetType,
                                            const BYTE *buffer,
                                            size_t bufferLengthInBytes
                                            )
    {
      UseICETransportPtr transport;

      {
        AutoRecursiveLock lock(*this);

        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_WARNING(Debug, log("cannot send encrypted packet while shutdown") + ZS_PARAM("send over component", IICETypes::toString(sendOverICETransport)) + ZS_PARAM("packet type", packetType) + ZS_PARAM("buffer length", bufferLengthInBytes))
          return false;
        }

        transport = mICETransport;
        if (!transport) {
          ZS_LOG_WARNING(Debug, log("ice transport is not available") + ZS_PARAM("send over component", IICETypes::toString(sendOverICETransport)) + ZS_PARAM("packet type", packetType) + ZS_PARAM("buffer length", bufferLengthInBytes))
          return false;
        }

        ASSERT(sendOverICETransport == transport->component())
      }

      ZS_EVENTING_6(
                    x, i, Trace, DtlsTransportSendEncryptedRtpPacket, ol, DtlsTransport, Send,
                    puid, id, mID,
                    puid, iceTransportId, transport->getID(),
                    enum, sendOverComponent, zsLib::to_underlying(sendOverICETransport),
                    enum, packetType, zsLib::to_underlying(packetType),
                    buffer, packet, buffer,
                    size, size, bufferLengthInBytes
                    );

      return transport->sendPacket(buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::handleReceivedDecryptedPacket(
                                                      IICETypes::Components viaTransport,
                                                      IICETypes::Components packetType,
                                                      const BYTE *buffer,
                                                      size_t bufferLengthInBytes
                                                      )
    {
      {
        AutoRecursiveLock lock(*this);

        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_WARNING(Debug, log("cannot send encrypted packet while shutdown") + ZS_PARAM("packet type", packetType) + ZS_PARAM("buffer length", bufferLengthInBytes))
          return false;
        }
      }

      ZS_EVENTING_6(
                    x, i, Trace, DtlsTransportForwardingPacketToRtpListener, ol, DtlsTransport, Deliver,
                    puid, id, mID,
                    puid, rtpListenerId, mRTPListener->getID(),
                    enum, viaTransport, zsLib::to_underlying(viaTransport),
                    enum, packetType, zsLib::to_underlying(packetType),
                    buffer, packet, buffer,
                    size, size, bufferLengthInBytes
                    );

      ZS_LOG_INSANE(log("forwarding packet to RTP listener") + ZS_PARAM("rtp listener id", mRTPListener->getID()) + ZS_PARAM("via", IICETypes::toString(viaTransport)) + ZS_PARAM("packet type", IICETypes::toString(packetType)) + ZS_PARAM("buffer length", bufferLengthInBytes))

      return mRTPListener->handleRTPPacket(mComponent, packetType, buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => ISecureTransportForSRTPTransport
    #pragma mark

    //-------------------------------------------------------------------------
    RTPListenerPtr DTLSTransport::getListener() const
    {
      return RTPListener::convert(mRTPListener);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => ISecureTransportForDataTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ISecureTransportSubscriptionPtr DTLSTransport::subscribe(ISecureTransportDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to secure transport state"))

      AutoRecursiveLock lock(*this);

      ISecureTransportSubscriptionPtr subscription = mSecureTransportSubscriptions.subscribe(ISecureTransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));

      ISecureTransportDelegatePtr delegate = mSecureTransportSubscriptions.delegate(subscription, true);

      if (delegate) {
        DTLSTransportPtr pThis = mThisWeak.lock();

        if (ISecureTransport::State_Pending != mSecureTransportState) {
          delegate->onSecureTransportStateChanged(pThis, mSecureTransportState);
        }
      }

      if (isShutdown()) {
        mSecureTransportSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    ISecureTransportTypes::States DTLSTransport::state(ISecureTransportTypes::States ignored) const
    {
      return mSecureTransportState; // no lock needed
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::isClientRole() const
    {
      AutoRecursiveLock lock(*this);

      if (!mAdapter) return false;

      switch (mAdapter->role())
      {
        case Adapter::SSL_CLIENT: return true;
        case Adapter::SSL_SERVER: return false;
      }

      return false;
    }

    //-------------------------------------------------------------------------
    DTLSTransport::UseDataTransportPtr DTLSTransport::getDataTransport() const
    {
      return mDataTransport;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::sendDataPacket(
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       )
    {
      ZS_EVENTING_3(
                    x, i, Trace, DtlsTransportSendDataPacket, ol, DtlsTransport, Send,
                    puid, id, mID,
                    buffer, packet, buffer,
                    size, size, bufferLengthInBytes
                    );

      ZS_LOG_TRACE(log("sending data packet") + ZS_PARAM("length", bufferLengthInBytes))

      {
        AutoRecursiveLock lock(*this);
        if (!mICETransport) {
          ZS_LOG_WARNING(Debug, log("no ice transport is attached"))
          return false;
        }

        if ((isShutdown()) ||
            (isShuttingDown())) {
          ZS_LOG_WARNING(Debug, log("cannot send data packet while shutdown/shutting down") + ZS_PARAM("packet length", bufferLengthInBytes))
          return false;
        }

        if (!isValidated()) {
          ZS_LOG_WARNING(Debug, log("cannot send data packets while stream is not validated") + ZS_PARAM("packet length", bufferLengthInBytes))
          return false;
        }

        ZS_THROW_BAD_STATE_IF(!mAdapter)

        size_t written {};
        int error {};

        auto result = mAdapter->write(buffer, bufferLengthInBytes, &written, &error);

        wakeUpIfNeeded();

        switch (result) {
          case SR_SUCCESS: {
            goto send_packets;
          }
          case SR_BLOCK: {
            ZS_LOG_TRACE(log("dtls packet consumed") + ZS_PARAM("packet length", bufferLengthInBytes))
            return true;
          }
          case SR_EOS:  {
            ZS_LOG_DEBUG(log("end of stream reached (thus shutting down)"))
            cancel();
            return false;
          }
          case SR_ERROR: {
            ZS_LOG_ERROR(Debug, log("write error found (thus shutting down)") + ZS_PARAM("error code", error))
            cancel();
            return false;
          }
        }
      }

    send_packets:
      {
        IDTLSTransportAsyncDelegateProxy::create(mThisWeak.lock())->onAdapterSendPacket();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::onTimer(ITimerPtr timer)
    {
      ZS_EVENTING_2(
                    x, i, Trace, DtlsTransportInternalTimerEvent, ol, DtlsTransport, InternalEvent,
                    puid, id, mID,
                    puid, timerId, timer->getID()
                    );
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()));

      AutoRecursiveLock lock(*this);
      mAdapter->onTimer(timer);

      wakeUpIfNeeded();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IDTLSTransportAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::onAdapterSendPacket()
    {
      ZS_LOG_DEBUG(log("on adapter send packet"))

      UseICETransportPtr transport;
      PacketQueue packets;

      {
        AutoRecursiveLock lock(*this);
        if (!mICETransport) {
          ZS_LOG_WARNING(Debug, log("no ice transport is attached"))
          return;
        }

        transport = mICETransport;

        packets = mPendingOutgoingDTLS;
        mPendingOutgoingDTLS = PacketQueue();
        goto send_packets;
      }

    send_packets:
      {
        BYTE fillBuffer[kMaxDtlsPacketLen] {};
        size_t filled = 0;

        // combine smaller packets into a single packet
        while (packets.size() > 0) {
          SecureByteBlockPtr packet = packets.front();
          packets.pop();

          if (filled + packet->SizeInBytes() > sizeof(fillBuffer)) {
            // cannot fit next packet into fill buffer so data filled thus far
            if (filled > 0) {
              ZS_EVENTING_4(
                            x, i, Trace, DtlsTransportForwardDataPacketToIceTransport, ol, DtlsTransport, Send,
                            puid, id, mID,
                            puid, iceTransportId, transport->getID(),
                            buffer, packet, &(fillBuffer[0]),
                            size, size, filled
                            );
              if (!transport->sendPacket(&(fillBuffer[0]), filled)) return;
              filled = 0;
            }

            if (packet->SizeInBytes() > sizeof(fillBuffer)) {
              // packet size exceed buffer capacity so send it immediately (anything previous put into fill buffer has been sent already)
              ZS_EVENTING_4(
                            x, i, Trace, DtlsTransportForwardDataPacketToIceTransport, ol, DtlsTransport, Send,
                            puid, id, mID,
                            puid, iceTransportId, transport->getID(),
                            buffer, packet, packet->BytePtr(),
                            size, size, packet->SizeInBytes()
                            );
              if (!transport->sendPacket(*packet, packet->SizeInBytes())) return;
              continue;
            }
          }

          memcpy(&(fillBuffer[filled]), packet->BytePtr(), packet->SizeInBytes());
          filled += packet->SizeInBytes();
        }

        if (0 != filled) {
          // final push of filled buffer over the wire
          ZS_EVENTING_4(
                        x, i, Trace, DtlsTransportForwardDataPacketToIceTransport, ol, DtlsTransport, Send,
                        puid, id, mID,
                        puid, iceTransportId, transport->getID(),
                        buffer, packet, &(fillBuffer[0]),
                        size, size, filled
                        );
          if (!transport->sendPacket(&(fillBuffer[0]), filled)) return;
          filled = 0;
        }
      }
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::onDeliverPendingIncomingRTP()
    {
      PacketQueue pendingPackets;
      UseSRTPTransportPtr srtpTransport;

      IICETypes::Components viaTransport = component();

      {
        AutoRecursiveLock lock(*this);

        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_WARNING(Debug, log("cannot deliver pending RTP packets as shutdown / shutting down"))
          return;
        }

        pendingPackets = mPendingIncomingRTP;
        mPendingIncomingRTP = PacketQueue();

        srtpTransport = mSRTPTransport;
        if (!srtpTransport) {
          ZS_LOG_WARNING(Debug, log("cannot process SRTP packets as no SRTP transport attached"))
          return;
        }
      }

      while (pendingPackets.size() > 0) {
        auto packet = pendingPackets.front();

        ZS_EVENTING_5(
                      x, i, Trace, DtlsTransportForwardingEncryptedPacketToSrtpTransport, ol, DtlsTransport, Deliver,
                      puid, id, mID,
                      puid, srtpTransportId, srtpTransport->getID(),
                      enum, viaTransportId, zsLib::to_underlying(viaTransport),
                      buffer, packet, packet->BytePtr(),
                      size, size, packet->SizeInBytes() 
                      );

        bool delivered = srtpTransport->handleReceivedPacket(viaTransport, packet->BytePtr(), packet->SizeInBytes());
        if (!delivered) {
          ZS_LOG_WARNING(Debug, log("failed to process SRTP packet"))
        }
        pendingPackets.pop();
      }

      {
        AutoRecursiveLock lock(*this);
        if (mPendingIncomingRTP.size() > 0) {
          // some more packets were put into queue after deliverying these packets, try again...
          IDTLSTransportAsyncDelegateProxy::create(mThisWeak.lock())->onDeliverPendingIncomingRTP();
          return;
        }

        // no longer need queue to deliver packets
        mPutIncomingRTPIntoPendingQueue = false;
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IICETransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::onICETransportStateChange(
                                                  IICETransportPtr transport,
                                                  IICETransport::States state
                                                  )
    {
      ZS_EVENTING_3(
                    x, i, Trace, DtlsTransportInternalIceStateChangeEvent, ol, DtlsTransport, InternalEvent,
                    puid, id, mID,
                    puid, iceTransport, transport->getID(),
                    string, state, IICETransportTypes::toString(state)
                    );
      ZS_LOG_DEBUG(log("on ice transport state changed"))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::onICETransportCandidatePairAvailable(
                                                             IICETransportPtr transport,
                                                             CandidatePairPtr candidatePair
                                                             )
    {
      // ignored
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::onICETransportCandidatePairGone(
                                                        IICETransportPtr transport,
                                                        CandidatePairPtr candidatePair
                                                        )
    {
      // ignored
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::onICETransportCandidatePairChanged(
                                                           IICETransportPtr transport,
                                                           CandidatePairPtr candidatePair
                                                           )
    {
      // ignored
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IICETransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::onSRTPTransportLifetimeRemaining(
                                                         ISRTPTransportPtr transport,
                                                         ULONG leastLifetimeRemainingPercentageForAllKeys,
                                                         ULONG overallLifetimeRemainingPercentage
                                                         )
    {
      ZS_EVENTING_4(
                    x, i, Debug, DtlsTransportInternalSrtpTransportLifetimeRemainingEvent, ol, DtlsTransport, InternalEvent,
                    puid, id, mID,
                    puid, srtpTransportId, transport->getID(),
                    ulong, leastLifetimeRemainingPercentageForAllKeys, leastLifetimeRemainingPercentageForAllKeys,
                    ulong, overallLifetimeRemainingPercentage, overallLifetimeRemainingPercentage
                    );
#define TODO_IN_FUTURE_WHEN_PERCENT_REMAINING_GETS_TOO_LOW_AS_A_CLIENT_RENEGOTIATE_A_NEW_SRTP_KEY 1
#define TODO_IN_FUTURE_WHEN_PERCENT_REMAINING_GETS_TOO_LOW_AS_A_CLIENT_RENEGOTIATE_A_NEW_SRTP_KEY 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => friend BIO_
    #pragma mark

    //-------------------------------------------------------------------------
    StreamResult DTLSTransport::bioRead(
                                        void* data,
                                        size_t data_len,
                                        size_t* read,
                                        int* error
                                        )
    {
      if (NULL != read) *read = 0;

      AutoRecursiveLock lock(*this);
      if (!mAdapter) {
        if (error) *error = SSL_ERROR_ZERO_RETURN;
        return SR_ERROR;
      }

      return mAdapter->bioRead(data, data_len, read, error);
    }

    //-------------------------------------------------------------------------
    StreamResult DTLSTransport::bioWrite(
                                         const void* data,
                                         size_t data_len,
                                         size_t* written,
                                         int* error
                                         )
    {
      if (NULL != written) *written = 0;

      AutoRecursiveLock lock(*this);
      if (!mAdapter) {
        if (error) *error = SSL_ERROR_ZERO_RETURN;
        return SR_ERROR;
      }

      return mAdapter->bioWrite(data, data_len, written, error);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => friend DTLSTransport::Adapter
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::adapterSendPacket(
                                          const BYTE *buffer,
                                          size_t bufferLengthInBytes
                                          )
    {
      ZS_LOG_TRACE(log("adding dtls packet to send to outgoing queue") + ZS_PARAM("buffer length", bufferLengthInBytes))
      mPendingOutgoingDTLS.push(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes));

      IDTLSTransportAsyncDelegateProxy::create(mThisWeak.lock())->onAdapterSendPacket();
    }

    //-------------------------------------------------------------------------
    size_t DTLSTransport::adapterReadPacket(BYTE *buffer, size_t bufferLengthInBytes)
    {
      size_t currentSize = SafeInt<CryptoPP::lword>(mPendingIncomingDTLS.CurrentSize());
      if (currentSize < 1) return 0;

      auto readSize = (currentSize < bufferLengthInBytes ? currentSize : bufferLengthInBytes);

      mPendingIncomingDTLS.Get(buffer, readSize);

      return readSize;
    }

    //-------------------------------------------------------------------------
    ITimerPtr DTLSTransport::adapterCreateTimeout(Milliseconds timeout)
    {
      return ITimer::create(mThisWeak.lock(), zsLib::now() + timeout);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params DTLSTransport::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::DTLSTransport");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params DTLSTransport::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr DTLSTransport::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::DTLSTransport");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "subscriptions", mSubscriptions.size());
      IHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      IHelper::debugAppend(resultEl, "state", IDTLSTransport::toString(mCurrentState));

      IHelper::debugAppend(resultEl, "secure transport subscriptions", mSecureTransportSubscriptions.size());
      IHelper::debugAppend(resultEl, "secure transport state", ISecureTransportTypes::toString(mSecureTransportState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      IHelper::debugAppend(resultEl, "ice transport", mICETransport ? mICETransport->getID() : 0);

      IHelper::debugAppend(resultEl, "certificates", mCertificates.size());

      IHelper::debugAppend(resultEl, "local params", mLocalParams.toDebug());
      IHelper::debugAppend(resultEl, "remote params", mRemoteParams.toDebug());

      IHelper::debugAppend(resultEl, "adapter", mAdapter ? mAdapter->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "max pending dtls buffer", mMaxPendingDTLSBuffer);
      IHelper::debugAppend(resultEl, "max pending rtp packets", mMaxPendingRTPPackets);

      IHelper::debugAppend(resultEl, "put pending incoming RTP packets into queue", mPutIncomingRTPIntoPendingQueue);
      IHelper::debugAppend(resultEl, "pending incoming RTP packets", mPendingIncomingRTP.size());
      IHelper::debugAppend(resultEl, "pending incoming dtls buffer size (bytes)", mPendingIncomingDTLS.CurrentSize());

      IHelper::debugAppend(resultEl, "pending outgoing dtls packets", mPendingOutgoingDTLS.size());

      IHelper::debugAppend(resultEl, "fixed role", mFixedRole);

      IHelper::debugAppend(resultEl, "validation", Adapter::toString(mValidation));

      IHelper::debugAppend(resultEl, "srtp transport", mSRTPTransport ? mSRTPTransport->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    IICETypes::Components DTLSTransport::component() const
    {
      return mComponent;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return ((IDTLSTransportTypes::State_Closed == mCurrentState) ||
              (IDTLSTransportTypes::State_Failed == mCurrentState));
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      ZS_EVENTING_1(x, i, Debug, DtlsTransportStep, ol, DtlsTransport, Step, puid, id, mID);

      // ... other steps here ...
      if (!stepStartSSL()) goto not_ready;
      if (!stepValidate()) goto not_ready;
      if (!stepFixState()) goto not_ready;
      if (!stepNotifyReady()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
      }
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::stepStartSSL()
    {
      ZS_EVENTING_1(x, i, Debug, DtlsTransportStep, ol, DtlsTransport, Step, puid, id, mID);

      if (mFixedRole) {
        ZS_LOG_TRACE(log("ssl already started"))
        return true;
      }

      if (!mICETransport) {
        ZS_LOG_WARNING(Debug, log("ice transport gone"))
        cancel();
        return false;
      }

      auto iceState = mICETransport->state();
      switch (iceState) {
          
        case IICETransportTypes::State_Connected:
        case IICETransportTypes::State_Completed:
        {
          IICETypes::Roles role = mICETransport->getRole();   // example of how to get the ice role
          switch (role) {
            case IICETypes::Role_Controlling: {
              ZS_EVENTING_2(
                            x, i, Detail, DtlsTransportRoleSet, ol, DtlsTransport, Info,
                            puid, id, mID,
                            string, role, IDTLSTransportTypes::toString(IDTLSTransportTypes::Role_Server)
                            );
              mAdapter->setServerRole();
              break;
            }
            case IICETypes::Role_Controlled:  {
              ZS_EVENTING_2(
                            x, i, Detail, DtlsTransportRoleSet, ol, DtlsTransport, Info,
                            puid, id, mID,
                            string, role, IDTLSTransportTypes::toString(IDTLSTransportTypes::Role_Client)
                            );
              break;
            }
          }

          mFixedRole = true;
          mAdapter->startSSLWithPeer();
          return true;
        }

        case IICETransportTypes::State_New:
        case IICETransportTypes::State_Checking:
        case IICETransportTypes::State_Disconnected:
        case IICETransportTypes::State_Failed:
        case IICETransportTypes::State_Closed: {
          break;
        }
      }

      ZS_LOG_DEBUG(log("ice transport is not connected"))
      return false;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::stepValidate()
    {
      ZS_EVENTING_1(x, i, Debug, DtlsTransportStep, ol, DtlsTransport, Step, puid, id, mID);

      if (Adapter::VALIDATION_NA != mValidation) {
        ZS_LOG_TRACE(log("validation already completed"))
        return true;
      }

      if (mRemoteParams.mFingerprints.size() < 1) {
        ZS_LOG_TRACE(log("waiting for remote parameters"))
        return true;
      }

      X509 *peerCert = NULL;
      if (!mAdapter->getPeerCertificate(&peerCert)) {
        ZS_LOG_TRACE(log("do not have a peer certificate yet"))
        return true;
      }

      ZS_LOG_DEBUG(log("validating"))

      bool passed = false;

      for (auto iter = mRemoteParams.mFingerprints.begin(); iter != mRemoteParams.mFingerprints.end(); ++iter)
      {
        auto fingerprint = (*iter);

        ZS_EVENTING_4(
                      x, i, Detail, DtlsTransportInitializationInstallFingerprint, ol, DtlsTransport, Initialization,
                      puid, id, mID,
                      string, algorithm, fingerprint.mAlgorithm,
                      string, fingerprint, fingerprint.mValue,
                      bool, isLocal, false
                      );

        auto result = mAdapter->setPeerCertificateDigest(fingerprint.mAlgorithm, fingerprint.mValue);
        switch (result) {
          case Adapter::VALIDATION_NA:      break;
          case Adapter::VALIDATION_PASSED:  passed = true; break;
          case Adapter::VALIDATION_FAILED:  {
            ZS_LOG_ERROR(Debug, log("fingerprint validation failed") + fingerprint.toDebug())
            cancel();
            break;
          }
        }
      }

      if (!passed) {
        ZS_LOG_WARNING(Debug, log("could not validate using any validation route"))
        cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("at least one validation route passed"))
      mValidation = Adapter::VALIDATION_PASSED;
      return true;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::stepFixState()
    {
      ZS_EVENTING_1(x, i, Debug, DtlsTransportStep, ol, DtlsTransport, Step, puid, id, mID);

      auto state = mAdapter->getState();
      switch (state) {
        case Adapter::SS_OPENING: {
          setState(IDTLSTransportTypes::State_Connecting);
          break;
        }
        case Adapter::SS_OPEN: {
          // ensure all the previous states in the state machine have occured before immediately jumping into final state
          if (IDTLSTransportTypes::State_New == mCurrentState) setState(IDTLSTransportTypes::State_Connecting);
          switch (mValidation)
          {
            case Adapter::VALIDATION_NA:      break;
            case Adapter::VALIDATION_PASSED:
            {
              setState(IDTLSTransportTypes::State_Connected);
              return true;
            }
            case Adapter::VALIDATION_FAILED:
            {
              ZS_LOG_WARNING(Debug, log("ssl validation failed"))
              setState(IDTLSTransportTypes::State_Failed);
              cancel();
              return false;
            }
          }
          break;
        }
        default:
        {
          ZS_LOG_WARNING(Debug, log("ssl connection was closed"))
          cancel();
          break;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::stepNotifyReady()
    {
      ZS_EVENTING_1(x, i, Debug, DtlsTransportStep, ol, DtlsTransport, Step, puid, id, mID);

      if (IDTLSTransportTypes::State_Connected != mCurrentState) {
        ZS_LOG_TRACE(log("step notify ready is not validated yet"))
        return true;
      }

      auto iceState = mICETransport->state();
      switch (iceState) {

        case IICETransportTypes::State_Connected:
        case IICETransportTypes::State_Completed:
        {
          setState(ISecureTransport::State_Connected);
          return true;
        }

        case IICETransportTypes::State_New:
        case IICETransportTypes::State_Checking:
        case IICETransportTypes::State_Disconnected:
        case IICETransportTypes::State_Failed:
        case IICETransportTypes::State_Closed: {
          setState(ISecureTransport::State_Disconnected);
          break;
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, DtlsTransportCancel, ol, DtlsTransport, Cancel, puid, id, mID);

      //.......................................................................
      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      setState(ISecureTransport::State_Closed);

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_DTLS_SESSION_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_DTLS_SESSION_IS_SHUTDOWN 2
      }

      //.......................................................................
      // final cleanup

      setState(IDTLSTransportTypes::State_Closed);

      mAdapter->close();

      if (mICETransport) {
        mICETransport->notifyDetached(mID);
        mICETransport.reset();
      }

      if (mICETransportSubscription) {
        mICETransportSubscription->cancel();
        mICETransportSubscription.reset();
      }

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::setState(IDTLSTransportTypes::States state)
    {
      IDTLSTransportTypes::States currentState = mCurrentState;

      if (state == currentState) return;
      switch (currentState)
      {
        case IDTLSTransportTypes::State_Closed:
        case IDTLSTransportTypes::State_Failed: {
          ZS_LOG_TRACE(log("already in final state (cannot be changed)"))
          return;
        }
        default: break;
      }

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", IDTLSTransport::toString(state)) + ZS_PARAM("old state", IDTLSTransport::toString(currentState)))

      mCurrentState = state;
      ZS_EVENTING_2(x, i, Debug, DtlsTransportStateChangedEvent, ol, DtlsTransport, StateEvent, puid, id, mID, string, state, IDTLSTransportTypes::toString(state));

      if (IDTLSTransportTypes::State_Connected == state) {
        setupSRTP();
      }

      if (IDTLSTransportTypes::State_Connected == state) {
        IDTLSTransportAsyncDelegateProxy::create(mThisWeak.lock())->onDeliverPendingIncomingRTP();
      }

      DTLSTransportPtr pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onDTLSTransportStateChange(pThis, mCurrentState);
      }
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != mLastError) {
        ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
        return;
      }

      ZS_EVENTING_3(
                    x, e, Debug, DtlsTransportErrorEvent, ol, DtlsTransport, ErrorEvent,
                    puid, id, mID,
                    word, errorCode, errorCode,
                    string, reason, reason
                    );

      mLastError = errorCode;
      mLastErrorReason = reason;

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::setState(ISecureTransportTypes::States state)
    {
      if (state == mSecureTransportState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", ISecureTransportTypes::toString(state)) + ZS_PARAM("old state", ISecureTransportTypes::toString(mSecureTransportState)))

      mSecureTransportState = state;
      ZS_EVENTING_2(
                    x, i, Debug, DtlsTransportStateChangedEvent, ol, DtlsTransport, StateEvent,
                    puid, id, mID,
                    string, state, ISecureTransportTypes::toString(state)
                    );

      DTLSTransportPtr pThis = mThisWeak.lock();
      if (pThis) {
        mSecureTransportSubscriptions.delegate()->onSecureTransportStateChanged(pThis, mSecureTransportState);
      }
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::wakeUpIfNeeded()
    {
      switch ((States)mCurrentState) {
        case IDTLSTransportTypes::State_New:        {
          if (!mFixedRole) return;
          break;
        }
        case IDTLSTransportTypes::State_Connecting: {
          if (Adapter::SS_OPENING == mAdapter->getState()) return;
          if (Adapter::SS_OPEN == mAdapter->getState()) {
            if (mRemoteParams.mFingerprints.size() < 1) return;
          }
          break;
        }
        case IDTLSTransportTypes::State_Connected:  {
          if (Adapter::SS_OPEN == mAdapter->getState()) return;
          break;
        }
        case IDTLSTransportTypes::State_Closed:     
        case IDTLSTransportTypes::State_Failed: return;
      }

      ZS_LOG_TRACE(log("need to wake up to adjust state"))
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::setupSRTP()
    {
      typedef ISRTPSDESTransportTypes::CryptoParameters CryptoParameters;
      typedef ISRTPSDESTransportTypes::Parameters Parameters;
      typedef ISRTPSDESTransportTypes::KeyParameters KeyParameters;

      if (mSRTPTransport) return; // already setup

      SecureByteBlock dtlsBuffer(SRTP_MASTER_KEY_KEY_LEN * 2 +
                                 SRTP_MASTER_KEY_SALT_LEN * 2);

      if (!mAdapter->exportKeyingMaterial(kDtlsSrtpExporterLabel, NULL, 0, false, dtlsBuffer.BytePtr(), dtlsBuffer.SizeInBytes())) {
        ZS_LOG_WARNING(Detail, log("failed to extract DTLS-SRTP keying material"))
        ASSERT(false)
        return;
      }

      SecureByteBlock clientWriteKey(SRTP_MASTER_KEY_KEY_LEN + SRTP_MASTER_KEY_SALT_LEN);
      SecureByteBlock serverWriteKey(SRTP_MASTER_KEY_KEY_LEN + SRTP_MASTER_KEY_SALT_LEN);

      size_t offset = 0;
      memcpy(&clientWriteKey[0], &dtlsBuffer[offset], SRTP_MASTER_KEY_KEY_LEN);
      offset += SRTP_MASTER_KEY_KEY_LEN;
      memcpy(&serverWriteKey[0], &dtlsBuffer[offset], SRTP_MASTER_KEY_KEY_LEN);
      offset += SRTP_MASTER_KEY_KEY_LEN;
      memcpy(&clientWriteKey[SRTP_MASTER_KEY_KEY_LEN], &dtlsBuffer[offset], SRTP_MASTER_KEY_SALT_LEN);
      offset += SRTP_MASTER_KEY_SALT_LEN;
      memcpy(&serverWriteKey[SRTP_MASTER_KEY_KEY_LEN], &dtlsBuffer[offset], SRTP_MASTER_KEY_SALT_LEN);

      SecureByteBlock *sendKey {};
      SecureByteBlock *receiveKey {};

      switch (mAdapter->role()) {
        case Adapter::SSL_SERVER:   sendKey = &serverWriteKey; receiveKey = &clientWriteKey; break;
        case Adapter::SSL_CLIENT:   sendKey = &clientWriteKey; receiveKey = &serverWriteKey; break;
      }

      String cipher;
      if (!mAdapter->getDtlsSrtpCipher(&cipher)) {
        ZS_LOG_WARNING(Detail, log("failed to negotiate SRTP cipher suite"))
        return;
      }

      CryptoParameters sendingParams;
      CryptoParameters receivingParams;

      sendingParams.mCryptoSuite = cipher;
      receivingParams.mCryptoSuite = cipher;


      KeyParameters sendingKeyParams;
      KeyParameters receivingKeyParams;

      sendingKeyParams.mKeyMethod = "inline";
      receivingKeyParams.mKeyMethod = "inline";

      sendingKeyParams.mKeySalt = IHelper::convertToBase64(sendKey->BytePtr(), sendKey->SizeInBytes());
      receivingKeyParams.mKeySalt = IHelper::convertToBase64(receiveKey->BytePtr(), receiveKey->SizeInBytes());

      sendingKeyParams.mLifetime = "2^31";
      receivingKeyParams.mLifetime = "2^31";

      sendingParams.mKeyParams.push_back(sendingKeyParams);
      receivingParams.mKeyParams.push_back(receivingKeyParams);

      mSRTPTransport = UseSRTPTransport::create(mThisWeak.lock(), mThisWeak.lock(), sendingParams, receivingParams);

      ZS_EVENTING_6(
                    x, i, Debug, DtlsTransportSrtpKeyingMaterialSetup, ol, DtlsTransport, Info,
                    puid, id, mID,
                    puid, srtpTransportId, ((bool)mSRTPTransport) ? mSRTPTransport->getID() : 0,
                    string, direction, "send",
                    string, cipher, cipher,
                    buffer, key, sendKey->BytePtr(),
                    size, size, sendKey->SizeInBytes()
                    );

      ZS_EVENTING_6(
                    x, i, Debug, DtlsTransportSrtpKeyingMaterialSetup, ol, DtlsTransport, Info,
                    puid, id, mID,
                    puid, srtpTransportId, ((bool)mSRTPTransport) ? mSRTPTransport->getID() : 0,
                    string, direction, "receive",
                    string, cipher, cipher,
                    buffer, key, receiveKey->BytePtr(),
                    size, size, receiveKey->SizeInBytes()
                    );
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportFactory::Adapter
    #pragma mark

    //-------------------------------------------------------------------------
    const char *DTLSTransport::Adapter::toString(StreamState state)
    {
      switch (state) {
        case SS_CLOSED:           return "closed";
        case SS_OPENING:          return "opening";
        case SS_OPEN:             return "open";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    const char *DTLSTransport::Adapter::toString(SSLRole role)
    {
      switch (role) {
        case SSL_CLIENT:          return "client";
        case SSL_SERVER:          return "server";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    const char *DTLSTransport::Adapter::toString(SSLMode mode)
    {
      switch (mode) {
        case SSL_MODE_TLS:        return "tls";
        case SSL_MODE_DTLS:       return "dtls";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    const char *DTLSTransport::Adapter::toString(SSLProtocolVersion version)
    {
      switch (version) {
        case SSL_PROTOCOL_TLS_10:       return "1.0";
        case SSL_PROTOCOL_TLS_11:       return "1.1/DTLS1.0";
        case SSL_PROTOCOL_TLS_12:       return "1.2/DTLS1.2";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    const char *DTLSTransport::Adapter::toString(Validation validation)
    {
      switch (validation) {
        case VALIDATION_NA:       return "NA";
        case VALIDATION_PASSED:   return "passed";
        case VALIDATION_FAILED:   return "failed";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    DTLSTransport::Adapter::Adapter(DTLSTransportPtr outer) :
      mOuter(outer)
    {
    }

    //-------------------------------------------------------------------------
    DTLSTransport::Adapter::~Adapter()
    {
      cleanup();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::setIdentity(UseCertificatePtr identity) {
      ZS_THROW_INVALID_ARGUMENT_IF(!identity)
      identity_ = identity;
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::setServerRole(SSLRole role) {
      role_ = role;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::getPeerCertificate(X509** cert) const {
      if (!peer_certificate_)
        return false;

      *cert = peer_certificate_;
      return true;
    }

    //-------------------------------------------------------------------------
    DTLSTransport::Adapter::Validation DTLSTransport::Adapter::setPeerCertificateDigest(
                                                                                        const String &digest_alg,
                                                                                        const String &digest_value
                                                                                        )
    {
      ASSERT(NULL != peer_certificate_);
      ASSERT(ssl_server_name_.empty());

      SecureByteBlockPtr digest = UseCertificate::getDigest(digest_alg, peer_certificate_);
      if (!digest) return VALIDATION_NA;

      String temp(digest_value);
      temp.replaceAll(":", "");
      temp.trim();

      SecureByteBlockPtr fingerprint = IHelper::convertFromHex(temp);
      if (!fingerprint) return VALIDATION_NA;

      if (0 != IHelper::compare(*digest, *fingerprint)) return VALIDATION_FAILED;

      return VALIDATION_PASSED;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::getSslCipher(String *cipher)
    {
      if (state_ != SSL_CONNECTED)
        return false;

      const SSL_CIPHER* current_cipher = SSL_get_current_cipher(ssl_);
      if (current_cipher == NULL) {
        return false;
      }

      char* cipher_name = SSL_CIPHER_get_rfc_name(current_cipher);
      if (cipher_name == NULL) {
        return false;
      }

      *cipher = cipher_name;
      OPENSSL_free(cipher_name);
      return true;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::exportKeyingMaterial(
                                                      const String &label,
                                                      const BYTE *context,
                                                      size_t context_len,
                                                      bool use_context,
                                                      BYTE *result,
                                                      size_t result_len
                                                      )
    {
      int i = SSL_export_keying_material(
                                         ssl_,
                                         result,
                                         result_len,
                                         label.c_str(),
                                         label.length(),
                                         const_cast<uint8_t *>(context),
                                         context_len,
                                         use_context
                                         );

      if (i != 1) return false;

      return true;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::setDtlsSrtpCiphers(const std::vector<String> &ciphers)
    {
      std::string internal_ciphers;

      if (state_ != SSL_NONE)
        return false;

      for (auto cipher = ciphers.begin(); cipher != ciphers.end(); ++cipher) {
        bool found = false;
        for (SrtpCipherMapEntry *entry = SrtpCipherMap; entry->internal_name; ++entry) {
          if (*cipher == entry->external_name) {
            found = true;
            if (!internal_ciphers.empty())
              internal_ciphers += ":";
            internal_ciphers += entry->internal_name;
            break;
          }
        }

        if (!found) {
          ZS_LOG_ERROR(Detail, log("could not find cipher") + ZS_PARAM("cipher", *cipher))
          return false;
        }
      }

      if (internal_ciphers.empty())
        return false;

      srtp_ciphers_ = internal_ciphers;
      return true;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::getDtlsSrtpCipher(String *cipher)
    {
      ASSERT(state_ == SSL_CONNECTED);
      if (state_ != SSL_CONNECTED) return false;

      const SRTP_PROTECTION_PROFILE *srtp_profile =
      SSL_get_selected_srtp_profile(ssl_);

      if (!srtp_profile)
        return false;

      for (SrtpCipherMapEntry *entry = SrtpCipherMap;
           entry->internal_name; ++entry) {
        if (!strcmp(entry->internal_name, srtp_profile->name)) {
          *cipher = entry->external_name;
          return true;
        }
      }

      ASSERT(false);  // This should never happen
      return false;
    }

    //-------------------------------------------------------------------------
    int DTLSTransport::Adapter::startSSLWithServer(const char* server_name) {
      ssl_server_name_ = String(server_name);
      ASSERT(ssl_server_name_.hasData());
      return startSSL();
    }

    //-------------------------------------------------------------------------
    int DTLSTransport::Adapter::startSSLWithPeer() {
      ASSERT(ssl_server_name_.empty());
      // It is permitted to specify peer_certificate_ only later.
      return startSSL();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::setMode(SSLMode mode) {
      ASSERT(state_ == SSL_NONE);
      ssl_mode_ = mode;
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::setMaxProtocolVersion(SSLProtocolVersion version) {
      ASSERT(ssl_ctx_ == NULL);
      ssl_max_version_ = version;
    }

    //-------------------------------------------------------------------------
    StreamResult DTLSTransport::Adapter::bioRead(
                                                 void* data,
                                                 size_t data_len,
                                                 size_t *read,
                                                 int *error
                                                 )
    {
      if (read) *read = 0;
      if (error) *error = 0;

      ZS_LOG_TRACE(log("bio read") + ZS_PARAM("data length", data_len))

      auto outer = mOuter.lock();
      if (!outer) return SR_ERROR;

      while (true) {
        auto bytesRead = outer->adapterReadPacket(static_cast<BYTE *>(data), data_len);
        if (0 == bytesRead) break;

        if (bytesRead > data_len) {
          ZS_LOG_ERROR(Detail, log("dropped dtls packet that is too large") + ZS_PARAM("output size", bytesRead) + ZS_PARAM("read size", data_len))
          continue;
        }

        if (read) *read = bytesRead;
        return SR_SUCCESS;
      }

      return SR_BLOCK;
    }

    //-------------------------------------------------------------------------
    StreamResult DTLSTransport::Adapter::bioWrite(
                                                  const void* data,
                                                  size_t data_len,
                                                  size_t* written,
                                                  int* error
                                                  )
    {
      if (written) *written = 0;
      if (error) *error = 0;

      ZS_LOG_TRACE(log("bio write") + ZS_PARAM("data length", data_len))

      auto outer = mOuter.lock();
      if (!outer) return SR_ERROR;

      outer->adapterSendPacket(static_cast<const BYTE *>(data), data_len);

      if (written) *written = data_len;
      return SR_SUCCESS;
    }

    //-------------------------------------------------------------------------
    StreamResult DTLSTransport::Adapter::write(
                                               const void* data,
                                               size_t data_len,
                                               size_t *written,
                                               int *error
                                               )
    {
      if (error) *error = 0;

      ZS_LOG_TRACE(log("write") + ZS_PARAM("data length", data_len))

      switch (state_) {
        case SSL_NONE:
          // pass-through in clear text
          return bioWrite(data, data_len, written, error);
        case SSL_WAIT:
        case SSL_CONNECTING:
          return SR_BLOCK;

        case SSL_CONNECTED:
          break;

        case SSL_ERROR:
        case SSL_CLOSED:
        default:
          if (error)
            *error = ssl_error_code_;
          return SR_ERROR;
      }

      // OpenSSL will return an error if we try to write zero bytes
      if (data_len == 0) {
        if (written)
          *written = 0;
        return SR_SUCCESS;
      }

      ssl_write_needs_read_ = false;

      int code = SSL_write(ssl_, data, SafeInt<decltype(data_len)>(data_len));
      int ssl_error = SSL_get_error(ssl_, code);
      switch (ssl_error) {
        case SSL_ERROR_NONE:
          ZS_LOG_TRACE(log("ssl write success"))
          ASSERT(0 < code && static_cast<unsigned>(code) <= data_len);
          if (written)
            *written = code;
          return SR_SUCCESS;
        case SSL_ERROR_WANT_READ:
          ZS_LOG_TRACE(log("ssl write wants read"))
          ssl_write_needs_read_ = true;
          return SR_BLOCK;
        case SSL_ERROR_WANT_WRITE:
          ZS_LOG_TRACE(log("ssl write wants write"))
          return SR_BLOCK;

        case SSL_ERROR_ZERO_RETURN:
        default:
          Adapter::error("SSL_write", (ssl_error ? ssl_error : -1), false);
          if (error)
            *error = ssl_error_code_;
          return SR_ERROR;
      }
      // not reached
    }

    //-------------------------------------------------------------------------
    StreamResult DTLSTransport::Adapter::read(
                                              void* data,
                                              size_t data_len,
                                              size_t* read,
                                              int *error
                                              )
    {
      ZS_LOG_INSANE(log("read") + ZS_PARAM("data length", data_len))

      switch (state_) {
        case SSL_NONE:
          // pass-through in clear text
          return bioRead(data, data_len, read, error);

        case SSL_WAIT:
        case SSL_CONNECTING: {
          int result = continueSSL();
          if (result < 0) return SR_ERROR;
          return SR_BLOCK;
        }

        case SSL_CONNECTED:
          break;

        case SSL_CLOSED:
          return SR_EOS;

        case SSL_ERROR:
        default:
          if (error)
            *error = ssl_error_code_;
          return SR_ERROR;
      }

      // Don't trust OpenSSL with zero byte reads
      if (data_len == 0) {
        if (read)
          *read = 0;
        return SR_SUCCESS;
      }

      ssl_read_needs_write_ = false;

      int code = SSL_read(ssl_, data, SafeInt<decltype(data_len)>(data_len));
      int ssl_error = SSL_get_error(ssl_, code);
      switch (ssl_error) {
        case SSL_ERROR_NONE:
          ZS_LOG_TRACE(log("ssl read success"))
          ASSERT(0 < code && static_cast<unsigned>(code) <= data_len);
          if (read)
            *read = code;

          if (ssl_mode_ == SSL_MODE_DTLS) {
            // Enforce atomic reads -- this is a short read
            unsigned int pending = SSL_pending(ssl_);

            if (pending) {
              ZS_LOG_WARNING(Trace, log("short DTLS read (thus flushing)"))
              flushInput(pending);
              if (error)
                *error = SSE_MSG_TRUNC;
              return SR_ERROR;
            }
          }
          return SR_SUCCESS;
        case SSL_ERROR_WANT_READ:
          ZS_LOG_TRACE(log("ssl read wants read"))
          return SR_BLOCK;
        case SSL_ERROR_WANT_WRITE:
          ZS_LOG_TRACE(log("ssl read wants write"))
          ssl_read_needs_write_ = true;
          return SR_BLOCK;
        case SSL_ERROR_ZERO_RETURN:
          ZS_LOG_TRACE(log("remote side closed"))
          return SR_EOS;
        default:
          ZS_LOG_ERROR(Debug, log("error") + ZS_PARAM("code", code))
          Adapter::error("SSL_read", (ssl_error ? ssl_error : -1), false);
          if (error)
            *error = ssl_error_code_;
          return SR_ERROR;
      }
      // not reached
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::flushInput(unsigned int left)
    {
      unsigned char buf[2048] {};

      while (left) {
        // This should always succeed
        int toread = (sizeof(buf) < left) ? sizeof(buf) : left;
        int code = SSL_read(ssl_, buf, toread);

        int ssl_error = SSL_get_error(ssl_, code);
        ASSERT(ssl_error == SSL_ERROR_NONE);

        if (ssl_error != SSL_ERROR_NONE) {
          ZS_LOG_ERROR(Debug, log("error") + ZS_PARAM("code", code))
          Adapter::error("SSL_read", (ssl_error ? ssl_error : -1), false);
          return;
        }

        ZS_LOG_DEBUG(log("flushed") + ZS_PARAM("bytes", code))
        left -= code;
      }
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::close()
    {
      cleanup();
      ASSERT(state_ == SSL_CLOSED || state_ == SSL_ERROR);
    }

    //-------------------------------------------------------------------------
    DTLSTransport::Adapter::StreamState DTLSTransport::Adapter::getState() const
    {
      switch (state_) {
        case SSL_WAIT:
        case SSL_CONNECTING:
          return SS_OPENING;
        case SSL_CONNECTED:
          return SS_OPEN;
        default:
          return SS_CLOSED;
      };
      // not reached
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::onEvent(
                                         int events,
                                         int err
                                         )
    {
#if 0
      int events_to_signal = 0;
      int signal_error = 0;

      if ((events & SE_OPEN)) {
        LOG(LS_VERBOSE) << "DTLSTransport::Adapter::OnEvent SE_OPEN";
        if (state_ != SSL_WAIT) {
          ASSERT(state_ == SSL_NONE);
          events_to_signal |= SE_OPEN;
        } else {
          state_ = SSL_CONNECTING;
          if (int err = BeginSSL()) {
            Error("BeginSSL", err, true);
            return;
          }
        }
      }
      if ((events & (SE_READ|SE_WRITE))) {
        LOG(LS_VERBOSE) << "DTLSTransport::Adapter::OnEvent"
        << ((events & SE_READ) ? " SE_READ" : "")
        << ((events & SE_WRITE) ? " SE_WRITE" : "");
        if (state_ == SSL_NONE) {
          events_to_signal |= events & (SE_READ|SE_WRITE);
        } else if (state_ == SSL_CONNECTING) {
          if (int err = ContinueSSL()) {
            Error("ContinueSSL", err, true);
            return;
          }
        } else if (state_ == SSL_CONNECTED) {
          if (((events & SE_READ) && ssl_write_needs_read_) ||
              (events & SE_WRITE)) {
            LOG(LS_VERBOSE) << " -- onStreamWriteable";
            events_to_signal |= SE_WRITE;
          }
          if (((events & SE_WRITE) && ssl_read_needs_write_) ||
              (events & SE_READ)) {
            LOG(LS_VERBOSE) << " -- onStreamReadable";
            events_to_signal |= SE_READ;
          }
        }
      }
      if ((events & SE_CLOSE)) {
        LOG(LS_VERBOSE) << "DTLSTransport::Adapter::OnEvent(SE_CLOSE, " << err << ")";
        Cleanup();
        events_to_signal |= SE_CLOSE;
        // SE_CLOSE is the only event that uses the final parameter to OnEvent().
        ASSERT(signal_error == 0);
        signal_error = err;
      }
      if (events_to_signal)
        StreamAdapterInterface::OnEvent(stream, events_to_signal, signal_error);
#endif //0
    }

    //-------------------------------------------------------------------------
    const char *DTLSTransport::Adapter::toString(SSLState state)
    {
      switch (state) {
        case SSL_NONE:            return "none";
        case SSL_WAIT:            return "wait";
        case SSL_CONNECTING:      return "connecting";
        case SSL_CONNECTED:       return "connected";
        case SSL_ERROR:           return "error";
        case SSL_CLOSED:          return "closed";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    int DTLSTransport::Adapter::startSSL() {
      ASSERT(state_ == SSL_NONE);

      state_ = SSL_CONNECTING;
      if (int err = beginSSL()) {
        Adapter::error("beginSSL", err, false);
        return err;
      }

      return 0;
    }

    //-------------------------------------------------------------------------
    Log::Params DTLSTransport::Adapter::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::DTLSTransport::Adapter");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }


    //-------------------------------------------------------------------------
    // This dumps the SSL error stack to the log.
    void DTLSTransport::Adapter::logSSLErrors(const String &prefix)
    {
      char error_buf[200] {};
      uint32_t err {};

      while ((err = ERR_get_error()) != 0) {
        ERR_error_string_n(err, error_buf, sizeof(error_buf));
        ZS_LOG_ERROR(Debug, log(prefix) + ZS_PARAM("error", error_buf))
      }
    }
    //-------------------------------------------------------------------------
    int DTLSTransport::Adapter::beginSSL()
    {
      ASSERT(state_ == SSL_CONNECTING);

      ZS_LOG_DEBUG(log("begin ssl") + ZS_PARAM("server name", (!ssl_server_name_.empty() ? ssl_server_name_ : "with peer")))

      BIO *bio = NULL;

      // First set up the context
      ASSERT(ssl_ctx_ == NULL);

      ssl_ctx_ = setupSSLContext();
      if (!ssl_ctx_)
        return -1;

      bio = BIO_new_stream(mOuter.lock());
      if (!bio)
        return -1;

      ssl_ = SSL_new(ssl_ctx_);
      if (!ssl_) {
        BIO_free(bio);
        return -1;
      }

      SSL_set_app_data(ssl_, this);

      SSL_set_bio(ssl_, bio, bio);  // the SSL object owns the bio now.

      SSL_set_mode(ssl_, SSL_MODE_ENABLE_PARTIAL_WRITE |
                   SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

      // Specify an ECDH group for ECDHE ciphers, otherwise they cannot be
      // negotiated when acting as the server. Use NIST's P-256 which is commonly
      // supported.
      EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
      if (ecdh == NULL)
        return -1;
      SSL_set_options(ssl_, SSL_OP_SINGLE_ECDH_USE);
      SSL_set_tmp_ecdh(ssl_, ecdh);
      EC_KEY_free(ecdh);

      // Do the connect
      return continueSSL();
    }

    //-------------------------------------------------------------------------
    int DTLSTransport::Adapter::continueSSL() {
      ZS_LOG_TRACE(log("continue ssl"))
      ASSERT(state_ == SSL_CONNECTING);

      if (mTimer) {
        mTimer->cancel();
        mTimer.reset();
      }

      int code = (role_ == SSL_CLIENT) ? SSL_connect(ssl_) : SSL_accept(ssl_);
      int ssl_error;
      switch (ssl_error = SSL_get_error(ssl_, code)) {
        case SSL_ERROR_NONE:
          ZS_LOG_DEBUG(log("ssl connect/accept success"))

          if (!sslPostConnectionCheck(
                                      ssl_,
                                      ssl_server_name_.c_str())) {
            ZS_LOG_ERROR(Debug, log("TLS post connection check failure"))
            return -1;
          }

          state_ = SSL_CONNECTED;
          break;

        case SSL_ERROR_WANT_READ: {
          ZS_LOG_TRACE(log("continue ssl wants read"))

          struct timeval timeout;
          if (DTLSv1_get_timeout(ssl_, &timeout)) {
            Milliseconds delay(zsLib::toMilliseconds(Seconds(timeout.tv_sec)) + zsLib::toMilliseconds(Microseconds(timeout.tv_usec)));

            auto outer = mOuter.lock();
            if (!outer) {
              ZS_LOG_WARNING(Debug, log("outer gone"))
              return -1;
            }

            if (mTimer) {
              mTimer->cancel();
              mTimer.reset();
            }

            mTimer = outer->adapterCreateTimeout(delay);
          }
          break;
        }

        case SSL_ERROR_WANT_WRITE:
          ZS_LOG_TRACE(log("continue ssl wants write"))
          break;

        case SSL_ERROR_ZERO_RETURN:
        default:
          ZS_LOG_ERROR(Debug, log("error") + ZS_PARAM("code", code))
          return (ssl_error != 0) ? ssl_error : -1;
      }

      return 0;
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::error(
                                       const char* context,
                                       int err,
                                       bool signal
                                       ) {
      ZS_LOG_ERROR(Debug, log("error") + ZS_PARAM("context", context) + ZS_PARAM("error", err) + ZS_PARAM("signal", signal))

      state_ = SSL_ERROR;
      ssl_error_code_ = err;
      cleanup();
      if (signal) {
        //StreamAdapterInterface::OnEvent(stream(), SE_CLOSE, err);
      }
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::cleanup() {
      ZS_LOG_DEBUG(log("cleanup"))

      if (state_ != SSL_ERROR) {
        state_ = SSL_CLOSED;
        ssl_error_code_ = 0;
      }

      if (ssl_) {
        int ret = SSL_shutdown(ssl_);
        if (ret < 0) {
          ZS_LOG_WARNING(Debug, log("SSL_shutdown failed") + ZS_PARAM("error", SSL_get_error(ssl_, ret)))
        }

        SSL_free(ssl_);
        ssl_ = NULL;
      }
      if (ssl_ctx_) {
        SSL_CTX_free(ssl_ctx_);
        ssl_ctx_ = NULL;
      }

      identity_.reset();

      if (peer_certificate_) {
        X509_free(peer_certificate_);
        peer_certificate_ = NULL;
      }

      if (mTimer) {
        mTimer->cancel();
        mTimer.reset();
      }
    }


    //-------------------------------------------------------------------------
    void DTLSTransport::Adapter::onTimer(ITimerPtr timer)
    {
      if (!mTimer) return;
      if (timer != mTimer) {
        ZS_LOG_WARNING(Trace, log("notified about obselete timer (probably okay)") + ZS_PARAM("timer id", timer->getID()))
        return;
      }

      ZS_LOG_TRACE(log("dtls timeout"))

      DTLSv1_handle_timeout(ssl_);
      continueSSL();
    }

    //-------------------------------------------------------------------------
    SSL_CTX* DTLSTransport::Adapter::setupSSLContext()
    {
      SSL_CTX *ctx = NULL;

      ctx = SSL_CTX_new(ssl_mode_ == SSL_MODE_DTLS ? DTLS_method() : TLS_method());
      // Version limiting for BoringSSL will be done below.

      if (ctx == NULL)
        return NULL;

      SSL_CTX_set_min_version(ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_VERSION : TLS1_VERSION);
      switch (ssl_max_version_) {
        case SSL_PROTOCOL_TLS_10:
          SSL_CTX_set_max_version(ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_VERSION : TLS1_VERSION);
          break;
        case SSL_PROTOCOL_TLS_11:
          SSL_CTX_set_max_version(ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_VERSION : TLS1_1_VERSION);
          break;
        case SSL_PROTOCOL_TLS_12:
        default:
          SSL_CTX_set_max_version(ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_2_VERSION : TLS1_2_VERSION);
          break;
      }

      if (identity_) {
        if (SSL_CTX_use_certificate(ctx, identity_->getCertificate()) != 1 ||
            SSL_CTX_use_PrivateKey(ctx, identity_->getKeyPair()) != 1) {
          ZS_LOG_ERROR(Debug, log("configure key and certificate failure"))

          logSSLErrors("Configuring key and certificate");
          SSL_CTX_free(ctx);
          return NULL;
        }
      }

#ifdef _DEBUG
//      SSL_CTX_set_info_callback(ctx, DTLSTransport::Adapter::sllInfoCallback);
#endif

      int mode = SSL_VERIFY_PEER;
      if (client_auth_enabled()) {
        // Require a certificate from the client.
        // Note: Normally this is always true in production, but it may be disabled
        // for testing purposes (e.g. SSLAdapter unit tests).
        mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
      }

      SSL_CTX_set_verify(ctx, mode, sslVerifyCallback);
      SSL_CTX_set_verify_depth(ctx, 4);
      // Select list of available ciphers. Note that !SHA256 and !SHA384 only
      // remove HMAC-SHA256 and HMAC-SHA384 cipher suites, not GCM cipher suites
      // with SHA256 or SHA384 as the handshake hash.
      // This matches the list of SSLClientSocketOpenSSL in Chromium.
      SSL_CTX_set_cipher_list(ctx, "DEFAULT:!NULL:!aNULL:!SHA256:!SHA384:!aECDH:!AESGCM+AES256:!aPSK");

      if (!srtp_ciphers_.empty()) {
        if (SSL_CTX_set_tlsext_use_srtp(ctx, srtp_ciphers_.c_str())) {
          SSL_CTX_free(ctx);
          return NULL;
        }
      }

      return ctx;
    }

    //-------------------------------------------------------------------------
    int DTLSTransport::Adapter::sslVerifyCallback(int ok, X509_STORE_CTX* store)
    {
      // Get our SSL structure from the store
      SSL* ssl = reinterpret_cast<SSL*>(X509_STORE_CTX_get_ex_data(
                                                                   store,
                                                                   SSL_get_ex_data_X509_STORE_CTX_idx()));

      DTLSTransport::Adapter *pThis = reinterpret_cast<DTLSTransport::Adapter*>(SSL_get_app_data(ssl));

      X509* cert = X509_STORE_CTX_get_current_cert(store);
      int depth = X509_STORE_CTX_get_error_depth(store);
      
      // For now We ignore the parent certificates and verify the leaf against
      // the digest.
      //
      // TODO(jiayl): Verify the chain is a proper chain and report the chain to
      // |stream->peer_certificate_|, like what NSS does.
      if (depth > 0) {
        ZS_LOG_TRACE(pThis->log("ignored chained certificate at depth") + ZS_PARAM("depth", depth))
        return 1;
      }

      ZS_LOG_TRACE(pThis->log("accepted peer certificate (but will verify fully later)"))

      // Record the peer's certificate.
      pThis->peer_certificate_ = cert;
      X509_up_ref(pThis->peer_certificate_);

      return 1;
    }
    
    // This code is taken from the "Network Security with OpenSSL"
    // sample in chapter 5
    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::sslPostConnectionCheck(
                                                        SSL* ssl,
                                                        const char* server_name
                                                        )
    {
      ASSERT(server_name != NULL);
      bool ok;
#if 0
      if (server_name[0] != '\0') {  // traditional mode
        ok = OpenSSLAdapter::VerifyServerName(ssl, server_name, ignore_bad_cert());
        
        if (ok) {
          ok = (SSL_get_verify_result(ssl) == X509_V_OK ||
                custom_verification_succeeded_);
        }
      } else {  // peer-to-peer mode
#else 
      {
#endif //0
        ASSERT(peer_certificate_ != NULL);
        // no server name validation
        ok = true;
#if 0
      }
#else
      }
#endif //0

      if (!ok && ignore_bad_cert()) {
        ZS_LOG_ERROR(Debug, log("SSL_get_verify_result(ssl)") + ZS_PARAM("result", SSL_get_verify_result(ssl)))
        ZS_LOG_DEBUG(log("other DTLS post connection checks failed"))
        ok = true;
      }
      
      return ok;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::haveDtls() {
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::haveDtlsSrtp()
    {
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool DTLSTransport::Adapter::haveExporter()
    {
      return true;
    }
    
    //-------------------------------------------------------------------------
    String DTLSTransport::Adapter::getDefaultSslCipher(SSLProtocolVersion version)
    {
      switch (version) {
        case SSL_PROTOCOL_TLS_10:
        case SSL_PROTOCOL_TLS_11:
          return kDefaultSslCipher10;
        case SSL_PROTOCOL_TLS_12:
        default:
          if (EVP_has_aes_hardware()) {
            return kDefaultSslCipher12;
          } else {
            return kDefaultSslCipher12NoAesGcm;
          }
      }
    }

    //-------------------------------------------------------------------------
    ElementPtr DTLSTransport::Adapter::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::DTLSTransport::Adapter");

      IHelper::debugAppend(resultEl, "id", mID);
      IHelper::debugAppend(resultEl, "outer", (bool)(mOuter.lock()));

      IHelper::debugAppend(resultEl, "timer", mTimer ? mTimer->getID() : 0);

      IHelper::debugAppend(resultEl, "state", toString(state_));
      IHelper::debugAppend(resultEl, "role", toString(role_));
      IHelper::debugAppend(resultEl, "sll error code", ssl_error_code_);

      IHelper::debugAppend(resultEl, "sll read needs write", ssl_read_needs_write_);
      IHelper::debugAppend(resultEl, "sll write needs read", ssl_write_needs_read_);

      IHelper::debugAppend(resultEl, "sll", ssl_ ? true : false);
      IHelper::debugAppend(resultEl, "sll context", ssl_ctx_ ? true : false);

      IHelper::debugAppend(resultEl, "use certificate", UseCertificate::toDebug(identity_));

      IHelper::debugAppend(resultEl, "ssl server name", ssl_server_name_);

      IHelper::debugAppend(resultEl, "peer certificate", peer_certificate_ ? true : false);

      IHelper::debugAppend(resultEl, "custom verification succeeded", custom_verification_succeeded_);

      IHelper::debugAppend(resultEl, "srtp ciphers", srtp_ciphers_);

      IHelper::debugAppend(resultEl, "ssl mode", toString(ssl_mode_));

      IHelper::debugAppend(resultEl, "ssl max version", toString(ssl_max_version_));

      IHelper::debugAppend(resultEl, "client auth enabled", client_auth_enabled_);
      IHelper::debugAppend(resultEl, "ignore bad certificate", ignore_bad_cert_);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IDTLSTransportFactory &IDTLSTransportFactory::singleton()
    {
      return DTLSTransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr IDTLSTransportFactory::create(
                                                   IDTLSTransportDelegatePtr delegate,
                                                   IICETransportPtr iceTransport,
                                                   const CertificateList &certificates
                                                   )
    {
      if (this) {}
      return internal::DTLSTransport::create(delegate, iceTransport, certificates);
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark (helpers)
  #pragma mark

  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message)
  {
    return Log::Params(message, "ortc::IDTLSTransportTypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportTypes::States
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IDTLSTransportTypes::toString(States state)
  {
    switch (state) {
      case State_New:           return "new";
      case State_Connecting:    return "connecting";
      case State_Connected:     return "connected";
      case State_Closed:        return "closed";
      case State_Failed:        return "failed";
    }
    return "UNDEFINED";
  }


  //---------------------------------------------------------------------------
  IDTLSTransportTypes::States IDTLSTransportTypes::toState(const char *state) throw (InvalidParameters)
  {
    String str(state);
    for (IDTLSTransportTypes::States index = IDTLSTransportTypes::State_First; index <= IDTLSTransportTypes::State_Last; index = static_cast<IDTLSTransportTypes::States>(static_cast<std::underlying_type<IDTLSTransportTypes::States>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IDTLSTransportTypes::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return State_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportTypes::Roles
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IDTLSTransportTypes::toString(Roles state)
  {
    switch (state) {
      case Role_Auto:      return "auto";
      case Role_Client:    return "client";
      case Role_Server:    return "server";
    }
    return "UNDEFINED";
  }


  //---------------------------------------------------------------------------
  IDTLSTransportTypes::Roles IDTLSTransportTypes::toRole(const char *role) throw (InvalidParameters)
  {
    String str(role);
    for (IDTLSTransportTypes::Roles index = IDTLSTransportTypes::Role_First; index <= IDTLSTransportTypes::Role_Last; index = static_cast<IDTLSTransportTypes::Roles>(static_cast<std::underlying_type<IDTLSTransportTypes::Roles>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IDTLSTransportTypes::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return Role_First;
  }
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportTypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  IDTLSTransportTypes::Parameters::Parameters(ElementPtr elem)
  {
    if (!elem) return;

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("role"));
      if (str.hasData()) {
        try {
          mRole = IDTLSTransportTypes::toRole(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("role invalid") + ZS_PARAM("value", str))
        }
      }
    }

    {
      ElementPtr fingerprintsEl = elem->findFirstChildElement("fingerprints");
      if (fingerprintsEl) {
        ElementPtr fingerprintEl = fingerprintsEl->findFirstChildElement("fingerprint");
        while (fingerprintEl) {
          ICertificate::Fingerprint fingerprint(fingerprintEl);

          mFingerprints.push_back(fingerprint);

          fingerprintEl = fingerprintEl->findNextSiblingElement("fingerprint");
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IDTLSTransportTypes::Parameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "role", IDTLSTransportTypes::toString(mRole), false);

    if (mFingerprints.size() > 0) {
      ElementPtr fingerprintsEl = Element::create("fingerprints");

      for (auto iter = mFingerprints.begin(); iter != mFingerprints.end(); ++iter) {
        auto &fingerprint = (*iter);

        fingerprintsEl->adoptAsLastChild(fingerprint.createElement());
      }

      elem->adoptAsLastChild(fingerprintsEl);
    }

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  IDTLSTransportTypes::ParametersPtr IDTLSTransportTypes::Parameters::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(Parameters, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IDTLSTransportTypes::Parameters::toDebug() const
  {
    return createElement("ortc::IDTLSTransportTypes::Parameters");
  }

  //---------------------------------------------------------------------------
  String IDTLSTransportTypes::Parameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("IDTLSTransportTypes:Parameters:");
    hasher->update(toString(mRole));

    for (auto iter = mFingerprints.begin(); iter != mFingerprints.end(); ++iter) {
      auto fingerprint = (*iter);

      hasher->update(":");
      hasher->update(fingerprint.mAlgorithm);
      hasher->update(":");
      hasher->update(fingerprint.mValue);
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransport
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IDTLSTransport::toDebug(IDTLSTransportPtr transport)
  {
    return internal::DTLSTransport::toDebug(internal::DTLSTransport::convert(transport));
  }

  //---------------------------------------------------------------------------
  IDTLSTransportPtr IDTLSTransport::convert(IRTPTransportPtr rtpTransport)
  {
    return internal::DTLSTransport::convert(rtpTransport);
  }

  //---------------------------------------------------------------------------
  IDTLSTransportPtr IDTLSTransport::convert(IRTCPTransportPtr rtcpTransport)
  {
    return internal::DTLSTransport::convert(rtcpTransport);
  }
  //---------------------------------------------------------------------------
  IDTLSTransportPtr IDTLSTransport::create(
                                           IDTLSTransportDelegatePtr delegate,
                                           IICETransportPtr iceTransport,
                                           const CertificateList &certificates
                                           )
  {
    return internal::IDTLSTransportFactory::singleton().create(delegate, iceTransport, certificates);
  }


}
