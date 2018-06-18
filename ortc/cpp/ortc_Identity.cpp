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

#include <ortc/internal/ortc_Identity.h>
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


namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);


  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(IdentitySettingsDefaults);

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
    // IdentitySettingsDefaults
    //

    class IdentitySettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~IdentitySettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static IdentitySettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<IdentitySettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static IdentitySettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<IdentitySettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() noexcept override
      {
        //      ISettings::setUInt(ORTC_SETTING__IDENTITY_, 0);
      }
      
    };

    //-------------------------------------------------------------------------
    void installIdentitySettingsDefaults() noexcept
    {
      IdentitySettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Identity
    //
    
    //-------------------------------------------------------------------------
    Identity::Identity(
                       const make_private &,
                       IMessageQueuePtr queue,
                       IDTLSTransportPtr transport
                       ) noexcept :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      ZS_LOG_DETAIL(debug("created"));

      ZS_ASSERT_FAIL("identity API in specification is not ready");
    }

    //-------------------------------------------------------------------------
    void Identity::init() noexcept
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    Identity::~Identity() noexcept
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"));
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    IdentityPtr Identity::convert(IIdentityPtr object) noexcept
    {
      return ZS_DYNAMIC_PTR_CAST(Identity, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Identity => IIdentity
    //
    
    //-------------------------------------------------------------------------
    ElementPtr Identity::toDebug(IdentityPtr object) noexcept
    {
      if (!object) return ElementPtr();
      return object->toDebug();
    }

    //-------------------------------------------------------------------------
    IdentityPtr Identity::create(IDTLSTransportPtr transport) noexcept
    {
      IdentityPtr pThis(make_shared<Identity>(make_private {}, IORTCForInternal::queueORTC(), transport));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IIdentityTypes::AssertionPtr Identity::peerIdentity() const noexcept
    {
      ZS_ASSERT_FAIL("identity API in specification is not ready");
      return AssertionPtr();
    }

    //-------------------------------------------------------------------------
    IDTLSTransportPtr Identity::transport() const noexcept
    {
      ZS_ASSERT_FAIL("identity API in specification is not ready");
      return IDTLSTransportPtr();
    }

    //-------------------------------------------------------------------------
    IIdentityTypes::PromiseWithResultPtr Identity::getIdentityAssertion(
                                                                        ZS_MAYBE_USED() const char *provider,
                                                                        ZS_MAYBE_USED() const char *protoocol,
                                                                        ZS_MAYBE_USED() const char *username
                                                                        ) noexcept(false)
    {
      ZS_MAYBE_USED(provider);
      ZS_MAYBE_USED(protoocol);
      ZS_MAYBE_USED(username);
      ZS_ASSERT_FAIL("identity API in specification is not ready")
      return PromiseWithResultPtr();
    }

    //-------------------------------------------------------------------------
    IIdentityTypes::PromiseWithAssertionPtr Identity::setIdentityAssertion(ZS_MAYBE_USED() const String &assertion) noexcept
    {
      ZS_MAYBE_USED(assertion);
      ZS_ASSERT_FAIL("identity API in specification is not ready")
      return PromiseWithAssertionPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Identity => IIdentityForRTPSender
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Identity => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void Identity::onWake()
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
    // Identity => ITimerDelegate
    //

    //-------------------------------------------------------------------------
    void Identity::onTimer(ITimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()));

      AutoRecursiveLock lock(*this);
#pragma ZS_BUILD_NOTE("NEEDED?","Is a timer needed?")
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Identity => IIdentityAsyncDelegate
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // Identity => (internal)
    //

    //-------------------------------------------------------------------------
    Log::Params Identity::log(const char *message) const noexcept
    {
      ElementPtr objectEl = Element::create("ortc::Identity");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params Identity::debug(const char *message) const noexcept
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr Identity::toDebug() const noexcept
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::Identity");

      IHelper::debugAppend(resultEl, "id", mID);

      IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      IHelper::debugAppend(resultEl, "shutdown", mShutdown);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool Identity::isShuttingDown() const noexcept
    {
      if (mGracefulShutdownReference) return true;
      return false;
    }

    //-------------------------------------------------------------------------
    bool Identity::isShutdown() const noexcept
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }

    //-------------------------------------------------------------------------
    void Identity::step() noexcept
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"));
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepBogusDoSomething()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("not ready"));
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"));
      }
    }

    //-------------------------------------------------------------------------
    bool Identity::stepBogusDoSomething() noexcept
    {
      if ( /* step already done */ false ) {
        ZS_LOG_TRACE(log("already completed do something"));
        return true;
      }

      if ( /* cannot do step yet */ false) {
        ZS_LOG_DEBUG(log("waiting for XYZ to complete before continuing"));
        return false;
      }

      ZS_LOG_DEBUG(log("doing step XYZ"));

      // ....
#pragma ZS_BUILD_NOTE("TODO","Do something when identity is a real thing")
      
      return true;
    }

    //-------------------------------------------------------------------------
    void Identity::cancel() noexcept
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#pragma ZS_BUILD_NOTE("NEEDED?","Keep object alive if needed during graceful shutdown")

        // grace shutdown process done here

        return;
      }

      //.......................................................................
      // final cleanup

      mShutdown = true;

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IIdentityFactory
    //

    //-------------------------------------------------------------------------
    IIdentityFactory &IIdentityFactory::singleton() noexcept
    {
      return IdentityFactory::singleton();
    }

    //-------------------------------------------------------------------------
    IdentityPtr IIdentityFactory::create(IDTLSTransportPtr transport) noexcept
    {
      if (this) {}
      return internal::Identity::create(transport);
    }

  } // internal namespace

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IIdentityTypes::Result
  //

  //---------------------------------------------------------------------------
  IIdentityTypes::ResultPtr IIdentityTypes::Result::convert(AnyPtr any) noexcept
  {
    return ZS_DYNAMIC_PTR_CAST(Result, any);
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IIdentityTypes::Result
  //

  //---------------------------------------------------------------------------
  IIdentityTypes::ErrorPtr IIdentityTypes::Error::convert(AnyPtr any) noexcept
  {
    return ZS_DYNAMIC_PTR_CAST(Error, any);
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IIdentityTypes
  //

  //---------------------------------------------------------------------------
  ElementPtr IIdentity::toDebug(IIdentityPtr transport) noexcept
  {
    return internal::Identity::toDebug(internal::Identity::convert(transport));
  }

  //---------------------------------------------------------------------------
  IIdentityPtr IIdentity::create(IDTLSTransportPtr transport) noexcept
  {
    return internal::IIdentityFactory::singleton().create(transport);
  }


}
