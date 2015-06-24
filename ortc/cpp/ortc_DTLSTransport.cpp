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
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
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

#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;
#ifndef RTC_UNUSED
#define RTC_UNUSED(x) RtcUnused(static_cast<const void*>(&x))
#endif //RTC_UNUSED

  namespace internal
  {
    typedef DTLSTransport::StreamResult StreamResult;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

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
        return static_cast<int>(read);
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
        return static_cast<int>(written);
      } else if (result == DTLSTransport::SR_BLOCK) {
        BIO_set_retry_write(b);
      }
      return -1;
    }

    //-------------------------------------------------------------------------
    static int stream_puts(BIO* b, const char* str) {
      return stream_write(b, str, static_cast<int>(strlen(str)));
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
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IDTLSTransportForSettings::applyDefaults()
    {
      // Strength of generated keys. Those are RSA.
      UseSettings::setUInt(ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_DTLS_PACKETS, 1);

      UseSettings::setUInt(ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_RTP_PACKETS, 50);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportForDataTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IDTLSTransportForDataTransport::toDebug(ForDataTransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(DTLSTransport, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr DTLSTransport::getStats() const throw(InvalidStateError)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return PromiseWithStatsReportPtr();
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
                                 ICertificatePtr certificate
                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mICETransport(ICETransport::convert(iceTransport)),
      mCertificate(Certificate::convert(certificate)),
      mMaxPendingDTLSPackets(UseSettings::getUInt(ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_DTLS_PACKETS)),
      mMaxPendingRTPPackets(UseSettings::getUInt(ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_RTP_PACKETS))
    {
      ZS_LOG_DETAIL(debug("created"))

      mDefaultSubscription = mSubscriptions.subscribe(IDTLSTransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate), queue);
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::init()
    {
      UseICETransportPtr transport;

      // scope: setup generator
      {
        AutoRecursiveLock lock(*this);

        mAdaptor = AdapterPtr(make_shared<Adapter>(mThisWeak.lock()));
        mAdaptor->setIdentity(mCertificate);

        transport = mICETransport;

        mICETransportSubscription = transport->subscribe(mThisWeak.lock());

        auto fingerprints = mCertificate->fingerprints();
        if (fingerprints) {
          mLocalParams.mFingerprints = (*fingerprints);
        }
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
    DTLSTransportPtr DTLSTransport::convert(ForICETransportPtr object)
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
                                           ICertificatePtr certificate
                                           )
    {
      DTLSTransportPtr pThis(make_shared<DTLSTransport>(make_private {}, IORTCForInternal::queueORTC(), delegate, iceTransport, certificate));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
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
          delegate->onDTLSTransportStateChanged(pThis, mCurrentState);
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IICETransportPtr DTLSTransport::transport() const
    {
      AutoRecursiveLock lock(*this);
      return ICETransport::convert(mICETransport);
    }

    //-------------------------------------------------------------------------
    IDTLSTransportTypes::States DTLSTransport::getState() const
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
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return SecureByteBlockListPtr();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::start(const Parameters &remoteParameters) throw (
                                                                         InvalidStateError,
                                                                         InvalidParameters
                                                                         )
    {
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
            mFixedRole = true;
            mAdaptor->setServerRole();
            mAdaptor->startSSLWithPeer();
            break;
          }
          case IDTLSTransportTypes::Role_Server: {
            mFixedRole = true;
            mAdaptor->startSSLWithPeer();
            break;
          }
        }
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::stop()
    {
      ZS_LOG_DEBUG(log("stop called"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IDTLSTransportForDataTransport
    #pragma mark

    //-------------------------------------------------------------------------
    bool DTLSTransport::sendDataPacket(
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       )
    {
      ZS_LOG_TRACE(log("sending data packet") + ZS_PARAM("length", bufferLengthInBytes))

      UseICETransportPtr transport;
      PacketQueue packets;

      {
        AutoRecursiveLock lock(*this);
        if (!mICETransport) {
          ZS_LOG_WARNING(Debug, log("no ice transport is attached"))
          return false;
        }
        transport = mICETransport;

        if ((isShutdown()) ||
            (isShuttingDown())) {
          ZS_LOG_WARNING(Debug, log("cannot send data packet while shutdown/shutting down") + ZS_PARAM("packet length", bufferLengthInBytes))
          return false;
        }

        if (!isValidated()) {
          ZS_LOG_WARNING(Debug, log("cannot send data packets while stream is not validated") + ZS_PARAM("packet length", bufferLengthInBytes))
          return false;
        }

        ZS_THROW_BAD_STATE_IF(!mAdaptor)

        size_t written {};
        int error {};

        auto result = mAdaptor->write(buffer, bufferLengthInBytes, &written, &error);

        wakeUpIfNeeded();

        switch (result) {
          case SR_SUCCESS: {
            packets = mPendingOutgoingDTLS;
            mPendingOutgoingDTLS = PacketQueue();
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
        while (packets.size() > 0) {
          SecureByteBlockPtr packet = packets.front();
          if (!transport->sendPacket(*packet, packet->SizeInBytes())) return false;
          packets.pop();
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IDTLSTransportForRTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    bool DTLSTransport::sendPacket(
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   )
    {
      ZS_LOG_TRACE(log("sending rtp packet") + ZS_PARAM("length", bufferLengthInBytes))

      UseICETransportPtr transport;

      {
        AutoRecursiveLock lock(*this);
        if (!mICETransport) {
          ZS_LOG_WARNING(Debug, log("no ice transport is attached"))
          return false;
        }

        transport = mICETransport;

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

      // WARNING: Best to not send packet to ice transport inside an object lock
      return transport->sendPacket(buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IDTLSTransportForICETransport
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::handleReceivedPacket(
                                             IICETypes::Components viaComponent,
                                             const BYTE *buffer,
                                             size_t bufferLengthInBytes
                                             )
    {
      ZS_LOG_TRACE(log("handle receive packet") + ZS_PARAM("length", bufferLengthInBytes))

      SecureByteBlockPtr decryptedPacket;

      // scope: pre-validation check
      {
        AutoRecursiveLock lock(*this);

        if ((isShutdown())) {
          ZS_LOG_WARNING(Debug, log("received packet after already shutdown (thus discarding)") + ZS_PARAM("buffer length", bufferLengthInBytes))
          return;
        }

        if (isShuttingDown()) {
          if (isRtpPacket(buffer, bufferLengthInBytes)) {
            ZS_LOG_WARNING(Debug, log("received RTP packet after shutting down (thus discarding)") + ZS_PARAM("buffer length", bufferLengthInBytes))
            return;
          }
        }

        ZS_THROW_BAD_STATE_IF(!mAdaptor)

        if (isDtlsPacket(buffer, bufferLengthInBytes)) {
          // Sanity check we're not passing junk that
          // just looks like DTLS.
          const BYTE * tmp_data = buffer;
          size_t tmp_size = bufferLengthInBytes;
          while (tmp_size > 0) {
            if (tmp_size < kDtlsRecordHeaderLen) {
              ZS_LOG_WARNING(Trace, log("too short for the DTLS header") + ZS_PARAM("buffer length", bufferLengthInBytes) + ZS_PARAM("tmp size", tmp_size))
              return;
            }

            size_t record_len = (tmp_data[11] << 8) | (tmp_data[12]);
            if ((record_len + kDtlsRecordHeaderLen) > tmp_size) {
              ZS_LOG_WARNING(Trace, log("DTLS body is too short") + ZS_PARAM("buffer length", bufferLengthInBytes) + ZS_PARAM("record len", record_len) + ZS_PARAM("tmp size", tmp_size))
              return;
            }

            tmp_data += record_len + kDtlsRecordHeaderLen;
            tmp_size -= record_len + kDtlsRecordHeaderLen;
          }

          mPendingIncomingDTLS.push(SecureByteBlockPtr(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes)));
          if (mPendingIncomingDTLS.size() > mMaxPendingDTLSPackets) {
            ZS_LOG_WARNING(Debug, log("too many pending dtls packets (thus popping first packet)"))
            mPendingIncomingDTLS.pop();
          }

          if (!mFixedRole) {
            mFixedRole = true;
            mAdaptor->setServerRole();
            mAdaptor->startSSLWithPeer();
          }

          BYTE extractedBuffer[kMaxDtlsPacketLen] {};

          size_t read = 0;
          int error = 0;
          auto result = mAdaptor->read(extractedBuffer, sizeof(extractedBuffer), &read, &error);

          wakeUpIfNeeded();

          switch (result) {
            case SR_SUCCESS: {
              decryptedPacket = SecureByteBlockPtr(make_shared<SecureByteBlock>(extractedBuffer, read));
              goto handle_data_packet;
            }
            case SR_BLOCK: {
              ZS_LOG_TRACE(log("dtls packet consumed"))
              return;
            }
            case SR_EOS:  {
              ZS_LOG_DEBUG(log("end of stream reached (thus shutting down)"))
              cancel();
              return;
            }
            case SR_ERROR: {
              ZS_LOG_ERROR(Debug, log("read error found (thus shutting down)") + ZS_PARAM("error code", error))
              cancel();
              return;
            }
          }

          return;
        }

        if (!isRtpPacket(buffer, bufferLengthInBytes)) {
           ZS_LOG_WARNING(Debug, log("received non DTLS nor RTP packet (thus discarding)") + ZS_PARAM("buffer length", bufferLengthInBytes))
          return;
        }

        if (!isValidated()) {
          ZS_LOG_TRACE(log("transport not verified thus pushing RTP packet onto pending queue") + ZS_PARAM("buffer length", bufferLengthInBytes))
          mPendingIncomingRTP.push(SecureByteBlockPtr(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes)));
          if (mPendingIncomingRTP.size() > mMaxPendingRTPPackets) {
            ZS_LOG_WARNING(Debug, log("too many pending rtp packets (thus popping first packet)"))
            mPendingIncomingDTLS.pop();
          }
          return;
        }

        goto handle_rtp;
      }

      // WARNING: Forward packet to data channel or RTP listener outside of object lock
    handle_rtp:
      {
#define TODO_FORWARD_TO_RTP_LISTENER 1
#define TODO_FORWARD_TO_RTP_LISTENER 2
        return;
      }

    handle_data_packet:
      {
#define TODO_FORWARD_TO_DATACHANNEL 1
#define TODO_FORWARD_TO_DATACHANNEL 2
        return;
      }
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::handleReceivedSTUNPacket(
                                                 IICETypes::Components viaComponent,
                                                 STUNPacketPtr packet
                                                 )
    {
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
    void DTLSTransport::onTimer(TimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
      mAdaptor->onTimer(timer);

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
        while (packets.size() > 0) {
          SecureByteBlockPtr packet = packets.front();
          if (!transport->sendPacket(*packet, packet->SizeInBytes())) return;
          packets.pop();
        }
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
    void DTLSTransport::onICETransportStateChanged(
                                                   IICETransportPtr transport,
                                                   IICETransport::States state
                                                   )
    {
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
      if (!mAdaptor) {
        if (error) *error = SSL_ERROR_ZERO_RETURN;
        return SR_ERROR;
      }

      return mAdaptor->bioRead(data, data_len, read, error);
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
      if (!mAdaptor) {
        if (error) *error = SSL_ERROR_ZERO_RETURN;
        return SR_ERROR;
      }

      return mAdaptor->bioWrite(data, data_len, written, error);
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
      mPendingOutgoingDTLS.push(SecureByteBlockPtr(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes)));

      IDTLSTransportAsyncDelegateProxy::create(mThisWeak.lock())->onAdapterSendPacket();
    }

    //-------------------------------------------------------------------------
    SecureByteBlockPtr DTLSTransport::adapterReadPacket()
    {
      if (mPendingIncomingDTLS.size() < 1) return SecureByteBlockPtr();

      SecureByteBlockPtr result = mPendingIncomingDTLS.front();
      mPendingIncomingDTLS.pop();
      return result;
    }

    //-------------------------------------------------------------------------
    TimerPtr DTLSTransport::adapterCreateTimeout(Milliseconds timeout)
    {
      return Timer::create(mThisWeak.lock(), zsLib::now() + timeout);
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
      UseServicesHelper::debugAppend(objectEl, "id", mID);
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

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", IDTLSTransport::toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "ice transport", mICETransport ? mICETransport->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "certificate", UseCertificate::toDebug(mCertificate));

      return resultEl;
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
      return State_Closed == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::step()
    {
      ZS_LOG_DEBUG(debug("step") + toDebug())

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

#define TODO_GET_INTO_VALIDATED_STATE 1
#define TODO_GET_INTO_VALIDATED_STATE 2

      // ... other steps here ...
      if (!stepStartSSL()) goto not_ready;
      if (!stepValidate()) goto not_ready;
      if (!stepFixState()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(log("dtls is not ready") + toDebug())
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
            case IICETypes::Role_Controlling: mAdaptor->setServerRole(); break;
            case IICETypes::Role_Controlled:  break;
          }

          mFixedRole = true;
          mAdaptor->startSSLWithPeer();
          return true;
        }
        default: {
#define WARNING_SHOULD_THIS_AUTO_STOP_EVEN_IN_FAILED_STATE 1
#define WARNING_SHOULD_THIS_AUTO_STOP_EVEN_IN_FAILED_STATE 2
          break;
        }
      }

      ZS_LOG_DEBUG(log("ice transport is not connected"))
      return false;
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::stepValidate()
    {
      if (Adapter::VALIDATION_NA != mValidation) {
        ZS_LOG_TRACE(log("validation already completed"))
        return true;
      }

      if (mRemoteParams.mFingerprints.size() < 1) {
        ZS_LOG_TRACE(log("waiting for remote parameters"))
        return true;
      }

      X509 *peerCert = NULL;
      if (!mAdaptor->getPeerCertificate(&peerCert)) {
        ZS_LOG_TRACE(log("do not have a peer certificate yet"))
        return true;
      }

      ZS_LOG_DEBUG(log("validating"))

      bool passed = false;

      for (auto iter = mRemoteParams.mFingerprints.begin(); iter != mRemoteParams.mFingerprints.end(); ++iter)
      {
        auto fingerprint = (*iter);

        auto result = mAdaptor->setPeerCertificateDigest(fingerprint.mAlgorithm, fingerprint.mValue);
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
      auto state = mAdaptor->getState();
      switch (state) {
        case Adapter::SS_OPENING: {
          setState(IDTLSTransport::State_Connecting);
          break;
        }
        case Adapter::SS_OPEN: {
          if (Adapter::VALIDATION_PASSED == mValidation) {
            setState(IDTLSTransport::State_Validated);
            return true;
          }
          setState(IDTLSTransport::State_Connected);
          break;
        }
        default: {
          ZS_LOG_WARNING(Debug, log("ssl connection was closed"))
          cancel();
          break;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_DTLS_SESSION_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_DTLS_SESSION_IS_SHUTDOWN 2
      }

      //.......................................................................
      // final cleanup

      setState(State_Closed);

      mAdaptor->close();

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
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", IDTLSTransport::toString(state)) + ZS_PARAM("old state", IDTLSTransport::toString(mCurrentState)))

      mCurrentState = state;

      DTLSTransportPtr pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onDTLSTransportStateChanged(pThis, mCurrentState);
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

      mLastError = errorCode;
      mLastErrorReason = reason;

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::wakeUpIfNeeded()
    {
      switch (mCurrentState) {
        case IDTLSTransportTypes::State_New:        {
          if (!mFixedRole) return;
          break;
        }
        case IDTLSTransportTypes::State_Connecting: {
          if (Adapter::SS_OPENING == mAdaptor->getState()) return;
          break;
        }
        case IDTLSTransportTypes::State_Connected:  {
          if (Adapter::SS_OPEN == mAdaptor->getState()) return;
          if (mRemoteParams.mFingerprints.size() < 1) return;
          break;
        }
        case IDTLSTransportTypes::State_Validated:  {
          if (Adapter::SS_OPEN == mAdaptor->getState()) return;
          break;
        }
        case IDTLSTransportTypes::State_Closed:     return;
      }

      ZS_LOG_TRACE(log("need to wake up to adjust state"))
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportFactory::Adapter
    #pragma mark

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
      ASSERT(!peer_certificate_);
      ASSERT(ssl_server_name_.empty());

      SecureByteBlockPtr digest = UseCertificate::getDigest(digest_alg, peer_certificate_);
      if (!digest) return VALIDATION_NA;

      String temp(digest_value);
      temp.replaceAll(":", "");
      temp.trim();

      SecureByteBlockPtr fingerprint = UseServicesHelper::convertFromHex(temp);
      if (!fingerprint) return VALIDATION_NA;

      if (0 != UseServicesHelper::compare(*digest, *fingerprint)) return VALIDATION_FAILED;

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
    bool DTLSTransport::Adapter::setDtlsSrtpCiphers(
                                                    const std::vector<String> &ciphers
                                                    )
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
    bool DTLSTransport::Adapter::getDtlsSrtpCipher(String *cipher) {
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
        auto buffer = outer->adapterReadPacket();
        if (!buffer) break;

        if (buffer->SizeInBytes() > data_len) {
          ZS_LOG_WARNING(Debug, log("dropped dtls packet that is too large") + ZS_PARAM("buffer size", buffer->SizeInBytes()) + ZS_PARAM("read size", data_len))
          continue;
        }

        memcpy(data, buffer->BytePtr(), buffer->SizeInBytes());
        if (read) *read = buffer->SizeInBytes();
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

      int code = SSL_write(ssl_, data, static_cast<int>(data_len));
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
        case SSL_CONNECTING:
          return SR_BLOCK;

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

      int code = SSL_read(ssl_, data, static_cast<int>(data_len));
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
      UseServicesHelper::debugAppend(objectEl, "id", mID);
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

            outer->adapterCreateTimeout(delay);
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
    void DTLSTransport::Adapter::onTimer(TimerPtr timer)
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
      }

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
                                                   ICertificatePtr certificate
                                                   )
    {
      if (this) {}
      return internal::DTLSTransport::create(delegate, iceTransport, certificate);
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IDTLSTransportTypes::toString(States state)
  {
    switch (state) {
      case State_New:           return "new";
      case State_Connecting:    return "connecting";
      case State_Connected:     return "connected";
      case State_Validated:     return "validated";
      case State_Closed:        return "closed";
    }
    return "UNDEFINED";
  }


  //---------------------------------------------------------------------------
  IDTLSTransportTypes::States IDTLSTransportTypes::toState(const char *state)
  {
    static States states[] = {
      State_New,
      State_Connecting,
      State_Connected,
      State_Validated,
      State_Closed,
    };

    String compareStr(state);
    for (size_t loop = 0; loop < (sizeof(states) / sizeof(states[0])); ++loop) {
      if (compareStr == toString(states[loop])) return states[loop];
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + compareStr)
    return State_Closed;
  }

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
  IDTLSTransportTypes::Roles IDTLSTransportTypes::toRole(const char *role)
  {
    static Roles roles[] = {
      Role_Auto,
      Role_Client,
      Role_Server,
    };

    String compareStr(role);
    for (size_t loop = 0; loop < (sizeof(roles) / sizeof(roles[0])); ++loop) {
      if (compareStr == toString(roles[loop])) return roles[loop];
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + compareStr)
    return Role_Auto;
  }
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTLSTransportTypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IDTLSTransportTypes::Parameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IDTLSTransportTypes::Parameters");

    UseServicesHelper::debugAppend(resultEl, "role", toString(mRole));
    UseServicesHelper::debugAppend(resultEl, "fingerprints", mFingerprints.size());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IDTLSTransportTypes::Parameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IDTLSTransportTypes:Parameters:");
    hasher.update(toString(mRole));

    for (auto iter = mFingerprints.begin(); iter != mFingerprints.end(); ++iter) {
      auto fingerprint = (*iter);

      hasher.update(":");
      hasher.update(fingerprint.mAlgorithm);
      hasher.update(":");
      hasher.update(fingerprint.mValue);
    }

    return hasher.final();
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
  IDTLSTransportPtr IDTLSTransport::create(
                                           IDTLSTransportDelegatePtr delegate,
                                           IICETransportPtr iceTransport,
                                           ICertificatePtr certificate
                                           )
  {
    return internal::IDTLSTransportFactory::singleton().create(delegate, iceTransport, certificate);
  }


}
