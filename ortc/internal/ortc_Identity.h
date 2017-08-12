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

#pragma once

#include <ortc/internal/types.h>

#include <ortc/IIdentity.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>

//#define ORTC_SETTING_SRTP_TRANSPORT_WARN_OF_KEY_LIFETIME_EXHAUGSTION_WHEN_REACH_PERCENTAGE_USSED "ortc/srtp/warm-key-lifetime-exhaustion-when-reach-percentage-used"

namespace ortc
{
  namespace internal
  {    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Identity
    #pragma mark
    
    class Identity : public Noop,
                     public MessageQueueAssociator,
                     public SharedRecursiveLock,
                     public IIdentity,
                     public IWakeDelegate,
                     public zsLib::ITimerDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IIdentity;
      friend interaction IIdentityFactory;

    public:
      Identity(
               const make_private &,
               IMessageQueuePtr queue,
               IDTLSTransportPtr transport
               );

    protected:
      Identity(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~Identity();

      static IdentityPtr convert(IIdentityPtr object);

    protected:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Identity => IIdentity
      #pragma mark

      static ElementPtr toDebug(IdentityPtr object);

      static IdentityPtr create(IDTLSTransportPtr transport);

      virtual PUID getID() const override {return mID;}

      virtual AssertionPtr peerIdentity() const override;

      virtual IDTLSTransportPtr transport() const override;

      virtual PromiseWithResultPtr getIdentityAssertion(
                                                        const char *provider,
                                                        const char *protoocol = "default",
                                                        const char *username = NULL
                                                        ) throw (InvalidStateError) override;

      virtual PromiseWithAssertionPtr setIdentityAssertion(const String &assertion) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Identity => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Identity => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Identity => IIdentityAsyncDelegate
      #pragma mark


    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Identity => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();

      bool stepBogusDoSomething();

      void cancel();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Identity => (data)
      #pragma mark

      AutoPUID mID;
      IdentityWeakPtr mThisWeak;
      IdentityPtr mGracefulShutdownReference;

      bool mShutdown {false};
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IIdentityFactory
    #pragma mark

    interaction IIdentityFactory
    {
      static IIdentityFactory &singleton();

      virtual IdentityPtr create(IDTLSTransportPtr transport);
    };

    class IdentityFactory : public IFactory<IIdentityFactory> {};
  }
}
