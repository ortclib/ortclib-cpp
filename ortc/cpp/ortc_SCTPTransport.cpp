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
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <usrsctp/usrsctp.h>

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
    #pragma mark ISCTPTransportForDTLSTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ISCTPTransportForDTLSTransport::toDebug(ForDTLSTransportPtr transport)
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
        case State_ShuttingDown:  return "shutting down";
        case State_Shutdown:      return "shutdown";
      }
      return "UNDEFINED";
    }
    
    //-------------------------------------------------------------------------
    SCTPTransport::SCTPTransport(
                                 const make_private &,
                                 IMessageQueuePtr queue,
                                 ISCTPTransportDelegatePtr originalDelegate,
                                 IDTLSTransportPtr dtlsTransport
                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mDTLSTransport(DTLSTransport::convert(dtlsTransport))
    {
      ZS_LOG_DETAIL(debug("created"))

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      recv_thread_init();
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
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SCTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SCTPTransportPtr SCTPTransport::convert(ForDTLSTransportPtr object)
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
    SCTPTransportPtr SCTPTransport::create(
                                           ISCTPTransportDelegatePtr delegate,
                                           IDTLSTransportPtr transport
                                           )
    {
      SCTPTransportPtr pThis(make_shared<SCTPTransport>(make_private {}, IORTCForInternal::queueORTC(), delegate, transport));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
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
      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
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
      if (!originalDelegate) return mDefaultSubscription;

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
    #pragma mark SCTPTransport => ISCTPTransportForDTLSTransport
    #pragma mark

    //-------------------------------------------------------------------------
    bool SCTPTransport::handleDataPacket(
                                         const BYTE *buffer,
                                         size_t bufferLengthInBytes
                                         )
    {
#define TODO 1
#define TODO 2
      return false;
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
    void SCTPTransport::onDTLSTransportStateChanged(
                                                    IDTLSTransportPtr transport,
                                                    IDTLSTransport::States state
                                                    )
    {
      ZS_LOG_DEBUG(log("dtls transport state changed") + ZS_PARAM("dtls transport id", transport->getID()) + ZS_PARAM("state", IDTLSTransport::toString(state)))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    void SCTPTransport::onDTLSTransportError(
                                             IDTLSTransportPtr transport,
                                             ErrorCode errorCode,
                                             String errorReason
                                             )
    {
      ZS_LOG_DEBUG(log("dtls transport state changed") + ZS_PARAM("dtls transport id", transport->getID()) + ZS_PARAM("error", errorCode) + ZS_PARAM("reason", errorReason))

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
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "dtls transport", mDTLSTransport ? mDTLSTransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "dtls transport subscription", (bool)mDTLSTransportSubscription);

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
      if (!stepBogusDoSomething()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("dtls is not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
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

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      if (mDTLSTransportSubscription) {
        mDTLSTransportSubscription->cancel();
        mDTLSTransportSubscription.reset();
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
    SCTPTransportPtr ISCTPTransportFactory::create(
                                                   ISCTPTransportDelegatePtr delegate,
                                                   IDTLSTransportPtr transport
                                                   )
    {
      if (this) {}
      return internal::SCTPTransport::create(delegate, transport);
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
    hasher.update(string(mMaxMessageSize));
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
