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

#define ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

#include <ortc/internal/ortc_SCTPTransport.h>
#include <ortc/internal/ortc_SCTPTransportListener.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_DataChannel.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>
#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
#include <zsLib/Numeric.h>
#include <zsLib/SafeInt.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/queue.h>
#include <cryptopp/sha.h>

#include <usrsctp.h>

#include <sstream>

//#include <netinet/sctp_os.h>
#ifdef _WIN32
# include <winsock2.h>

enum WindowsErrorCompatibility {
  ESHUTDOWN = WSAESHUTDOWN,
};

#else
# include <sys/socket.h>
#endif  //_WIN32

// The expression ARRAY_SIZE(a) is a compile-time constant of type
// size_t which represents the number of elements of the given
// array. You should only use ARRAY_SIZE on statically allocated
// arrays.

#define ARRAY_SIZE(a)                               \
  ((sizeof(a) / sizeof(*(a))) /                     \
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_sctp_data_channel) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  using zsLib::Log;
  using zsLib::Numeric;

  ZS_DECLARE_INTERACTION_TEAR_AWAY(ISCTPTransport, internal::SCTPTransport::TearAwayData)

  typedef CryptoPP::ByteQueue ByteQueue;

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SCTPTransportSettingsDefaults);

    struct SCTPHelper;

    ZS_DECLARE_TYPEDEF_PTR(SCTPHelper, UseSCTPHelper);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (helpers)
    //

    const uint32_t kMaxSctpSid = 1023;
    static const size_t kSctpMtu = 1200;

    enum PreservedErrno {
      SCTP_EINPROGRESS = EINPROGRESS,
      SCTP_EWOULDBLOCK = EWOULDBLOCK
    };

    //-------------------------------------------------------------------------
    const char *toString(SCTPPayloadProtocolIdentifier ppid) noexcept
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
    //
    // SCTPPacketIncoming
    //

    //-------------------------------------------------------------------------
    ElementPtr SCTPPacketIncoming::toDebug() const noexcept
    {
      ElementPtr resultEl = Element::create("ortc::SCTPPacketIncoming");

      IHelper::debugAppend(resultEl, "type", toString(mType));
      IHelper::debugAppend(resultEl, "sessiond id", mSessionID);
      IHelper::debugAppend(resultEl, "sequence number", mSequenceNumber);
      IHelper::debugAppend(resultEl, "timestamp", mTimestamp);
      IHelper::debugAppend(resultEl, "flags", mFlags);
      IHelper::debugAppend(resultEl, "buffer", mBuffer ? mBuffer->SizeInBytes() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPPacketOutgoing
    //

    //-------------------------------------------------------------------------
    ElementPtr SCTPPacketOutgoing::toDebug() const noexcept
    {
      ElementPtr resultEl = Element::create("ortc::SCTPPacketOutgoing");

      IHelper::debugAppend(resultEl, "type", toString(mType));
      IHelper::debugAppend(resultEl, "sessiond id", mSessionID);
      IHelper::debugAppend(resultEl, "ordered", mOrdered);
      IHelper::debugAppend(resultEl, "max packet lifetime (ms)", mMaxPacketLifetime);
      IHelper::debugAppend(resultEl, "max retransmits", mMaxRetransmits);
      IHelper::debugAppend(resultEl, "buffer", mBuffer ? mBuffer->SizeInBytes() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPHelper
    //

    struct SCTPHelper
    {
      //-----------------------------------------------------------------------
      static sockaddr_conn getAddress(
        WORD port,
        SCTPTransportWeakPtr *thisSocket
      ) noexcept
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
      static Log::Params slog(const char *message) noexcept
      {
        ElementPtr objectEl = Element::create("ortc::SCTPHelper");
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      static String listFlags(int flags) noexcept
      {
        std::stringstream result;
        bool first = true;
        // Skip past the first 12 chars (strlen("SCTP_STREAM_"))

        struct flaginfo_t {
          int value;
          const char* name;
        } flaginfo[] = {
          {SCTP_STREAM_RESET_INCOMING_SSN, "RESET_INCOMING_SSN"},
          {SCTP_STREAM_RESET_OUTGOING_SSN, "RESET_OUTGOING_SSN"},
          {SCTP_STREAM_RESET_DENIED, "RESET_DENIED"},
          {SCTP_STREAM_RESET_FAILED,"RESET_FAILED"},
          {SCTP_STREAM_CHANGE_DENIED, "CHANGE_DENIED"}
        };

        for (int i = 0; i < ARRAY_SIZE(flaginfo); ++i) {
          if (flags & flaginfo[i].value) {
            if (!first) result << " | ";
            result << flaginfo[i].name;
            first = false;
          }
        }
        return result.str();
      }

      //-----------------------------------------------------------------------
      static String listArray(
        const WORD *array,
        size_t numElements
      ) noexcept
      {
        std::stringstream result;
        for (size_t i = 0; i < numElements; ++i) {
          if (i) {
            result << ", " << array[i];
          }
          else {
            result << array[i];
          }
        }
        return result.str();
      }
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransportSettingsDefaults
    //

    class SCTPTransportSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~SCTPTransportSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static SCTPTransportSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<SCTPTransportSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static SCTPTransportSettingsDefaultsPtr create()noexcept
      {
        auto pThis(make_shared<SCTPTransportSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
        // http://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-05#section-6.2
        ISettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT, kMaxSctpSid);
      }

    };

    //-------------------------------------------------------------------------
    void installSCTPTransportSettingsDefaults() noexcept
    {
      SCTPTransportSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPInit
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPInit
    //

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
      ZS_DECLARE_TYPEDEF_PTR(IDataChannelForSCTPTransport, UseDataChannel);

    public:
      //-----------------------------------------------------------------------
      SCTPInit(const make_private &) noexcept
      {
        ZS_EVENTING_1(x, i, Detail, SctpInitCreate, ol, SctpInit, Start, puid, id, mID);
        ZS_LOG_BASIC(log("created"))
      }

    protected:
      //-----------------------------------------------------------------------
      void init() noexcept
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
        usrsctp_sysctl_set_sctp_nr_outgoing_streams_default(SafeInt<uint32_t>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT)));

        mInitialized = true;
      }

      //-----------------------------------------------------------------------
      static SCTPInitPtr create() noexcept
      {
        SCTPInitPtr pThis(make_shared<SCTPInit>(make_private{}));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

    public:
      //-----------------------------------------------------------------------
      ~SCTPInit() noexcept
      {
        mThisWeak.reset();
        ZS_LOG_BASIC(log("destroyed"))
          cancel();
        ZS_EVENTING_1(x, i, Detail, SctpInitDestroy, ol, SctpInit, Stop, puid, id, mID);
      }

      //-----------------------------------------------------------------------
      static SCTPInitPtr singleton() noexcept
      {
        AutoRecursiveLock lock(*IHelper::getGlobalLock());
        static SingletonLazySharedPtr<SCTPInit> singleton(create());
        SCTPInitPtr result = singleton.singleton();

        static zsLib::SingletonManager::Register registerSingleton("org.ortc.SCTPInit", result);

        if (!result) {
          ZS_LOG_WARNING(Detail, slog("singleton gone"))
        }

        return result;
      }

    protected:
      //-----------------------------------------------------------------------
      //
      // SCTPInit => ISingletonManagerDelegate
      //

      //-----------------------------------------------------------------------
      virtual void notifySingletonCleanup() noexcept override
      {
        cancel();
      }

      //-----------------------------------------------------------------------
      //
      // SCTPInit => usrscpt callbacks
      //

      //-----------------------------------------------------------------------
      // This is the callback usrsctp uses when there's data to send on the network
      // that has been wrapped appropriatly for the SCTP protocol.
      static int OnSctpOutboundPacket(
        void* addr,
        void* data,
        size_t length,
        uint8_t tos,
        uint8_t set_df
      ) noexcept
      {
        ZS_ASSERT(addr);

        SCTPTransportPtr transport = (*(static_cast<SCTPTransportWeakPtr *>(addr))).lock();

        ZS_LOG_TRACE(slog("on sctp output packet") + ZS_PARAM("address", ((PTRNUMBER)addr)) + ZS_PARAM("length", length) + ZS_PARAM("tos", tos) + ZS_PARAM("set_df", set_df))

          if (ZS_IS_LOGGING(Insane)) {
            String str = IHelper::convertToBase64((const BYTE *)data, length);
            ZS_LOG_INSANE(slog("sctp outgoing packet") + ZS_PARAM("wire out", str))
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
      static void debug_sctp_printf(const char *format, ...) noexcept
      {
        char s[1024]{};

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
        ZS_MAYBE_USED() union sctp_sockstore addr,
        void* data,
        size_t length,
        struct sctp_rcvinfo rcv,
        int flags,
        void* ulp_info
      ) noexcept
      {
        ZS_MAYBE_USED(addr);
        ZS_ASSERT(ulp_info);

        SCTPTransportPtr transport = (*(static_cast<SCTPTransportWeakPtr *>(ulp_info))).lock();

        const SCTPPayloadProtocolIdentifier ppid = static_cast<SCTPPayloadProtocolIdentifier>(ntohl(rcv.rcv_ppid));

        if (0 == (flags & MSG_NOTIFICATION)) {
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
        }

        SCTPPacketIncomingPtr packet(make_shared<SCTPPacketIncoming>());

        packet->mType = ppid;
        packet->mSessionID = rcv.rcv_sid;
        packet->mSequenceNumber = rcv.rcv_ssn;
        packet->mTimestamp = rcv.rcv_tsn;
        packet->mFlags = flags;
        packet->mBuffer = IHelper::convertToBuffer((const BYTE *)data, length);

        if (!transport) {
          ZS_LOG_WARNING(Trace, slog("transport is gone (thus cannot receive packet)") + ZS_PARAM("socket", ((PTRNUMBER)sock)) + ZS_PARAM("length", length) + ZS_PARAM("flags", flags) + ZS_PARAM("ulp", ((PTRNUMBER)ulp_info)))
            errno = ESHUTDOWN;
          return -1;
        }

        auto queue = transport->getDeliveryQueue();

        queue->postClosure([transport, packet]() {
          transport->onIncomingPacket(packet);
        });
        return 0;
      }

    protected:
      //-----------------------------------------------------------------------
      //
      // SCTPInit => (internal)
      //

      //-----------------------------------------------------------------------
      Log::Params log(const char *message) const noexcept
      {
        ElementPtr objectEl = Element::create("ortc::SCTPInit");
        IHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      static Log::Params slog(const char *message) noexcept
      {
        return Log::Params(message, "ortc::SCTPInit");
      }

      //-----------------------------------------------------------------------
      Log::Params debug(const char *message) const noexcept
      {
        return Log::Params(message, toDebug());
      }

      //-----------------------------------------------------------------------
      virtual ElementPtr toDebug() const noexcept
      {
        AutoRecursiveLock lock(mLock);
        ElementPtr resultEl = Element::create("ortc::SCTPInit");

        IHelper::debugAppend(resultEl, "id", mID);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      void cancel() noexcept
      {
        ZS_EVENTING_1(x, i, Detail, SctpInitCancel, ol, SctpInit, Cancel, puid, id, mID);

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
      //
      // SCTPInit => (data)
      //

      AutoPUID mID;
      mutable RecursiveLock mLock;
      SCTPInitWeakPtr mThisWeak;

      std::atomic<bool> mInitialized{ false };
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportForDataChannel
    //

    //-------------------------------------------------------------------------
    ISCTPTransportForSCTPTransportListener::ForListenerPtr ISCTPTransportForSCTPTransportListener::create(
      UseListenerPtr listener,
      UseSecureTransportPtr secureTransport,
      WORD localPort
    ) noexcept
    {
      return ISCTPTransportFactory::singleton().create(listener, secureTransport, localPort);
    }

    //-------------------------------------------------------------------------
    ElementPtr ISCTPTransportForDataChannel::toDebug(ForDataChannelPtr transport) noexcept
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport
    //

    //---------------------------------------------------------------------------
    const char *SCTPTransport::toString(InternalStates state) noexcept
    {
      switch (state) {
        case InternalState_New:                     return "new";
        case InternalState_Connecting:              return "connecting";
        case InternalState_ConnectingDisrupted:     return "connecting disrupted";
        case InternalState_Ready:                   return "ready";
        case InternalState_Disconnected:            return "disconnected";
        case InternalState_ShuttingDown:            return "shutting down";
        case InternalState_Shutdown:                return "shutdown";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    ISCTPTransportTypes::States SCTPTransport::toState(InternalStates state) noexcept
    {
      switch (state) {
        case InternalState_New:                   return ISCTPTransportTypes::State_New;
        case InternalState_Connecting:            return ISCTPTransportTypes::State_Connecting;
        case InternalState_ConnectingDisrupted:   return ISCTPTransportTypes::State_Connecting;
        case InternalState_Ready:                 return ISCTPTransportTypes::State_Connected;
        case InternalState_Disconnected:          return ISCTPTransportTypes::State_Connected;
        case InternalState_ShuttingDown:          return ISCTPTransportTypes::State_Closed;
        case InternalState_Shutdown:              return ISCTPTransportTypes::State_Closed;
      }
      ZS_ASSERT_FAIL("state is not implemented:");
      return ISCTPTransportTypes::State_Closed;
    }

    //-------------------------------------------------------------------------
    SCTPTransport::SCTPTransport(
                                 const make_private &,
                                 IMessageQueuePtr queue,
                                 ISCTPTransportDelegatePtr originalDelegate,
                                 UseListenerPtr listener,
                                 UseSecureTransportPtr secureTransport,
                                 WORD localPort,
                                 WORD remotePort
                                 ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSCTPInit(SCTPInit::singleton()),
      mMaxSessionsPerPort(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT)),
      mSubscriptions(decltype(mSubscriptions)::create()),
      mDataChannelSubscriptions(decltype(mDataChannelSubscriptions)::create()),
      mListener(listener),
      mSecureTransport(secureTransport),
      mDeliveryQueue(IORTCForInternal::queueORTCPipeline()),
      mIncoming(0 != localPort),
      mLocalPort(localPort),
      mRemotePort(remotePort)
    {
      ZS_ASSERT(secureTransport);

      ZS_EVENTING_6(
        x, i, Detail, SctpTransportCreate, ol, SctpTransport, Start,
        puid, id, mID,
        puid, listenerId, ((bool)listener) ? listener->getID() : 0,
        size_t, maxSessionsPerPort, mMaxSessionsPerPort,
        bool, incoming, mIncoming,
        word, localPort, mLocalPort,
        word, remotePort, mRemotePort
      );

      ZS_LOG_DETAIL(debug("created"));

      ZS_ASSERT(mSCTPInit);

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::init() noexcept
    {
      AutoRecursiveLock lock(*this);

      ZS_LOG_DETAIL(debug("SCTP init"));

      auto secureTransport = mSecureTransport.lock();
      if (secureTransport) {
        mSecureTransportSubscription = secureTransport->subscribe(mThisWeak.lock());
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    SCTPTransport::~SCTPTransport() noexcept
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"));
      mThisWeak.reset();
      mThisSocket->reset();

      cancel();

      delete mThisSocket;
      mThisSocket = NULL;
      ZS_EVENTING_1(x, i, Detail, SctpTransportDestroy, ol, SctpTransport, Stop, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ISCTPTransportPtr object) noexcept
    {
      ISCTPTransportPtr original = ISCTPTransportTearAway::original(object);
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, original);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(IDataTransportPtr object) noexcept
    {
      ISCTPTransportPtr sctpTransport = ZS_DYNAMIC_PTR_CAST(ISCTPTransport, object);
      return convert(sctpTransport);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ForSettingsPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ForDataChannelPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ForListenerPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => IStatsProvider
    //

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr SCTPTransport::getStats(ZS_MAYBE_USED() const StatsTypeSet &stats) const noexcept
    {
      ZS_MAYBE_USED(stats);
#pragma ZS_BUILD_NOTE("TODO","Implement getStats")
      return PromiseWithStatsReport::createRejected(IORTCForInternal::queueDelegate());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => ISCTPTransport
    //

    //-------------------------------------------------------------------------
    ElementPtr SCTPTransport::toDebug(SCTPTransportPtr transport) noexcept
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    PUID SCTPTransport::getID() const noexcept
    {
      AutoRecursiveLock lock(*this);

      // Caller has seen the real ID thus do not hide the real ID.
      if (mObtainedID) return mID;

      // Ideally the redirect transport and this transport woud have the same
      // ID but that's not possible. To the external world hide the ID of
      // this transport's real ID if the redirect transport is in place. But
      // do not hide the ID if the real ID has been revealed before the
      // redirect was put into place.
      if (mRedirectToExistingTransport) {
        // Once a redirect is in place the redirect linkage cannot be broken.
        return mRedirectToExistingTransport->getID();
      }

      // Caller has obtained real ID so this ID cannot change for this object.
      mObtainedID = true; // mutuable
      return mID;
    }

    //-------------------------------------------------------------------------
    ISCTPTransportPtr SCTPTransport::create(
      ISCTPTransportDelegatePtr delegate,
      IDTLSTransportPtr transport,
      WORD inLocalPort
    ) noexcept(false)
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!transport);

      UseSecureTransportPtr useSecureTransport = DTLSTransport::convert(transport);
      ZS_ASSERT(((bool)useSecureTransport));

      auto dataTransport = useSecureTransport->getDataTransport();
      ORTC_THROW_INVALID_STATE_IF(!dataTransport);

      UseListenerPtr listener = SCTPTransportListener::convert(dataTransport);
      ORTC_THROW_INVALID_STATE_IF(!listener);

      Optional<WORD> allocatedLocalPort{};
      if (0 == inLocalPort) {
        allocatedLocalPort = listener->allocateLocalPort();
        ORTC_THROW_INVALID_STATE_IF(0 == allocatedLocalPort.value());
        inLocalPort = allocatedLocalPort.value();
      }

      SCTPTransportPtr pThis(make_shared<SCTPTransport>(make_private{}, IORTCForInternal::queueORTC(), delegate, listener, useSecureTransport));
      pThis->mThisWeak = pThis;
      pThis->mThisSocket = new SCTPTransportWeakPtr(pThis);

      ISCTPTransportPtr registeredTransport = pThis;

      // A tear away is created to allow caller to hold an instance of the tear
      // away as a means to ensure an instance of the listener is held as
      // well as this object since both objects are needed. This object cannot
      // hold an instance directly to the listener as the listener holds
      // instances to every SCTP transport as that would cause a circular
      // reference. Thus by using a tear away to hold the references to this
      // object and the listener prevents the circular reference.
      auto tearAway = ISCTPTransportTearAway::create(registeredTransport, make_shared<TearAwayData>());
      ORTC_THROW_INVALID_STATE_IF(!tearAway);

      auto tearAwayData = ISCTPTransportTearAway::data(tearAway);
      ORTC_THROW_INVALID_STATE_IF(!tearAwayData);

      tearAwayData->mListener = listener;

      AutoRecursiveLock lock(*pThis);
      pThis->mTearAway = tearAway;
      pThis->mLocalPort = inLocalPort;
      pThis->mAllocatedLocalPort = allocatedLocalPort;
      pThis->init();
      return tearAway;
    }

    //-------------------------------------------------------------------------
    ISCTPTransportTypes::States SCTPTransport::state() const noexcept
    {
      AutoRecursiveLock lock(*this);
      if (mRedirectToExistingTransport) return mRedirectToExistingTransport->state();
      return mLastReportedState;
    }

    //-------------------------------------------------------------------------
    IDTLSTransportPtr SCTPTransport::transport() const noexcept
    {
      // NOTE: Redirect and this object must share same secure transport.
      return DTLSTransport::convert(mSecureTransport.lock());
    }

    //-------------------------------------------------------------------------
    WORD SCTPTransport::port() const noexcept
    {
      // NOTE: Redirect and this object must share same secure port.
      return mLocalPort;
    }

    //-------------------------------------------------------------------------
    WORD SCTPTransport::localPort() const noexcept
    {
      // NOTE: Redirect and this object must share same secure port.
      return mLocalPort;
    }

    //-------------------------------------------------------------------------
    Optional<WORD> SCTPTransport::remotePort() const noexcept
    {
      // NOTE: Redirect and this object must share same secure port.
      return mRemotePort;
    }

    //-------------------------------------------------------------------------
    ISCTPTransportTypes::PromiseWithSocketOptionsPtr SCTPTransport::getOptions(const SocketOptions &inWhichOptions) noexcept
    {
      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) return mRedirectToExistingTransport->getOptions(inWhichOptions);

      auto promise = PromiseWithSocketOptions::create(IORTCForInternal::queueDelegate());

      if ((isShuttingDown()) ||
          (isShutdown())) {
        promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "transport is closing or closed"));
        return promise;
      }

      if (!mSocket) {
        mGetSocketOptions.push_back(PromiseWithSocketOptionsPair(promise, make_shared<SocketOptions>(inWhichOptions)));
        return promise;
      }

      auto tempOptions = make_shared<SocketOptions>(inWhichOptions);
      auto result = internalGetOptions(*tempOptions);

      if (result) {
        promise->resolve(tempOptions);
      } else {
        promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_BadRequest, "failed to get socket option"));
      }
      return promise;
    }

    //-------------------------------------------------------------------------
    PromisePtr SCTPTransport::setOptions(const SocketOptions &inOptions) noexcept
    {
      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) return mRedirectToExistingTransport->setOptions(inOptions);

      auto promise = Promise::create(IORTCForInternal::queueDelegate());

      if ((isShuttingDown()) ||
          (isShutdown())) {
        promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "transport is closing or closed"));
        return promise;
      }

      if (!mSocket) {
        mSetSocketOptions.push_back(PromiseSocketOptionsPair(promise, make_shared<SocketOptions>(inOptions)));
        return promise;
      }

      auto tempOptions = make_shared<SocketOptions>(inOptions);
      auto result = internalSetOptions(*tempOptions);

      if (result) {
        promise->resolve();
      } else {
        promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_BadRequest, "failed to set socket option"));
      }
      return promise;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::start(
                              const Capabilities &remoteCapabilities,
                              WORD inRemotePort
                              ) noexcept(false)
    {
      ZS_EVENTING_7(
                    x, i, Detail, SctpTransportStart, ol, SctpTransport, Start,
                    puid, id, mID,
                    size_t, maxMessageSize, remoteCapabilities.mMaxMessageSize,
                    word, minPort, remoteCapabilities.mMinPort,
                    word, maxPort, remoteCapabilities.mMaxPort,
                    word, maxUsablePorts, remoteCapabilities.mMaxUsablePorts,
                    word, maxSessionsPerPort, remoteCapabilities.mMaxSessionsPerPort,
                    word, remotePort, inRemotePort
                    );

      ZS_LOG_DEBUG(log("start called") + remoteCapabilities.toDebug());

      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) {
        mRedirectToExistingTransport->start(remoteCapabilities, inRemotePort);
        return;
      }

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ORTC_THROW_INVALID_STATE("already shutting down");
      }

      if (mCapabilities) {
        ORTC_THROW_INVALID_STATE_IF(!mStartedFromListener);
        if (mStartedFromListener) {
          // Not allowed to change capabilities after declaring the
          // capabilities to the sctp transport listener.
          ORTC_THROW_INVALID_STATE_IF(mCapabilities->mMaxMessageSize != remoteCapabilities.mMaxMessageSize);
          ORTC_THROW_INVALID_STATE_IF(mCapabilities->mMaxSessionsPerPort != remoteCapabilities.mMaxSessionsPerPort);
        }
        ZS_LOG_DEBUG(log("already started"));
        mStartedFromListener = false;

        // Needed to ensure any moved requests from a redirection get handled.
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
        return;
      }

      mStartedFromListener = false;

      if (0 != inRemotePort) {
        mRemotePort = inRemotePort;
      }

      auto secureTransport = mSecureTransport.lock();
      ORTC_THROW_INVALID_STATE_IF(!secureTransport);

      auto dataTransport = secureTransport->getDataTransport();

      ForListenerPtr forListener = mThisWeak.lock();
      auto originalForListener = forListener;

      UseListenerPtr listener = SCTPTransportListener::convert(dataTransport);
      ORTC_THROW_INVALID_STATE_IF(!listener);

      WORD localPort = mLocalPort;
      WORD remotePort = mRemotePort.hasValue() ? mRemotePort.value() : 0;
      listener->registerNewTransport(DTLSTransport::convert(secureTransport), forListener, localPort, mAllocatedLocalPort.hasValue(), remotePort);
      if (!forListener) {
        ZS_LOG_WARNING(Debug, slog("unable to allocate port") + ZS_PARAM("local port", mLocalPort) + ZS_PARAM("remote port", mRemotePort));
        ORTC_THROW_INVALID_PARAMETERS("unable to allocate port, local port=" + string(mLocalPort) + ", remote port=" + string(remotePort));
      }

      mLocalPort = localPort;
      mRemotePort = remotePort;
      mCapabilities = make_shared<Capabilities>(remoteCapabilities);

      // The SCTP transport used in the mapping of local:remote may already
      // exist if the remote party initialized an SCTP transport before the
      // local side has a chance to start its SCTP transport. This ensures
      // this object remaps all requrests to an existing SCTP transport rather
      // than creating a duplicate transport.
      if (forListener != originalForListener) {
        mRedirectToExistingTransport = SCTPTransport::convert(forListener);

        if (mRedirectToExistingTransport) {
          AutoRecursiveLock lock2(*mRedirectToExistingTransport);

          if ((mRedirectToExistingTransport->isShuttingDown()) ||
              (mRedirectToExistingTransport->isShutdown())) {
            ORTC_THROW_INVALID_STATE("already shutting down");
          }

          // Need to move get/set options to new transport
          {
            for (auto iter = mGetSocketOptions.begin(); iter != mGetSocketOptions.end(); ++iter) {
              auto &value = (*iter);
              mRedirectToExistingTransport->mGetSocketOptions.push_back(value);
            }
            for (auto iter = mSetSocketOptions.begin(); iter != mSetSocketOptions.end(); ++iter) {
              auto &value = (*iter);
              mRedirectToExistingTransport->mSetSocketOptions.push_back(value);
            }

            mGetSocketOptions.clear();
            mSetSocketOptions.clear();
          }

          // Need to move the existing data channels to the redirected transport.
          {
            for (auto iter = mSessions.begin(); iter != mSessions.end(); ++iter) {
              auto &sessionID = (*iter).first;
              auto &dataChannel = (*iter).second;
              if (!dataChannel) continue;

              bool remapped = false;

              try {
                SessionID tempID = sessionID;
                UseDataChannelPtr channel = dataChannel;

                if (!mRedirectToExistingTransport->isSessionAvailable(sessionID)) goto skip_register;
                mRedirectToExistingTransport->registerNewDataChannel(channel, tempID);
                remapped = true;
              } catch(const InvalidParameters &) {
                goto skip_register;
              } catch(const InvalidStateError &) {
                goto skip_register;
              }

            skip_register:
              {
                if (!remapped) {
                  mFilterSessionRequests.insert(sessionID);
                  dataChannel->notifyRemapFailure();
                  continue;
                }

                auto found = mAnnouncedIncomingDataChannels.find(dataChannel->getID());
                if (found != mAnnouncedIncomingDataChannels.end()) {
                  mRedirectToExistingTransport->mAnnouncedIncomingDataChannels[dataChannel->getID()] = dataChannel;
                }
              }
            }

            // sessions being reset should also be moved to a new transport
            for (auto iter = mPendingResetSessions.begin(); iter != mPendingResetSessions.end(); ++iter) {
              auto &sessionID = (*iter).first;
              auto &dataChannel = (*iter).second;
              if (!dataChannel) continue;

              if (!mRedirectToExistingTransport->isSessionAvailable(sessionID)) {
                mFilterSessionRequests.insert(sessionID);
                dataChannel->notifyClosed();
                continue;
              }
              auto found = mAnnouncedIncomingDataChannels.find(dataChannel->getID());
              if (found != mAnnouncedIncomingDataChannels.end()) {
                mRedirectToExistingTransport->mAnnouncedIncomingDataChannels[dataChannel->getID()] = dataChannel;
              }
              mRedirectToExistingTransport->mPendingResetSessions[sessionID] = dataChannel;
            }

            // sessions queued to reset should also be moved to a new transport
            for (auto iter = mQueuedResetSessions.begin(); iter != mQueuedResetSessions.end(); ++iter) {
              auto &sessionID = (*iter).first;
              auto &dataChannel = (*iter).second;
              if (!dataChannel) continue;

              if (!mRedirectToExistingTransport->isSessionAvailable(sessionID)) {
                mFilterSessionRequests.insert(sessionID);
                dataChannel->notifyClosed();
                continue;
              }
              auto found = mAnnouncedIncomingDataChannels.find(dataChannel->getID());
              if (found != mAnnouncedIncomingDataChannels.end()) {
                mRedirectToExistingTransport->mAnnouncedIncomingDataChannels[dataChannel->getID()] = dataChannel;
              }
              mRedirectToExistingTransport->mQueuedResetSessions[sessionID] = dataChannel;
            }

            // everything is now moved
            mSessions.clear();
            mPendingResetSessions.clear();
            mQueuedResetSessions.clear();
            mAnnouncedIncomingDataChannels.clear();

            // finally is socket is setup then kill the socket
            if (mSocket) {
              usrsctp_close(mSocket);
              mSocket = NULL;
              usrsctp_deregister_address(mThisSocket);
            }
          }

          mRedirectToExistingTransportSubscription = mRedirectToExistingTransport->subscribe(mThisWeak.lock());
          mRedirectToExistingTransport->start(remoteCapabilities, inRemotePort);
        }
        return;
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::stop() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, SctpTransportStop, ol, SctpTransport, Stop, puid, id, mID);
      ZS_LOG_DEBUG(log("stop called"))

      AutoRecursiveLock lock(*this);
      if (mRedirectToExistingTransport) return mRedirectToExistingTransport->stop();
      cancel();
    }

    //-------------------------------------------------------------------------
    ISCTPTransportSubscriptionPtr SCTPTransport::subscribe(ISCTPTransportDelegatePtr originalDelegate) noexcept
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"));

      AutoRecursiveLock lock(*this);

      ISCTPTransportSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      ISCTPTransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        SCTPTransportPtr pThis = mThisWeak.lock();

        if (mRedirectToExistingTransport) {
          AutoRecursiveLock lock2(*mRedirectToExistingTransport);

          for (auto iter = mRedirectToExistingTransport->mAnnouncedIncomingDataChannels.begin(); iter != mRedirectToExistingTransport->mAnnouncedIncomingDataChannels.end(); ++iter) {
            // NOTE: ID of data channels are always greater than last so order should be guarenteed.
            auto dataChannel = (*iter).second;
            delegate->onSCTPTransportDataChannel(pThis, DataChannel::convert(dataChannel));
          }

          if (State_New != mRedirectToExistingTransport->mLastReportedState) {
            delegate->onSCTPTransportStateChange(pThis, mRedirectToExistingTransport->mLastReportedState);
          }
        } else {
          for (auto iter = mAnnouncedIncomingDataChannels.begin(); iter != mAnnouncedIncomingDataChannels.end(); ++iter) {
            // NOTE: ID of data channels are always greater than last so order
            // should be guarenteed.
            auto dataChannel = (*iter).second;
            delegate->onSCTPTransportDataChannel(pThis, DataChannel::convert(dataChannel));
          }

          if (State_New != mLastReportedState) {
            delegate->onSCTPTransportStateChange(pThis, mLastReportedState);
          }
        }
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
    //
    // SCTPTransport => ISCTPTransportForDataChannel
    //

    //-------------------------------------------------------------------------
    void SCTPTransport::registerNewDataChannel(
                                               UseDataChannelPtr &ioDataChannel,
                                               WORD &ioSessionID
                                               ) noexcept(false)
    {
      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) {
        mRedirectToExistingTransport->registerNewDataChannel(ioDataChannel, ioSessionID);
        return;
      }

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
          ZS_EVENTING_3(
                        x, i, Detail, SctpTransportRegisterNewDataChannel, ol, SctpTransport, Info,
                        puid, id, mID,
                        puid, dataChannelId, ((bool)ioDataChannel) ? ioDataChannel->getID() : 0,
                        word, sessionId, ioSessionID
                        );
          return;
        }

        if (mSessions.size() >= mMaxSessionsPerPort) {
          ZS_LOG_WARNING(Debug, log("too many sessions on sctp port open") + ZS_PARAM("active sessions", mSessions.size()) + ZS_PARAM("max", mMaxSessionsPerPort))
          ORTC_THROW_INVALID_STATE("too many sessions open on port, sessions=" + string(mSessions.size()) + ", max=" + string(mMaxSessionsPerPort))
        }
        
        ioSessionID = sessionID;
        ioDataChannel = dataChannel;
        mSessions[sessionID] = dataChannel;

        ZS_EVENTING_3(
                      x, i, Detail, SctpTransportRegisterNewDataChannel, ol, SctpTransport, Info,
                      puid, id, mID,
                      puid, dataChannelId, ((bool)ioDataChannel) ? ioDataChannel->getID() : 0,
                      word, sessionId, ioSessionID
                      );

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
      ioSessionID = sessionID;
      mSessions[sessionID] = dataChannel;

      ZS_EVENTING_3(
                    x, i, Detail, SctpTransportRegisterNewDataChannel, ol, SctpTransport, Info,
                    puid, id, mID,
                    puid, dataChannelId, ((bool)ioDataChannel) ? ioDataChannel->getID() : 0,
                    word, sessionId, ioSessionID
                    );
    }

    //-------------------------------------------------------------------------
    ISCTPTransportForDataChannelSubscriptionPtr SCTPTransport::subscribe(ISCTPTransportForDataChannelDelegatePtr originalDelegate) noexcept
    {
      ZS_LOG_DETAIL(log("datachannel subscribing to SCTP Transport"));
      
      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) return mRedirectToExistingTransport->subscribe(originalDelegate);

      ISCTPTransportForDataChannelSubscriptionPtr subscription = mDataChannelSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueORTC());
      
      ISCTPTransportForDataChannelDelegatePtr delegate = mDataChannelSubscriptions.delegate(subscription, true);
      
      if (delegate) {
        SCTPTransportPtr pThis = mThisWeak.lock();

        if (InternalState_New != mCurrentState) {
          delegate->onSCTPTransportStateChanged();
        }
      }
      
      if (isShutdown()) {
        mSubscriptions.clear();
      }
      
      return subscription;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::isReady() const noexcept
    {
      AutoRecursiveLock lock(*this);
      if (mRedirectToExistingTransport) return mRedirectToExistingTransport->isReady();
      return InternalState_Ready == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::announceIncoming(
                                         UseDataChannelPtr dataChannel,
                                         ParametersPtr params
                                         ) noexcept
    {
      ZS_ASSERT(dataChannel);
      ZS_ASSERT(params);

      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) {
        if (mFilterSessionRequests.end() != mFilterSessionRequests.find(params->mID.value())) return;
        mRedirectToExistingTransport->announceIncoming(dataChannel, params);
        return;
      }

      auto found = mSessions.find(params->mID.value());
      if (found == mSessions.end()) {
        ZS_LOG_WARNING(Debug, log("cannot announce incoming session as it is not active") + params->toDebug())
        return;
      }

      if ((isShutdown()) ||
          (isShuttingDown())) {
        ZS_LOG_WARNING(Debug, log("cannot announce incoming session while shutting down / shutdown") + params->toDebug())
        return;
      }

      mAnnouncedIncomingDataChannels[dataChannel->getID()] = dataChannel;

      ZS_LOG_DEBUG(log("notify delegates of incoming data channel") + ZS_PARAM("data channel", dataChannel->getID()));

      ZS_EVENTING_2(
                    x, i, Detail, SctpTransportDataChannelEvent, ol, SctpTransport, Event,
                    puid, id, mID,
                    puid, dataChannelId, dataChannel->getID()
                    );

      mSubscriptions.delegate()->onSCTPTransportDataChannel(mThisWeak.lock(), DataChannel::convert(dataChannel));
    }

    //-------------------------------------------------------------------------
    PromisePtr SCTPTransport::sendDataNow(SCTPPacketOutgoingPtr packet) noexcept
    {
      ZS_EVENTING_8(
                    x, i, Trace, SctpTransportSendOutgoingPacket, ol, SctpTransport, Send,
                    puid, id, mID,
                    word, sessionId, packet->mSessionID,
                    bool, ordered, packet->mOrdered,
                    duration, maxPacketLifetimeInMilliseconds, packet->mMaxPacketLifetime.count(),
                    bool, hasMaxRetransmits, packet->mMaxRetransmits.hasValue(),
                    ulong, maxRetransmits, packet->mMaxRetransmits.value(),
                    buffer, data, ((bool)packet->mBuffer) ? packet->mBuffer->BytePtr() : NULL,
                    size, size, ((bool)packet->mBuffer) ? size_t((SafeInt<unsigned int>(packet->mBuffer->SizeInBytes()))) : 0
                    );

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel::RejectReason, RejectReason);

      {
        AutoRecursiveLock lock(*this);

        if (mRedirectToExistingTransport) {
          if (mFilterSessionRequests.end() != mFilterSessionRequests.find(packet->mSessionID)) return Promise::createRejected(RejectReason::create(UseHTTP::HTTPStatusCode_Conflict, "cannot send because of a remap conflict"), IORTCForInternal::queueORTC());;
          return mRedirectToExistingTransport->sendDataNow(packet);
        }

        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_WARNING(Debug, log("cannot send data while shutting down / shutdown"))
          return Promise::createRejected(RejectReason::create(UseHTTP::HTTPStatusCode_ClientClosedRequest, "cannot send during shutdown"), IORTCForInternal::queueORTC());
        }

        if (InternalState_Ready != mCurrentState) goto waiting_to_send;
        if (!mWriteReady) goto waiting_to_send;

        if (packet->mBuffer) {
          if (packet->mBuffer->SizeInBytes() > mCapabilities->mMaxMessageSize) {
            ZS_LOG_ERROR(Detail, log("attempting to send packet larger than remote is capable") + ZS_PARAM("buffer size", packet->mBuffer->SizeInBytes()) + mCapabilities->toDebug())
            return Promise::createRejected(RejectReason::create(UseHTTP::HTTPStatusCode_BandwidthLimitExceeded, "buffer too large to send"), IORTCForInternal::queueORTC());
          }
        }

        bool wouldBlock = false;
        if (!attemptSend(*packet, wouldBlock)) {
          if (wouldBlock) goto waiting_to_send;

          ZS_LOG_WARNING(Debug, log("unable to send packet at this time"))
          return Promise::createRejected(RejectReason::create(UseHTTP::HTTPStatusCode_ExpectationFailed, "unexpected error"), IORTCForInternal::queueORTC());
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
                                        ) noexcept
    {
      ZS_EVENTING_3(
                    x, i, Detail, SctpTransportShutdownDataChannel, ol, SctpTransport, Info,
                    puid, id, mID,
                    puid, dataChannelId, dataChannel->getID(),
                    word, sessionId, sessionID
                    );

      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) {
        if (mFilterSessionRequests.end() != mFilterSessionRequests.find(sessionID)) return;
        mRedirectToExistingTransport->requestShutdown(dataChannel, sessionID);
        return;
      }

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
    //
    // SCTPTransport => ISCTPTransportForSecureTransport
    //

    //-------------------------------------------------------------------------
    SCTPTransport::ForListenerPtr SCTPTransport::create(
                                                        UseListenerPtr listener,
                                                        UseSecureTransportPtr secureTransport,
                                                        WORD localPort
                                                        ) noexcept
    {
      SCTPTransportPtr pThis(make_shared<SCTPTransport>(make_private {}, IORTCForInternal::queueORTC(), ISCTPTransportDelegatePtr(), listener, secureTransport, localPort));
      pThis->mThisWeak = pThis;
      pThis->mThisSocket = new SCTPTransportWeakPtr(pThis);
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::startFromListener(
                                          const Capabilities &remoteCapabilities,
                                          WORD remotePort
                                          ) noexcept(false)
    {
      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) {
        mRedirectToExistingTransport->startFromListener(remoteCapabilities, remotePort);
        return;
      }

      if (mCapabilities) {
        ZS_LOG_DEBUG(log("already started from remote"));
        return;
      }
      start(remoteCapabilities, remotePort);
      mStartedFromListener = true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::handleDataPacket(
                                         const BYTE *buffer,
                                         size_t bufferLengthInBytes
                                         ) noexcept
    {
      ZS_EVENTING_3(
                    x, i, Trace, SctpTransportReceivedIncomingDataPacket, ol, SctpTransport, Receive,
                    puid, id, mID,
                    buffer, data, buffer,
                    size, size, bufferLengthInBytes
                    );

      if (bufferLengthInBytes < sizeof(DWORD)) {
        ZS_LOG_WARNING(Trace, log("packet length is too small to be an SCTP packet"))
        return false;
      }

      {
        AutoRecursiveLock lock(*this);

        if (mRedirectToExistingTransport) return mRedirectToExistingTransport->handleDataPacket(buffer, bufferLengthInBytes);

        {
          if (isShutdown()) {
            ZS_LOG_WARNING(Debug, log("packet arrived after shutdown"))
            return false;
          }

          if (!mCapabilities) goto queue_packet;
          if (mPendingIncomingBuffers.size() > 0) goto queue_packet;
          if (!mSocket) goto queue_packet;

          usrsctp_conninput(mThisSocket, buffer, bufferLengthInBytes, 0);

          return true;
        }

      queue_packet:
        {
          ZS_EVENTING_3(
                        x, i, Trace, SctpTransportBufferIncomingDataPacket, ol, SctpTransport, Buffer,
                        puid, id, mID,
                        buffer, data, buffer,
                        size, size, bufferLengthInBytes
                        );

          mPendingIncomingBuffers.push(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes));
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::notifyShutdown() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, SctpTransportInternalShutdownEvent, ol, SctpTransport, InternalEvent, puid, id, mID);

      ISCTPTransportAsyncDelegateProxy::create(mThisWeak.lock())->onNotifiedToShutdown();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => friend SCTPInit
    //

    //-------------------------------------------------------------------------
    bool SCTPTransport::notifySendSCTPPacket(
                                             const BYTE *buffer,
                                             size_t bufferLengthInBytes
                                             ) noexcept
    {
      // WARNING: DO NOT ENTER A LOCK AS IT COULD CAUSE A DEADLOCK.
      //          usrsctp calls this method which has a lock and an attempt
      //          could be made to send a packet into usrsctp while attempting
      //          to deliver a packet from usrsctp.

      UseSecureTransportPtr transport = mSecureTransport.lock();

      if (!transport) {
        ZS_LOG_WARNING(Trace, log("secure transport is gone (thus send packet is not available)") + ZS_PARAM("buffer length", bufferLengthInBytes))
        return false;
      }

      ZS_EVENTING_4(
                    x, i, Detail, SctpTransportSendOutgoingDataPacket, ol, SctpTransport, Send,
                    puid, id, mID,
                    puid, secureTransportId, transport->getID(),
                    buffer, data, buffer,
                    size, size, bufferLengthInBytes
                    );

      // NOTE: No need to redirect to existing transport as same secure
      // secure transport is used.
      return transport->sendDataPacket(buffer, bufferLengthInBytes);
    }



    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => ISCTPTransportDelegate
    //

    //-------------------------------------------------------------------------
    void SCTPTransport::onSCTPTransportStateChange(
                                                   ISCTPTransportPtr transport,
                                                   ISCTPTransportTypes::States state
                                                   )
    {
      auto pThis = mThisWeak.lock();

      ZS_EVENTING_1(x, i, Detail, SctpTransportInternalSCTPTransportStateChange, ol, SctpTransport, InternalEvent, puid, id, mID);

      AutoRecursiveLock lock(*this);

      // simulate redirect transport event as if it came from this transport
      mSubscriptions.delegate()->onSCTPTransportStateChange(pThis, state);
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::onSCTPTransportDataChannel(
                                                   ISCTPTransportPtr transport,
                                                   IDataChannelPtr channel
                                                   )
    {
      auto pThis = mThisWeak.lock();

      ZS_EVENTING_1(x, i, Detail, SctpTransportInternalSCTPTransportDataChannel, ol, SctpTransport, InternalEvent, puid, id, mID);

      AutoRecursiveLock lock(*this);

      // simulate redirect transport event as if it came from this transport
      mSubscriptions.delegate()->onSCTPTransportDataChannel(pThis, channel);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void SCTPTransport::onWake()
    {
      ZS_EVENTING_1(x, i, Detail, SctpTransportInternalWakeEvent, ol, SctpTransport, InternalEvent, puid, id, mID);
      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => ITimerDelegate
    //

    //-------------------------------------------------------------------------
    void SCTPTransport::onTimer(ITimerPtr timer)
    {
      // UNUSED
      // ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))
      // AutoRecursiveLock lock(*this);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => ISCTPTransportAsyncDelegate
    //

    //-------------------------------------------------------------------------
    void SCTPTransport::onIncomingPacket(SCTPPacketIncomingPtr packet)
    {
      ZS_EVENTING_7(
                    x, i, Trace, SctpTransportReceivedIncomingPacket, ol, SctpTransport, Receive,
                    puid, id, mID,
                    word, sessionId, packet->mSessionID,
                    word, sequenceNumber, packet->mSequenceNumber,
                    dword, timestamp, packet->mTimestamp,
                    int, flags, packet->mFlags,
                    buffer, data, ((bool)packet->mBuffer) ? packet->mBuffer->BytePtr() : NULL,
                    size, size, ((bool)packet->mBuffer) ? size_t(SafeInt<unsigned int>(packet->mBuffer->SizeInBytes())) : 0
                    );

      ZS_LOG_TRACE(log("on incoming packet") + packet->toDebug())

      if (0 != (packet->mFlags & MSG_NOTIFICATION)) {
        ZS_LOG_TRACE(log("incoming packet is a notification packet") + packet->toDebug())

        if (!packet->mBuffer) {
          ZS_LOG_WARNING(Detail, log("incoming notification packet missing data") + packet->toDebug())
          return;
        }

        const sctp_notification &notification = reinterpret_cast<const sctp_notification&>(*(packet->mBuffer->BytePtr()));
        ZS_THROW_INVALID_ASSUMPTION_IF(notification.sn_header.sn_length != packet->mBuffer->SizeInBytes())

        AutoRecursiveLock lock(*this);
        if (mRedirectToExistingTransport) {
          mRedirectToExistingTransport->handleNotificationPacket(notification);
          return;
        }
        handleNotificationPacket(notification);
        return;
      }

      UseDataChannelPtr dataChannel;

      {
        AutoRecursiveLock lock(*this);

        if (mRedirectToExistingTransport) {
          if (mFilterSessionRequests.end() != mFilterSessionRequests.find(packet->mSessionID)) return;
          mRedirectToExistingTransport->onIncomingPacket(packet);
          return;
        }

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
        if (!dataChannel) {
          ZS_LOG_WARNING(Detail, log("data channel is not known (likely already closed)") + packet->toDebug());
          return;
        }
        ZS_EVENTING_8(
                      x, i, Trace, SctpTransportDeliverIncomingPacket, ol, SctpTransport, Deliver,
                      puid, id, mID,
                      puid, dataChannelId, dataChannel->getID(),
                      word, sessionId, packet->mSessionID,
                      word, sequenceNumber, packet->mSequenceNumber,
                      dword, timestamp, packet->mTimestamp,
                      int, flags, packet->mFlags,
                      buffer, data, ((bool)packet->mBuffer) ? packet->mBuffer->BytePtr() : NULL,
                      size, size, ((bool)packet->mBuffer) ? size_t(SafeInt<unsigned int>(packet->mBuffer->SizeInBytes())) : 0
                      );
        ZS_LOG_TRACE(log("forwarding to data channel") + ZS_PARAM("data channel", dataChannel->getID()) + packet->toDebug());
        dataChannel->handleSCTPPacket(packet);
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::onNotifiedToShutdown()
    {
      ZS_LOG_TRACE(log("on notified to shutdown"))

      AutoRecursiveLock lock(*this);
      if (mRedirectToExistingTransport) {
        mRedirectToExistingTransport->onNotifiedToShutdown();
        return;
      }
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => IICETransportDelegate
    //

    //-------------------------------------------------------------------------
    void SCTPTransport::onSecureTransportStateChanged(
                                                      ISecureTransportPtr transport,
                                                      ISecureTransportTypes::States state
                                                      )
    {
      ZS_EVENTING_3(
                    x, i, Trace, SctpTransportInternalSecureTransportStateChangedEvent, ol, SctpTransport, InternalEvent,
                    puid, id, mID,
                    puid, secureTransportId, transport->getID(),
                    string, state, ISecureTransportTypes::toString(state)
                    );

      ZS_LOG_DEBUG(log("secure transport state changed") + ZS_PARAM("secure transport id", transport->getID()) + ZS_PARAM("state", ISecureTransportTypes::toString(state)))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransport => (internal)
    //

    //-------------------------------------------------------------------------
    Log::Params SCTPTransport::log(const char *message) const noexcept
    {
      ElementPtr objectEl = Element::create("ortc::SCTPTransport");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SCTPTransport::slog(const char *message) noexcept
    {
      ElementPtr objectEl = Element::create("ortc::SCTPTransport");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SCTPTransport::debug(const char *message) const noexcept
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr SCTPTransport::toDebug() const noexcept
    {
      AutoRecursiveLock lock(*this);

      if (mRedirectToExistingTransport) return mRedirectToExistingTransport->toDebug();

      ElementPtr resultEl = Element::create("ortc::SCTPTransport");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "sctp init", (bool)mSCTPInit);
      IHelper::debugAppend(resultEl, "mMaxSessionsPerPort", mMaxSessionsPerPort);

      IHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());

      IHelper::debugAppend(resultEl, "data channel subscriptions", mDataChannelSubscriptions.size());

      IHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      IHelper::debugAppend(resultEl, "error", mLastError);
      IHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto listener = mListener.lock();
      IHelper::debugAppend(resultEl, "listener", listener ? listener->getID() : 0);

      auto secureTransport = mSecureTransport.lock();
      IHelper::debugAppend(resultEl, "secure transport", secureTransport ? secureTransport->getID() : 0);
      IHelper::debugAppend(resultEl, "secure transport subscription", (bool)mSecureTransportSubscription);

      IHelper::debugAppend(resultEl, mCapabilities ? mCapabilities->toDebug() : ElementPtr());

      IHelper::debugAppend(resultEl, "this socket", (PTRNUMBER)mThisSocket);

      IHelper::debugAppend(resultEl, "incoming", mIncoming);

      IHelper::debugAppend(resultEl, "socket", (PTRNUMBER)mSocket);

      IHelper::debugAppend(resultEl, "local port", mLocalPort);
      IHelper::debugAppend(resultEl, "remote port", mRemotePort);

      IHelper::debugAppend(resultEl, "announced data channels", mAnnouncedIncomingDataChannels.size());

      IHelper::debugAppend(resultEl, "sessions", mSessions.size());

      IHelper::debugAppend(resultEl, "pending reset", mPendingResetSessions.size());
      IHelper::debugAppend(resultEl, "queued reset", mQueuedResetSessions.size());

      IHelper::debugAppend(resultEl, "settled role", mSettledRole);
      IHelper::debugAppend(resultEl, "current allocation", mCurrentAllocationSessionID);
      IHelper::debugAppend(resultEl, "min allocation", mMinAllocationSessionID);
      IHelper::debugAppend(resultEl, "max allocation", mMaxAllocationSessionID);
      IHelper::debugAppend(resultEl, "next allocation increment", mNextAllocationIncrement);

      IHelper::debugAppend(resultEl, "waiting to send", mWaitingToSend.size());

      IHelper::debugAppend(resultEl, "connected", mConnected);
      IHelper::debugAppend(resultEl, "write ready", mWriteReady);

      IHelper::debugAppend(resultEl, "pending incoming buffers", mPendingIncomingBuffers.size());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::isShuttingDown() const noexcept
    {
      return InternalState_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::isShutdown() const noexcept
    {
      return InternalState_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::step() noexcept
    {
      ZS_LOG_DEBUG(debug("step"));

      if (mRedirectToExistingTransport) {
        mRedirectToExistingTransport->step();
        return;
      }

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      ZS_EVENTING_1(x, i, Debug, SctpTransportStep, ol, SctpTransport, Step, puid, id, mID);

      // ... other steps here ...
      if (!stepStartCalled()) goto not_ready;
      if (!stepSecureTransport()) goto not_ready;
      if (!stepOpen()) goto not_ready;
      if (!stepDeliverIncomingPackets()) goto not_ready;
      if (!stepConnected()) goto not_ready;
      if (!stepResetStream()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("SCTP is NOT ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("SCTP is ready"))
        setState(InternalState_Ready);
      }
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepStartCalled() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, SctpTransportStep, ol, SctpTransport, Step, puid, id, mID);

      if (!mCapabilities) {
        ZS_LOG_TRACE(log("waiting for start to be called"))
        return false;
      }

      ZS_LOG_TRACE(log("start is called"))
      if (InternalState_New == mCurrentState) {
        setState(InternalState_Connecting);
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepSecureTransport() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, SctpTransportStep, ol, SctpTransport, Step, puid, id, mID);

      auto secureTransport = mSecureTransport.lock();
      if (!secureTransport) {
        ZS_LOG_WARNING(Detail, log("secure transport is now gone (thus must shutdown)"))
        cancel();
        return false;
      }

      switch (secureTransport->state()) {
        case ISecureTransportTypes::State_Pending:
        {
          ZS_LOG_TRACE(log("waiting for secure transport to be ready"))
          return false;
        }
        case ISecureTransportTypes::State_Connected:
        {
          ZS_LOG_TRACE(log("secure transport already notified ready"));
          InternalStates currentState = mCurrentState;
          switch (currentState)
          {
            case InternalState_New:
            case InternalState_Connecting:
            case InternalState_ConnectingDisrupted:
            {
              setState(InternalState_Connecting);
              break;
            }
            case InternalState_Ready:
            case InternalState_Disconnected:
            {
              setState(InternalState_Ready);
              break;
            }
            case InternalState_ShuttingDown:
            case InternalState_Shutdown:
            {
              ZS_LOG_WARNING(Trace, log("already shutting down"))
              cancel();
              return false;
            }
          }
          break;
        }
        case ISecureTransportTypes::State_Disconnected:
        {
          InternalStates currentState = mCurrentState;
          switch (currentState)
          {
            case InternalState_New:
            case InternalState_Connecting:
            case InternalState_ConnectingDisrupted:
            {
              setState(InternalState_ConnectingDisrupted);
              break;
            }
            case InternalState_Ready:
            case InternalState_Disconnected:
            {
              setState(InternalState_Disconnected);
              break;
            }
            case InternalState_ShuttingDown:
            case InternalState_Shutdown:
            {
              ZS_LOG_WARNING(Trace, log("already shutting down"))
              cancel();
              return false;
            }
          }
          return false;
        }
        case ISecureTransportTypes::State_Closed:
        {
          ZS_LOG_WARNING(Detail, log("secure transport is now gone (thus must shutdown)"))
          cancel();
          return false;
        }
      }

      if (mSettledRole) {
        ZS_LOG_TRACE(log("role of secure transport already settled"))
        return true;
      }

      ZS_LOG_DEBUG(log("secure transport notified ready"))

      mSettledRole = true;

      auto clientRole = secureTransport->isClientRole();
      if (clientRole) {
        mCurrentAllocationSessionID = mMinAllocationSessionID;    // client picks even port numbers
      } else {
        mCurrentAllocationSessionID = mMinAllocationSessionID+1;  // server picks odd port numbers
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepOpen() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, SctpTransportStep, ol, SctpTransport, Step, puid, id, mID);

      ZS_LOG_TRACE(log("open connect socket"))
      if (!openConnectSCTPSocket()) {
        ZS_LOG_ERROR(Detail, log("failed to open connect port"))
        cancel();
        return false;
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepDeliverIncomingPackets() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, SctpTransportStep, ol, SctpTransport, Step, puid, id, mID);

      ZS_ASSERT(mSocket);

      if (mPendingIncomingBuffers.size() < 1) {
        ZS_LOG_TRACE(log("no pending packets to deliver"))
        return true;
      }

      ZS_LOG_DEBUG(log("delivering pending packets") + ZS_PARAM("pending packets", mPendingIncomingBuffers.size()))

      BufferQueue pending = mPendingIncomingBuffers;
      mPendingIncomingBuffers = BufferQueue();

      while (pending.size() > 0) {
        SecureByteBlockPtr buffer = pending.front();
        handleDataPacket(buffer->BytePtr(), buffer->SizeInBytes());
        ZS_EVENTING_3(
                      x, i, Trace, SctpTransportDisposeBufferedIncomingDataPacket, ol, SctpTransport, Dispose,
                      puid, id, mID,
                      buffer, data, buffer->BytePtr(),
                      size, size, buffer->SizeInBytes()
                      );
        pending.pop();
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepConnected() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, SctpTransportStep, ol, SctpTransport, Step, puid, id, mID);

      if (!mConnected) {
        ZS_LOG_TRACE(log("waiting to be connected"))
        return false;
      }

      ZS_LOG_TRACE(log("connected"))
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepResetStream() noexcept
    {
      ZS_EVENTING_1(x, i, Debug, SctpTransportStep, ol, SctpTransport, Step, puid, id, mID);

      if (mAttemptResetLater) {
        ZS_LOG_TRACE(log("waiting for previous reset in progress to complete"))
        return true;
      }
      if (mPendingResetSessions.size() > 0) {
        ZS_LOG_TRACE(log("still waiting for stream resets to complete"))
        return true;
      }

      if (mQueuedResetSessions.size() < 1) {
        ZS_LOG_TRACE(log("no streams waiting to reset"))
        return true;
      }

      for (auto iter = mQueuedResetSessions.begin(); iter != mQueuedResetSessions.end(); ++iter) {
        auto sessionID = (*iter).first;
        auto dataChannel = (*iter).second;
        mPendingResetSessions[sessionID] = dataChannel;
      }

      const size_t numStreams = mPendingResetSessions.size();
      const size_t numBytes = sizeof(struct sctp_reset_streams) + (numStreams * sizeof(WORD));

      SecureByteBlockPtr buffer(make_shared<SecureByteBlock>(numBytes));

      struct sctp_reset_streams *pReset = reinterpret_cast<struct sctp_reset_streams *>(buffer->BytePtr());

      pReset->srs_assoc_id = SCTP_ALL_ASSOC;
      pReset->srs_flags = SCTP_STREAM_RESET_INCOMING | SCTP_STREAM_RESET_OUTGOING;
      pReset->srs_number_streams = SafeInt<decltype(pReset->srs_number_streams)>(numStreams);

      int index = 0;
      for (auto iter = mPendingResetSessions.begin(); iter != mPendingResetSessions.end(); ++iter, ++index)
      {
        auto sessionID = (*iter).first;
        pReset->srs_stream_list[index] = sessionID;
        ZS_LOG_TRACE(log("will need to reset stream") + ZS_PARAM("session ID", sessionID))
      }

      ZS_LOG_DEBUG(log("sending stream reset request") + ZS_PARAM("total to reset", mPendingResetSessions.size()))

      auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_RESET_STREAMS, pReset, SafeInt<socklen_t>(buffer->SizeInBytes()));

      if (result < 0) {
        if (EALREADY == errno) {
          mAttemptResetLater = true;
          ZS_LOG_DEBUG(log("reset already in progress (thus attempt reset later)"))
          mPendingResetSessions.clear();
          return true;
        }
        ZS_LOG_ERROR(Detail, log("failed to perform stream reset") + ZS_PARAM("total to reset", mPendingResetSessions.size()) + ZS_PARAM("errno", errno))
        return false;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::cancel() noexcept
    {
      ZS_EVENTING_1(x, i, Detail, SctpTransportCancel, ol, SctpTransport, Cancel, puid, id, mID);

      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) {
        ZS_LOG_TRACE(log("already shutdown"))
        return;
      }

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      setState(InternalState_ShuttingDown);

      for (auto iter = mGetSocketOptions.begin(); iter != mGetSocketOptions.end(); ++iter) {
        auto promise = (*iter).first;
        promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "transport is shutting down"));
      }
      mGetSocketOptions.clear();
      for (auto iter = mSetSocketOptions.begin(); iter != mSetSocketOptions.end(); ++iter) {
        auto promise = (*iter).first;
        promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "transport is shutting down"));
      }
      mSetSocketOptions.clear();

      if (mGracefulShutdownReference) {

        {
          // scope: attmempt to shutdown transport and check if ready
          {
            for (auto iter = mSessions.begin(); iter != mSessions.end(); ++iter)
            {
              auto session = (*iter).second;
              session->requestShutdown();
            }

            if (!mCapabilities) {
              ZS_LOG_WARNING(Detail, log("start was not called"))
              goto transport_not_available;
            }

            auto secureTransport = mSecureTransport.lock();
            if (secureTransport) {
              switch (secureTransport->state()) {
                case ISecureTransportTypes::State_Pending:
                case ISecureTransportTypes::State_Disconnected:
                {
                  ZS_LOG_WARNING(Detail, log("secure transport is not connected"))
                  goto transport_not_available;
                }
                case ISecureTransportTypes::State_Connected:
                {
                  ZS_LOG_TRACE(log("seecure transport is connected"))
                  break;
                }
                case ISecureTransportTypes::State_Closed:
                {
                  ZS_LOG_WARNING(Detail, log("secure transport is already closed"))
                  goto transport_not_available;
                }
              }
            }

            if (!mSocket) {
              ZS_LOG_WARNING(Detail, log("socket was not open"))
              goto transport_not_available;
            }

            stepDeliverIncomingPackets();
            if (!stepResetStream()) {
              ZS_LOG_WARNING(Detail, log("failed to reset streams during shutdown"))
              goto transport_not_available;
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

      setState(InternalState_Shutdown);

      mSubscriptions.clear();
      mDataChannelSubscriptions.clear();

      if (mSecureTransportSubscription) {
        mSecureTransportSubscription->cancel();
        mSecureTransportSubscription.reset();
      }

      mAnnouncedIncomingDataChannels.clear();

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
        if (session) {
          session->notifyClosed();
        }
      }
      mPendingResetSessions.clear();

      for (auto iter = mQueuedResetSessions.begin(); iter != mQueuedResetSessions.end(); ++iter)
      {
        auto session = (*iter).second;
        if (session) {
          session->notifyClosed();
        }
      }
      mQueuedResetSessions.clear();

      while (mWaitingToSend.size() > 0) {
        auto promise = mWaitingToSend.front();
        auto reason = make_shared<RejectReason>(mLastError, mLastErrorReason);
        promise->reject(reason);
        mWaitingToSend.pop();
      }

      mPendingIncomingBuffers = BufferQueue();

      auto listener = mListener.lock();
      if (listener) {
        if (mAllocatedLocalPort.hasValue()) {
          listener->deallocateLocalPort(mAllocatedLocalPort.value());
        }
        listener->notifyShutdown(*this, mLocalPort, mRemotePort);
      }

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::setState(InternalStates state) noexcept
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;
      ZS_EVENTING_2(x, i, Detail, SctpTransportStateChangedEvent, ol, SctpTransport, StateEvent, puid, id, mID, string, state, toString(state));
      mDataChannelSubscriptions.delegate()->onSCTPTransportStateChanged();

      if (InternalState_Ready == state) {
        notifyWriteReady();
      }

      auto newState = toState(mCurrentState);
      if (newState != mLastReportedState)
      {
        mLastReportedState = newState;
        SCTPTransportPtr pThis = mThisWeak.lock();
        if (pThis) {
          mSubscriptions.delegate()->onSCTPTransportStateChange(pThis, mLastReportedState);
        }
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::setError(WORD errorCode, const char *inReason) noexcept
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
      ZS_EVENTING_3(
                    x, i, Detail, SctpTransportErrorEvent, ol, SctpTransport, StateEvent,
                    puid, id, mID,
                    word, errorCode, errorCode,
                    string, reason, reason
                    );

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::openConnectSCTPSocket() noexcept
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
        ZS_LOG_ERROR(Detail, log("unable to connect to remote socket") + ZS_PARAM("errno", errno))
        return false;
      }

      ZS_LOG_DEBUG(log("successful bind and connect"))
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::openSCTPSocket() noexcept
    {
      mSocket = usrsctp_socket(AF_CONN, SOCK_STREAM, IPPROTO_SCTP, SCTPInit::OnSctpInboundPacket, NULL, 0, mThisSocket);
      if (!mSocket) {
        ZS_LOG_ERROR(Detail, log("failed to create sctp socket") + ZS_PARAM("errno", errno))
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
    bool SCTPTransport::prepareSocket(struct socket *sock) noexcept
    {
      // Make the socket non-blocking. Connect, close, shutdown etc will not
      // block the thread waiting for the socket operation to complete.

      if (usrsctp_set_non_blocking(sock, 1) < 0) {
        ZS_LOG_ERROR(Detail, log("failed to set SCTP socket to non-blocking") + ZS_PARAM("errno", errno))
        return false;
      }

      // This ensures that the usrsctp close call deletes the association. This
      // prevents usrsctp from calling OnSctpOutboundPacket with references to
      // this class as the address.
      linger linger_opt {};
      linger_opt.l_onoff = 1;
      linger_opt.l_linger = 0;
      if (usrsctp_setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt))) {
        ZS_LOG_ERROR(Detail, log("failed to set SCTP socket SO_LINGER") + ZS_PARAM("errno", errno))
        return false;
      }

      // Enable stream ID resets.
      struct sctp_assoc_value stream_rst {};
      stream_rst.assoc_id = SCTP_ALL_ASSOC;
      stream_rst.assoc_value = 1;
      if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_ENABLE_STREAM_RESET, &stream_rst, sizeof(stream_rst))) {
        ZS_LOG_ERROR(Detail, log("failed to set SCTP_ENABLE_STREAM_RESET") + ZS_PARAM("errno", errno))
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
        ZS_LOG_ERROR(Detail, log("failed to set SCTP_PEER_ADDR_PARAMS") + ZS_PARAM("errno", errno))
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
        event.se_type = static_cast<uint16_t>(event_types[i]);
        if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_EVENT, &event, sizeof(event)) < 0) {
          ZS_LOG_ERROR(Detail, log("failed to set SCTP_EVENT type") + ZS_PARAM("event", event.se_type) + ZS_PARAM("errno", errno))
          return false;
        }
      }

      for (auto iter = mSetSocketOptions.begin(); iter != mSetSocketOptions.end(); ++iter) {
        auto promise = (*iter).first;
        auto options = (*iter).second;

        auto result = internalSetOptions(*options);
        if (result) {
          promise->resolve(options);
        } else {
          promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_BadRequest, "failed to set socket options"));
        }
      }
      mSetSocketOptions.clear();

      for (auto iter = mGetSocketOptions.begin(); iter != mGetSocketOptions.end(); ++iter) {
        auto promise = (*iter).first;
        auto options = (*iter).second;

        auto result = internalGetOptions(*options);
        if (result) {
          promise->resolve(options);
        } else {
          promise->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_BadRequest, "failed to get socket options"));
        }
      }
      mGetSocketOptions.clear();

      ZS_LOG_DEBUG(log("sctp socket prepared") + ZS_PARAM("socket", (PTRNUMBER)sock))
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::isSessionAvailable(WORD sessionID) noexcept
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
                                    ) noexcept
    {
      outWouldBlock = false;

      auto socket = mSocket;

      if (!socket) {
        ZS_LOG_WARNING(Trace, log("cannot send packet (no socket)"))
        return false;
      }

      auto found = mSessions.find(inPacket.mSessionID);
      if (found == mSessions.end()) {
        ZS_LOG_WARNING(Trace, log("cannot send packet (session was not found)"))
        return false;
      }

      struct sctp_sendv_spa spa = {};

      spa.sendv_flags |= SCTP_SEND_SNDINFO_VALID;
      spa.sendv_sndinfo.snd_sid = inPacket.mSessionID;
      spa.sendv_sndinfo.snd_ppid = htonl(inPacket.mType);

      if (!inPacket.mOrdered) {
        spa.sendv_sndinfo.snd_flags = SCTP_UNORDERED;
      }

      if (inPacket.mMaxRetransmits.hasValue()) {
        spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
        spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_RTX;
        spa.sendv_prinfo.pr_value = inPacket.mMaxRetransmits.value();
      } else if (0 != inPacket.mMaxPacketLifetime.count()) {
        spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
        spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_TTL;
        spa.sendv_prinfo.pr_value = SafeInt<decltype(spa.sendv_prinfo.pr_value)>(inPacket.mMaxPacketLifetime.count());
      }

      auto result = usrsctp_sendv(
                                  socket,
                                  (inPacket.mBuffer ? inPacket.mBuffer->BytePtr() : NULL),
                                  (inPacket.mBuffer ? inPacket.mBuffer->SizeInBytes() : 0),
                                  NULL, 0,
                                  &spa, SafeInt<socklen_t>(sizeof(spa)),
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
    void SCTPTransport::notifyWriteReady() noexcept
    {
      if (!mConnected) {
        ZS_LOG_DEBUG(log("connected (as notified write ready)"))
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

        if (mIncoming) {
          auto listener = mListener.lock();
          if (listener) {
            listener->announceTransport(mThisWeak.lock(), mLocalPort, mRemotePort);
          }
        }
      }

      mConnected = true;
      mWriteReady = true;

      while (mWaitingToSend.size() > 0) {
        auto promise = mWaitingToSend.front();
        promise->resolve();
        mWaitingToSend.pop();
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::handleNotificationPacket(const sctp_notification &notification) noexcept
    {

      switch (notification.sn_header.sn_type) {
        case SCTP_ASSOC_CHANGE:
          ZS_LOG_TRACE(log("SCTP_ASSOC_CHANGE"))
          handleNotificationAssocChange(notification.sn_assoc_change);
          break;
        case SCTP_REMOTE_ERROR:
          ZS_LOG_TRACE(log("SCTP_REMOTE_ERROR"))
          break;
        case SCTP_SHUTDOWN_EVENT:
          ZS_LOG_TRACE(log("SCTP_SHUTDOWN_EVENT"))
          break;
        case SCTP_ADAPTATION_INDICATION:
          ZS_LOG_TRACE(log("SCTP_ADAPTATION_INDICATION"))
          break;
        case SCTP_PARTIAL_DELIVERY_EVENT:
          ZS_LOG_TRACE(log("SCTP_PARTIAL_DELIVERY_EVENT"))
          break;
        case SCTP_AUTHENTICATION_EVENT:
          ZS_LOG_TRACE(log("SCTP_AUTHENTICATION_EVENT"))
          break;
        case SCTP_SENDER_DRY_EVENT:
          ZS_LOG_TRACE(log("SCTP_SENDER_DRY_EVENT"))
          notifyWriteReady();
          break;
        case SCTP_NOTIFICATIONS_STOPPED_EVENT:
          ZS_LOG_TRACE(log("SCTP_NOTIFICATIONS_STOPPED_EVENT"))
          break;
        case SCTP_SEND_FAILED_EVENT:
          {
            auto &failure = notification.sn_send_failed_event;
            ZS_LOG_ERROR(Debug, log("SCTP_SEND_FAILED_EVENT") + 
              ZS_PARAM("ssfe_assoc_id", failure.ssfe_assoc_id) + 
              ZS_PARAM("ssfe_type", failure.ssfe_type) + 
              ZS_PARAM("ssfe_flags", failure.ssfe_flags) +
              ZS_PARAM("ssfe_length", failure.ssfe_length) +
              ZS_PARAM("ssfe_error", failure.ssfe_error) +
              ZS_PARAM("ssfe_info_snd_sid", failure.ssfe_info.snd_sid) +
              ZS_PARAM("ssfe_info_snd_flags", failure.ssfe_info.snd_flags) +
              ZS_PARAM("ssfe_info_snd_ppid", failure.ssfe_info.snd_ppid) +
              ZS_PARAM("ssfe_info_snd_context", failure.ssfe_info.snd_context) +
              ZS_PARAM("ssfe_info_snd_assoc_id", failure.ssfe_info.snd_assoc_id)
            );
          }
          break;
        case SCTP_STREAM_RESET_EVENT:
          ZS_LOG_TRACE(log("SCTP_STREAM_RESET_EVENT"))
          handleStreamResetEvent(notification.sn_strreset_event);
          break;
        case SCTP_ASSOC_RESET_EVENT:
          ZS_LOG_TRACE(log("SCTP_ASSOC_RESET_EVENT"))
          break;
        case SCTP_STREAM_CHANGE_EVENT:
          ZS_LOG_TRACE(log("SCTP_STREAM_CHANGE_EVENT"))
          // An acknowledgment we get after our stream resets have gone through,
          // if they've failed.  We log the message, but don't react -- we don't
          // keep around the last-transmitted set of SSIDs we wanted to close for
          // error recovery.  It doesn't seem likely to occur, and if so, likely
          // harmless within the lifetime of a single SCTP association.
          break;
        default:
          ZS_LOG_WARNING(Debug, log("Uknown SCTP event") + ZS_PARAM("type", notification.sn_header.sn_type))
          break;
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::handleNotificationAssocChange(const sctp_assoc_change &change) noexcept
    {
      switch (change.sac_state) {
        case SCTP_COMM_UP:
          ZS_LOG_TRACE(log("Association change SCTP_COMM_UP"))
          notifyWriteReady();
          break;
        case SCTP_COMM_LOST:
          ZS_LOG_TRACE(log("Association change SCTP_COMM_LOST"))
          cancel();
          break;
        case SCTP_RESTART:
          ZS_LOG_TRACE(log("Association change SCTP_RESTART"))
          break;
        case SCTP_SHUTDOWN_COMP:
          ZS_LOG_TRACE(log("Association change SCTP_SHUTDOWN_COMP"))
          break;
        case SCTP_CANT_STR_ASSOC:
          ZS_LOG_TRACE(log("Association change SCTP_CANT_STR_ASSOC"))
          break;
        default:
          ZS_LOG_TRACE(log("Association change UNKNOWN"))
          break;
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::handleStreamResetEvent(const sctp_stream_reset_event &event) noexcept
    {
      if (mAttemptResetLater) {
        mAttemptResetLater = false;
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      }
      // A stream reset requires both sides to reset the stream. Thus if the
      // stream was closed locally it expects the remote side to cause a reset
      // too. If the remote side resets a stream then the local side will
      // reflect back the reset in return (if there is a stream to reset).

      const size_t numSSRCs = (event.strreset_length - sizeof(event)) / sizeof(event.strreset_stream_list[0]);

      ZS_LOG_TRACE(log("SCTP_STREAM_RESET_EVENT") + ZS_PARAM("flags (hex)", Stringize<decltype(event.strreset_flags)>(event.strreset_flags, 16)) + ZS_PARAM("flags", UseSCTPHelper::listFlags(event.strreset_flags)) + ZS_PARAM("association", event.strreset_assoc_id) + ZS_PARAM("streams", UseSCTPHelper::listArray(event.strreset_stream_list, numSSRCs)))

      // If both sides try to reset some streams at the same time (even if they're
      // disjoint sets), we can get reset failures.
      if ((0 != (event.strreset_flags & SCTP_STREAM_RESET_FAILED)) ||
          (0 != (event.strreset_flags & SCTP_STREAM_RESET_DENIED))) {
        ZS_LOG_WARNING(Detail, log("stream reset failed (will try again later)"))
        // OK, just try again.  The stream IDs sent over when the RESET_FAILED flag
        // is set seem to be garbage values.  Ignore them.

        mPendingResetSessions.clear();
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
        return;
      }

      if (0 != (event.strreset_flags & SCTP_STREAM_RESET_INCOMING_SSN)) {
        for (size_t i = 0; i < numSSRCs; ++i) {
          const SessionID sessionID = event.strreset_stream_list[i];

          UseDataChannelPtr notifyClosedDataChannel;

          {
            auto found = mPendingResetSessions.find(sessionID);
            if (found != mPendingResetSessions.end()) {
              auto dataChannel = (*found).second;
              if (!notifyClosedDataChannel) notifyClosedDataChannel = dataChannel;
              mPendingResetSessions.erase(sessionID);
            }
          }
          {
            auto found = mQueuedResetSessions.find(sessionID);
            if (found != mQueuedResetSessions.end()) {
              auto dataChannel = (*found).second;
              if (!notifyClosedDataChannel) notifyClosedDataChannel = dataChannel;
              mQueuedResetSessions.erase(sessionID);
            }
          }

          if (notifyClosedDataChannel) {
            ZS_LOG_TRACE(log("data channel is now considered closed") + ZS_PARAM("data channel id", notifyClosedDataChannel->getID()))
            notifyClosedDataChannel->notifyClosed();
          }

          {
            auto found = mSessions.find(sessionID);
            if (found != mSessions.end()) {
              auto dataChannel = (*found).second;
              ZS_LOG_DEBUG(log("remote party is closing session") + ZS_PARAM("session id", sessionID))
              dataChannel->requestShutdown();
              mSessions.erase(found);

              auto objectID = dataChannel->getID();
              auto foundAnnounced = mAnnouncedIncomingDataChannels.find(objectID);
              if (foundAnnounced != mAnnouncedIncomingDataChannels.end()) {
                ZS_LOG_TRACE(log("removing announced data channel") + ZS_PARAM("data channel id", objectID))
                mAnnouncedIncomingDataChannels.erase(foundAnnounced);
              }
            }
          }
        }

        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
        return;
      }
      if (0 != (event.strreset_flags & SCTP_STREAM_RESET_OUTGOING_SSN)) {
#if 0
        for (size_t i = 0; i < numSSRCs; ++i) {
          const SessionID sessionID = event.strreset_stream_list[i];
        }
#endif //0
      }
    }


    //-------------------------------------------------------------------------
    bool SCTPTransport::internalGetOptions(SocketOptions &ioOptions) const noexcept
    {
      AutoRecursiveLock lock(*this);

      if (!mSocket) return false;

      if (ioOptions.mRTO.hasValue()) {
        sctp_rtoinfo info {};
        memset(&info, 0, sizeof(info));
        info.srto_assoc_id = SCTP_FUTURE_ASSOC;
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_RTOINFO, &info, &len);
        if (result < 0) return false;

        ioOptions.mRTO.value().initial_ = Milliseconds(info.srto_initial);
        ioOptions.mRTO.value().min_ = Milliseconds(info.srto_min);
        ioOptions.mRTO.value().max_ = Milliseconds(info.srto_max);
      }

      if (ioOptions.mAssocParams.hasValue()) {
        sctp_assocparams info{};
        memset(&info, 0, sizeof(info));
        info.sasoc_assoc_id = SCTP_FUTURE_ASSOC;
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_ASSOCINFO, &info, &len);
        if (result < 0) return false;

        ioOptions.mAssocParams.value().peer_rwnd_ = info.sasoc_peer_rwnd;
        ioOptions.mAssocParams.value().local_rwnd_ = info.sasoc_local_rwnd;
        ioOptions.mAssocParams.value().cookie_life_ = info.sasoc_cookie_life;
        ioOptions.mAssocParams.value().asocmaxrxt_ = info.sasoc_asocmaxrxt;
        ioOptions.mAssocParams.value().number_peer_destinations_ = info.sasoc_number_peer_destinations;
      }

      if (ioOptions.mInitMsg.hasValue()) {
        sctp_initmsg info{};
        memset(&info, 0, sizeof(info));
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_INITMSG, &info, &len);
        if (result < 0) return false;

        ioOptions.mInitMsg.value().num_ostreams_ = info.sinit_num_ostreams;
        ioOptions.mInitMsg.value().max_instreams_ = info.sinit_max_instreams;
        ioOptions.mInitMsg.value().max_attempts_ = info.sinit_max_attempts;
        ioOptions.mInitMsg.value().max_init_timeo_ = Milliseconds(info.sinit_max_init_timeo);
      }

      if (ioOptions.mAutoClose.hasValue()) {
        int info {};
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_AUTOCLOSE, &info, &len);
        if (result < 0) return false;

        ioOptions.mAutoClose.value() = Seconds(info);
      }

      if (ioOptions.mDisableFragments.hasValue()) {
        int info{};
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_DISABLE_FRAGMENTS, &info, &len);
        if (result < 0) return false;

        ioOptions.mDisableFragments.value() = (0 == info ? false : true);
      }

      if (ioOptions.mPAddrParams.hasValue()) {
        sctp_paddrparams  info{};
        memset(&info, 0, sizeof(info));
        info.spp_assoc_id = SCTP_FUTURE_ASSOC;
        info.spp_address.ss_family = AF_CONN;

        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS, &info, &len);
        if (result < 0) return false;

        if (0 != (info.spp_flags & SPP_HB_DEMAND)) ioOptions.mPAddrParams.value().hb_demand_ = true;
        if (0 != (info.spp_flags & SPP_HB_ENABLE)) ioOptions.mPAddrParams.value().hb_enabled_ = true;
        if (0 != (info.spp_flags & SPP_HB_TIME_IS_ZERO)) ioOptions.mPAddrParams.value().hb_is_zero_ = true;

        ioOptions.mPAddrParams.value().hbinterval_ = Milliseconds(info.spp_hbinterval);
        ioOptions.mPAddrParams.value().pathmaxrxt_ = info.spp_pathmaxrxt;
      }

      if (ioOptions.mMaxSeg.hasValue()) {
        sctp_assoc_value info{};
        memset(&info, 0, sizeof(info));
        info.assoc_id = SCTP_FUTURE_ASSOC;
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_MAXSEG, &info, &len);
        if (result < 0) return false;

        ioOptions.mMaxSeg.value() = info.assoc_value;
      }

      if (ioOptions.mFragmentInterleave.hasValue()) {
        int info{};
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_FRAGMENT_INTERLEAVE, &info, &len);
        if (result < 0) return false;

        ioOptions.mFragmentInterleave.value() = info;
      }

      if (ioOptions.mPartialDeliveryPoint.hasValue()) {
        int info{};
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_PARTIAL_DELIVERY_POINT, &info, &len);
        if (result < 0) return false;

        ioOptions.mPartialDeliveryPoint.value() = SafeInt<uint32_t>(info);
      }

      if (ioOptions.mAutoASCONF.hasValue()) {
        int info{};
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_AUTO_ASCONF, &info, &len);
        if (result < 0) return false;

        ioOptions.mAutoASCONF.value() = (0 == info ? false : true);
      }

      if (ioOptions.mMaximumBurst.hasValue()) {
        sctp_assoc_value info{};
        memset(&info, 0, sizeof(info));
        info.assoc_id = SCTP_FUTURE_ASSOC;
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_MAX_BURST, &info, &len);
        if (result < 0) return false;

        ioOptions.mMaximumBurst.value() = info.assoc_value;
      }

      if (ioOptions.mContext.hasValue()) {
        sctp_assoc_value info{};
        memset(&info, 0, sizeof(info));
        info.assoc_id = SCTP_FUTURE_ASSOC;
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_CONTEXT, &info, &len);
        if (result < 0) return false;

        ioOptions.mContext.value() = info.assoc_value;
      }

      if (ioOptions.mSackInfo.hasValue()) {
        sctp_sack_info info{};
        memset(&info, 0, sizeof(info));
        info.sack_assoc_id = SCTP_FUTURE_ASSOC;
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_DELAYED_SACK, &info, &len);
        if (result < 0) return false;

        ioOptions.mSackInfo.value().delay_ = Milliseconds(info.sack_delay);
        ioOptions.mSackInfo.value().freq_ = info.sack_freq;
      }

      if (ioOptions.mStatus.hasValue()) {
        sctp_status info{};
        memset(&info, 0, sizeof(info));
        socklen_t len = sizeof(info);
        auto result = usrsctp_getsockopt(mSocket, IPPROTO_SCTP, SCTP_STATUS, &info, &len);
        if (result < 0) return false;

        ioOptions.mStatus.value().state_ = info.sstat_state;
        ioOptions.mStatus.value().rwnd_ = info.sstat_rwnd;
        ioOptions.mStatus.value().unackdata_ = info.sstat_unackdata;
        ioOptions.mStatus.value().penddata_ = info.sstat_penddata;
        ioOptions.mStatus.value().instrms_ = info.sstat_instrms;
        ioOptions.mStatus.value().outstrms_ = info.sstat_outstrms;
        ioOptions.mStatus.value().fragmentation_point_ = info.sstat_fragmentation_point;
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::internalSetOptions(const SocketOptions &inOptions) noexcept
    {
      AutoRecursiveLock lock(*this);

      if (!mSocket) return false;

      if (inOptions.mRTO.hasValue()) {
        sctp_rtoinfo info{};
        memset(&info, 0, sizeof(info));
        info.srto_assoc_id = SCTP_FUTURE_ASSOC;
        info.srto_initial = SafeInt<decltype(info.srto_initial)>(inOptions.mRTO.value().initial_.count());
        info.srto_min = SafeInt<decltype(info.srto_min)>(inOptions.mRTO.value().min_.count());
        info.srto_max = SafeInt<decltype(info.srto_max)>(inOptions.mRTO.value().max_.count());
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_RTOINFO, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mAssocParams.hasValue()) {
        sctp_assocparams info{};
        memset(&info, 0, sizeof(info));
        info.sasoc_assoc_id = SCTP_FUTURE_ASSOC;
        info.sasoc_peer_rwnd = inOptions.mAssocParams.value().peer_rwnd_;
        info.sasoc_local_rwnd = inOptions.mAssocParams.value().local_rwnd_;
        info.sasoc_cookie_life = inOptions.mAssocParams.value().cookie_life_;
        info.sasoc_asocmaxrxt = inOptions.mAssocParams.value().asocmaxrxt_;
        info.sasoc_number_peer_destinations = inOptions.mAssocParams.value().number_peer_destinations_;

        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_ASSOCINFO, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mInitMsg.hasValue()) {
        sctp_initmsg info{};
        memset(&info, 0, sizeof(info));
        info.sinit_num_ostreams = inOptions.mInitMsg.value().num_ostreams_;
        info.sinit_max_instreams = inOptions.mInitMsg.value().max_instreams_;
        info.sinit_max_attempts = inOptions.mInitMsg.value().max_attempts_;
        info.sinit_max_init_timeo = SafeInt<decltype(info.sinit_max_init_timeo)>(inOptions.mInitMsg.value().max_init_timeo_.count());
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_INITMSG, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mAutoClose.hasValue()) {
        int info  = SafeInt<decltype(info)>(inOptions.mAutoClose.value().count());
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_AUTOCLOSE, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mDisableFragments.hasValue()) {
        int info{};
        info = inOptions.mDisableFragments.value() ? 1 : 0;
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_DISABLE_FRAGMENTS, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mPAddrParams.hasValue()) {
        sctp_paddrparams  info{};
        memset(&info, 0, sizeof(info));
        info.spp_assoc_id = SCTP_FUTURE_ASSOC;
        info.spp_address.ss_family = AF_CONN;
        if (inOptions.mPAddrParams.value().hb_demand_.hasValue()) {
          if (inOptions.mPAddrParams.value().hb_demand_.value()) {
            info.spp_flags = info.spp_flags | SPP_HB_DEMAND;
          }
        }
        if (inOptions.mPAddrParams.value().hb_enabled_.hasValue()) {
          if (inOptions.mPAddrParams.value().hb_enabled_.value()) {
            info.spp_flags = info.spp_flags | SPP_HB_ENABLE;
          } else {
            info.spp_flags = info.spp_flags | SPP_HB_DISABLE;
          }
        }
        if (inOptions.mPAddrParams.value().hb_is_zero_.hasValue()) {
          if (inOptions.mPAddrParams.value().hb_is_zero_.value()) {
            info.spp_flags = info.spp_flags | SPP_HB_TIME_IS_ZERO;
          }
        }
        info.spp_hbinterval = SafeInt<decltype(info.spp_hbinterval)>(inOptions.mPAddrParams.value().hbinterval_.count());
        info.spp_pathmaxrxt = inOptions.mPAddrParams.value().pathmaxrxt_;

        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mMaxSeg.hasValue()) {
        sctp_assoc_value info{};
        memset(&info, 0, sizeof(info));
        info.assoc_id = SCTP_FUTURE_ASSOC;
        info.assoc_value = inOptions.mMaxSeg.value();
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_MAXSEG, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mFragmentInterleave.hasValue()) {
        int info{};
        info = inOptions.mFragmentInterleave.value();
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_FRAGMENT_INTERLEAVE, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mPartialDeliveryPoint.hasValue()) {
        int info{};
        info = SafeInt<int>(inOptions.mPartialDeliveryPoint.value());
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_PARTIAL_DELIVERY_POINT, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mAutoASCONF.hasValue()) {
        int info{};
        info = inOptions.mAutoASCONF.value() ? 1 : 0;
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_AUTO_ASCONF, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mMaximumBurst.hasValue()) {
        sctp_assoc_value info{};
        memset(&info, 0, sizeof(info));
        info.assoc_id = SCTP_ALL_ASSOC;
        info.assoc_value = inOptions.mMaximumBurst.value();
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_MAX_BURST, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mContext.hasValue()) {
        sctp_assoc_value info{};
        memset(&info, 0, sizeof(info));
        info.assoc_id = SCTP_ALL_ASSOC;
        info.assoc_value = inOptions.mContext.value();
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_CONTEXT, &info, sizeof(info));
        if (result < 0) return false;
      }

      if (inOptions.mSackInfo.hasValue()) {
        sctp_sack_info info{};
        memset(&info, 0, sizeof(info));
        info.sack_assoc_id = SCTP_ALL_ASSOC;
        info.sack_delay = SafeInt<decltype(info.sack_delay)>(inOptions.mSackInfo.value().delay_.count());
        info.sack_freq = inOptions.mSackInfo.value().freq_;
        auto result = usrsctp_setsockopt(mSocket, IPPROTO_SCTP, SCTP_DELAYED_SACK, &info, sizeof(info));
        if (result < 0) return false;
      }

      return true;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportFactory
    //

    //-------------------------------------------------------------------------
    ISCTPTransportFactory &ISCTPTransportFactory::singleton() noexcept
    {
      return SCTPTransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ISCTPTransportFactory::ForListenerPtr ISCTPTransportFactory::create(
                                                                        UseListenerPtr listener,
                                                                        UseSecureTransportPtr secureTransport,
                                                                        WORD localPort
                                                                        ) noexcept
    {
      if (this) {}
      return internal::SCTPTransport::create(listener, secureTransport, localPort);
    }

    //-------------------------------------------------------------------------
    ISCTPTransportPtr ISCTPTransportFactory::create(
                                                    ISCTPTransportDelegatePtr delegate,
                                                    IDTLSTransportPtr transport,
                                                    WORD localPort
                                                    ) noexcept
    {
      if (this) {}
      return internal::SCTPTransport::create(delegate, transport, localPort);
    }

  } // internal namespace

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISCTPTransportTypes::Parameters
  //

  //---------------------------------------------------------------------------
  ISCTPTransportTypes::Capabilities::Capabilities(ElementPtr elem) noexcept
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::ISCTPTransportTypes::Capabilities", "maxMessageSize", mMaxMessageSize);
    IHelper::getElementValue(elem, "ortc::ISCTPTransportTypes::Capabilities", "minPort", mMinPort);
    IHelper::getElementValue(elem, "ortc::ISCTPTransportTypes::Capabilities", "maxPort", mMaxPort);
    IHelper::getElementValue(elem, "ortc::ISCTPTransportTypes::Capabilities", "maxUsablePorts", mMaxUsablePorts);
    IHelper::getElementValue(elem, "ortc::ISCTPTransportTypes::Capabilities", "maxSessionsPerPort", mMaxSessionsPerPort);
  }

  //---------------------------------------------------------------------------
  ElementPtr ISCTPTransportTypes::Capabilities::createElement(const char *objectName) const noexcept
  {
    if (!objectName) objectName = "capabilities";

    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "maxMessageSize", mMaxMessageSize);
    IHelper::adoptElementValue(elem, "minPort", mMinPort);
    IHelper::adoptElementValue(elem, "maxPort", mMaxPort);
    IHelper::adoptElementValue(elem, "maxUsablePorts", mMaxUsablePorts);
    IHelper::adoptElementValue(elem, "maxSessionsPerPort", mMaxSessionsPerPort);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr ISCTPTransportTypes::Capabilities::toDebug() const noexcept
  {
    return createElement("ortc::ISCTPTransportTypes::Capabilities");
  }

  //---------------------------------------------------------------------------
  String ISCTPTransportTypes::Capabilities::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ISCTPTransportTypes:Capabilities:");
    hasher->update(mMaxMessageSize);
    hasher->update(":");
    hasher->update(mMinPort);
    hasher->update(":");
    hasher->update(mMaxPort);
    hasher->update(":");
    hasher->update(mMaxUsablePorts);
    hasher->update(":");
    hasher->update(mMaxSessionsPerPort);
    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISCTPTransportTypes::SocketOptions
  //

  //---------------------------------------------------------------------------
  bool ISCTPTransportTypes::SocketOptions::hasValue() const noexcept
  {
    if (mRTO.hasValue() ||
        mAssocParams.hasValue() ||
        mInitMsg.hasValue() ||
        mAutoClose.hasValue() ||
        mDisableFragments.hasValue() ||
        mPAddrParams.hasValue() ||
        mMaxSeg.hasValue() ||
        mFragmentInterleave.hasValue() ||
        mPartialDeliveryPoint.hasValue() ||
        mAutoASCONF.hasValue() ||
        mMaximumBurst.hasValue() ||
        mContext.hasValue() ||
        mSackInfo.hasValue() ||
        mStatus.hasValue()) return true;

    return false;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISCTPTransportTypes
  //

  //---------------------------------------------------------------------------
  const char *ISCTPTransportTypes::toString(States state) noexcept
  {
    switch (state) {
    case State_New:           return "new";
    case State_Connecting:    return "connecting";
    case State_Connected:     return "connected";
    case State_Closed:        return "close";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // ISCTPTransport
  //

  //---------------------------------------------------------------------------
  ElementPtr ISCTPTransport::toDebug(ISCTPTransportPtr transport) noexcept
  {
    return internal::SCTPTransport::toDebug(internal::SCTPTransport::convert(transport));
  }

  //---------------------------------------------------------------------------
  ISCTPTransportPtr ISCTPTransport::convert(IDataTransportPtr object) noexcept
  {
    return internal::SCTPTransport::convert(object);
  }

  //---------------------------------------------------------------------------
  ISCTPTransportPtr ISCTPTransport::create(
                                           ISCTPTransportDelegatePtr delegate,
                                           IDTLSTransportPtr transport,
                                           WORD localPort
                                           ) noexcept(false)
  {
    return internal::ISCTPTransportFactory::singleton().create(delegate, transport, localPort);
  }

  //---------------------------------------------------------------------------
  ISCTPTransportListenerSubscriptionPtr ISCTPTransport::listen(
                                                               ISCTPTransportListenerDelegatePtr delegate,
                                                               IDTLSTransportPtr transport,
                                                               const Capabilities &remoteCapabilities
                                                               ) noexcept
  {
    return internal::ISCTPTransportListenerFactory::singleton().listen(delegate, transport, remoteCapabilities);
  }

  //---------------------------------------------------------------------------
  ISCTPTransportTypes::CapabilitiesPtr ISCTPTransport::getCapabilities() noexcept
  {
    return internal::ISCTPTransportListenerFactory::singleton().getCapabilities();
  }

}
