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
#include <ortc/internal/ortc_ICEGatherer.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>


#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/XML.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

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
    #pragma mark IICETransportForICEGatherer
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForRTPTransport
    #pragma mark

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
                               IICETransportDelegatePtr originalDelegate
                               ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      ZS_LOG_BASIC(debug("created"))

      mDefaultSubscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate), queue);
    }

    //-------------------------------------------------------------------------
    void ICETransport::init()
    {
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
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
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(ForICEGathererPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::convert(ForRTPTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransport, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr ICETransport::getStats() const throw(InvalidStateError)
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
    #pragma mark ICETransport => IICETransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr ICETransport::toDebug(ICETransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr ICETransport::create(IICETransportDelegatePtr delegate)
    {
      ICETransportPtr pThis(new ICETransport(IORTCForInternal::queueORTC(), delegate));
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

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IICETransportSubscriptionPtr subscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));

      IICETransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        ICETransportPtr pThis = mThisWeak.lock();

        if (IICETransportTypes::State_New != mCurrentState) {
          delegate->onICETransportStateChanged(pThis, mCurrentState);
        }

        if (!isShutdown()) {
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IICEGathererPtr ICETransport::iceGatherer() const
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return IICEGathererPtr();
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::Roles ICETransport::role() const
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return IICETransportTypes::Role_Controlled;
    }

    //-------------------------------------------------------------------------
    IICETypes::Components ICETransport::component() const
    {
      return mComponent;
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::States ICETransport::state() const
    {
      AutoRecursiveLock lock(*this);
      return mCurrentState;
    }

    //-------------------------------------------------------------------------
    IICETypes::CandidateListPtr ICETransport::getRemoteCandidates() const
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return CandidateListPtr();
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::CandidatePairPtr ICETransport::getNominatedCandidatePair() const
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return CandidatePairPtr();
    }

    //-------------------------------------------------------------------------
    void ICETransport::start(
                             IICEGathererPtr gatherer,
                             Parameters remoteParameters,
                             Optional<Roles> role
                             ) throw (InvalidParameters)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::stop()
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    IICETransportTypes::ParametersPtr ICETransport::getRemoteParameters() const
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return ParametersPtr();
    }

    //-------------------------------------------------------------------------
    IICETransportPtr ICETransport::createAssociatedTransport() throw (InvalidStateError)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return IICETransportPtr();
    }

    //-------------------------------------------------------------------------
    void ICETransport::addRemoteCandidate(const GatherCandidate &remoteCandidate)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    void ICETransport::setRemoteCandidates(const CandidateList &remoteCandidates)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportForICEGatherer
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransport::notifyRouteAdded(
                                        PUID routeID,
                                        IICETypes::CandidatePtr localCandidate,
                                        const IPAddress &fromIP
                                        )
    {
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyRouteRemoved(PUID routeID)
    {
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyPacket(
                                    PUID routeID,
                                    STUNPacketPtr packet
                                    )
    {
    }

    //-------------------------------------------------------------------------
    void ICETransport::notifyPacket(
                                    PUID routeID,
                                    const BYTE *buffer,
                                    size_t bufferSizeInBytes
                                    )
    {
    }
    
    //-------------------------------------------------------------------------
    bool ICETransport::needsMoreCandidates() const
    {
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport => IICETransportForRTPTransport
    #pragma mark

    bool ICETransport::sendPacket(
                                  const BYTE *buffer,
                                  size_t bufferSizeInBytes
                                  )
    {
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

      AutoRecursiveLock lock(*this);
      step();
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

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

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
      return IICETransportTypes::State_Closed == mCurrentState;
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

    }

    //-------------------------------------------------------------------------
    void ICETransport::cancel()
    {
      //.......................................................................
      // start the shutdown process
      if (isShutdown()) {
        ZS_LOG_WARNING(Trace, log("already shutdown"))
        return;
      }

      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
      }

      //.......................................................................
      // final cleanup

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();

      setState(IICETransportTypes::State_Closed);
    }

    //-------------------------------------------------------------------------
    void ICETransport::setState(IICETransport::States state)
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
    #pragma mark IICETransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IICETransportFactory &IICETransportFactory::singleton()
    {
      return ICETransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr IICETransportFactory::create(IICETransportDelegatePtr delegate)
    {
      if (this) {}
      return internal::ICETransport::create(delegate);
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransport
  #pragma mark

}
