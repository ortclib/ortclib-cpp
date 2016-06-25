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

#include <ortc/internal/ortc_DTMFSender.h>
#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtpsender) }

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
    #pragma mark IDTMFSenderForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IDTMFSenderForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTMFSenderForRTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IDTMFSenderForRTPSender::toDebug(ForRTPSenderPtr transport)
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(DTMFSender, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTMFSender
    #pragma mark
    
    //-------------------------------------------------------------------------
    DTMFSender::DTMFSender(
                           const make_private &,
                           IMessageQueuePtr queue,
                           IDTMFSenderDelegatePtr originalDelegate,
                           IRTPSenderPtr sender
                           ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mRTPSender(RTPSender::convert(sender))
    {
      ZS_LOG_DETAIL(debug("created"))

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void DTMFSender::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    DTMFSender::~DTMFSender()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr DTMFSender::convert(IDTMFSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTMFSender, object);
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr DTMFSender::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTMFSender, object);
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr DTMFSender::convert(ForRTPSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DTMFSender, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTMFSender => IDTMFSender
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr DTMFSender::toDebug(DTMFSenderPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr DTMFSender::create(
                                     IDTMFSenderDelegatePtr delegate,
                                     IRTPSenderPtr sender
                                     )
    {
      DTMFSenderPtr pThis(make_shared<DTMFSender>(make_private {}, IORTCForInternal::queueORTC(), delegate, sender));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IDTMFSenderSubscriptionPtr DTMFSender::subscribe(IDTMFSenderDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to dtmf sender"));

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IDTMFSenderSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IDTMFSenderDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        DTMFSenderPtr pThis = mThisWeak.lock();

        if (mCurrentTone.hasData()) {
          delegate->onDTMFSenderToneChanged(pThis, mCurrentTone);
        }
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }
      
      return subscription;
    }

    //-------------------------------------------------------------------------
    bool DTMFSender::canInsertDTMF() const
    {
#define TODO 1
#define TODO 2
      return false;
    }

    //-------------------------------------------------------------------------
    void DTMFSender::insertDTMF(
                                const char *tones,
                                Milliseconds duration,
                                Milliseconds interToneGap
                                ) throw (
                                         InvalidStateError,
                                         InvalidCharacterError
                                         )
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    IRTPSenderPtr DTMFSender::sender() const
    {
#define TODO 1
#define TODO 2
      return RTPSender::convert(mRTPSender.lock());
    }

    //-------------------------------------------------------------------------
    String DTMFSender::toneBuffer() const
    {
#define TODO 1
#define TODO 2
      return String();
    }

    //-------------------------------------------------------------------------
    Milliseconds DTMFSender::duration() const
    {
#define TODO 1
#define TODO 2
      return Milliseconds();
    }

    //-------------------------------------------------------------------------
    Milliseconds DTMFSender::interToneGap() const
    {
#define TODO 1
#define TODO 2
      return Milliseconds();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTMFSender => IDTMFSenderForRTPSender
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTMFSender => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTMFSender::onWake()
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
    #pragma mark DTMFSender => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTMFSender::onTimer(TimerPtr timer)
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
    #pragma mark DTMFSender => IDTMFSenderAsyncDelegate
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTMFSender => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params DTMFSender::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::DTMFSender");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params DTMFSender::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr DTMFSender::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::DTMFSender");

      auto rtpSender = mRTPSender.lock();

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "shutdown", mShutdown);

      UseServicesHelper::debugAppend(resultEl, "data transport", rtpSender ? rtpSender->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool DTMFSender::isShuttingDown() const
    {
      if (mGracefulShutdownReference) return true;
      return false;
    }

    //-------------------------------------------------------------------------
    bool DTMFSender::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }

    //-------------------------------------------------------------------------
    void DTMFSender::step()
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
        ZS_LOG_TRACE(debug("not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
      }
    }

    //-------------------------------------------------------------------------
    bool DTMFSender::stepBogusDoSomething()
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
    void DTMFSender::cancel()
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

      mShutdown = true;

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTMFSenderFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IDTMFSenderFactory &IDTMFSenderFactory::singleton()
    {
      return DTMFSenderFactory::singleton();
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr IDTMFSenderFactory::create(
                                             IDTMFSenderDelegatePtr delegate,
                                             IRTPSenderPtr sender
                                             )
    {
      if (this) {}
      return internal::DTMFSender::create(delegate, sender);
    }

  } // internal namespace


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDTMFSenderTypes
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IDTMFSender::toDebug(IDTMFSenderPtr transport)
  {
    return internal::DTMFSender::toDebug(internal::DTMFSender::convert(transport));
  }

  //---------------------------------------------------------------------------
  IDTMFSenderPtr IDTMFSender::create(
                                     IDTMFSenderDelegatePtr delegate,
                                     IRTPSenderPtr sender
                                     )
  {
    return internal::IDTMFSenderFactory::singleton().create(delegate, sender);
  }


}
