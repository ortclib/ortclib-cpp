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

#include <ortc/IDTLSTransport.h>
#include <ortc/IICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_ISRTPTransport.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>

#include <cryptopp/queue.h>

#include <openssl/base.h>

#include <queue>

#define ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_DTLS_BUFFER "ortc/dtls/max-pending-dtls-buffer"
#define ORTC_SETTING_DTLS_TRANSPORT_MAX_PENDING_RTP_PACKETS "ortc/dtls/max-pending-rtp-packets"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(ICertificateForDTLSTransport)
    ZS_DECLARE_INTERACTION_PTR(ISRTPTransportForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(IRTPListenerForSecureTransport)
    ZS_DECLARE_INTERACTION_PTR(IDataTransportForSecureTransport)

    ZS_DECLARE_INTERACTION_PTR(IDTLSTransportForSettings)

    ZS_DECLARE_INTERACTION_PROXY(IDTLSTransportAsyncDelegate)

    typedef struct ssl_st SSL;
    typedef struct ssl_ctx_st SSL_CTX;
    typedef struct x509_store_ctx_st X509_STORE_CTX;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportForSettings
    #pragma mark

    interaction IDTLSTransportForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IDTLSTransportForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportAsyncDelegate
    #pragma mark

    interaction IDTLSTransportAsyncDelegate
    {
      virtual void onAdapterSendPacket() = 0;
      virtual void onDeliverPendingIncomingRTP() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport
    #pragma mark
    
    class DTLSTransport : public Noop,
                          public MessageQueueAssociator,
                          public SharedRecursiveLock,
                          public IDTLSTransport,
                          public ISecureTransport,
                          public ISecureTransportForRTPSender,
                          public ISecureTransportForRTPReceiver,
                          public ISecureTransportForICETransport,
                          public ISecureTransportForSRTPTransport,
                          public ISecureTransportForRTPListener,
                          public ISecureTransportForDataTransport,
                          public IDTLSTransportForSettings,
                          public IWakeDelegate,
                          public zsLib::ITimerDelegate,
                          public IDTLSTransportAsyncDelegate,
                          public IICETransportDelegate,
                          public ISRTPTransportDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IDTLSTransport;
      friend interaction IDTLSTransportFactory;
      friend interaction ISecureTransportForRTPSender;
      friend interaction ISecureTransportForRTPReceiver;
      friend interaction ISecureTransportForICETransport;
      friend interaction ISecureTransportForRTPListener;
      friend interaction ISecureTransportForSRTPTransport;
      friend interaction ISecureTransportForDataTransport;
      friend interaction IDTLSTransportForSettings;

      typedef IDTLSTransport::States States;

      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportTypes::CertificateList, CertificateList)

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForSecureTransport, UseICETransport)
      ZS_DECLARE_TYPEDEF_PTR(ICertificateForDTLSTransport, UseCertificate)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPTransportForSecureTransport, UseSRTPTransport)
      ZS_DECLARE_TYPEDEF_PTR(IRTPListenerForSecureTransport, UseRTPListener)
      ZS_DECLARE_TYPEDEF_PTR(IDataTransportForSecureTransport, UseDataTransport)

      ZS_DECLARE_TYPEDEF_PTR(std::list<UseCertificatePtr>, UseCertificateList)

      ZS_DECLARE_CLASS_PTR(Adapter)

      enum StreamResult { SR_ERROR, SR_SUCCESS, SR_BLOCK, SR_EOS };

      typedef CryptoPP::ByteQueue ByteQueue;
      typedef std::queue<SecureByteBlockPtr> PacketQueue;

      typedef std::list<PromisePtr> PromiseList;

    public:
      DTLSTransport(
                    const make_private &,
                    IMessageQueuePtr queue,
                    IDTLSTransportDelegatePtr delegate,
                    IICETransportPtr iceTransport,
                    const CertificateList &certificates
                    );

    protected:
      DTLSTransport(
                    Noop,
                    IMessageQueuePtr queue = IMessageQueuePtr()
                    ) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~DTLSTransport();

      static DTLSTransportPtr convert(IDTLSTransportPtr object);
      static DTLSTransportPtr convert(ForDataTransportPtr object);
      static DTLSTransportPtr convert(ForRTPSenderPtr object);
      static DTLSTransportPtr convert(ForRTPReceiverPtr object);
      static DTLSTransportPtr convert(ForICETransportPtr object);
      static DTLSTransportPtr convert(ForSRTPPtr object);
      static DTLSTransportPtr convert(ForRTPListenerPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransport
      #pragma mark

      static ElementPtr toDebug(DTLSTransportPtr transport);

      static DTLSTransportPtr create(
                                     IDTLSTransportDelegatePtr delegate,
                                     IICETransportPtr iceTransport,
                                     const CertificateList &certificate
                                     );

      static DTLSTransportPtr convert(IRTPTransportPtr rtpTransport);
      static DTLSTransportPtr convert(IRTCPTransportPtr rtcpTransport);

      virtual PUID getID() const override {return mID;}

      virtual IDTLSTransportSubscriptionPtr subscribe(IDTLSTransportDelegatePtr delegate) override;

      virtual CertificateListPtr certificates() const override;
      virtual IICETransportPtr transport() const override;

      virtual States state() const override;

      virtual ParametersPtr getLocalParameters() const override;
      virtual ParametersPtr getRemoteParameters() const override;

      virtual SecureByteBlockListPtr getRemoteCertificates() const override;

      virtual void start(const Parameters &remoteParameters) throw (
                                                                    InvalidStateError,
                                                                    InvalidParameters
                                                                    ) override;

      virtual void stop() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ISecureTransport
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ISecureTransportForRTPSender
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      // (duplicate) virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) = 0;

      // (duplicate) virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored = ISecureTransportTypes::States()) const = 0;

      virtual bool sendPacket(
                              IICETypes::Components sendOverICETransport,
                              IICETypes::Components packetType,
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) override;

      virtual IICETransportPtr getICETransport() const override;


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ISecureTransportForRTPReceiver
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      // (duplicate) virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) = 0;

      // (duplicate) virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored = ISecureTransportTypes::States()) const = 0;

      // (duplicate) virtual bool sendPacket(
      //                                     IICETypes::Components sendOverICETransport,
      //                                     IICETypes::Components packetType,
      //                                     const BYTE *buffer,
      //                                     size_t bufferLengthInBytes
      //                                     ) override;

      // (duplicate) virtual IICETransportPtr getICETransport() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ISecureTransportForICETransport
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual void notifyAssociateTransportCreated(
                                                   IICETypes::Components associatedComponent,
                                                   ICETransportPtr assoicated
                                                   ) override;

      virtual bool handleReceivedPacket(
                                        IICETypes::Components viaTransport,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) override;
      virtual void handleReceivedSTUNPacket(
                                            IICETypes::Components viaComponent,
                                            STUNPacketPtr packet
                                            ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ISecureTransportForSRTPTransport
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForSRTPPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual bool sendEncryptedPacket(
                                       IICETypes::Components sendOverICETransport,
                                       IICETypes::Components packetType,
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       ) override;

      virtual bool handleReceivedDecryptedPacket(
                                                 IICETypes::Components viaTransport,
                                                 IICETypes::Components packetType,
                                                 const BYTE *buffer,
                                                 size_t bufferLengthInBytes
                                                 ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ISecureTransportForRTPListener
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPListenerPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual RTPListenerPtr getListener() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ISecureTransportForDataTransport
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForDataTransport transport);

      // (duplicate) virtual PUID getID() const = 0;

      virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) override;

      virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored) const override;

      virtual bool isClientRole() const override;

      virtual UseDataTransportPtr getDataTransport() const override;

      virtual bool sendDataPacket(
                                  const BYTE *buffer,
                                  size_t bufferLengthInBytes
                                  ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransportAsyncDelegate
      #pragma mark

      virtual void onAdapterSendPacket() override;
      virtual void onDeliverPendingIncomingRTP() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IICETransportDelegate
      #pragma mark

      virtual void onICETransportStateChange(
                                             IICETransportPtr transport,
                                             IICETransport::States state
                                             ) override;

      virtual void onICETransportCandidatePairAvailable(
                                                        IICETransportPtr transport,
                                                        CandidatePairPtr candidatePair
                                                        ) override;
      virtual void onICETransportCandidatePairGone(
                                                   IICETransportPtr transport,
                                                   CandidatePairPtr candidatePair
                                                   ) override;
      virtual void onICETransportCandidatePairChanged(
                                                      IICETransportPtr transport,
                                                      CandidatePairPtr candidatePair
                                                      ) override;


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => ISRTPTransportDelegate
      #pragma mark

      virtual void onSRTPTransportLifetimeRemaining(
                                                    ISRTPTransportPtr transport,
                                                    ULONG leastLifetimeRemainingPercentageForAllKeys,
                                                    ULONG overallLifetimeRemainingPercentage
                                                    ) override;

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => friend BIO_
      #pragma mark

      StreamResult bioRead(
                           void* data,
                           size_t data_len,
                           size_t* read,
                           int* error
                           );

      StreamResult bioWrite(
                            const void* data,
                            size_t data_len,
                            size_t* written,
                            int* error
                            );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => friend DTLSTransport::Adapter
      #pragma mark

      void adapterSendPacket(
                             const BYTE *buffer,
                             size_t bufferLengthInBytes
                             );

      size_t adapterReadPacket(BYTE *buffer, size_t bufferLengthInBytes);

      ITimerPtr adapterCreateTimeout(Milliseconds timeout);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      IICETypes::Components component() const;

      bool isValidated() const {return IDTLSTransportTypes::State_Connected == mCurrentState;}

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepStartSSL();
      bool stepValidate();
      bool stepFixState();
      bool stepNotifyReady();

      void cancel();

      void setState(IDTLSTransportTypes::States state);
      void setError(WORD error, const char *reason = NULL);

      void setState(ISecureTransportTypes::States state);

      void wakeUpIfNeeded();

      void setupSRTP();

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport::Adapter
      #pragma mark

      class Adapter
      {
      public:
        enum StreamState { SS_CLOSED, SS_OPENING, SS_OPEN };

        enum SSLRole { SSL_CLIENT, SSL_SERVER };
        enum SSLMode { SSL_MODE_TLS, SSL_MODE_DTLS };
        enum SSLProtocolVersion {
          SSL_PROTOCOL_TLS_10,
          SSL_PROTOCOL_TLS_11,
          SSL_PROTOCOL_TLS_12,
          SSL_PROTOCOL_DTLS_10 = SSL_PROTOCOL_TLS_11,
          SSL_PROTOCOL_DTLS_12 = SSL_PROTOCOL_TLS_12,
        };
        enum { SSE_MSG_TRUNC = 0xff0001 };
        enum Validation {VALIDATION_NA, VALIDATION_PASSED, VALIDATION_FAILED};

        static const char *toString(StreamState state);
        static const char *toString(SSLRole role);
        static const char *toString(SSLMode mode);
        static const char *toString(SSLProtocolVersion version);
        static const char *toString(Validation validation);

      public:
        Adapter(DTLSTransportPtr outer);
        ~Adapter();

        void setIdentity(UseCertificatePtr identity);

        // Default argument is for compatibility
        SSLRole role() const {return role_;}
        void setServerRole(SSLRole role = SSL_SERVER);

        bool getPeerCertificate(X509 **cert) const;

        Validation setPeerCertificateDigest(
                                            const String &digest_alg,
                                            const String &digest_value
                                            );

        int startSSLWithServer(const char *server_name);
        int startSSLWithPeer();
        void setMode(SSLMode mode);
        void setMaxProtocolVersion(SSLProtocolVersion version);

        StreamResult bioRead(
                             void* data,
                             size_t data_len,
                             size_t* read,
                             int* error
                             );
        StreamResult bioWrite(
                              const void* data,
                              size_t data_len,
                              size_t* written,
                              int* error
                              );

        StreamResult read(
                          void* data,
                          size_t data_len,
                          size_t* read,
                          int* error
                          );
        StreamResult write(
                           const void* data,
                           size_t data_len,
                           size_t* written,
                           int* error
                           );
        void close();
        StreamState getState() const;

        bool getSslCipher(String * cipher);

        // Key Extractor interface
        bool exportKeyingMaterial(
                                  const String &label,
                                  const BYTE *context,
                                  size_t context_len,
                                  bool use_context,
                                  BYTE *result,
                                  size_t result_len
                                  );

        // DTLS-SRTP interface
        bool setDtlsSrtpCiphers(const std::vector<String> &ciphers);
        bool getDtlsSrtpCipher(String * cipher);

        // Capabilities interfaces
        static bool haveDtls();
        static bool haveDtlsSrtp();
        static bool haveExporter();
        static String getDefaultSslCipher(SSLProtocolVersion version);

        // debug helpers
        ElementPtr toDebug() const;

        // Override MessageHandler
        void onTimer(ITimerPtr timer);

      protected:
        void onEvent(int events, int err);

        Log::Params log(const char *message) const;
        void logSSLErrors(const String &prefix);

        void set_client_auth_enabled(bool enabled) { client_auth_enabled_ = enabled; }
        bool client_auth_enabled() const { return client_auth_enabled_; }

        void set_ignore_bad_cert(bool ignore) { ignore_bad_cert_ = ignore; }
        bool ignore_bad_cert() const { return ignore_bad_cert_; }

      private:
        enum SSLState {
          // Before calling one of the startSSL methods, data flows
          // in clear text.
          SSL_NONE,
          SSL_WAIT,       // waiting for the stream to open to start SSL negotiation
          SSL_CONNECTING, // SSL negotiation in progress
          SSL_CONNECTED,  // SSL stream successfully established
          SSL_ERROR,      // some SSL error occurred, stream is closed
          SSL_CLOSED      // Clean close
        };

        static const char *toString(SSLState state);

        // The following three methods return 0 on success and a negative
        // error code on failure. The error code may be from OpenSSL or -1
        // on some other error cases, so it can't really be interpreted
        // unfortunately.

        // Go from state SSL_NONE to either SSL_CONNECTING or SSL_WAIT,
        // depending on whether the underlying stream is already open or
        // not.
        int startSSL();
        // Prepare SSL library, state is SSL_CONNECTING.
        int beginSSL();
        // Perform SSL negotiation steps.
        int continueSSL();

        // Error handler helper. signal is given as true for errors in
        // asynchronous contexts (when an error method was not returned
        // through some other method), and in that case an SE_CLOSE event is
        // raised on the stream with the specified error.
        // A 0 error means a graceful close, otherwise there is not really enough
        // context to interpret the error code.
        void error(const char* context, int err, bool signal);
        void cleanup();

        // Flush the input buffers by reading left bytes (for DTLS)
        void flushInput(unsigned int left);

        // SSL library configuration
        SSL_CTX* setupSSLContext();
        // SSL verification check
        bool sslPostConnectionCheck(
                                    SSL* ssl,
                                    const char *server_name
                                    );
        // SSL certification verification error handler, called back from
        // the openssl library. Returns an int interpreted as a boolean in
        // the C style: zero means verification failure, non-zero means
        // passed.
        static int sslVerifyCallback(
                                     int ok,
                                     X509_STORE_CTX* store
                                     );

      protected:
        AutoPUID mID;
        DTLSTransportWeakPtr mOuter;

        ITimerPtr mTimer;

        SSLState state_ {SSL_NONE};
        SSLRole role_ {SSL_CLIENT};
        int ssl_error_code_ {};  // valid when state_ == SSL_ERROR or SSL_CLOSED
        // Whether the SSL negotiation is blocked on needing to read or
        // write to the wrapped stream.
        bool ssl_read_needs_write_ {false};
        bool ssl_write_needs_read_ {false};

        SSL* ssl_ {NULL};
        SSL_CTX* ssl_ctx_ {NULL};

        // Our key and certificate, mostly useful in peer-to-peer mode.
        UseCertificatePtr identity_;
        // in traditional mode, the server name that the server's certificate
        // must specify. Empty in peer-to-peer mode.
        String ssl_server_name_;
        // The certificate that the peer must present or did present. Initially
        // null in traditional mode, until the connection is established.
        X509 *peer_certificate_ {NULL};

        // OpenSSLAdapter::custom_verify_callback_ result
        bool custom_verification_succeeded_ {false};
        
        // The DtlsSrtp ciphers
        String srtp_ciphers_;
        
        // Do DTLS or not
        SSLMode ssl_mode_ {SSL_MODE_DTLS};

        // Max. allowed protocol version
        SSLProtocolVersion ssl_max_version_ {SSL_PROTOCOL_DTLS_10};

        bool client_auth_enabled_ {true};
        bool ignore_bad_cert_ {false};
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => (data)
      #pragma mark

      AutoPUID mID;
      DTLSTransportWeakPtr mThisWeak;
      DTLSTransportPtr mGracefulShutdownReference;

      IDTLSTransportDelegateSubscriptions mSubscriptions;
      IDTLSTransportSubscriptionPtr mDefaultSubscription;

      std::atomic<IDTLSTransportTypes::States> mCurrentState {IDTLSTransportTypes::State_New};

      ISecureTransportDelegateSubscriptions mSecureTransportSubscriptions;
      std::atomic<ISecureTransport::States> mSecureTransportState {ISecureTransport::State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseICETransportPtr mICETransport;
      IICETransportSubscriptionPtr mICETransportSubscription;

      IICETypes::Components mComponent {IICETypes::Component_RTP};

      UseCertificateList mCertificates;

      Parameters mLocalParams;
      Parameters mRemoteParams;

      AdapterPtr mAdapter;

      size_t mMaxPendingDTLSBuffer {};
      size_t mMaxPendingRTPPackets {};

      bool mPutIncomingRTPIntoPendingQueue {true};
      PacketQueue mPendingIncomingRTP;
      ByteQueue mPendingIncomingDTLS;

      PacketQueue mPendingOutgoingDTLS;

      bool mFixedRole {false};

      Adapter::Validation mValidation {Adapter::VALIDATION_NA};

      UseSRTPTransportPtr mSRTPTransport;

      UseRTPListenerPtr mRTPListener;     // no lock needed
      UseDataTransportPtr mDataTransport; // no lock needed
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportFactory
    #pragma mark

    interaction IDTLSTransportFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportTypes::CertificateList, CertificateList)

      static IDTLSTransportFactory &singleton();

      virtual DTLSTransportPtr create(
                                      IDTLSTransportDelegatePtr delegate,
                                      IICETransportPtr iceTransport,
                                      const CertificateList &certificates
                                      );
    };

    class DTLSTransportFactory : public IFactory<IDTLSTransportFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IDTLSTransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::PromisePtr, PromisePtr)
ZS_DECLARE_PROXY_METHOD_0(onAdapterSendPacket)
ZS_DECLARE_PROXY_METHOD_0(onDeliverPendingIncomingRTP)
ZS_DECLARE_PROXY_END()
