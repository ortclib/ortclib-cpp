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


namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_rtp_sender) }

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
    //
    // (helpers)
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // DTMFSenderSettingsDefaults
    //

    class DTMFSenderSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~DTMFSenderSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static DTMFSenderSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<DTMFSenderSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static DTMFSenderSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<DTMFSenderSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installDTMFSenderSettingsDefaults() noexcept
    {
      DTMFSenderSettingsDefaults::singleton();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IDTMFSenderForRTPSender
    //

    //-------------------------------------------------------------------------
    ElementPtr IDTMFSenderForRTPSender::toDebug(ForRTPSenderPtr transport) noexcept
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(DTMFSender, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // DTMFSender
    //
    
    //-------------------------------------------------------------------------
    DTMFSender::DTMFSender(
                           const make_private &,
                           IMessageQueuePtr queue,
                           IDTMFSenderDelegatePtr originalDelegate,
                           IRTPSenderPtr sender
                           ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSubscriptions(decltype(mSubscriptions)::create()),
      mRTPSender(RTPSender::convert(sender))
    {
      ZS_LOG_DETAIL(debug("created"))

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void DTMFSender::init() noexcept
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    DTMFSender::~DTMFSender() noexcept
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr DTMFSender::convert(IDTMFSenderPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(DTMFSender, object);
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr DTMFSender::convert(ForRTPSenderPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(DTMFSender, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // DTMFSender => IDTMFSender
    //
    
    //-------------------------------------------------------------------------
    ElementPtr DTMFSender::toDebug(DTMFSenderPtr transport) noexcept
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr DTMFSender::create(
                                     IDTMFSenderDelegatePtr delegate,
                                     IRTPSenderPtr sender
                                     ) noexcept
    {
      DTMFSenderPtr pThis(make_shared<DTMFSender>(make_private {}, IORTCForInternal::queueORTC(), delegate, sender));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IDTMFSenderSubscriptionPtr DTMFSender::subscribe(IDTMFSenderDelegatePtr originalDelegate) noexcept
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
    bool DTMFSender::canInsertDTMF() const noexcept
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
                                ) noexcept(false)
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
    IRTPSenderPtr DTMFSender::sender() const noexcept
    {
      return RTPSender::convert(mRTPSender.lock());
    }

    //-------------------------------------------------------------------------
    String DTMFSender::toneBuffer() const noexcept
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
    Milliseconds DTMFSender::duration() const noexcept
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
    Milliseconds DTMFSender::interToneGap() const noexcept
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
    //
    // DTMFSender => IDTMFSenderForRTPSender
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // DTMFSender => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void DTMFSender::onWake()
    {
      ZS_LOG_DEBUG(log("wake"));

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // DTMFSender => IDTMFSenderDelegate
    //

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
    //
    // DTMFSender => IDTMFSenderAsyncDelegate
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // DTMFSender => (internal)
    //

    //-------------------------------------------------------------------------
    Log::Params DTMFSender::log(const char *message) const noexcept
    {
      ElementPtr objectEl = Element::create("ortc::DTMFSender");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params DTMFSender::debug(const char *message) const noexcept
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr DTMFSender::toDebug() const noexcept
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
    bool DTMFSender::isShuttingDown() const noexcept
    {
      if (mGracefulShutdownReference) return true;
      return false;
    }

    //-------------------------------------------------------------------------
    bool DTMFSender::isShutdown() const noexcept
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }

    //-------------------------------------------------------------------------
    void DTMFSender::step() noexcept
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"));
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
    bool DTMFSender::stepSubscribeSender() noexcept
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
    void DTMFSender::cancel() noexcept
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#pragma ZS_BUILD_NOTE("TODO", "If anything is required to keep DTMF object gracefully alive during shutdown")
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
    //
    // IDTMFSenderFactory
    //

    //-------------------------------------------------------------------------
    IDTMFSenderFactory &IDTMFSenderFactory::singleton() noexcept
    {
      return DTMFSenderFactory::singleton();
    }

    //-------------------------------------------------------------------------
    DTMFSenderPtr IDTMFSenderFactory::create(
                                             IDTMFSenderDelegatePtr delegate,
                                             IRTPSenderPtr sender
                                             ) noexcept
    {
      if (this) {}
      return internal::DTMFSender::create(delegate, sender);
    }

  } // internal namespace


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IDTMFSenderTypes
  //

  //---------------------------------------------------------------------------
  ElementPtr IDTMFSender::toDebug(IDTMFSenderPtr transport) noexcept
  {
    return internal::DTMFSender::toDebug(internal::DTMFSender::convert(transport));
  }

  //---------------------------------------------------------------------------
  IDTMFSenderPtr IDTMFSender::create(
                                     IDTMFSenderDelegatePtr delegate,
                                     IRTPSenderPtr sender
                                     ) noexcept
  {
    return internal::IDTMFSenderFactory::singleton().create(delegate, sender);
  }

}
