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

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/ISettings.h>
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
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);
  ZS_DECLARE_USING_PTR(zsLib, ISettings);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(DTMFSenderSettingsDefaults);

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
    #pragma mark DTMFSenderSettingsDefaults
    #pragma mark

    class DTMFSenderSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~DTMFSenderSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static DTMFSenderSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<DTMFSenderSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static DTMFSenderSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<DTMFSenderSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installDTMFSenderSettingsDefaults()
    {
      DTMFSenderSettingsDefaults::singleton();
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
        //DTMFSenderPtr pThis = mThisWeak.lock();

      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }
      
      return subscription;
    }

    //-------------------------------------------------------------------------
    bool DTMFSender::canInsertDTMF() const
    {
      UseRTPSenderPtr sender;

      {
        AutoRecursiveLock lock(*this);

        if (isClosed()) {
          ZS_LOG_WARNING(Trace, log("already closed"));
          return false;
        }

        sender = mRTPSender.lock();
      }

      if (!sender) {
        ZS_LOG_WARNING(Trace, log("sender is gone"));
        return false;
      }

      return sender->canInsertDTMF();
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
      UseRTPSenderPtr sender;

      {
        AutoRecursiveLock lock(*this);

        if (isClosed()) {
          ZS_LOG_WARNING(Trace, log("already closed"));
          return;
        }

        sender = mRTPSender.lock();
      }

      if (!sender) {
        ZS_LOG_WARNING(Trace, log("sender is gone"));
        return;
      }

      sender->insertDTMF(tones, duration, interToneGap);
    }

    //-------------------------------------------------------------------------
    IRTPSenderPtr DTMFSender::sender() const
    {
      return RTPSender::convert(mRTPSender.lock());
    }

    //-------------------------------------------------------------------------
    String DTMFSender::toneBuffer() const
    {
      UseRTPSenderPtr sender;

      {
        AutoRecursiveLock lock(*this);

        if (isClosed()) {
          ZS_LOG_WARNING(Trace, log("already closed"));
          return String();
        }

        sender = mRTPSender.lock();
      }

      if (!sender) {
        ZS_LOG_WARNING(Trace, log("sender is gone"));
        return String();
      }

      return sender->toneBuffer();
    }

    //-------------------------------------------------------------------------
    Milliseconds DTMFSender::duration() const
    {
      UseRTPSenderPtr sender;

      {
        AutoRecursiveLock lock(*this);

        if (isClosed()) {
          ZS_LOG_WARNING(Trace, log("already closed"));
          return Milliseconds();
        }

        sender = mRTPSender.lock();
      }

      if (!sender) {
        ZS_LOG_WARNING(Trace, log("sender is gone"));
        return Milliseconds();
      }

      return sender->duration();
    }

    //-------------------------------------------------------------------------
    Milliseconds DTMFSender::interToneGap() const
    {
      UseRTPSenderPtr sender;

      {
        AutoRecursiveLock lock(*this);

        if (isClosed()) {
          ZS_LOG_WARNING(Trace, log("already closed"));
          return Milliseconds();
        }

        sender = mRTPSender.lock();
      }

      if (!sender) {
        ZS_LOG_WARNING(Trace, log("sender is gone"));
        return Milliseconds();
      }

      return sender->duration();
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
    #pragma mark DTMFSender => IDTMFSenderDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DTMFSender::onDTMFSenderToneChanged(
                                             IDTMFSenderPtr sender,
                                             String tone
                                             )
    {
      ZS_LOG_DEBUG(log("notified of DTMF tone event") + ZS_PARAM("tone", tone));

      AutoRecursiveLock lock(*this);

      auto pThis = mThisWeak.lock();
      mSubscriptions.delegate()->onDTMFSenderToneChanged(pThis, tone);
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
      IHelper::debugAppend(objectEl, "id", mID);
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

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      IHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      IHelper::debugAppend(resultEl, "shutdown", mShutdown);

      IHelper::debugAppend(resultEl, "data transport", rtpSender ? rtpSender->getID() : 0);

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
      if (!stepSubscribeSender()) goto not_ready;
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
    bool DTMFSender::stepSubscribeSender()
    {
      if (mRTPSenderSubscription) {
        ZS_LOG_TRACE(log("already subscribed"));
        return true;
      }

      UseRTPSenderPtr sender = mRTPSender.lock();
      if (!sender) {
        ZS_LOG_WARNING(Debug, log("sender object gone"));
        cancel();
        return false;
      }

      mRTPSenderSubscription = sender->subscribe(mThisWeak.lock());
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
