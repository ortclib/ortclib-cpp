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
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

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
                                 IICETransportPtr iceTransport
                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mICETransport(ICETransport::convert(iceTransport))
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
        transport = mICETransport;

        mCertificateGenerator = transport->getAssociatedGenerator();
        if (!mCertificateGenerator) mCertificateGenerator = DTLSCertficateGenerator::create();

        mCertificateGeneratorPromise = mCertificateGenerator->getCertificate();
        mCertificateGeneratorPromise->thenWeak(mThisWeak.lock());

        mICETransportSubscription = transport->subscribe(mThisWeak.lock());
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
                                           IICETransportPtr iceTransport
                                           )
    {
      DTLSTransportPtr pThis(make_shared<DTLSTransport>(make_private {}, IORTCForInternal::queueORTC(), delegate, iceTransport));
      pThis->mThisWeak.lock();
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
    IDTLSTransportTypes::PromiseWithParametersPtr DTLSTransport::getLocalParameters() const
    {
      AutoRecursiveLock lock(*this);

      if ((isShutdown()) ||
          (isShuttingDown())) {
        ZS_LOG_WARNING(Detail, log("cannot get local parameters while shutdown (or shutting down)"))
        return PromiseWithParameters::createRejected(IORTCForInternal::queueDelegate());
      }

      auto pendingPromise = PromiseWithParameters::create(IORTCForInternal::queueDelegate());

      mPendingLocalParameters.push_back(pendingPromise);

      ZS_LOG_TRACE(log("will resolve get local paramters later"))

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      return pendingPromise;
    }

    //-------------------------------------------------------------------------
    IDTLSTransportTypes::ParametersPtr DTLSTransport::getRemoteParameters() const
    {
      AutoRecursiveLock lock(*this);
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return ParametersPtr();
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
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::stop()
    {
      AutoRecursiveLock lock(*this);
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
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
      ZS_LOG_TRACE(log("sending packet") + ZS_PARAM("length", bufferLengthInBytes))

      UseICETransportPtr transport;
      SecureByteBlockPtr packet;

      {
        AutoRecursiveLock lock(*this);
        if (!mICETransport) {
          ZS_LOG_WARNING(Debug, log("no ice transport is attached"))
          return false;
        }
        transport = mICETransport;

#define TODO_ENCRYPT_PACKET 1
#define TODO_ENCRYPT_PACKET 2
        packet = UseServicesHelper::convertToBuffer(buffer, bufferLengthInBytes);
      }

      // WARNING: Best to not send packet to ice transport inside an object lock
      return transport->sendPacket(*packet, packet->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport => IDTLSTransportForICETransport
    #pragma mark

    //-------------------------------------------------------------------------
    DTLSCertficateGeneratorPtr DTLSTransport::getCertificateGenerator() const
    {
      ZS_LOG_TRACE(log("get certificate generator"))
      AutoRecursiveLock lock(*this);
      return mCertificateGenerator;
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::handleReceivedPacket(
                                             IICETypes::Components viaComponent,
                                             const BYTE *buffer,
                                             size_t bufferLengthInBytes
                                             )
    {
      ZS_LOG_TRACE(log("handle receive packet") + ZS_PARAM("length", bufferLengthInBytes))

      // scope: pre-validation check
      {
        AutoRecursiveLock lock(*this);
#define TODO_DETERMINE_IF_DTLS_PACKET_VS_SRTP_PACKET_AND_HANDLE 1
#define TODO_DETERMINE_IF_DTLS_PACKET_VS_SRTP_PACKET_AND_HANDLE 2
      }

      // WARNING: Forward packet to data channel or RTP listener outside of object lock
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
#define TODO_WHERE_TO_FORWARD 1
#define TODO_WHERE_TO_FORWARD 2
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
    #pragma mark DTLSTransport => IICETransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTLSTransport::onICETransportStateChanged(
                                                   IICETransportPtr transport,
                                                   IICETransport::States state
                                                   )
    {
#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::onICETransportCandidatePairAvailable(
                                                             IICETransportPtr transport,
                                                             CandidatePairPtr candidatePair
                                                             )
    {
#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::onICETransportCandidatePairGone(
                                                        IICETransportPtr transport,
                                                        CandidatePairPtr candidatePair
                                                        )
    {
#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2
    }

    //-------------------------------------------------------------------------
    void DTLSTransport::onICETransportCandidatePairChanged(
                                                           IICETransportPtr transport,
                                                           CandidatePairPtr candidatePair
                                                           )
    {
#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2
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
      ElementPtr resultEl = Element::create("ortc::DTLSTransport");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", IDTLSTransport::toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "ice transport", mICETransport ? mICETransport->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "certificate generator", (bool)mCertificateGenerator);
      UseServicesHelper::debugAppend(resultEl, "certificate generator promise", (bool)mCertificateGeneratorPromise);

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
      if (!stepGetCertificate()) goto not_ready;
      if (!stepResolveLocalParameters()) goto not_ready;
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
        setState(State_Validated);
      }
    }

    //-------------------------------------------------------------------------
    bool DTLSTransport::stepGetCertificate()
    {
      if (mCertificate) {
        ZS_LOG_TRACE(log("already have certificate"))
        return true;
      }

      if (!mCertificateGeneratorPromise->isResolved()) {
        ZS_LOG_DEBUG(log("waiting for generator promise to resolve"))
        return false;
      }

      mCertificate = mCertificateGeneratorPromise->zsLib::Promise::value<CertificateHolder>();

      ZS_LOG_DEBUG(log("certicate generated"))
      return true;
    }

    bool DTLSTransport::stepResolveLocalParameters()
    {
      if (mPendingLocalParameters.size() < 1) {
        ZS_LOG_TRACE(log("no pending get local parameter promises to resolve"))
        return true;
      }

      for (auto iter = mPendingLocalParameters.begin(); iter != mPendingLocalParameters.end(); ++iter) {
        auto pendingPromise = (*iter);

        ParametersPtr param(make_shared<Parameters>());

#define TODO_FILL_IN_LOCAL_PARAM_VALUES 1
#define TODO_FILL_IN_LOCAL_PARAM_VALUES 2

        pendingPromise->resolve(param);
      }

      ZS_LOG_DEBUG(log("all pending promises are now resolve") + ZS_PARAM("total", mPendingLocalParameters.size()))

      mPendingLocalParameters.clear();

      return true;
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

      if (mPendingLocalParameters.size() > 0) {
        for (auto iter = mPendingLocalParameters.begin(); iter != mPendingLocalParameters.end(); ++iter) {
          auto pendingPromise = (*iter);

          pendingPromise->reject();
        }
        ZS_LOG_WARNING(Detail, log("failed get local parameter promise") + ZS_PARAM("total", mPendingLocalParameters.size()))
        mPendingLocalParameters.clear();
      }

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
                                                   IICETransportPtr iceTransport
                                                   )
    {
      if (this) {}
      return internal::DTLSTransport::create(delegate, iceTransport);
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
  #pragma mark IDTLSTransportTypes::Fingerprint
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IDTLSTransportTypes::Fingerprint::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IDTLSTransportTypes::Fingerprint");

    UseServicesHelper::debugAppend(resultEl, "algorithm", mAlgorithm);
    UseServicesHelper::debugAppend(resultEl, "value", mValue);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IDTLSTransportTypes::Fingerprint::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IDTLSTransportTypes:Fingerprint:");
    hasher.update(mAlgorithm);
    hasher.update(":");
    hasher.update(mValue);

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
                                           IICETransportPtr iceTransport
                                           )
  {
    return internal::IDTLSTransportFactory::singleton().create(delegate, iceTransport);
  }


}
