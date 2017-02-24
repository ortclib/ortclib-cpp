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

#include <ortc/internal/ortc_SCTPTransportListener.h>
#include <ortc/internal/ortc_SCTPTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>
#include <ortc/services/IHTTP.h>

#include <zsLib/ISettings.h>
#include <zsLib/SafeInt.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_sctp_datachannel) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SCTPTransportListenerSettingsDefaults);

    struct SCTPListenerHelper;

    ZS_DECLARE_TYPEDEF_PTR(SCTPListenerHelper, UseListenerHelper);

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
    #pragma mark SCTPTransportListenerSettingsDefaults
    #pragma mark

    class SCTPTransportListenerSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~SCTPTransportListenerSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static SCTPTransportListenerSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<SCTPTransportListenerSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static SCTPTransportListenerSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<SCTPTransportListenerSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        //https://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-13#section-6.6
        ISettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 16 * 1024);

        //only allow 1/4 of the range to be filled
        ISettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MAX_PORTS, (65535 - 5000) / 4);
        ISettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MIN_PORT, 5000);
        ISettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MAX_PORT, 65535);
        ISettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_DEFAULT_PORT, 5000);
      }
      
    };

    //-------------------------------------------------------------------------
    void installSCTPTransportListenerSettingsDefaults()
    {
      SCTPTransportListenerSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPHelper
    #pragma mark

    struct SCTPListenerHelper
    {
      enum Directions
      {
        Direction_Incoming,
        Direction_Outgoing,
      };

      //-------------------------------------------------------------------------
      static DWORD createTuple(
                               WORD localPort,
                               WORD remotePort
                               )
      {
        DWORD result {};

        WORD *pLocal = &(((WORD *)(&result))[0]);
        WORD *pRemote = &(((WORD *)(&result))[1]);

        memcpy(pLocal, &localPort, sizeof(localPort));
        memcpy(pRemote, &remotePort, sizeof(remotePort));

        return result;
      }

      //-------------------------------------------------------------------------
      static void splitTuple(
                             DWORD localRemoteTupleID,
                             WORD &outLocalPort,
                             WORD &outRemotePort
                             )
      {
        WORD *pLocal = &(((WORD *)(&localRemoteTupleID))[0]);
        WORD *pRemote = &(((WORD *)(&localRemoteTupleID))[1]);

        memcpy(&outLocalPort, pLocal, sizeof(outLocalPort));
        memcpy(&outRemotePort, pRemote, sizeof(outRemotePort));
      }
      
      //-------------------------------------------------------------------------
      static DWORD getLocalRemoteTuple(
                                       const BYTE *packet,
                                       size_t bufferLengthInBytes,
                                       Directions direction,
                                       WORD *outLocalPort = NULL,
                                       WORD *outRemotePort = NULL
                                       )
      {
        if (outLocalPort) *outLocalPort = 0;
        if (outRemotePort) *outRemotePort = 0;
        if (bufferLengthInBytes < sizeof(DWORD)) {
          ZS_LOG_WARNING(Trace, slog("SCTP packet is too small to be valid") + ZS_PARAM("buffer length", bufferLengthInBytes))
          return 0;
        }

        WORD *pSourcePort = &(((WORD *)packet)[0]);
        WORD *pDestPort = &(((WORD *)packet)[1]);

        WORD sourcePort = 0;
        WORD destPort = 0;

        // perform memcpy to extract data (as not all processors like accessing
        // buffers at non-32 byte boundaries)
        memcpy(&sourcePort, pSourcePort, sizeof(sourcePort));
        memcpy(&destPort, pDestPort, sizeof(destPort));

        sourcePort = ntohs(sourcePort);
        destPort = ntohs(destPort);

        WORD localPort {};
        WORD remotePort {};

        switch (direction) {
          case Direction_Incoming:
          {
            localPort = destPort;
            remotePort = sourcePort;
            break;
          }
          case Direction_Outgoing:
          {
            localPort = sourcePort;
            remotePort = destPort;
            break;
          }
        }

        if (outLocalPort) {
          memcpy(outLocalPort, &localPort, sizeof(localPort));
        }
        if (outRemotePort) {
          memcpy(outRemotePort, &remotePort, sizeof(remotePort));
        }

        return createTuple(localPort, remotePort);
      }

      //-------------------------------------------------------------------------
      static Log::Params slog(const char *message)
      {
        ElementPtr objectEl = Element::create("ortc::SCTPListenerHelper");
        return Log::Params(message, objectEl);
      }
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportListenerForSCTPTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ISCTPTransportListenerForSCTPTransport::toDebug(ForSCTPTransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(SCTPTransportListener, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    SCTPTransportListener::SCTPTransportListener(
                                                 const make_private &,
                                                 IMessageQueuePtr queue,
                                                 UseSecureTransportPtr secureTransport
                                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSecureTransport(DTLSTransport::convert(secureTransport)),
      mMaxPorts(SafeInt<decltype(mMaxPorts)>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MAX_PORTS))),
      mCurrentAllocationPort(SafeInt<decltype(mCurrentAllocationPort)>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_DEFAULT_PORT))),
      mMinAllocationPort(SafeInt<decltype(mMinAllocationPort)>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MIN_PORT))),
      mMaxAllocationPort(SafeInt<decltype(mMaxAllocationPort)>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MAX_PORT)))
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!secureTransport);

      ZS_EVENTING_5(
                    x, i, Detail, SctpTransportListenerCreate, ol, SctpTransportListener, Start,
                    puid, id, mID,
                    size_t, maxPorts, mMaxPorts,
                    word, currentAllocationPort, mCurrentAllocationPort,
                    word, minAllocationPort, mMinAllocationPort,
                    word, maxAllocationPort, mMaxAllocationPort
                    );

      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    SCTPTransportListener::~SCTPTransportListener()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, SctpTransportListenerDestroy, ol, SctpTransportListener, Stop, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    SCTPTransportListenerPtr SCTPTransportListener::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransportListener, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportListenerPtr SCTPTransportListener::convert(ForSCTPTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransportListener, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportListenerPtr SCTPTransportListener::convert(ForSecureTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransportListener, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => ISCTPTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr SCTPTransportListener::toDebug(SCTPTransportListenerPtr object)
    {
      if (!object) return ElementPtr();
      return object->toDebug();
    }

    //-------------------------------------------------------------------------
    SCTPTransportListener::CapabilitiesPtr SCTPTransportListener::getCapabilities()
    {
      CapabilitiesPtr result(make_shared<Capabilities>());
      result->mMaxMessageSize = ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE);
      result->mMinPort = SafeInt<decltype(result->mMinPort)>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MIN_PORT));
      result->mMaxPort = SafeInt<decltype(result->mMaxPort)>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MAX_PORT));
      result->mMaxUsablePorts = SafeInt<decltype(result->mMaxUsablePorts)>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MAX_PORTS));
      result->mMaxSessionsPerPort = SafeInt<decltype(result->mMaxSessionsPerPort)>(ISettings::getUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_SESSIONS_PER_PORT));
      return result;
    }

    //-------------------------------------------------------------------------
    ISCTPTransportListenerSubscriptionPtr SCTPTransportListener::listen(
                                                                        ISCTPTransportListenerDelegatePtr delegate,
                                                                        IDTLSTransportPtr transport,
                                                                        const Capabilities &remoteCapabilities
                                                                        )
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!transport)

      UseSecureTransportPtr secureTransport = DTLSTransport::convert(transport);
      ORTC_THROW_INVALID_PARAMETERS_IF(!secureTransport)

      ForSecureTransportPtr dataTransport = secureTransport->getDataTransport();
      ORTC_THROW_INVALID_STATE_IF(!dataTransport)

      auto listener = SCTPTransportListener::convert(dataTransport);
      ORTC_THROW_INVALID_STATE_IF(!listener);

      ZS_EVENTING_6(
                    x, i, Detail, SctpTransportListenerListen, ol, SctpTransportListener, Listen,
                    puid, id, listener->getID(),
                    size_t, remoteMaxMessageSize, remoteCapabilities.mMaxMessageSize,
                    word, remoteMinPort, remoteCapabilities.mMinPort,
                    word, remoteMaxPort,remoteCapabilities.mMaxPort,
                    word, remoteMaxUsablePorts, remoteCapabilities.mMaxUsablePorts,
                    word, remoteMaxSessionsPerPort, remoteCapabilities.mMaxSessionsPerPort
                    );

      return listener->subscribe(delegate, remoteCapabilities);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => ISCTPTransportListenerForSCTPTransport
    #pragma mark

    //-------------------------------------------------------------------------
    WORD SCTPTransportListener::allocateLocalPort()
    {
      AutoRecursiveLock lock(*this);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        return 0;
      }

      // allocate a local port without knowing the remote port
      return allocateLocalPort(0);
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::deallocateLocalPort(WORD previouslyAllocatedLocalPort)
    {
      if (0 == previouslyAllocatedLocalPort) return;
      deallocatePort(mAllocatedLocalPorts, previouslyAllocatedLocalPort);
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::registerNewTransport(
                                                     IDTLSTransportPtr dtlsTransport,
                                                     UseSCTPTransportPtr &ioTransport,
                                                     WORD &ioLocalPort,
                                                     bool localPortWasPreallocated,
                                                     WORD &ioRemotePort
                                                     )
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!dtlsTransport);
      ORTC_THROW_INVALID_PARAMETERS_IF(!ioTransport);

      UseSecureTransportPtr secureTransport = DTLSTransport::convert(dtlsTransport);
      ORTC_THROW_INVALID_STATE_IF(!secureTransport)

      auto dataTransport = secureTransport->getDataTransport();
      ORTC_THROW_INVALID_STATE_IF(!dataTransport);

      WORD localPort = ioLocalPort;
      WORD remotePort = ioRemotePort;

      ioLocalPort = 0;
      ioRemotePort = 0;

      if (0 == remotePort) remotePort = localPort;

      {
        AutoRecursiveLock lock(*this);

        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_WARNING(Debug, log("unable to register transport is shutting down (or shutdown)"))
          ORTC_THROW_INVALID_STATE("unable to register transport as listener is shutting down")
          return;
        }

        if (0 != localPort) {
          DWORD tupleID = UseListenerHelper::createTuple(localPort, remotePort);
          auto found = mTransports.find(tupleID);
          if (found != mTransports.end()) {
            ioTransport = (*found).second;

            ZS_LOG_TRACE(log("found existing transport") + ZS_PARAM("transport", ioTransport->getID()) + ZS_PARAM("local port", localPort) + ZS_PARAM("remote port", remotePort) + ZS_PARAM("tuple id", tupleID));

            if ((ioTransport->isShuttingDown()) ||
                (ioTransport->isShutdown())) {
              ZS_LOG_WARNING(Debug, log("unable to create new transport as port is in use and shutting down / shutdown"))
              ioTransport = UseSCTPTransportPtr();
              return;
            }

            ioLocalPort = localPort;
            ioRemotePort = remotePort;

            ZS_EVENTING_4(
                          x, i, Detail, SctpTransportListenerRegisterNewTransport, ol, SctpTransportListener, Info,
                          puid, id, mID,
                          puid, secureTransportId, secureTransport->getID(),
                          word, localPort, ioLocalPort,
                          word, remotePort, ioRemotePort
                          );

            ZS_LOG_DEBUG(log("found existing transport") + ZS_PARAM("transport", ioTransport->getID()) + ZS_PARAM("local port", localPort) + ZS_PARAM("remote port", remotePort) + ZS_PARAM("tuple id", tupleID));
            return;
          }

          if (!localPortWasPreallocated) {
            auto foundLocalPort = mAllocatedLocalPorts.find(localPort);
            if (foundLocalPort != mAllocatedLocalPorts.end()) {
              ZS_LOG_WARNING(Detail, log("port already in use (and mapped to a different remote port)") + ZS_PARAM("local port", localPort) + ZS_PARAM("remote port", remotePort) + ZS_PARAM("tuple id", tupleID))
                ioTransport = UseSCTPTransportPtr();
              return;
            }
          }

          allocatePort(mAllocatedLocalPorts, localPort);
          allocatePort(mAllocatedRemotePorts, remotePort);

          ioLocalPort = localPort;
          ioRemotePort = remotePort;
          mTransports[tupleID] = ioTransport;
          ZS_EVENTING_4(
                        x, i, Detail, SctpTransportListenerRegisterNewTransport, ol, SctpTransportListener, Info,
                        puid, id, mID,
                        puid, secureTransportId, secureTransport->getID(),
                        word, localPort, ioLocalPort,
                        word, remotePort, ioRemotePort
                        );
          return;
        }

        if (mTransports.size() + 1 > mMaxPorts) {
          ZS_LOG_WARNING(Detail, log("too many ports already in use") + ZS_PARAM("total", mTransports.size()) + ZS_PARAM("max ports", mMaxPorts))
          ioTransport = UseSCTPTransportPtr();
          return;
        }

        localPort = allocateLocalPort(remotePort);
        if (0 == localPort) {
          ZS_LOG_WARNING(Detail, log("unable to find an available non used port"))
          ioTransport = UseSCTPTransportPtr();
          return;
        }
        if (0 == remotePort) remotePort = localPort;

        ioLocalPort = localPort;
        ioRemotePort = remotePort;

        DWORD tupleID = UseListenerHelper::createTuple(localPort, remotePort);

        ZS_EVENTING_4(
                      x, i, Detail, SctpTransportListenerRegisterNewTransport, ol, SctpTransportListener, Info,
                      puid, id, mID,
                      puid, secureTransportId, secureTransport->getID(),
                      word, localPort, ioLocalPort,
                      word, remotePort, ioRemotePort
                      );
        ZS_LOG_DEBUG(log("registered local/remote port pairing") + ZS_PARAM("transport", ioTransport->getID()) + ZS_PARAM("local port", localPort) + ZS_PARAM("remote port", remotePort) + ZS_PARAM("tuple id", tupleID));

        mTransports[tupleID] = ioTransport;
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::announceTransport(
                                                  UseSCTPTransportPtr transport,
                                                  WORD localPort,
                                                  WORD remotePort
                                                  )
    {
      AutoRecursiveLock lock(*this);

      if (isShutdown()) {
        ZS_LOG_WARNING(Detail, log("cannot announce transport (as shutting down)"))
        return;
      }

      auto tuple = UseListenerHelper::createTuple(localPort, remotePort);
      auto found = mTransports.find(tuple);
      if (found == mTransports.end()) {
        ZS_LOG_WARNING(Detail, log("cannot announce incoming transport (as transport was not found in transport list)"))
        return;
      }

      ZS_EVENTING_4(
                    x, i, Detail, SctpTransportListenerSctpTransportEvent, ol, SctpTransportListener, InternalEvent,
                    puid, id, mID,
                    puid, secureTransportId, transport->getID(),
                    word, localPort, localPort,
                    word, remotePort, remotePort
                    );

      ZS_LOG_DEBUG(log("announcing incoming transport") + ZS_PARAM("transport", transport->getID()))

      mSubscriptions.delegate()->onSCTPTransport(SCTPTransport::convert(transport));

      mAnnouncedTransports[transport->getID()] = transport;
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::notifyShutdown(
                                               UseSCTPTransport &transport,
                                               WORD localPort,
                                               WORD remotePort
                                               )
    {
      ZS_EVENTING_4(
                    x, i, Detail, SctpTransportListenerSctpTransportShutdownEvent, ol, SctpTransportListener, InternalEvent,
                    puid, id, mID,
                    puid, sctpTransportId, transport.getID(),
                    word, localPort, localPort,
                    word, remotePort, remotePort
                    );

      AutoRecursiveLock lock(*this);

      if (isShutdown()) {
        ZS_LOG_TRACE(log("ignoring SCTP transport shutdown notification"))
        return;
      }

      auto tuple = UseListenerHelper::createTuple(localPort, remotePort);

      ZS_LOG_DETAIL(log("notified shutdown of SCTP transport") + ZS_PARAM("tuple", tuple) + ZS_PARAM("local port", localPort) + ZS_PARAM("remote port", remotePort) + ZS_PARAM("transport id", transport.getID()))

      {
        auto found = mTransports.find(tuple);
        if (found != mTransports.end()) {
          auto registeredTransport = (*found).second;
          if (registeredTransport->getID() == transport.getID()) {
            deallocatePort(mAllocatedLocalPorts, localPort);
            deallocatePort(mAllocatedRemotePorts, remotePort);
            mTransports.erase(found);
          }
        }
      }

      {
        auto found = mAnnouncedTransports.find(transport.getID());
        if (found != mAnnouncedTransports.end()) mAnnouncedTransports.erase(found);
      }

      {
        auto found = mPendingTransports.find(transport.getID());
        if (found != mPendingTransports.end()) mPendingTransports.erase(found);
      }

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => ISCTPTransportForSecureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    SCTPTransportListener::ForSecureTransportPtr SCTPTransportListener::create(UseSecureTransportPtr transport)
    {
      SCTPTransportListenerPtr pThis(make_shared<SCTPTransportListener>(make_private {}, IORTCForInternal::queueORTC(), transport));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransportListener::handleDataPacket(
                                                 const BYTE *buffer,
                                                 size_t bufferLengthInBytes
                                                 )
    {
      ZS_EVENTING_3(
                    x, i, Trace, SctpTransportListenerReceivedIncomingDataPacket, ol, SctpTransportListener, Receive,
                    puid, id, mID,
                    buffer, dataPacket, buffer,
                    size, size, bufferLengthInBytes
                    );

      if (bufferLengthInBytes < sizeof(DWORD)) {
        ZS_LOG_WARNING(Trace, log("packet length is too small to be an SCTP packet"))
        return false;
      }

      WORD localPort {};
      WORD remotePort {};
      DWORD tupleID = UseListenerHelper::getLocalRemoteTuple(buffer, bufferLengthInBytes, UseListenerHelper::Direction_Incoming, &localPort, &remotePort);
      if (0 == tupleID) {
        ZS_LOG_WARNING(Trace, log("incoming packet is not valid") + ZS_PARAM("buffer length", bufferLengthInBytes))
        return false;
      }

      UseSCTPTransportPtr transport;

      {
        AutoRecursiveLock lock(*this);

        auto found = mTransports.find(tupleID);
        if (found == mTransports.end()) {
          transport = UseSCTPTransport::create(mThisWeak.lock(), mSecureTransport.lock(), localPort);
          if (!transport) {
            ZS_LOG_WARNING(Debug, log("unable to create sctp transport"))
            return false;
          }
          ZS_EVENTING_4(
                        x, i, Detail, SctpTransportListenerSctpTransportCreatedEvent, ol, SctpTransportListener, Receive,
                        puid, id, mID,
                        puid, sctpTransportId, transport->getID(),
                        word, localPort, localPort,
                        word, remotePort, remotePort
                        );

          if (mRemoteCapabilities) {
            transport->start(*mRemoteCapabilities, remotePort);
          } else {
            mPendingTransports[transport->getID()] = TransportPortPair(transport, remotePort);
          }
          allocatePort(mAllocatedLocalPorts, localPort);
          allocatePort(mAllocatedRemotePorts, remotePort);
          mTransports[tupleID] = transport;
        } else {
          transport = (*found).second;
        }
      }

      ZS_EVENTING_4(
                    x, i, Trace, SctpTransportListenerDeliverIncomingDataPacket, ol, SctpTransportListener, Receive,
                    puid, id, mID,
                    puid, sctpTransportId, transport->getID(),
                    buffer, dataPacket, buffer,
                    size,size, bufferLengthInBytes
                    );
      return transport->handleDataPacket(buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => ISCTPTransportListenerAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SCTPTransport => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SCTPTransportListener::onWake()
    {
      ZS_LOG_TRACE(log("on wake"))
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
    Log::Params SCTPTransportListener::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::SCTPTransportListener");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SCTPTransportListener::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::SCTPTransportListener");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SCTPTransportListener::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr SCTPTransportListener::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::SCTPTransportListener");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());

      IHelper::debugAppend(resultEl, "shutdown", mShutdown);

      auto secureTransport = mSecureTransport.lock();
      IHelper::debugAppend(resultEl, "secure transport", secureTransport ? secureTransport->getID() : 0);

      IHelper::debugAppend(resultEl, "transports", mTransports.size());
      IHelper::debugAppend(resultEl, "pending transports", mPendingTransports.size());
      IHelper::debugAppend(resultEl, "announced transports", mAnnouncedTransports.size());

      IHelper::debugAppend(resultEl, "allocated local ports", mAllocatedLocalPorts.size());
      IHelper::debugAppend(resultEl, "allocated remote ports", mAllocatedRemotePorts.size());

      IHelper::debugAppend(resultEl, "max ports", mMaxPorts);

      IHelper::debugAppend(resultEl, "current allocate port", mCurrentAllocationPort);
      IHelper::debugAppend(resultEl, "min allocation port", mMinAllocationPort);
      IHelper::debugAppend(resultEl, "max allocation port", mMaxAllocationPort);
      IHelper::debugAppend(resultEl, "next allocation increment", mNextAllocationIncremement);

      IHelper::debugAppend(resultEl, "remote capabilities", mRemoteCapabilities ? mRemoteCapabilities->toDebug() : ElementPtr());

      return resultEl;
    }
    
    //-------------------------------------------------------------------------
    bool SCTPTransportListener::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }

    //-------------------------------------------------------------------------
    bool SCTPTransportListener::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      ZS_EVENTING_1(x, i, Debug, SctpTransportListenerStep, ol, SctpTransportListener, Step, puid, id, mID);

      // ... other steps here ...
      // ... other steps here ...

      goto ready;

#if 0
    not_ready:
      {
        ZS_LOG_TRACE(debug("not ready"))
        return;
      }
#endif //0

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, SctpTransportListenerCancel, ol, SctpTransportListener, Cancel, puid, id, mID);

      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) {
        ZS_LOG_TRACE(log("already shutdown"))
        return;
      }

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
        for (auto iter_doNotUse = mTransports.begin(); iter_doNotUse != mTransports.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto tupleID = (*current).first;
          auto transport = (*current).second;
          transport->notifyShutdown();

          if (transport->isShutdown()) {
            WORD localPort = 0;
            WORD remotePort = 0;
            UseListenerHelper::splitTuple(tupleID, localPort, remotePort);

            deallocatePort(mAllocatedLocalPorts, localPort);
            deallocatePort(mAllocatedRemotePorts, remotePort);

            mTransports.erase(current);

            {
              auto found = mPendingTransports.find(transport->getID());
              if (found != mPendingTransports.end()) mPendingTransports.erase(found);
            }
            {
              auto found = mAnnouncedTransports.find(transport->getID());
              if (found != mAnnouncedTransports.end()) mAnnouncedTransports.erase(found);
            }
            continue;
          }
        }

        if (mTransports.size() > 0) {
          ZS_LOG_TRACE(log("waiting for transports to shutdown") + ZS_PARAM("transport", mTransports.size()))
          return;
        }
      }

      //.......................................................................
      // final cleanup

      mShutdown = true;

      mSubscriptions.clear();

      // scope: final removal of all transports
      {
        for (auto iter_doNotUse = mTransports.begin(); iter_doNotUse != mTransports.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto transport = (*current).second;
          transport->notifyShutdown();
        }

        mTransports.clear();

        mAllocatedLocalPorts.clear();
        mAllocatedRemotePorts.clear();
      }

      mPendingTransports.clear();
      mAnnouncedTransports.clear();

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    ISCTPTransportListenerSubscriptionPtr SCTPTransportListener::subscribe(
                                                                           ISCTPTransportListenerDelegatePtr originalDelegate,
                                                                           const Capabilities &remoteCapabilities
                                                                           )
    {
      ZS_LOG_DETAIL(log("subscribing to transport listener"))

      AutoRecursiveLock lock(*this);

      ISCTPTransportListenerSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      ISCTPTransportListenerDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      mRemoteCapabilities = make_shared<Capabilities>(remoteCapabilities);

      for (auto iter = mPendingTransports.begin(); iter != mPendingTransports.end(); ++iter)
      {
        auto port = (*iter).second.second;
        UseSCTPTransportPtr transport = (*iter).second.first;
        transport->start(*mRemoteCapabilities, port);
      }
      mPendingTransports.clear();

      if (delegate) {
        SCTPTransportListenerPtr pThis = mThisWeak.lock();

        for (auto iter = mAnnouncedTransports.begin(); iter != mAnnouncedTransports.end(); ++iter) {
          // NOTE: ID of data channels are always greater than last so order
          // should be guarenteed.
          UseSCTPTransportPtr sctpTransport = (*iter).second;
          delegate->onSCTPTransport(SCTPTransport::convert(sctpTransport));
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }
      
      return subscription;
    }
    
    //-------------------------------------------------------------------------
    WORD SCTPTransportListener::allocateLocalPort(WORD remotePort)
    {
      WORD maxSearching = mMaxAllocationPort - mMinAllocationPort;

      while (true) {

        {
          auto found = mAllocatedLocalPorts.find(mCurrentAllocationPort);
          if (found != mAllocatedLocalPorts.end()) goto next_port;
        }

        if (0 == remotePort) {
          auto found = mAllocatedRemotePorts.find(mCurrentAllocationPort);
          if (found != mAllocatedRemotePorts.end()) goto next_port;
        }

        goto found_port;

      next_port:
        mCurrentAllocationPort = mCurrentAllocationPort + mNextAllocationIncremement;

        if (mCurrentAllocationPort < mMinAllocationPort) mCurrentAllocationPort = mMinAllocationPort + (mCurrentAllocationPort % 2);
        if (mCurrentAllocationPort > mMaxAllocationPort) mCurrentAllocationPort = mMinAllocationPort + (mCurrentAllocationPort % 2);

        if (0 == maxSearching) return 0;
        --maxSearching;
      }

    found_port:
      {
        allocatePort(mAllocatedLocalPorts, mCurrentAllocationPort);
        if (0 == remotePort) remotePort = mCurrentAllocationPort;
        allocatePort(mAllocatedRemotePorts, remotePort);
      }

      return mCurrentAllocationPort;
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::allocatePort(
                                             AllocatedPortMap &useMap,
                                             WORD port
                                             )
    {
      auto found = useMap.find(port);
      if (found == useMap.end()) {
        useMap[port] = 1;
        return;
      }

      size_t &total = (*found).second;
      ++total;
    }

    //-------------------------------------------------------------------------
    void SCTPTransportListener::deallocatePort(
                                               AllocatedPortMap &useMap,
                                               WORD port
                                               )
    {
      auto found = useMap.find(port);
      if (found == useMap.end()) {
        ZS_LOG_WARNING(Debug, log("allocation was not found") + ZS_PARAM("port", port))
        return;
      }

      size_t &total = (*found).second;
      --total;

      if (0 != total) return;
      useMap.erase(found);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    ISCTPTransportListenerFactory &ISCTPTransportListenerFactory::singleton()
    {
      return SCTPTransportListenerFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ISCTPTransportListenerSubscriptionPtr ISCTPTransportListenerFactory::listen(
                                                                                ISCTPTransportListenerDelegatePtr delegate,
                                                                                IDTLSTransportPtr transport,
                                                                                const Capabilities &remoteCapabilities
                                                                                )
    {
      if (this) {}
      return internal::SCTPTransportListener::listen(delegate, transport, remoteCapabilities);
    }

    //-------------------------------------------------------------------------
    ISCTPTransportListenerFactory::ForSecureTransportPtr ISCTPTransportListenerFactory::create(UseSecureTransportPtr transport)
    {
      if (this) {}
      return internal::SCTPTransportListener::create(transport);
    }

    //-------------------------------------------------------------------------
    ISCTPTransportFactory::CapabilitiesPtr ISCTPTransportListenerFactory::getCapabilities()
    {
      if (this) {}
      return SCTPTransportListener::getCapabilities();
    }

  } // internal namespace



}
