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

#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ORTC.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    static IICESocket::Types convert(IICETransport::CandidateTypes type)
    {
      switch (type) {
        case IICETransport::CandidateType_Unknown:          return IICESocket::Type_Unknown;
        case IICETransport::CandidateType_Local:            return IICESocket::Type_Local;
        case IICETransport::CandidateType_ServerReflexive:  return IICESocket::Type_ServerReflexive;
        case IICETransport::CandidateType_PeerReflexive:    return IICESocket::Type_PeerReflexive;
        case IICETransport::CandidateType_Relayed:          return IICESocket::Type_Relayed;
      }
      return IICESocket::Type_Unknown;
    }

    //-------------------------------------------------------------------------
    static IICETransport::CandidateTypes convert(IICESocket::Types type)
    {
      switch (type) {
        case IICESocket::Type_Unknown:          return IICETransport::CandidateType_Unknown;
        case IICESocket::Type_Local:            return IICETransport::CandidateType_Local;
        case IICESocket::Type_ServerReflexive:  return IICETransport::CandidateType_ServerReflexive;
        case IICESocket::Type_PeerReflexive:    return IICETransport::CandidateType_PeerReflexive;
        case IICESocket::Type_Relayed:          return IICETransport::CandidateType_Relayed;
      }
      return IICETransport::CandidateType_Unknown;
    }

    //-------------------------------------------------------------------------
    static bool hasRelated(IICETransport::CandidateTypes type)
    {
      switch (type) {
        case IICETransport::CandidateType_Unknown:          return false;
        case IICETransport::CandidateType_Local:            return false;
        case IICETransport::CandidateType_ServerReflexive:  return true;
        case IICETransport::CandidateType_PeerReflexive:    return true;
        case IICETransport::CandidateType_Relayed:          return true;
      }
      return false;
    }

    //-------------------------------------------------------------------------
    static IICESocket::ICEControls convert(IICETransport::Roles role)
    {
      switch (role) {
        case IICETransport::Role_Controlling:  return IICESocket::ICEControl_Controlling;
        case IICETransport::Role_Controlled:   return IICESocket::ICEControl_Controlled;
      }
      return IICESocket::ICEControl_Controlling;
    }

    //-------------------------------------------------------------------------
    static IICETransport::Roles convert(IICESocket::ICEControls role)
    {
      switch (role) {
        case IICESocket::ICEControl_Controlling:  return IICETransport::Role_Controlling;
        case IICESocket::ICEControl_Controlled:   return IICETransport::Role_Controlled;
      }
      return IICETransport::Role_Controlling;
    }

    //-------------------------------------------------------------------------
    static bool isMatch(
                 const IICETransport::CandidateInfo &candidate1,
                 const IICESocket::Candidate &candidate2
                 )
    {
      if (candidate1.mType != convert(candidate2.mType)) return false;
      if (candidate1.mFoundation != candidate2.mFoundation) return false;
      if (candidate1.mComponent != candidate2.mComponentID) return false;
      if (candidate1.mPriority != candidate2.mPriority) return false;
      if (!candidate1.mConnectionAddress.isAddressEqual(candidate2.mIPAddress)) return false;
      if (hasRelated(candidate1.mType)) {
        if (!candidate1.mRelatedAddress.isAddressEqual(candidate2.mRelatedIP)) return false;
      }

      return true;
    }
    //-------------------------------------------------------------------------
    static bool isMatch(
                 const IICESocket::Candidate &candidate2,
                 const IICETransport::CandidateInfo &candidate1
                 )
    {
      return isMatch(candidate2, candidate1);
    }

    //-------------------------------------------------------------------------
    static IICETransport::CandidateInfoPtr createFrom(const IICESocket::Candidate &source)
    {
      IICETransport::CandidateInfoPtr result = IICETransport::CandidateInfo::create();

      result->mFoundation = source.mFoundation;
      result->mComponent = source.mComponentID;
      result->mTransport = "udp";
      result->mPriority = source.mPriority;
      result->mConnectionAddress = source.mIPAddress;
      result->mType = convert(source.mType);
      result->mRelatedAddress = source.mRelatedIP;

      return result;
    }

    //-------------------------------------------------------------------------
    static IICESocket::CandidatePtr createFrom(const IICETransport::CandidateInfo &source)
    {
      IICESocket::CandidatePtr result = IICESocket::Candidate::create();

      result->mType = convert(source.mType);
      result->mFoundation = source.mFoundation;
      result->mComponentID = source.mComponent;
      result->mIPAddress = source.mConnectionAddress;
      result->mPriority = source.mPriority;
      result->mLocalPreference = 0;
      result->mRelatedIP = source.mRelatedAddress;

      return result;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForDTLSTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IICETransportForDTLSTransport::toDebug(ForDTLSTransportPtr transport)
    {
      return ICETransport::toDebug(boost::dynamic_pointer_cast<ICETransport>(transport));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ICETransport::ICETransport(
                               IMessageQueuePtr queue,
                               IICETransportDelegatePtr originalDelegate,
                               ServerListPtr servers
                               ) :
      MessageQueueAssociator(queue),
      mCurrentState(ConnectionState_New),
      mServers(servers)
    {
      ZS_LOG_BASIC(debug("created"))

      mDefaultSubscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate), queue);
    }

    //-------------------------------------------------------------------------
    void ICETransport::init()
    {
      // kick start the process
      AutoRecursiveLock lock(getLock());
      step();
    }

    //-------------------------------------------------------------------------
    ICETransport::~ICETransport()
    {
      if (isNoop()) return;

      ZS_LOG_BASIC(log("destroyed"))
      mThisWeak.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(IICETransportPtr object)
    {
      return boost::dynamic_pointer_cast<ICETransport>(object);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(ForDTLSTransportPtr object)
    {
      return boost::dynamic_pointer_cast<ICETransport>(object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr ICETransport::toDebug(IICETransportPtr transport)
    {
      if (!transport) return ElementPtr();
      ICETransportPtr pThis = ICETransport::convert(transport);
      return pThis->toDebug();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::create(
                                         IICETransportDelegatePtr delegate,
                                         ServerListPtr servers
                                         )
    {
      ICETransportPtr pThis(new ICETransport(IORTCForInternal::queueORTC(), delegate, servers));
      pThis->mThisWeak.lock();
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    PUID ICETransport::getID() const
    {
      return mID;
    }

    //-------------------------------------------------------------------------
    IICETransportSubscriptionPtr ICETransport::subscribe(IICETransportDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(getLock());
      if (!originalDelegate) return mDefaultSubscription;

      IICETransportSubscriptionPtr subscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));

      IICETransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        ICETransportPtr pThis = mThisWeak.lock();

        if (!isShutdown()) {
          for (CandidateListOuter::iterator iter = mLocalCandidatesOuter.begin(); iter != mLocalCandidatesOuter.end(); ++iter)
          {
            CandidateInfoPtr &info = (*iter);
            delegate->onICETransportCandidate(pThis, info);
          }

          if (mNotifiedCandidatesEnd) {
            delegate->onICETransportEndOfCandidates(pThis);
          }
        }

        if (ConnectionState_New != mCurrentState) {
          delegate->onICETransportStateChanged(pThis, mCurrentState);
        }

        if (!isShutdown()) {
          IICESocket::Candidate local;
          IICESocket::Candidate remote;
          bool result = mSession->getNominatedCandidateInformation(local, remote);

          if (result) {
            delegate->onICETransportActiveCandidate(mThisWeak.lock(), createFrom(local), createFrom(remote));
          }
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IICETransport::CapabilitiesPtr ICETransport::getCapabilities()
    {
      return CapabilitiesPtr();
    }

    //-------------------------------------------------------------------------
    IICETransport::TransportInfoPtr ICETransport::createParams(CapabilitiesPtr capabilities)
    {
      return TransportInfoPtr();
    }

    //-------------------------------------------------------------------------
    IICETransport::TransportInfoPtr ICETransport::filterParams(
                                                               TransportInfoPtr params,
                                                               CapabilitiesPtr capabilities
                                                               )
    {
      return TransportInfoPtr();
    }

    //-------------------------------------------------------------------------
    IICETransport::TransportInfoPtr ICETransport::getLocal()
    {
      AutoRecursiveLock lock(getLock());
      return mLocal;
    }

    //-------------------------------------------------------------------------
    IICETransport::TransportInfoPtr ICETransport::getRemote()
    {
      AutoRecursiveLock lock(getLock());
      return mRemote;
    }

    //-------------------------------------------------------------------------
    void ICETransport::setLocal(TransportInfoPtr info)
    {
      AutoRecursiveLock lock(getLock());

      ZS_LOG_TRACE(log("set local called") + ZS_PARAM("local", info ? info->toDebug() : ElementPtr()))

      mLocal = info;
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICETransport::setRemote(TransportInfoPtr info)
    {
      AutoRecursiveLock lock(getLock());

      ZS_LOG_TRACE(log("set remote called") + ZS_PARAM("remote", info ? info->toDebug() : ElementPtr()))

      mRemote = info;
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICETransport::start(
                             TransportInfoPtr localTransportInfo,
                             Roles role
                             )
    {
      AutoRecursiveLock lock(getLock());

      ZS_LOG_DEBUG(log("start called"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_WARNING(Detail, log("already shutdown"));
        return;
      }

      get(mStartCalled) = true;

      if (mSocket) {
        mSocket->wakeup();
      }

      step();
    }

    //-------------------------------------------------------------------------
    void ICETransport::stop()
    {
      AutoRecursiveLock lock(getLock());

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_WARNING(Detail, log("already shutting down/shutdown"))
        return;
      }

      mGracefulShutdownReference = mThisWeak.lock();

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    IICETransport::ConnectionStates ICETransport::getState(
                                                           WORD *outError,
                                                           String *outReason
                                                           )
    {
      AutoRecursiveLock lock(getLock());

      ZS_LOG_DEBUG(log("get state") + ZS_PARAM("current state", IICETransport::toString(mCurrentState)) + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))

      if (outError) {
        *outError = mLastError;
      }
      if (outReason) {
        *outReason = mLastErrorReason;
      }

      return mCurrentState;
    }

    //-------------------------------------------------------------------------
    IICETransport::Roles ICETransport::getRole()
    {
      AutoRecursiveLock lock(getLock());

      if (!mSession) {
        ZS_LOG_TRACE(log("no session so returning default") + ZS_PARAM("default", IICETransport::toString(mDefaultRole)))
        return mDefaultRole;
      }

      IICESocket::ICEControls control = mSession->getConnectedControlState();
      ZS_LOG_TRACE(log("controll returned from session") + ZS_PARAM("control", IICESocket::toString(control)))
      return internal::convert(control);
    }

    //-------------------------------------------------------------------------
    void ICETransport::addRemoteCandidate(CandidateInfoPtr candidate)
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!candidate)

      ZS_LOG_DEBUG(log("adding remote candidate") + candidate->toDebug())

      AutoRecursiveLock lock(getLock());
      mPendingRemoteCandidates.push_back(candidate);

      // process this request asynchronously
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportForDTLS
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::attach(DTLSTransportPtr inDtlsTransport)
    {
      UseDTLSTransportPtr dtlsTransport = inDtlsTransport;

      AutoRecursiveLock lock(getLock());

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_WARNING(Detail, log("attempting to attach DTLS to a closed ice transport"))
        return;
      }

      mAttachedDTLSTransportID = (dtlsTransport ? dtlsTransport->getID() : 0);
      mDTLSTransport = dtlsTransport;

      // kick start step just in case
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void ICETransport::detach(DTLSTransport &inDtlsTransport)
    {
      UseDTLSTransport &dtlsTransport = inDtlsTransport;

      AutoRecursiveLock lock(getLock());

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_TRACE(log("attempting to detach DTLS from a closed ice transport (probably okay)"))
        return;
      }

      PUID id = dtlsTransport.getID();

      if (id != mAttachedDTLSTransportID) {
        ZS_LOG_WARNING(Detail, log("detaching transport was not attached") + ZS_PARAM("attached", mAttachedDTLSTransportID) + ZS_PARAM("detaching", id))
        return;
      }

      mAttachedDTLSTransportID = 0;
      mDTLSTransport.reset();

      // kick start step just in case
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    bool ICETransport::sendPacket(
                                  const BYTE *buffer,
                                  size_t bufferLengthInBytes
                                  ) const
    {
      ZS_LOG_TRACE(log("sending packet over ICE socket session") + ZS_PARAM("legnth", bufferLengthInBytes))

      IICESocketSessionPtr session;

      {
        AutoRecursiveLock lock(getLock());
        if (!mSession) {
          ZS_LOG_WARNING(Debug, log("ice socket session is gone"))
          return false;
        }

        session = mSession;
      }

      return session->sendPacket(buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(getLock());
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICESocketDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onICESocketStateChanged(
                                               IICESocketPtr socket,
                                               ICESocketStates state
                                               )
    {
      ZS_LOG_DEBUG(log("ice socket state changed") + ZS_PARAM("session", socket->getID()) + ZS_PARAM("state", IICESocket::toString(state)))

      AutoRecursiveLock lock(getLock());
      step();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onICESocketCandidatesChanged(IICESocketPtr socket)
    {
      ZS_LOG_DEBUG(log("ice socket candidates"))

      AutoRecursiveLock lock(getLock());
      step();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICESocketSessionDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::onICESocketSessionStateChanged(
                                                      IICESocketSessionPtr session,
                                                      ICESocketSessionStates state
                                                      )
    {
      ZS_LOG_DEBUG(log("ice socket session state changed") + ZS_PARAM("session", session->getID()) + ZS_PARAM("state", IICESocketSession::toString(state)))

      AutoRecursiveLock lock(getLock());
      step();
    }

    //-------------------------------------------------------------------------
    void ICETransport::onICESocketSessionNominationChanged(IICESocketSessionPtr session)
    {
      ZS_LOG_DEBUG(log("ice socket nomination changed") + ZS_PARAM("session", session->getID()))

      AutoRecursiveLock lock(getLock());
      get(mNominationChanged) = true;
      step();
    }

    //-------------------------------------------------------------------------
    void ICETransport::handleICESocketSessionReceivedPacket(
                                                            IICESocketSessionPtr session,
                                                            const BYTE *buffer,
                                                            size_t bufferLengthInBytes
                                                            )
    {
      ZS_THROW_INVALID_ARGUMENT_IF(!buffer)
      ZS_THROW_INVALID_ARGUMENT_IF(bufferLengthInBytes < 1) // why is an empty packet being sent?

      ZS_LOG_TRACE(log("packet received from network") + ZS_PARAM("internal ice session", session->getID()) + ZS_PARAM("length", bufferLengthInBytes))

      UseDTLSTransportPtr dltsTransport;

      {
        AutoRecursiveLock lock(getLock());
        dltsTransport = mDTLSTransport.lock();
      }

      if (!dltsTransport) {
        ZS_LOG_WARNING(Debug, log("no dtls transport is attached (dropping packet)"))
        return;
      }

      // WARNING: perform forwarding of packet outside lock

      dltsTransport->handleReceivedPacket(mThisWeak.lock(), buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    bool ICETransport::handleICESocketSessionReceivedSTUNPacket(
                                                                IICESocketSessionPtr session,
                                                                STUNPacketPtr stun,
                                                                const String &localUsernameFrag,
                                                                const String &remoteUsernameFrag
                                                                )
    {
      // ignored
      return false;
    }

    //-------------------------------------------------------------------------
    void ICETransport::onICESocketSessionWriteReady(IICESocketSessionPtr session)
    {
      // ignored
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params ICETransport::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ICETransport");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ICETransport::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr ICETransport::toDebug() const
    {
      ElementPtr resultEl = Element::create("ICETransport");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", IICETransport::toString(mCurrentState));
      UseServicesHelper::debugAppend(resultEl, "start called", mStartCalled);

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "servers", mServers ? mServers->size() : 0);

      UseServicesHelper::debugAppend(resultEl, "default role", IICETransport::toString(mDefaultRole));

      UseServicesHelper::debugAppend(resultEl, "local", mLocal ? mLocal->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(resultEl, "remote", mRemote ? mRemote->toDebug() : ElementPtr());

      UseServicesHelper::debugAppend(resultEl, "ice socket", IICESocket::toDebug(mSocket));
      UseServicesHelper::debugAppend(resultEl, "ice socket subscription", (bool)mSocketSubscription);
      UseServicesHelper::debugAppend(resultEl, "notified candidates end", mNotifiedCandidatesEnd);
      UseServicesHelper::debugAppend(resultEl, "notified candidates version", mCandidatesVersion);
      UseServicesHelper::debugAppend(resultEl, "local candidates inner", mLocalCandidatesInner.size());
      UseServicesHelper::debugAppend(resultEl, "local candidates outer", mLocalCandidatesOuter.size());

      UseServicesHelper::debugAppend(resultEl, "ice session", IICESocketSession::toDebug(mSession));
      UseServicesHelper::debugAppend(resultEl, "ice session subscription", (bool)mSessionSubscription);

      UseServicesHelper::debugAppend(resultEl, "pending remote candidates", mPendingRemoteCandidates.size());
      UseServicesHelper::debugAppend(resultEl, "added remote candidates", mAddedRemoteCandidates.size());

      UseServicesHelper::debugAppend(resultEl, "dtls transport id", mAttachedDTLSTransportID);
      UseServicesHelper::debugAppend(resultEl, "dtls transport", (bool)mDTLSTransport.lock());

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return ConnectionState_Closed == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void ICETransport::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      if (!stepSocketCreate()) return;
      if (!stepSocket()) return;
      if (!stepSession()) return;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepSocketCreate()
    {
      typedef IICESocket::TURNServerInfo TURNServerInfo;
      typedef IICESocket::TURNServerInfoPtr TURNServerInfoPtr;
      typedef IICESocket::STUNServerInfo STUNServerInfo;
      typedef IICESocket::STUNServerInfoPtr STUNServerInfoPtr;
      typedef IICESocket::STUNServerInfoList STUNServerInfoList;
      typedef IICESocket::TURNServerInfoList TURNServerInfoList;
      typedef IICESocket::STUNServerInfoList STUNServerInfoList;
      typedef UseServicesHelper::SplitMap SplitMap;

      if (mSocket) {
        ZS_LOG_TRACE(log("socket already created"))
        return true;
      }

      ZS_LOG_DETAIL(log("creating ice socket"))

      TURNServerInfoList turnServers;
      STUNServerInfoList stunServers;

      if (mServers) {

        for (ServerList::iterator iter = mServers->begin(); iter != mServers->end(); ++iter) {

          ServerInfoPtr &info = (*iter);

          ZS_THROW_INVALID_USAGE_IF(!info)

          // scope: parse out stun / turn server information
          {
            SplitMap split;
            UseServicesHelper::split(info->mURL, split, ':');

            if (split.size() < 2) goto invalid_url;

            String type = (*(split.find(0))).second;
            String uri = (*(split.find(1))).second;

            String username;
            String domain;

            SplitMap userDomainSplit;
            UseServicesHelper::split(uri, userDomainSplit, '@');

            if (userDomainSplit.size() < 1) goto invalid_url;

            if (userDomainSplit.size() < 2) {
              // only domain
              domain = (*(userDomainSplit.find(0))).second;
            } else {
              username = (*(userDomainSplit.find(0))).second;
              domain = (*(userDomainSplit.find(1))).second;
            }

            if (domain.isEmpty()) goto invalid_url;

            if (!IPAddress::isConvertable(domain)) {
              if (!UseServicesHelper::isValidDomain(domain)) goto invalid_url;
            }

            if ("turn" == type) {
              TURNServerInfoPtr turnInfo = TURNServerInfo::create();
              turnInfo->mTURNServer = domain;
              turnInfo->mTURNServerUsername = username;
              turnInfo->mTURNServerPassword = info->mCredential;
              turnServers.push_back(turnInfo);

              ZS_LOG_DETAIL(log("adding turn server") + turnInfo->toDebug())
            } else if ("stun" == type) {
              STUNServerInfoPtr stunInfo = STUNServerInfo::create();
              stunInfo->mSTUNServer = domain;
              stunServers.push_back(stunInfo);

              ZS_LOG_DETAIL(log("adding stun server") + stunInfo->toDebug())
            }
          }

        invalid_url:
          ZS_LOG_ERROR(Basic, log("server url specified is not valid") + ZS_PARAM("url", info->mURL))
        }
      }

      mSocket = IICESocket::create(getAssociatedMessageQueue(), mThisWeak.lock(), turnServers, stunServers);

      mLocal = TransportInfo::create();
      mLocal->mUsernameFrag = mSocket->getUsernameFrag();
      mLocal->mPassword = mSocket->getPassword();
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepSocket()
    {
      ZS_THROW_INVALID_ASSUMPTION_IF(!mSocket)

      WORD error = 0;
      String reason;
      IICESocket::ICESocketStates state = mSocket->getState(&error, &reason);

      switch (state) {
        case IICESocket::ICESocketState_Pending:
        case IICESocket::ICESocketState_Ready:
        case IICESocket::ICESocketState_GoingToSleep:
        case IICESocket::ICESocketState_Sleeping:     {
          ZS_LOG_TRACE(log("ice socket reporting state") + ZS_PARAM("state", IICESocket::toString(state)))
          break;
        }
        case IICESocket::ICESocketState_ShuttingDown:
        case IICESocket::ICESocketState_Shutdown:
        {
          ZS_LOG_ERROR(Detail, log("ice socket is shutting down") + ZS_PARAM("state", IICESocket::toString(state)) + ZS_PARAM("error", error) + ZS_PARAM("reason", reason))
          setError(0 != error ? error : IHTTP::HTTPStatusCode_ClientClosedRequest, reason);
          cancel();
          return false;
        }
      }

      String version = mSocket->getLocalCandidatesVersion();

      if (version == mCandidatesVersion) {
        ZS_LOG_TRACE(log("candidates have not changed") + ZS_PARAM("version", version))
        return true;
      }

      if (mNotifiedCandidatesEnd) {
        // must be a network change then...
        mSubscriptions.delegate()->onICETransportCandidatesChangeDetected(mThisWeak.lock());
        get(mNotifiedCandidatesEnd) = false;
      }

      CandidateListInner candidates;
      mSocket->getLocalCandidates(candidates);

      CandidateListInner added;
      CandidateListInner removed;

      IICESocket::compare(mLocalCandidatesInner, candidates, added, removed);

      for (CandidateListInner::iterator iter = added.begin(); iter != added.end(); ++iter)
      {
        bool didCreate = false;
        IICESocket::Candidate &candidate = (*iter);

        CandidateInfoPtr foundCandidate = findOrCreate(candidate, &didCreate);

        if (didCreate) {
          ZS_LOG_DEBUG(log("notifying about new candidate") + foundCandidate->toDebug())
          mSubscriptions.delegate()->onICETransportCandidate(mThisWeak.lock(), foundCandidate);
        }
      }

      for (CandidateListInner::iterator iter = removed.begin(); iter != removed.end(); ++iter)
      {
        IICESocket::Candidate &candidate = (*iter);

        CandidateInfoPtr foundCandidate = findOrCreate(candidate);
        for (CandidateListOuter::iterator outerIter = mLocalCandidatesOuter.begin(); outerIter != mLocalCandidatesOuter.end();)
        {
          CandidateListOuter::iterator outerCurrent = outerIter; ++outerIter;

          CandidateInfoPtr &checkCandidate = (*outerCurrent);
          if (checkCandidate != foundCandidate) continue;

          ZS_LOG_WARNING(Debug, log("candidate is no longer available (thus removing reference)") + foundCandidate->toDebug())
          mLocalCandidatesOuter.erase(outerCurrent);
          break;
        }
      }

      switch (state) {
        case IICESocket::ICESocketState_Ready:
        case IICESocket::ICESocketState_Sleeping:
        {
          if (!mNotifiedCandidatesEnd) {
            ZS_LOG_DEBUG(log("notifying end of candidates"))

            get(mNotifiedCandidatesEnd) = true;
            mSubscriptions.delegate()->onICETransportEndOfCandidates(mThisWeak.lock());
          }
          break;
        }
        default: break; // these are not "end of candidate" states
      }

      ZS_LOG_DEBUG(log("candidates update completed"))
      mLocalCandidatesInner = candidates;
      return true;
    }

    //-------------------------------------------------------------------------
    bool ICETransport::stepSession()
    {
      if (!mStartCalled) {
        ZS_LOG_TRACE(log("waiting for start to be called"))
        return false;
      }

      if (!mRemote) {
        ZS_LOG_TRACE(log("waiting for remote ice information"))
        return false;
      }

      bool changed = false;

      for (CandidateListOuter::iterator iter = mPendingRemoteCandidates.begin(); iter != mPendingRemoteCandidates.end(); ++iter)
      {
        CandidateInfoPtr &candidate = (*iter);
        ZS_THROW_INVALID_USAGE_IF(!candidate)

        bool alreadyFound = false;

        // make sure it's not already added
        for (CandidateListInner::iterator checkIter = mAddedRemoteCandidates.begin(); checkIter != mAddedRemoteCandidates.end(); ++checkIter)
        {
          IICESocket::Candidate &existingCandidate = (*checkIter);
          if (!isMatch(existingCandidate, *candidate)) continue;

          alreadyFound = true;
          break;
        }

        if (alreadyFound) {
          ZS_LOG_WARNING(Detail, log("this candidate was already known") + candidate->toDebug())
          continue;
        }

        IICESocket::CandidatePtr converted = createFrom(*candidate);
        mAddedRemoteCandidates.push_back(*converted);

        changed = true;
      }

      mPendingRemoteCandidates.clear();

      if (!mSession) {
        mSession = IICESocketSession::create(
                                             mThisWeak.lock(),
                                             mSocket,
                                             mRemote->mUsernameFrag,
                                             mRemote->mPassword,
                                             mAddedRemoteCandidates,
                                             internal::convert(mDefaultRole)
                                             );

        if (!mSession) {
          ZS_LOG_ERROR(Basic, log("unable to create ice session"))
          setError(IHTTP::HTTPStatusCode_BadRequest, "unable to create ice session from ice socket (unknown reason)");
          cancel();
          return false;
        }
        changed = false;
      }

      WORD error = 0;
      String reason;
      IICESocketSession::ICESocketSessionStates state = mSession->getState(&error, &reason);

      ZS_LOG_TRACE(log("ice socket session reporting state") + ZS_PARAM("state", IICESocketSession::toString(state)))

      switch (state) {
        case IICESocketSession::ICESocketSessionState_Pending:    break;
        case IICESocketSession::ICESocketSessionState_Prepared:   break;
        case IICESocketSession::ICESocketSessionState_Searching:  setState(IICETransport::ConnectionState_Searching); break;
        case IICESocketSession::ICESocketSessionState_Haulted:    setState(IICETransport::ConnectionState_Haulted); break;
        case IICESocketSession::ICESocketSessionState_Nominating: setState(IICETransport::ConnectionState_Searching); break;
        case IICESocketSession::ICESocketSessionState_Nominated:  setState(IICETransport::ConnectionState_Connected); break;
        case IICESocketSession::ICESocketSessionState_Completed:  setState(IICETransport::ConnectionState_Completed); break;
        case IICESocketSession::ICESocketSessionState_Shutdown:
        {
          ZS_LOG_ERROR(Detail, log("ice socket session is shutdown") + ZS_PARAM("state", IICESocketSession::toString(state)) + ZS_PARAM("error", error) + ZS_PARAM("reason", reason))
          setError(0 != error ? error : IHTTP::HTTPStatusCode_ClientClosedRequest, reason);
          cancel();
          return false;
        }
      }

      switch (state) {
        case IICESocketSession::ICESocketSessionState_Nominated:
        case IICESocketSession::ICESocketSessionState_Completed:
        {
          IICESocket::Candidate local;
          IICESocket::Candidate remote;
          bool result = mSession->getNominatedCandidateInformation(local, remote);

          if (result) {
            mSubscriptions.delegate()->onICETransportActiveCandidate(mThisWeak.lock(), createFrom(local), createFrom(remote));
            get(mNominationChanged) = false;
          }
        }
        default:  break;
      }

      if (changed) {
        mSession->updateRemoteCandidates(mAddedRemoteCandidates);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void ICETransport::cancel()
    {
      //.......................................................................
      // start the shutdown process

      if (mSocket) {
        mSocket->shutdown();
      }

      if (mSession) {
        mSession->close();
      }

      mPendingRemoteCandidates.clear();

      //.......................................................................
      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
        if (IICESocket::ICESocketState_Shutdown != mSocket->getState()) {
          ZS_LOG_WARNING(Detail, log("waiting for ICE socket to shutdown"))
          return;
        }
      }

      //.......................................................................
      // final cleanup

      if (mSocketSubscription) {
        mSocketSubscription->cancel();
        mSocketSubscription.reset();
      }

      if (mSessionSubscription) {
        mSessionSubscription->cancel();
        mSessionSubscription.reset();
      }

      mSocket.reset();
      mSession.reset();

      mDTLSTransport.reset();

      mAttachedDTLSTransportID = 0;
      setState(ConnectionState_Closed);

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void ICETransport::setState(ConnectionStates state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("old state", IICETransport::toString(mCurrentState)) + ZS_PARAM("new state", state))

      mCurrentState = state;

      ICETransportPtr pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onICETransportStateChanged(pThis, mCurrentState);
      }
    }

    //-----------------------------------------------------------------------
    void ICETransport::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = IHTTP::toString(IHTTP::toStatusCode(errorCode));
      }

      if (0 != mLastError) {
        ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
        return;
      }

      get(mLastError) = errorCode;
      mLastErrorReason = reason;

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-----------------------------------------------------------------------
    IICETransport::CandidateInfoPtr ICETransport::findOrCreate(
                                                               const IICESocket::Candidate &candidate,
                                                               bool *outDidCreate
                                                               )
    {
      if (outDidCreate) {
        *outDidCreate = false;
      }

      for (CandidateListOuter::iterator iter = mLocalCandidatesOuter.begin(); iter != mLocalCandidatesOuter.end(); ++iter)
      {
        CandidateInfoPtr &checkCandidate = (*iter);

        if (!isMatch(*checkCandidate, candidate)) continue;

        ZS_LOG_TRACE(log("found candidate match") + ZS_PARAM("check", checkCandidate->toDebug()) + ZS_PARAM("search", candidate.toDebug()))
        return checkCandidate;
      }

      CandidateInfoPtr result = createFrom(candidate);

      ZS_LOG_DEBUG(log("creating new candidate from existing") + result->toDebug() + candidate.toDebug())

      mLocalCandidatesOuter.push_back(result);

      return result;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IICETransportFactory &IICETransportFactory::singleton()
    {
      return ICETransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr IICETransportFactory::create(
                                                 IICETransportDelegatePtr delegate,
                                                 IICETransport::ServerListPtr servers
                                                 )
    {
      if (this) {}
      return internal::ICETransport::create(delegate, servers);
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICETransport::toString(ConnectionStates state)
  {
    switch (state) {
      case ConnectionState_New:       return "New";
      case ConnectionState_Searching: return "Searching";
      case ConnectionState_Haulted:   return "Haulted";
      case ConnectionState_Connected: return "Connected";
      case ConnectionState_Completed: return "Completed";
      case ConnectionState_Closed:    return "Closed";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  const char *IICETransport::toString(Roles role)
  {
    switch (role) {
      case Role_Controlling:  return "Controlling";
      case Role_Controlled:   return "Controlled";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  const char *IICETransport::toString(Options option)
  {
    switch (option) {
      case Option_Unknown:  return "Unknown";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  const char *IICETransport::toString(CandidateTypes type)
  {
    switch (type) {
      case CandidateType_Unknown:         return "Unknown";
      case CandidateType_Local:           return "Local";
      case CandidateType_ServerReflexive: return "Server Reflexive";
      case CandidateType_PeerReflexive:   return "Peer Reflexive";
      case CandidateType_Relayed:         return "Relayed";
    }
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::toDebug(IICETransportPtr transport)
  {
    return internal::ICETransport::toDebug(transport);
  }

  //---------------------------------------------------------------------------
  IICETransportPtr IICETransport::create(
                                         IICETransportDelegatePtr delegate,
                                         ServerListPtr servers
                                         )
  {
    return internal::IICETransportFactory::singleton().create(delegate, servers);
  }

  //---------------------------------------------------------------------------
  IICETransport::CapabilitiesPtr IICETransport::getCapabilities()
  {
    return internal::ICETransport::getCapabilities();
  }

  //---------------------------------------------------------------------------
  IICETransport::TransportInfoPtr IICETransport::filterParams(
                                                              TransportInfoPtr params,
                                                              CapabilitiesPtr capabilities
                                                              )
  {
    return internal::ICETransport::filterParams(params, capabilities);
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport::Capabilities
  #pragma mark

  //---------------------------------------------------------------------------
  IICETransport::CapabilitiesPtr IICETransport::Capabilities::create()
  {
    return CapabilitiesPtr(new Capabilities);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::Capabilities::toDebug() const
  {
    if (mOptions.size() < 1) return ElementPtr();

    ElementPtr resultEl = Element::create("IICETransport::Capabilities");

    for (OptionsList::const_iterator iter = mOptions.begin(); iter != mOptions.end(); ++iter)
    {
      const Options &option = (*iter);
      UseServicesHelper::debugAppend(resultEl, "option", IICETransport::toString(option));
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport::ServerInfo
  #pragma mark

  //---------------------------------------------------------------------------
  IICETransport::ServerInfoPtr IICETransport::ServerInfo::create()
  {
    return ServerInfoPtr(new ServerInfo);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::ServerInfo::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICETransport::ServerInfo");
    UseServicesHelper::debugAppend(resultEl, "url", mURL);
    UseServicesHelper::debugAppend(resultEl, "credential", mCredential);

    return resultEl->hasChildren() ? resultEl : ElementPtr();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport::ServerList
  #pragma mark

  //---------------------------------------------------------------------------
  IICETransport::ServerListPtr IICETransport::ServerList::create()
  {
    return ServerListPtr(new ServerList);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::ServerList::toDebug() const
  {
    if ((*this).size() < 1) return ElementPtr();

    ElementPtr resultEl = Element::create("ortc::IICETransport::ServerList");

    for (ServerList::const_iterator iter = (*this).begin(); iter != (*this).end(); ++iter)
    {
      const ServerInfoPtr &info = (*iter);
      UseServicesHelper::debugAppend(resultEl, info->toDebug());
    }

    return resultEl->hasChildren() ? resultEl : ElementPtr();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport::TransportInfo
  #pragma mark

  //---------------------------------------------------------------------------
  IICETransport::TransportInfoPtr IICETransport::TransportInfo::create()
  {
    return TransportInfoPtr(new TransportInfo);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::TransportInfo::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICETransport::TransportInfo");
    UseServicesHelper::debugAppend(resultEl, "usernameFrag", mUsernameFrag);
    UseServicesHelper::debugAppend(resultEl, "password", mPassword);

    return resultEl->hasChildren() ? resultEl : ElementPtr();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport::CandidateInfo
  #pragma mark

  //---------------------------------------------------------------------------
  IICETransport::CandidateInfoPtr IICETransport::CandidateInfo::create()
  {
    return CandidateInfoPtr(new CandidateInfo);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETransport::CandidateInfo::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICETransport::CandidateInfo");

    UseServicesHelper::debugAppend(resultEl, "type", IICETransport::toString(mType));
    UseServicesHelper::debugAppend(resultEl, "foundation", mFoundation);
    UseServicesHelper::debugAppend(resultEl, "component", mComponent);
    UseServicesHelper::debugAppend(resultEl, "connectionAddress", mConnectionAddress.isEmpty() ? String() : mConnectionAddress.string());
    UseServicesHelper::debugAppend(resultEl, "priority", mPriority);
    UseServicesHelper::debugAppend(resultEl, "relatedAddress", mRelatedAddress.isEmpty() ? String() : mRelatedAddress.string());

    return resultEl->hasChildren() ? resultEl : ElementPtr();
  }

}
