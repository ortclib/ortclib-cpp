/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/ortc_SCTPTransport.h>
#include <ortc/internal/ortc_SCTPTransportListener.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_DataChannel.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <usrsctp.h>

//#include <netinet/sctp_os.h>
#ifdef _WIN32
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif  //_WIN32

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


// The expression ARRAY_SIZE(a) is a compile-time constant of type
// size_t which represents the number of elements of the given
// array. You should only use ARRAY_SIZE on statically allocated
// arrays.

#define ARRAY_SIZE(a)                               \
  ((sizeof(a) / sizeof(*(a))) /                     \
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  ZS_DECLARE_INTERACTION_TEAR_AWAY(ISCTPTransport, internal::SCTPTransport::TearAwayData)

  namespace internal
  {
    struct SCTPHelper;

    ZS_DECLARE_TYPEDEF_PTR(SCTPHelper, UseSCTPHelper)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    const uint32_t kMaxSctpSid = 1023;
    static const size_t kSctpMtu = 1200;

    enum PreservedErrno {
      SCTP_EINPROGRESS = EINPROGRESS,
      SCTP_EWOULDBLOCK = EWOULDBLOCK
    };

    //-------------------------------------------------------------------------
    const char *toString(SCTPPayloadProtocolIdentifier ppid)
    {
      switch (ppid) {
        case SCTP_PPID_NONE:              return "NONE";
        case SCTP_PPID_CONTROL:           return "CONTROL";
        case SCTP_PPID_BINARY_EMPTY:      return "BINARY_EMPTY";
        case SCTP_PPID_BINARY_PARTIAL:    return "BINARY_PARTIAL";
        case SCTP_PPID_BINARY_LAST:       return "BINARY_LAST";
        case SCTP_PPID_STRING_EMPTY:      return "STRING_EMPTY";
        case SCTP_PPID_STRING_PARTIAL:    return "STRING_PARTIAL";
        case SCTP_PPID_STRING_LAST:       return "STRING_LAST";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPPacketIncoming
    #pragma mark

    //---------------------------------------------------------------------------
    ElementPtr SCTPPacketIncoming::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::SCTPPacketIncoming");

      UseServicesHelper::debugAppend(resultEl, "type", toString(mType));
      UseServicesHelper::debugAppend(resultEl, "sessiond id", mSessionID);
      UseServicesHelper::debugAppend(resultEl, "sequence number", mSequenceNumber);
      UseServicesHelper::debugAppend(resultEl, "timestamp", mTimestamp);
      UseServicesHelper::debugAppend(resultEl, "flags", mFlags);
      UseServicesHelper::debugAppend(resultEl, "buffer", mBuffer ? mBuffer->SizeInBytes() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPPacketOutgoing
    #pragma mark

    //---------------------------------------------------------------------------
    ElementPtr SCTPPacketOutgoing::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::SCTPPacketOutgoing");

      UseServicesHelper::debugAppend(resultEl, "type", toString(mType));
      UseServicesHelper::debugAppend(resultEl, "sessiond id", mSessionID);
      UseServicesHelper::debugAppend(resultEl, "ordered", mOrdered);
      UseServicesHelper::debugAppend(resultEl, "max packet lifetime (ms)", mMaxPacketLifetime);
      UseServicesHelper::debugAppend(resultEl, "max retransmits", mMaxRetransmits);
      UseServicesHelper::debugAppend(resultEl, "buffer", mBuffer ? mBuffer->SizeInBytes() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPHelper
    #pragma mark

    struct SCTPHelper
    {
      //-----------------------------------------------------------------------
      static sockaddr_conn getAddress(
                                      WORD port,
                                      SCTPTransportWeakPtr *thisSocket
                                      )
      {
        sockaddr_conn sconn = {};
        sconn.sconn_family = AF_CONN;
#ifdef HAVE_SCONN_LEN
        sconn.sconn_len = sizeof(sockaddr_conn);
#endif
        // Note: conversion from int to uint16_t happens here.
        sconn.sconn_port = htons(port);
        sconn.sconn_addr = thisSocket;
        return sconn;
      }

      //-----------------------------------------------------------------------
      static Log::Params slog(const char *message)
      {
        ElementPtr objectEl = Element::create("ortc::SCTPHelper");
        return Log::Params(message, objectEl);
      }
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPInit
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPInit
    #pragma mark

    // code borrowed from:
    // https://chromium.googlesource.com/external/webrtc/+/master/talk/media/sctp/sctpdataengine.cc
    // https://chromium.googlesource.com/external/webrtc/+/master/talk/media/sctp/sctpdataengine.h

    class SCTPInit : public ISingletonManagerDelegate
    {
    public:
      friend class SCTPTransport;

    protected:
      struct make_private {};

    public:
      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, UseDataChannel)

    public:
      //-----------------------------------------------------------------------
      SCTPInit(const make_private &)
      {
        ZS_LOG_BASIC(log("created"))
      }

    protected:
      //-----------------------------------------------------------------------
      void init()
      {
        AutoRecursiveLock lock(mLock);

        // First argument is udp_encapsulation_port, which is not releveant for our
        // AF_CONN use of sctp.
        usrsctp_init(0, OnSctpOutboundPacket, debug_sctp_printf);

        // To turn on/off detailed SCTP debugging. You will also need to have the
        // SCTP_DEBUG cpp defines flag.
        // usrsctp_sysctl_set_sctp_debug_on(SCTP_DEBUG_ALL);
        // TODO(ldixon): Consider turning this on/off.
        usrsctp_sysctl_set_sctp_ecn_enable(0);
        // TODO(ldixon): Consider turning this on/off.
        // This is not needed right now (we don't do dynamic address changes):
        // If SCTP Auto-ASCONF is enabled, the peer is informed automatically
        // when a new address is added or removed. This feature is enabled by
        // default.
        // usrsctp_sysctl_set_sctp_auto_asconf(0);
        // TODO(ldixon): Consider turning this on/off.
        // Add a blackhole sysctl. Setting it to 1 results in no ABORTs
        // being sent in response to INITs, setting it to 2 results
        // in no ABORTs being sent for received OOTB packets.
        // This is similar to the TCP sysctl.
        //
        // See: http://lakerest.net/pipermail/sctp-coders/2012-January/009438.html
        // See: http://svnweb.freebsd.org/base?view=revision&revision=229805
        // usrsctp_sysctl_set_sctp_blackhole(2);
        // Set the number of default outgoing streams.  This is the number we'll
        // send in the SCTP INIT message.  The 'appropriate default' in the
        // second paragraph of
        // http://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-05#section-6.2
        // is cricket::kMaxSctpSid.
        usrsctp_sysctl_set_sctp_nr_outgoing_streams_default(static_cast<uint32_t>(UseSettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT)));

        mInitialized = true;
      }

      //-----------------------------------------------------------------------
      static SCTPInitPtr create()
      {
        SCTPInitPtr pThis(make_shared<SCTPInit>(make_private{}));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

    public:
      //-----------------------------------------------------------------------
      ~SCTPInit()
      {
        mThisWeak.reset();
        ZS_LOG_BASIC(log("destroyed"))
        cancel();
      }

      //-----------------------------------------------------------------------
      static SCTPInitPtr singleton()
      {
        static SingletonLazySharedPtr<SCTPInit> singleton(create());
        SCTPInitPtr result = singleton.singleton();

        static zsLib::SingletonManager::Register registerSingleton("openpeer::ortc::SCTPInit", result);

        if (!result) {
          ZS_LOG_WARNING(Detail, slog("singleton gone"))
        }
        
        return result;
      }

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPInit => ISingletonManagerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      virtual void notifySingletonCleanup() override
      {
        cancel();
      }

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPInit => usrscpt callbacks
      #pragma mark

      //-----------------------------------------------------------------------
      // This is the callback usrsctp uses when there's data to send on the network
      // that has been wrapped appropriatly for the SCTP protocol.
      static int OnSctpOutboundPacket(
                                      void* addr,
                                      void* data,
                                      size_t length,
                                      uint8_t tos,
                                      uint8_t set_df
                                      )
      {
        SCTPTransportPtr transport = (*(static_cast<SCTPTransportWeakPtr *>(addr))).lock();

        ZS_LOG_TRACE(slog("on sctp output backpet") + ZS_PARAM("address", ((PTRNUMBER)addr)) + ZS_PARAM("length", length) + ZS_PARAM("tos", tos) + ZS_PARAM("set_df", set_df))

        if (ZS_IS_LOGGING(Insane)) {
          String str = UseServicesHelper::getDebugString((const BYTE *)data, length);
          ZS_LOG_INSANE(slog("sctp outgoing packet") + ZS_PARAM("raw", "\n" + str))
        }

        if (!transport) {
          ZS_LOG_WARNING(Trace, slog("transport is gone (thus cannot send packet)") + ZS_PARAM("address", ((PTRNUMBER)addr)) + ZS_PARAM("length", length) + ZS_PARAM("tos", tos) + ZS_PARAM("set_df", set_df))
          errno = ESHUTDOWN;
          return -1;
        }

        bool result = transport->notifySendSCTPPacket((const BYTE *)data, length);
        if (!result) {
          ZS_LOG_WARNING(Trace, slog("transport is unable to send packet") + ZS_PARAM("address", ((PTRNUMBER)addr)) + ZS_PARAM("length", length) + ZS_PARAM("tos", tos) + ZS_PARAM("set_df", set_df))
          errno = EWOULDBLOCK;
          return -1;
        }

        return 0;
      }

      //-----------------------------------------------------------------------
      static void debug_sctp_printf(const char *format, ...)
      {
        char s[1024] {};

        va_list ap;
        va_start(ap, format);
#ifdef _WIN32
        vsnprintf_s(s, sizeof(s), format, ap);
#else
        vsnprintf(s, sizeof(s), format, ap);
#endif  //_WIN32

        ZS_LOG_BASIC(slog("debug") + ZS_PARAM("message", s))
        va_end(ap);
      }

      //-------------------------------------------------------------------------
      static int OnSctpInboundPacket(
                                     struct socket* sock,
                                     union sctp_sockstore addr,
                                     void* data,
                                     size_t length,
                                     struct sctp_rcvinfo rcv,
                                     int flags,
                                     void* ulp_info
                                     )
      {
        SCTPTransportPtr transport = (*(static_cast<SCTPTransportWeakPtr *>(ulp_info))).lock();

        const SCTPPayloadProtocolIdentifier ppid = static_cast<SCTPPayloadProtocolIdentifier>(ntohl(rcv.rcv_ppid));

        switch (ppid) {
          case SCTP_PPID_CONTROL:
          case SCTP_PPID_BINARY_EMPTY:
          case SCTP_PPID_BINARY_PARTIAL:
          case SCTP_PPID_BINARY_LAST:
          case SCTP_PPID_STRING_EMPTY:
          case SCTP_PPID_STRING_PARTIAL:
          case SCTP_PPID_STRING_LAST:
          {
            break;
          }
          case SCTP_PPID_NONE:
          default: {
            ZS_LOG_WARNING(Trace, slog("incoming protocol identifier type was not understood (dropping packet)") + ZS_PARAM("ppid", ppid))
            return 1;
          }
        }

        SCTPPacketIncomingPtr packet(make_shared<SCTPPacketIncoming>());

        packet->mType = ppid;
        packet->mSessionID = rcv.rcv_sid;
        packet->mSequenceNumber = rcv.rcv_ssn;
        packet->mTimestamp = rcv.rcv_tsn;
        packet->mFlags = flags;
        packet->mBuffer = UseServicesHelper::convertToBuffer((const BYTE *)data, length);

        if (!transport) {
          ZS_LOG_WARNING(Trace, slog("transport is gone (thus cannot receive packet)") + ZS_PARAM("socket", ((PTRNUMBER)sock)) + ZS_PARAM("length", length) + ZS_PARAM("flags", flags) + ZS_PARAM("ulp", ((PTRNUMBER)ulp_info)))
          errno = ESHUTDOWN;
          return -1;
        }

        ISCTPTransportAsyncDelegateProxy::create(transport)->onIncomingPacket(packet);
        return 1;
      }
      
    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPInit => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::SCTPInit");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      static Log::Params slog(const char *message)
      {
        return Log::Params(message, "ortc::SCTPInit");
      }

      //-----------------------------------------------------------------------
      Log::Params debug(const char *message) const
      {
        return Log::Params(message, toDebug());
      }

      //-----------------------------------------------------------------------
      virtual ElementPtr toDebug() const
      {
        AutoRecursiveLock lock(mLock);
        ElementPtr resultEl = Element::create("ortc::SCTPInit");

        UseServicesHelper::debugAppend(resultEl, "id", mID);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      void cancel()
      {
        ZS_LOG_DEBUG(log("cancel called"))

        bool initialized = mInitialized.exchange(false);
        if (!initialized) return;

        int count = 0;

        while ((0 != usrsctp_finish()) &&
               (count < 300))
        {
          std::this_thread::sleep_for(Milliseconds(10));
          ++count;
        }
      }

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPInit => (data)
      #pragma mark

      AutoPUID mID;
      mutable RecursiveLock mLock;
      SCTPInitWeakPtr mThisWeak;

      std::atomic<bool> mInitialized{ false };
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void ISCTPTransportForSettings::applyDefaults()
    {
      // http://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-05#section-6.2
      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT, kMaxSctpSid);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForDataChannel
    #pragma mark

    //-------------------------------------------------------------------------
    ISCTPTransportForSCTPTransportListener::ForListenerPtr ISCTPTransportForSCTPTransportListener::create(
                                                                                                          UseListenerPtr listener,
                                                                                                          UseSecureTransportPtr secureTransport,
                                                                                                          WORD localPort,
                                                                                                          WORD remotePort
                                                                                                          )
    {
      return ISCTPTransportFactory::singleton().create(listener, secureTransport, localPort, remotePort);
    }

    //-------------------------------------------------------------------------
    ElementPtr ISCTPTransportForDataChannel::toDebug(ForDataChannelPtr transport)
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *SCTPTransport::toString(States state)
    {
      switch (state) {
        case State_Pending:       return "pending";
        case State_Ready:         return "ready";
        case State_Disconnected:  return "disconnected";
        case State_ShuttingDown:  return "shutting down";
        case State_Shutdown:      return "shutdown";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    SCTPTransport::SCTPTransport(
                                 const make_private &,
                                 IMessageQueuePtr queue,
                                 UseListenerPtr listener,
                                 UseSecureTransportPtr secureTransport,
                                 WORD localPort,
                                 WORD remotePort
                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSCTPInit(SCTPInit::singleton()),
      mMaxSessionsPerPort(UseSettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT)),
      mListener(listener),
      mSecureTransport(secureTransport),
      mIncoming(0 != localPort),
      mLocalPort(localPort),
      mRemotePort(remotePort)
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!secureTransport)

      ZS_LOG_DETAIL(debug("created"))

      mSecureTransportReady = secureTransport->notifyWhenReady();
      mSecureTransportClosed = secureTransport->notifyWhenClosed();
      mICETransport = ICETransport::convert(secureTransport->getICETransport());

      ORTC_THROW_INVALID_STATE_IF(!mSCTPInit)
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::init()
    {
      AutoRecursiveLock lock(*this);
      //IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      
      ZS_LOG_DETAIL(debug("SCTP init"))

      mSecureTransportReady->thenWeak(mThisWeak.lock());
      mSecureTransportClosed->thenWeak(mThisWeak.lock());

      auto iceTransport = mICETransport.lock();
      if (iceTransport) {
        mICETransportSubscription = iceTransport->subscribe(mThisWeak.lock());
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    SCTPTransport::~SCTPTransport()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ISCTPTransportPtr object)
    {
      ISCTPTransportPtr original = ISCTPTransportTearAway::original(object);
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(IDataTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ForDataChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ForListenerPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr SCTPTransport::getStats() const throw(InvalidStateError)
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
    #pragma mark SCTPTransport => ISCTPTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr SCTPTransport::toDebug(SCTPTransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    ISCTPTransportPtr SCTPTransport::create(
                                            ISCTPTransportDelegatePtr delegate,
                                            IDTLSTransportPtr transport,
                                            WORD inLocalPort,
                                            WORD inRemotePort
                                            ) throw (InvalidParameters, InvalidStateError)
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!transport)

      UseSecureTransportPtr useSecureTransport = DTLSTransport::convert(transport);
      ASSERT(((bool)useSecureTransport))

      auto dataTransport = useSecureTransport->getDataTransport();
      ORTC_THROW_INVALID_STATE_IF(!dataTransport)

      UseListenerPtr listener = SCTPTransportListener::convert(dataTransport);
      ORTC_THROW_INVALID_STATE_IF(!listener)

      SCTPTransportPtr pThis(make_shared<SCTPTransport>(make_private {}, IORTCForInternal::queueORTC(), listener, useSecureTransport));

      ForListenerPtr forListener = pThis;

      WORD localPort = inLocalPort;
      WORD remotePort = inRemotePort;
      listener->registerNewTransport(transport, forListener, localPort, remotePort);

      if (!forListener) {
        ZS_LOG_WARNING(Debug, slog("unable to allocate port") + ZS_PARAM("local port", inLocalPort) + ZS_PARAM("remote port", inRemotePort))
        ORTC_THROW_INVALID_PARAMETERS("unable to allocate port, local port=" + string(inLocalPort) + ", remote port=" + string(remotePort))
      }

      ISCTPTransportPtr registeredTransport = SCTPTransport::convert(forListener);

      auto oldThis = pThis;
      pThis = SCTPTransport::convert(forListener);

      auto tearAway = ISCTPTransportTearAway::create(registeredTransport, make_shared<TearAwayData>());
      ORTC_THROW_INVALID_STATE_IF(!tearAway)

      auto tearAwayData = ISCTPTransportTearAway::data(tearAway);
      ORTC_THROW_INVALID_STATE_IF(!tearAwayData)

      tearAwayData->mListener = listener;

      if (delegate) {
        tearAwayData->mDefaultSubscription = registeredTransport->subscribe(delegate);
      }

      AutoRecursiveLock lock(*pThis);
      pThis->mLocalPort = localPort;
      pThis->mRemotePort = remotePort;
      if (pThis->getID() == oldThis->getID()) {
        pThis->init();
      } else {
        oldThis->cancel();
      }
      return tearAway;
    }

    //-------------------------------------------------------------------------
    ISCTPTransportTypes::CapabilitiesPtr SCTPTransport::getCapabilities()
    {
      CapabilitiesPtr result(make_shared<Capabilities>());
      result->mMaxMessageSize = UseSettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE);
      return result;
    }

    //-------------------------------------------------------------------------
    IDTLSTransportPtr SCTPTransport::transport() const
    {
      return DTLSTransport::convert(mSecureTransport.lock());
    }

    //-------------------------------------------------------------------------
    WORD SCTPTransport::port() const
    {
      return mLocalPort;
    }

    //-------------------------------------------------------------------------
    WORD SCTPTransport::localPort() const
    {
      return mLocalPort;
    }

    //-------------------------------------------------------------------------
    WORD SCTPTransport::remotePort() const
    {
      return mRemotePort;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::start(const Capabilities &remoteCapabilities)
    {
      ZS_LOG_DEBUG(log("start called") + remoteCapabilities.toDebug())

      AutoRecursiveLock lock(*this);
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ORTC_THROW_INVALID_STATE("already shutting down")
      }

      if (mCapabilities) {
        ORTC_THROW_INVALID_STATE("already started")
      }

      mCapabilities = CapabilitiesPtr(make_shared<Capabilities>(remoteCapabilities));
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::stop()
    {
      ZS_LOG_DEBUG(log("stop called"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    ISCTPTransportSubscriptionPtr SCTPTransport::subscribe(ISCTPTransportDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);

      ISCTPTransportSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      ISCTPTransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        SCTPTransportPtr pThis = mThisWeak.lock();

        for (auto iter = mAnnouncedIncomingDataChannels.begin(); iter != mAnnouncedIncomingDataChannels.end(); ++iter) {
          // NOTE: ID of data channels are always greater than last so order
          // should be guarenteed.
          auto dataChannel = (*iter).second;
          delegate->onSCTPTransportDataChannel(mThisWeak.lock(), DataChannel::convert(dataChannel));
        }

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => ISCTPTransportForDataChannel
    #pragma mark

    //-------------------------------------------------------------------------
    void SCTPTransport::registerNewDataChannel(
                                               UseDataChannelPtr &ioDataChannel,
                                               WORD &ioSessionID
                                               )
    {
      AutoRecursiveLock lock(*this);

      UseDataChannelPtr dataChannel = ioDataChannel;
      WORD sessionID = ioSessionID;

      // reset return results
      ioSessionID = ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID;
      ioDataChannel = UseDataChannelPtr();

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_WARNING(Debug, log("cannot create a data channel while shutting down"))
        ORTC_THROW_INVALID_STATE("cannot create a data channel while shutting down")
      }

      if (ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID != sessionID) {

        auto found = mSessions.find(sessionID);
        if (found != mSessions.end()) {
          auto existingDataChannel = (*found).second;
          ZS_LOG_DEBUG(log("already found session") + ZS_PARAM("existing data channel", existingDataChannel->getID()))

          found = mPendingResetSessions.find(sessionID);
          if (found != mPendingResetSessions.end()) {
            ZS_LOG_ERROR(Detail, log("session is pending reset (i.e. in use)") + ZS_PARAM("session id", sessionID))
            ORTC_THROW_INVALID_PARAMETERS("session is pending reset, sessions=" + string(sessionID))
          }
          found = mQueuedResetSessions.find(sessionID);
          if (found != mQueuedResetSessions.end()) {
            ZS_LOG_ERROR(Detail, log("session is queued to reset (i.e. in use)") + ZS_PARAM("session id", sessionID))
            ORTC_THROW_INVALID_PARAMETERS("session is queued pending reset, sessions=" + string(sessionID))
          }

          if (!existingDataChannel->isIncoming()) {
            ZS_LOG_ERROR(Detail, log("session is already in use") + ZS_PARAM("session id", sessionID))
            ORTC_THROW_INVALID_PARAMETERS("session is already in use, sessions=" + string(sessionID))
          }

          ioSessionID = sessionID;
          ioDataChannel = existingDataChannel;
          return;
        }

        if (mSessions.size() >= mMaxSessionsPerPort) {
          ZS_LOG_WARNING(Debug, log("too many sessions on sctp port open") + ZS_PARAM("active sessions", mSessions.size()) + ZS_PARAM("max", mMaxSessionsPerPort))
          ORTC_THROW_INVALID_STATE("too many sessions open on port, sessions=" + string(mSessions.size()) + ", max=" + string(mMaxSessionsPerPort))
        }
        
        ioSessionID = sessionID;
        ioDataChannel = dataChannel;
        mSessions[sessionID] = dataChannel;

        ZS_LOG_TRACE(log("registered new data channel to session ID") + ZS_PARAM("session id", sessionID))
        return;
      }

      if (mSessions.size() >= mMaxSessionsPerPort) {
        ZS_LOG_WARNING(Debug, log("too many sessions on sctp port open") + ZS_PARAM("active sessions", mSessions.size()) + ZS_PARAM("max", mMaxSessionsPerPort))
        ORTC_THROW_INVALID_STATE("too many sessions open on port, sessions=" + string(mSessions.size()) + ", max=" + string(mMaxSessionsPerPort))
      }

      while (true) {

        {
          if (!isSessionAvailable(mCurrentAllocationSessionID)) goto next_session;
          break;
        }

      next_session:
        {
          mCurrentAllocationSessionID = mCurrentAllocationSessionID + mNextAllocationIncrement;

          if (mCurrentAllocationSessionID < mMinAllocationSessionID) mCurrentAllocationSessionID = mMinAllocationSessionID + (mCurrentAllocationSessionID % 2);
          if (mCurrentAllocationSessionID > mMaxAllocationSessionID) mCurrentAllocationSessionID = mMinAllocationSessionID + (mCurrentAllocationSessionID % 2);
        }
      }

      sessionID = mCurrentAllocationSessionID;
      ioDataChannel = dataChannel;
      mSessions[sessionID] = dataChannel;
    }

    //-------------------------------------------------------------------------
    ISCTPTransportForDataChannelSubscriptionPtr SCTPTransport::subscribe(ISCTPTransportForDataChannelDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("datachannel subscribing to SCTP Transport"))
      
      AutoRecursiveLock lock(*this);
      
      ISCTPTransportForDataChannelSubscriptionPtr subscription = mDataChannelSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueORTC());
      
      ISCTPTransportForDataChannelDelegatePtr delegate = mDataChannelSubscriptions.delegate(subscription, true);
      
      if (delegate) {
        SCTPTransportPtr pThis = mThisWeak.lock();

        if (State_Pending != mCurrentState) {
          delegate->onSCTPTransportStateChanged();
        }
      }
      
      if (isShutdown()) {
        mSubscriptions.clear();
      }
      
      return subscription;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::isReady() const
    {
      return State_Ready == mCurrentState;
    }

    //-------------------------------------------------------------------------
    PromisePtr SCTPTransport::sendDataNow(SCTPPacketOutgoingPtr packet)
    {
      {
        AutoRecursiveLock lock(*this);

        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_WARNING(Debug, log("cannot send data while shutting down / shutdown"))
          return Promise::createRejected(IORTCForInternal::queueORTC());
        }

        if (State_Ready != mCurrentState) goto waiting_to_send;
        if (!mWriteReady) goto waiting_to_send;

        bool wouldBlock = false;
        if (!attemptSend(*packet, wouldBlock)) {
          if (wouldBlock) goto waiting_to_send;

          ZS_LOG_WARNING(Debug, log("unable to send packet at this time"))
          return Promise::createRejected(IORTCForInternal::queueORTC());
        }
        goto done;
      }

    waiting_to_send:
      {
        auto promise = Promise::create();
        mWaitingToSend.push(promise);
        return promise;
      }

    done:
      {
      }

      return PromisePtr();
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::requestShutdown(
                                        UseDataChannelPtr dataChannel,
                                        WORD sessionID
                                        )
    {
      AutoRecursiveLock lock(*this);

      if (isShutdown()) {
        ZS_LOG_WARNING(Trace, log("already shutdown transport"))
        dataChannel->notifyClosed();
        return;
      }

      auto objectID = dataChannel->getID();

      // remove from announced channels
      {
        auto found = mAnnouncedIncomingDataChannels.find(objectID);
        if (found != mAnnouncedIncomingDataChannels.end()) {
          mAnnouncedIncomingDataChannels.erase(found);
        }
      }

      if (ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID == sessionID) {
        dataChannel->notifyClosed();
        return;
      }

      bool wasActive = false;
      bool resetPending = false;

      // scope: remove from active sessions
      {
        auto found = mSessions.find(sessionID);
        if (found != mSessions.end()) {
          auto existingSession = (*found).second;
          if (existingSession->getID() != objectID) {
            ZS_LOG_WARNING(Trace, log("requested object to shutdown is not active session (thus ignoring)"))
            return;
          }
          mSessions.erase(found);
          wasActive = true;
        }
      }

      {
        auto found = mPendingResetSessions.find(sessionID);
        resetPending = resetPending || (found != mPendingResetSessions.end());
      }

      {
        auto found = mQueuedResetSessions.find(sessionID);
        resetPending = resetPending || (found != mQueuedResetSessions.end());
      }

      if (wasActive) {
        if (resetPending) {
          ZS_LOG_TRACE(log("already pending reset") + ZS_PARAM("session id", sessionID))
          return;
        }
        mQueuedResetSessions[sessionID] = dataChannel;
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
        return;
      }

      if (!resetPending) {
        ZS_LOG_TRACE(log("data channel was not active and no reset is pending") + ZS_PARAM("session id", sessionID))
        dataChannel->notifyClosed();
        return;
      }

      ZS_LOG_TRACE(log("reset is already pending") + ZS_PARAM("session id", sessionID))
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => ISCTPTransportForSecureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    SCTPTransport::ForListenerPtr SCTPTransport::create(
                                                        UseListenerPtr listener,
                                                        UseSecureTransportPtr secureTransport,
                                                        WORD localPort,
                                                        WORD remotePort
                                                        )
    {
      SCTPTransportPtr pThis(make_shared<SCTPTransport>(make_private {}, IORTCForInternal::queueORTC(), listener, secureTransport, localPort, remotePort));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::handleDataPacket(
                                         const BYTE *buffer,
                                         size_t bufferLengthInBytes
                                         )
    {
      if (bufferLengthInBytes < sizeof(DWORD)) {
        ZS_LOG_WARNING(Trace, log("packet length is too small to be an SCTP packet"))
        return false;
      }

      {
        AutoRecursiveLock lock(*this);

        {
          if (isShutdown()) {
            ZS_LOG_WARNING(Debug, log("packet arrived after shutdown"))
            return false;
          }

          if (!mCapabilities) goto queue_packet;
          if (mPendingIncomingBuffers.size() > 0) goto queue_packet;

          usrsctp_conninput(mThisSocket, buffer, bufferLengthInBytes, 0);

          attemptAccept();
        }

      queue_packet:
        {
          mPendingIncomingBuffers.push(SecureByteBlockPtr(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes)));
          return true;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::notifyShutdown()
    {
      ISCTPTransportAsyncDelegateProxy::create(mThisWeak.lock())->onNotifiedToShutdown();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => friend SCTPInit
    #pragma mark

    //-------------------------------------------------------------------------
    bool SCTPTransport::notifySendSCTPPacket(
                                             const BYTE *buffer,
                                             size_t bufferLengthInBytes
                                             )
    {
      UseSecureTransportPtr transport;

      {
        AutoRecursiveLock lock(*this);
        transport = mSecureTransport.lock();
      }

      if (!transport) {
        ZS_LOG_WARNING(Trace, log("secure transport is gone (thus send packet is not available)") + ZS_PARAM("buffer length", bufferLengthInBytes))
        return false;
      }

      return transport->sendDataPacket(buffer, bufferLengthInBytes);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SCTPTransport::onWake()
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
    #pragma mark SCTPTransport => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SCTPTransport::onTimer(TimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => ISCTPTransportAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SCTPTransport::onIncomingPacket(SCTPPacketIncomingPtr packet)
    {
      ZS_LOG_TRACE(log("on incoming packet") + packet->toDebug())

      UseDataChannelPtr dataChannel;

      {
        AutoRecursiveLock lock(*this);

        // scope: check active sessions
        {
          auto found = mSessions.find(packet->mSessionID);
          if (found != mSessions.end()) {
            dataChannel = (*found).second;
            goto forward_to_data_channel;
          }
        }

        // scope: check pending reset
        {
          auto found = mPendingResetSessions.find(packet->mSessionID);
          if (found != mPendingResetSessions.end()) {
            dataChannel = (*found).second;
            goto forward_to_data_channel;
          }
        }

        // scope: check queued reset
        {
          auto found = mQueuedResetSessions.find(packet->mSessionID);
          if (found != mQueuedResetSessions.end()) {
            dataChannel = (*found).second;
            goto forward_to_data_channel;
          }
        }

        // not found anywhere
        dataChannel = UseDataChannel::create(mThisWeak.lock(), packet->mSessionID);

        ZS_LOG_TRACE(log("creating new incoming data channel") + ZS_PARAM("data channel", dataChannel->getID()) + packet->toDebug())

        if (mSessions.size() >= mMaxSessionsPerPort) {
          ZS_LOG_ERROR(Detail, log("too many session active") + packet->toDebug())
          dataChannel->requestShutdown();
          mQueuedResetSessions[packet->mSessionID] = dataChannel;
          goto forward_to_data_channel;
        }

        mSessions[packet->mSessionID] = dataChannel;
        goto forward_to_data_channel;
      }

    forward_to_data_channel:
      {
        ZS_LOG_TRACE(log("forwarding to data channel") + ZS_PARAM("data channel", dataChannel->getID()) + packet->toDebug())
        dataChannel->handleSCTPPacket(packet);
      }
      // HERE
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::onNotifiedToShutdown()
    {
      ZS_LOG_TRACE(log("on notified to shutdown"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => IICETransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SCTPTransport::onICETransportStateChanged(
                                                   IICETransportPtr transport,
                                                   IICETransport::States state
                                                   )
    {
      ZS_LOG_DEBUG(log("ice transport state changed") + ZS_PARAM("ice transport id", transport->getID()) + ZS_PARAM("state", IICETransport::toString(state)))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::onICETransportCandidatePairAvailable(
                                                             IICETransportPtr transport,
                                                             CandidatePairPtr candidatePair
                                                             )
    {
      // ignored
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::onICETransportCandidatePairGone(
                                                        IICETransportPtr transport,
                                                        CandidatePairPtr candidatePair
                                                        )
    {
      // ignored
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::onICETransportCandidatePairChanged(
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
    #pragma mark SCTPTransport => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SCTPTransport::onPromiseSettled(PromisePtr promise)
    {
      ZS_LOG_TRACE(log("on promise settled"))
      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params SCTPTransport::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::SCTPTransport");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SCTPTransport::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::SCTPTransport");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SCTPTransport::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr SCTPTransport::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::SCTPTransport");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto listener = mListener.lock();
      UseServicesHelper::debugAppend(resultEl, "listener", listener ? listener->getID() : 0);

      auto secureTransport = mSecureTransport.lock();
      UseServicesHelper::debugAppend(resultEl, "secure transport", secureTransport ? secureTransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "secure transport ready promise", (bool)mSecureTransportReady);
      UseServicesHelper::debugAppend(resultEl, "secure transport closed promise", (bool)mSecureTransportClosed);

      auto iceTransport = mICETransport.lock();
      UseServicesHelper::debugAppend(resultEl, "ice transport", iceTransport ? iceTransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "ice transport subscription", mICETransportSubscription ? mICETransportSubscription->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, mCapabilities ? mCapabilities->toDebug() : ElementPtr());

      UseServicesHelper::debugAppend(resultEl, "announced data channels", mAnnouncedIncomingDataChannels.size());

      UseServicesHelper::debugAppend(resultEl, "this socket", (PTRNUMBER)mThisSocket);

      UseServicesHelper::debugAppend(resultEl, "incoming", mIncoming);

      UseServicesHelper::debugAppend(resultEl, "socket", (PTRNUMBER)mSocket);
      UseServicesHelper::debugAppend(resultEl, "socket", (PTRNUMBER)mAcceptSocket);

      UseServicesHelper::debugAppend(resultEl, "local port", mLocalPort);
      UseServicesHelper::debugAppend(resultEl, "remote port", mRemotePort);

      UseServicesHelper::debugAppend(resultEl, "sessions", mSessions.size());

      UseServicesHelper::debugAppend(resultEl, "pending reset", mPendingResetSessions.size());
      UseServicesHelper::debugAppend(resultEl, "queued reset", mQueuedResetSessions.size());

      UseServicesHelper::debugAppend(resultEl, "current allocation", mCurrentAllocationSessionID);
      UseServicesHelper::debugAppend(resultEl, "min allocation", mMinAllocationSessionID);
      UseServicesHelper::debugAppend(resultEl, "max allocation", mMaxAllocationSessionID);

      UseServicesHelper::debugAppend(resultEl, "next allocation increment", mNextAllocationIncrement);

      UseServicesHelper::debugAppend(resultEl, "waiting to send", mWaitingToSend.size());

      UseServicesHelper::debugAppend(resultEl, "write ready", mWriteReady);

      UseServicesHelper::debugAppend(resultEl, "pending incoming buffers", mPendingIncomingBuffers.size());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepStartCalled()) goto not_ready;
      if (!stepSecureTransport()) goto not_ready;
      if (!stepICETransport()) goto not_ready;
      if (!stepOpen()) goto not_ready;
      if (!stepDeliverIncomingPackets()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("SCTP is NOT ready!!!"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("SCTP is ready!!!"))
        setState(State_Ready);
      }
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepStartCalled()
    {
      if (!mCapabilities) {
        ZS_LOG_TRACE(log("waiting for start to be called"))
        return false;
      }

      ZS_LOG_TRACE(log("start is called"))
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepSecureTransport()
    {
      if (mSecureTransportClosed->isSettled()) {
        ZS_LOG_WARNING(Detail, log("secure transport is now closed (thus must shutdown)"))
        cancel();
        return false;
      }

      if (!mSecureTransportReady) {
        ZS_LOG_TRACE(log("secure transport already notified ready"))
        return true;
      }

      if (!mSecureTransportReady->isSettled()) {
        ZS_LOG_TRACE(log("waiting for secure transport to notify ready"))
        return false;
      }

      ZS_LOG_DEBUG(log("secure transport notified ready"))
      mSecureTransportReady.reset();

      auto secureTransport = mSecureTransport.lock();
      if (!secureTransport) {
        ZS_LOG_WARNING(Detail, log("secure transport is gone (thus must shutdown)"))
        cancel();
        return false;
      }

      auto clientRole = secureTransport->isClientRole();
      if (clientRole) {
        mCurrentAllocationSessionID = mMinAllocationSessionID;    // client picks even port numbers
      } else {
        mCurrentAllocationSessionID = mMinAllocationSessionID+1;  // server picks odd port numbers
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepICETransport()
    {
      auto iceTransport = mICETransport.lock();

      if (!iceTransport) {
        ZS_LOG_WARNING(Detail, log("ice transport is gone (thus must shutdown)"))
        cancel();
        return false;
      }

      auto state = iceTransport->state();
      switch (state) {
        case IICETransport::State_New:
        case IICETransport::State_Checking:
        {
          ZS_LOG_TRACE(log("ice is not ready"))
          setState(State_Disconnected);
          break;
        }
        case IICETransport::State_Connected:
        case IICETransport::State_Completed:
        {
          ZS_LOG_TRACE(log("ice is ready"))
          return true;
        }
        case IICETransport::State_Disconnected:
        case IICETransport::State_Failed:
        {
          ZS_LOG_TRACE(log("ice is disconnected"))
          setState(State_Disconnected);
          break;
        }
        case IICETransport::State_Closed:
        {
          ZS_LOG_WARNING(Detail, log("ice is closed"))
          cancel();
          break;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepOpen()
    {
      if (mIncoming) {
        ZS_LOG_TRACE(log("open listen socket"))
        if (!openListenSCTPSocket()) {
          ZS_LOG_ERROR(Detail, log("failed to open listen port"))
          cancel();
          return false;
        }
      } else {
        ZS_LOG_TRACE(log("open connect socket"))
        if (!openConnectSCTPSocket()) {
          ZS_LOG_ERROR(Detail, log("failed to open connect port"))
          cancel();
          return false;
        }
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepDeliverIncomingPackets()
    {
      if (mPendingIncomingBuffers.size() < 1) {
        ZS_LOG_TRACE(log("no pending packets to deliver"))
        return true;
      }

      BufferQueue pending = mPendingIncomingBuffers;
      mPendingIncomingBuffers = BufferQueue();

      while (pending.size() > 0) {
        SecureByteBlockPtr buffer = pending.front();
        handleDataPacket(buffer->BytePtr(), buffer->SizeInBytes());
        pending.pop();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      setState(State_ShuttingDown);

      if (mGracefulShutdownReference) {

        {
          // scope: attmempt to shutdown transport and check if ready
          {
            for (auto iter = mSessions.begin(); iter != mSessions.end(); ++iter)
            {
              auto session = (*iter).second;
              session->requestShutdown();
            }

            if (mSecureTransportReady) {
              if (!mSecureTransportReady->isSettled()) goto transport_not_available;
            }
            if (mSecureTransportClosed->isSettled()) goto transport_not_available;

            auto iceTransport = mICETransport.lock();
            if (!iceTransport) {
              ZS_LOG_WARNING(Debug, log("ice tranport is gone"))
              goto transport_not_available;
            }
            switch (iceTransport->state()) {
              case IICETransport::State_New:
              case IICETransport::State_Checking:
              case IICETransport::State_Disconnected:
              case IICETransport::State_Failed:
              case IICETransport::State_Closed:
              {
                ZS_LOG_WARNING(Debug, log("ice tranport is not available"))
                goto transport_not_available;
              }
              case IICETransport::State_Connected:
              case IICETransport::State_Completed:
              {
                break;
              }
            }

            if (mSessions.size() > 0) goto waiting_to_close;
            if (mPendingResetSessions.size() > 0) goto waiting_to_close;
            if (mQueuedResetSessions.size() > 0) goto waiting_to_close;

            goto done_waiting;
          }

        waiting_to_close:
          {
            ZS_LOG_TRACE(log("waiting for sessions to close") + ZS_PARAM("sessions", mSessions.size()) + ZS_PARAM("pending", mPendingResetSessions.size()) + ZS_PARAM("queued", mQueuedResetSessions.size()))
            return;
          }

        transport_not_available:
          {
          }

        done_waiting:
          {
          }
        }
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      mSubscriptions.clear();
      mDataChannelSubscriptions.clear();

      mSecureTransportReady.reset();
      mSecureTransportClosed.reset();

      if (mICETransportSubscription) {
        mICETransportSubscription->cancel();
        mICETransportSubscription.reset();
      }

      mAnnouncedIncomingDataChannels.clear();

      if (mAcceptSocket) {
        usrsctp_close(mAcceptSocket);
        mAcceptSocket = NULL;
      }
      if (mSocket) {
        usrsctp_close(mSocket);
        mSocket = NULL;
        usrsctp_deregister_address(mThisSocket);
      }

      for (auto iter = mSessions.begin(); iter != mSessions.end(); ++iter)
      {
        auto session = (*iter).second;
        session->notifyClosed();
      }
      mSessions.clear();

      for (auto iter = mPendingResetSessions.begin(); iter != mPendingResetSessions.end(); ++iter)
      {
        auto session = (*iter).second;
        session->notifyClosed();
      }
      mPendingResetSessions.clear();

      for (auto iter = mQueuedResetSessions.begin(); iter != mQueuedResetSessions.end(); ++iter)
      {
        auto session = (*iter).second;
        session->notifyClosed();
      }
      mQueuedResetSessions.clear();

      while (mWaitingToSend.size() > 0) {
        auto promise = mWaitingToSend.front();
        promise->reject();
        mWaitingToSend.pop();
      }

      mPendingIncomingBuffers = BufferQueue();

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;
      mDataChannelSubscriptions.delegate()->onSCTPTransportStateChanged();

//      SCTPTransportPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onSCTPTransportStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::setError(WORD errorCode, const char *inReason)
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
    bool SCTPTransport::openListenSCTPSocket()
    {
      if (mSocket) return true;

      if (!openSCTPSocket()) {
        ZS_LOG_ERROR(Detail, log("failed to open listen socket"))
        return false;
      }

      if (usrsctp_listen(mSocket, 1)) {
        ZS_LOG_ERROR(Detail, log("failed to listen on SCTP socket"))
        return false;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::openConnectSCTPSocket()
    {
      if (mSocket) return true;

      if (!openSCTPSocket()) {
        ZS_LOG_ERROR(Detail, log("failed to open connect socket"))
        return false;
      }

      auto localConn = UseSCTPHelper::getAddress(mLocalPort, mThisSocket);
      if (usrsctp_bind(mSocket, reinterpret_cast<sockaddr *>(&localConn), sizeof(localConn))) {
        ZS_LOG_ERROR(Detail, log("unable to bind to port") + ZS_PARAM("errno", errno))
        return false;
      }

      auto remoteConn = UseSCTPHelper::getAddress(mRemotePort, mThisSocket);

      auto result = usrsctp_connect(mSocket, reinterpret_cast<sockaddr *>(&remoteConn), sizeof(remoteConn));
      if (result < 0) {
        if (errno == SCTP_EINPROGRESS) {
          ZS_LOG_TRACE(log("connect with EINPROGRESS"))
          return true;
        }
        ZS_LOG_ERROR(Detail, log("unable to connect to remote socket"))
        return false;
      }

      ZS_LOG_DEBUG(log("successful bind and connect"))
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::openSCTPSocket()
    {
      mSocket = usrsctp_socket(AF_CONN, SOCK_STREAM, IPPROTO_SCTP, SCTPInit::OnSctpInboundPacket, NULL, 0, mThisSocket);
      if (!mSocket) {
        ZS_LOG_ERROR(Detail, log("failed to create sctp socket"))
        return false;
      }

      if (!prepareSocket(mSocket)) {
        ZS_LOG_ERROR(Detail, log("failed to setup socket"))
        return false;
      }

      usrsctp_register_address(mThisSocket);

      ZS_LOG_DEBUG(log("sctp socket open"))
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::prepareSocket(struct socket *sock)
    {
      // Make the socket non-blocking. Connect, close, shutdown etc will not
      // block the thread waiting for the socket operation to complete.

      if (usrsctp_set_non_blocking(sock, 1) < 0) {
        ZS_LOG_ERROR(Detail, log("failed to set SCTP socket to non-blocking"))
        return false;
      }

      // This ensures that the usrsctp close call deletes the association. This
      // prevents usrsctp from calling OnSctpOutboundPacket with references to
      // this class as the address.
      linger linger_opt {};
      linger_opt.l_onoff = 1;
      linger_opt.l_linger = 0;
      if (usrsctp_setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt))) {
        ZS_LOG_ERROR(Detail, log("failed to set SCTP socket SO_LINGER"))
        return false;
      }

      // Enable stream ID resets.
      struct sctp_assoc_value stream_rst {};
      stream_rst.assoc_id = SCTP_ALL_ASSOC;
      stream_rst.assoc_value = 1;
      if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_ENABLE_STREAM_RESET, &stream_rst, sizeof(stream_rst))) {
        ZS_LOG_ERROR(Detail, log("failed to set SCTP_ENABLE_STREAM_RESET"))
        cancel();
        return false;
      }

      // Nagle.
      uint32_t nodelay = 1;
      if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_NODELAY, &nodelay, sizeof(nodelay))) {
        ZS_LOG_ERROR(Detail, log("failed to set SCTP_NODELAY"))
        return false;
      }

      struct sctp_paddrparams params = {{0}};
      params.spp_assoc_id = 0;
      params.spp_flags = SPP_PMTUD_DISABLE;
      params.spp_pathmtu = kSctpMtu;
      if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS, &params, sizeof(params))) {
        ZS_LOG_ERROR(Detail, log("failed to set SCTP_PEER_ADDR_PARAMS"))
        cancel();
        return false;
      }

      int event_types[] = {
        SCTP_ASSOC_CHANGE,
        SCTP_PEER_ADDR_CHANGE,
        SCTP_SEND_FAILED_EVENT,
        SCTP_SENDER_DRY_EVENT,
        SCTP_STREAM_RESET_EVENT
      };
      struct sctp_event event = {0};
      event.se_assoc_id = SCTP_ALL_ASSOC;
      event.se_on = 1;
      for (size_t i = 0; i < ARRAY_SIZE(event_types); i++) {
        event.se_type = event_types[i];
        if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_EVENT, &event, sizeof(event)) < 0) {
          ZS_LOG_ERROR(Detail, log("failed to set SCTP_EVENT type") + ZS_PARAM("event", event.se_type))
          return false;
        }
      }

      ZS_LOG_DEBUG(log("sctp socket prepared") + ZS_PARAM("socket", (PTRNUMBER)sock))
      return true;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::attemptAccept()
    {
      if (!mIncoming) return;         // only applies to incoming sockets
      if (!mSocket) return;           // only applies to socket
      if (mAcceptSocket) return;      // already accepted

      auto localAddr = UseSCTPHelper::getAddress(mLocalPort, mThisSocket);

      socklen_t addressLength = sizeof(localAddr);
      mAcceptSocket = usrsctp_accept(mSocket, reinterpret_cast<sockaddr *>(&localAddr), &addressLength);

      if (NULL == mAcceptSocket) {
        if ((SCTP_EINPROGRESS == errno) ||
            (SCTP_EWOULDBLOCK == errno)) {
          ZS_LOG_TRACE(log("accept socket not ready"))
          return;
        }
        ZS_LOG_ERROR(Detail, log("attempt to accept failed"))
        cancel();
        return;
      }

      ZS_LOG_TRACE(log("socket accept succeeded"))
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      notifyWriteReady();
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::isSessionAvailable(WORD sessionID)
    {
      {
        auto found = mSessions.find(sessionID);
        if (found != mSessions.end()) return false;
      }
      {
        auto found = mPendingResetSessions.find(sessionID);
        if (found != mPendingResetSessions.end()) return false;
      }
      {
        auto found = mQueuedResetSessions.find(sessionID);
        if (found != mQueuedResetSessions.end()) return false;
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::attemptSend(
                                    const SCTPPacketOutgoing &inPacket,
                                    bool &outWouldBlock
                                    )
    {
      outWouldBlock = false;

      auto socket = mIncoming ? mAcceptSocket : mSocket;

      if (!socket) return false;

      auto found = mSessions.find(inPacket.mSessionID);
      if (found == mSessions.end()) return false;

      struct sctp_sendv_spa spa = {};

      spa.sendv_flags |= SCTP_SEND_SNDINFO_VALID;
      spa.sendv_sndinfo.snd_sid = inPacket.mSessionID;
      spa.sendv_sndinfo.snd_ppid = htonl(inPacket.mType);

      if (!inPacket.mOrdered) {
        spa.sendv_sndinfo.snd_flags = SCTP_UNORDERED;
        if ((inPacket.mMaxRetransmits >= 0) ||
            (Milliseconds() == inPacket.mMaxPacketLifetime)) {
          spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
          spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_RTX;
          spa.sendv_prinfo.pr_value = inPacket.mMaxRetransmits;
        } else {
          spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
          spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_TTL;
          spa.sendv_prinfo.pr_value = static_cast<decltype(spa.sendv_prinfo.pr_value)>(inPacket.mMaxPacketLifetime.count());
        }
      }

      auto result = usrsctp_sendv(
                                  socket,
                                  (inPacket.mBuffer ? inPacket.mBuffer->BytePtr() : NULL),
                                  (inPacket.mBuffer ? inPacket.mBuffer->SizeInBytes() : 0),
                                  NULL, 0,
                                  &spa, sizeof(spa),
                                  SCTP_SENDV_SPA,
                                  0);

      if (result < 0) {
        if (errno == SCTP_EWOULDBLOCK) {
          mWriteReady = false;
          outWouldBlock = true;
          ZS_LOG_WARNING(Trace, log("would block"))
        } else {
          ZS_LOG_ERROR(Debug, log("usrsctp failed") + ZS_PARAM("errno", errno))
        }
        return false;
      }

      ZS_LOG_INSANE(log("sctp outgoing data sent successfully"))
      return true;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::notifyWriteReady()
    {
      mWriteReady = true;

      while (mWaitingToSend.size() > 0) {
        auto promise = mWaitingToSend.front();
        promise->resolve();
        mWaitingToSend.pop();
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    ISCTPTransportFactory &ISCTPTransportFactory::singleton()
    {
      return SCTPTransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ISCTPTransportFactory::ForListenerPtr ISCTPTransportFactory::create(
                                                                        UseListenerPtr listener,
                                                                        UseSecureTransportPtr secureTransport,
                                                                        WORD localPort,
                                                                        WORD remotePort
                                                                        )
    {
      if (this) {}
      return internal::SCTPTransport::create(listener, secureTransport, localPort, remotePort);
    }

    //-------------------------------------------------------------------------
    ISCTPTransportPtr ISCTPTransportFactory::create(
                                                    ISCTPTransportDelegatePtr delegate,
                                                    IDTLSTransportPtr transport,
                                                    WORD localPort,
                                                    WORD remotePort
                                                    )
    {
      if (this) {}
      return internal::SCTPTransport::create(delegate, transport, localPort, remotePort);
    }

  } // internal namespace

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportTypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ISCTPTransportTypes::Capabilities::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ISCTPTransportTypes::Capabilities");

    UseServicesHelper::debugAppend(resultEl, "max message size", mMaxMessageSize);
    UseServicesHelper::debugAppend(resultEl, "min port", mMinPort);
    UseServicesHelper::debugAppend(resultEl, "max port", mMaxPort);
    UseServicesHelper::debugAppend(resultEl, "max useable", mMaxUsablePorts);
    UseServicesHelper::debugAppend(resultEl, "max sessions per port", mMaxSessionsPerPort);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ISCTPTransportTypes::Capabilities::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ISCTPTransportTypes:Capabilities:");
    hasher.update(mMaxMessageSize);
    hasher.update(":");
    hasher.update(mMinPort);
    hasher.update(":");
    hasher.update(mMaxPort);
    hasher.update(":");
    hasher.update(mMaxUsablePorts);
    hasher.update(":");
    hasher.update(mMaxSessionsPerPort);
    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransport
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ISCTPTransport::toDebug(ISCTPTransportPtr transport)
  {
    return internal::SCTPTransport::toDebug(internal::SCTPTransport::convert(transport));
  }

  //---------------------------------------------------------------------------
  ISCTPTransportPtr ISCTPTransport::create(
                                           ISCTPTransportDelegatePtr delegate,
                                           IDTLSTransportPtr transport,
                                           WORD localPort,
                                           WORD remotePort
                                           ) throw (InvalidParameters, InvalidStateError)
  {
    return internal::ISCTPTransportFactory::singleton().create(delegate, transport, localPort, remotePort);
  }

  //---------------------------------------------------------------------------
  ISCTPTransportListenerSubscriptionPtr ISCTPTransport::listen(
                                                               ISCTPTransportListenerDelegatePtr delegate,
                                                               IDTLSTransportPtr transport
                                                               )
  {
    return internal::ISCTPTransportListenerFactory::singleton().listen(delegate, transport);
  }

  //---------------------------------------------------------------------------
  ISCTPTransportTypes::CapabilitiesPtr ISCTPTransport::getCapabilities()
  {
    return internal::ISCTPTransportListenerFactory::singleton().getCapabilities();
  }

}
