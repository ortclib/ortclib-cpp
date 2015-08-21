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
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    const uint32_t kMaxSctpSid = 1023;

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
        usrsctp_sysctl_set_sctp_nr_outgoing_streams_default(kMaxSctpSid);

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
        UseDataChannelPtr dataChannel = *(static_cast<UseDataChannelPtr *>(addr));

        ZS_LOG_TRACE(slog("on sctp output backpet") + ZS_PARAM("address", ((PTRNUMBER)addr)) + ZS_PARAM("tos", tos) + ZS_PARAM("set_df", set_df))

        if (ZS_IS_LOGGING(Insane)) {
          String str = UseServicesHelper::getDebugString((const BYTE *)data, length);
          ZS_LOG_INSANE(slog("sctp outgoing packet") + ZS_PARAM("raw", "\n" + str))
        }

        dataChannel->notifySendSCTPPacket((const BYTE *)data, length);
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
      //https://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-13#section-6.6
      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 16*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportForDataChannel
    #pragma mark

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
        case State_DTLSComplete:  return "DTLS done";
        case State_Ready:         return "ready";
        case State_ShuttingDown:  return "shutting down";
        case State_Shutdown:      return "shutdown";
      }
      return "UNDEFINED";
    }
    
    //-------------------------------------------------------------------------
    SCTPTransport::SCTPTransport(
                                 const make_private &,
                                 IMessageQueuePtr queue,
                                 UseSecureTransportPtr secureTransport
                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSCTPInit(SCTPInit::singleton()),
      mSecureTransport(DTLSTransport::convert(secureTransport))
    {
      ZS_LOG_DETAIL(debug("created"))

      mSecureTransportReady = mSecureTransport->notifyWhenReady();
      mICETransport = ICETransport::convert(mSecureTransport->getICETransport());

      ORTC_THROW_INVALID_STATE_IF(!mSCTPInit)
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::init()
    {
      AutoRecursiveLock lock(*this);
      //IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      
      ZS_LOG_DETAIL(debug("SCTP init"))

      mSecureTransportReady->thenWeak(mThisWeak.lock());

      //Subscribe to DTLS Transport and Wait for DTLS to complete
      mICETransportSubscription = mICETransport->subscribe(mThisWeak.lock());

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
    SCTPTransportPtr SCTPTransport::convert(ForSecureTransportPtr object)
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
                                            IDTLSTransportPtr transport
                                            )
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!transport)

      UseSecureTransportPtr useSecureTransport = DTLSTransport::convert(transport);
      ASSERT(((bool)useSecureTransport))

      auto dataTransport = useSecureTransport->getDataTransport();
      ORTC_THROW_INVALID_STATE_IF(!dataTransport)

      auto sctpTransport = SCTPTransport::convert(dataTransport);
      ORTC_THROW_INVALID_STATE_IF(!sctpTransport)

      auto tearAway = ISCTPTransportTearAway::create(sctpTransport, make_shared<TearAwayData>());
      ORTC_THROW_INVALID_STATE_IF(!tearAway)

      auto tearAwayData = ISCTPTransportTearAway::data(tearAway);
      ORTC_THROW_INVALID_STATE_IF(!tearAwayData)

      tearAwayData->mSecureTransport = useSecureTransport;

      if (delegate) {
        tearAwayData->mDefaultSubscription = sctpTransport->subscribe(delegate);
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
    void SCTPTransport::start(const Capabilities &remoteCapabilities)
    {
#if 0
      struct socket *sock;
      short mediaPort = 0, remoteUDPEncapPort = 0;
      const int on = 1;
      unsigned int i;
      struct sctp_assoc_value av;
      struct sctp_udpencaps encaps;
      struct sctp_event event;
      uint16_t event_types[] = {SCTP_ASSOC_CHANGE,
        SCTP_PEER_ADDR_CHANGE,
        SCTP_REMOTE_ERROR,
        SCTP_SHUTDOWN_EVENT,
        SCTP_ADAPTATION_INDICATION,
        SCTP_PARTIAL_DELIVERY_EVENT};
#endif
      
      AutoRecursiveLock lock(*this);
      if ((isShuttingDown()) ||
          (isShutdown())) {
#define WARNING_WHAT_IF_SCTP_ERROR_CAUSED_CLOSE 1
#define WARNING_WHAT_IF_SCTP_ERROR_CAUSED_CLOSE 2
        ORTC_THROW_INVALID_STATE("already shutting down")
      }
      
      mCapabilities = remoteCapabilities;

#if 0
      //Retrieve media port over which SCTP to be established
#define TODO_FIND_MEDIA_PORT 1
#define TODO_FIND_MEDIA_PORT 2
      usrsctp_init(mediaPort, NULL, NULL);
#ifdef SCTP_DEBUG
      usrsctp_sysctl_set_sctp_debug_on(0);
#endif
      //Enable SCTP blackholing
      usrsctp_sysctl_set_sctp_blackhole(SCTPCTL_BLACKHOLE_MAX);
      
      //Pass DTLS packet handler to usrsctp socket; DTLS will pass it to SCTP after decryption
      if ((sock = usrsctp_socket(AF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP,
                                 mDTLSTransport->handleReceivedPacket, NULL, 0)) == NULL) {
        setError(errno);
      }
      if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_I_WANT_MAPPED_V4_ADDR,
                             (const void*)&on, (socklen_t)sizeof(int)) < 0) {
        setError(errno);
      }
      memset(&av, 0, sizeof(struct sctp_assoc_value));
      av.assoc_id = SCTP_ALL_ASSOC;
      av.assoc_value = 47;
      
      if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_CONTEXT, (const void*)&av,
                             (socklen_t)sizeof(struct sctp_assoc_value)) < 0) {
        setError(errno, "SCTP ");
      }
      
#define TODO_REMOTE_UDP_ENCAPS 1
#define TODO_REMOTE_UDP_ENCAPS 2
      if (/* UDP encapsulation enabled */ false) {
        memset(&encaps, 0, sizeof(struct sctp_udpencaps));
        encaps.sue_address.ss_family = AF_INET6;
        encaps.sue_port = htons(remoteUDPEncapPort);
        if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_REMOTE_UDP_ENCAPS_PORT,
                               (const void*)&encaps,
                               (socklen_t)sizeof(struct sctp_udpencaps)) < 0) {
          setError(errno, "SCTP Remote UDP Encapsulation port config failed!!!");
        }
      }
      memset(&event, 0, sizeof(event));
      event.se_assoc_id = SCTP_FUTURE_ASSOC;
      event.se_on = 1;
      for (i = 0; i < (unsigned int)(sizeof(event_types)/sizeof(uint16_t)); i++) {
        event.se_type = event_types[i];
        if (usrsctp_setsockopt(sock, IPPROTO_SCTP, SCTP_EVENT, &event,
                               sizeof(struct sctp_event)) < 0) {
          setError(errno, "SCTP Event association failed!!!");
        }
      }
#endif
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::stop()
    {
      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
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
    #pragma mark SCTPTransport => ISCTPTransportForSecureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    SCTPTransport::ForSecureTransportPtr SCTPTransport::create(UseSecureTransportPtr transport)
    {
      SCTPTransportPtr pThis(make_shared<SCTPTransport>(make_private {}, IORTCForInternal::queueORTC(), transport));
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
      if (bufferLengthInBytes > 0) {
        
        return true;
      }
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => ISCTPTransportForDataChannel
    #pragma mark

    //-------------------------------------------------------------------------
    ISCTPTransportForDataChannelSubscriptionPtr SCTPTransport::subscribe(ISCTPTransportForDataChannelDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("datachannel subscribing to SCTP Transport"))
      
      AutoRecursiveLock lock(*this);
      
      ISCTPTransportForDataChannelSubscriptionPtr subscription = mDataChannelSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueORTC());
      
      ISCTPTransportForDataChannelDelegatePtr delegate = mDataChannelSubscriptions.delegate(subscription, true);
      
      if (delegate) {
        SCTPTransportPtr pThis = mThisWeak.lock();
        
        switch (mCurrentState) {
          case State_Ready:
            delegate->onSCTPTransportReady();
            break;
            
          case State_Shutdown:
            delegate->onSCTPTransportClosed();
            break;
            
          default:
            break;
        }
      }
      
      if (isShutdown()) {
        mSubscriptions.clear();
      }
      
      return subscription;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::notifySendSCTPPacket(
                                             const BYTE *buffer,
                                             size_t bufferLengthInBytes
                                             )
    {
      UseSecureTransportPtr transport;

      {
        AutoRecursiveLock lock(*this);
        transport = mSecureTransport;
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
#define TODO 1
#define TODO 2
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

      UseServicesHelper::debugAppend(resultEl, "secure transport", mSecureTransport ? mSecureTransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "secure transport ready promise", (bool)mSecureTransportReady);

      UseServicesHelper::debugAppend(resultEl, "ice transport", mICETransport ? mICETransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "ice transport subscription", mICETransportSubscription ? mICETransportSubscription->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, mCapabilities.toDebug());

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
      if (!stepDTLSTransportError()) goto not_ready;
      if (!stepDTLSTransportReady()) goto not_ready;
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
    bool SCTPTransport::stepBogusDoSomething()
    {
      if ( /* step already done */ false ) {
        ZS_LOG_TRACE(log("already completed do something"))
        return true;
      }

      if ( /* cannot do step yet */ false) {
        ZS_LOG_DEBUG(log("waiting for XYZ to complete before continuing"))
        return false;
      }

      ZS_LOG_DEBUG(log("doing step XYZ"))

      // ....
#define TODO 1
#define TODO 2

      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepDTLSTransportReady()
    {
      if (mCurrentState > State_DTLSComplete) {
        ZS_LOG_TRACE(log("Not waiting on DTLS Transport"))
        return true;
      }
      
      if (mCurrentState < State_DTLSComplete) {
        ZS_LOG_DEBUG(log("waiting for DTLS Transport to complete before continuing"))
        return false;
      }
      
      ZS_LOG_DEBUG(log("DTLS Transport is done"))
      
      // Set new self state
      setState(State_DTLSComplete);
      
      // Update DataChannel Transport
#define TODO_UPDATE_DATACHANNEL_TRANSPORT_LAYER 1
#define TODO_UPDATE_DATACHANNEL_TRANSPORT_LAYER 2
      
      return true;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransport::stepDTLSTransportError()
    {
      if (mCurrentState < State_DTLSComplete) {
        ZS_LOG_TRACE(log("DTLS Transport was not ready before, no impact"))
        return true;
      }
      
      if (mCurrentState > State_DTLSComplete) {
        ZS_LOG_DEBUG(log("DTLS Transport broken"))
      
        // Set new self state
        setState(State_Pending);
      
        // Update DataChannel Transport
#define TODO_UPDATE_DATACHANNEL_TRANSPORT_LAYER 1
#define TODO_UPDATE_DATACHANNEL_TRANSPORT_LAYER 2
      }
      
      return false;
    }
    
    //-------------------------------------------------------------------------
    void SCTPTransport::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SCTP_SESSION_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SCTP_SESSION_IS_SHUTDOWN 2

        // grace shutdown process done here

        return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      mSubscriptions.clear();

      if (mICETransportSubscription) {
        mICETransportSubscription->cancel();
        mICETransportSubscription.reset();
      }

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

      switch (mCurrentState) {
        case State_Ready:
          mDataChannelSubscriptions.delegate()->onSCTPTransportReady();
          break;
          
        case State_Shutdown:
          mDataChannelSubscriptions.delegate()->onSCTPTransportClosed();
          break;
          
        default:
          break;
      }

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
    ISCTPTransportPtr ISCTPTransportFactory::create(
                                                    ISCTPTransportDelegatePtr delegate,
                                                    IDTLSTransportPtr transport
                                                    )
    {
      if (this) {}
      return internal::SCTPTransport::create(delegate, transport);
    }

    //-------------------------------------------------------------------------
    ISCTPTransportFactory::ForSecureTransportPtr ISCTPTransportFactory::create(UseSecureTransportPtr transport)
    {
      if (this) {}
      return internal::SCTPTransport::create(transport);
    }

    //-------------------------------------------------------------------------
    ISCTPTransportFactory::CapabilitiesPtr ISCTPTransportFactory::getCapabilities()
    {
      if (this) {}
      return SCTPTransport::getCapabilities();
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

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ISCTPTransportTypes::Capabilities::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ISCTPTransportTypes:Capabilities:");
    hasher.update(mMaxMessageSize);
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
                                           IDTLSTransportPtr transport
                                           )
  {
    return internal::ISCTPTransportFactory::singleton().create(delegate, transport);
  }

  //---------------------------------------------------------------------------
  ISCTPTransportTypes::CapabilitiesPtr ISCTPTransport::getCapabilities()
  {
    return internal::ISCTPTransportFactory::singleton().getCapabilities();
  }

}
