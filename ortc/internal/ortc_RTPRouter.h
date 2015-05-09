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

#pragma once

#include <ortc/internal/types.h>

#include <ortc/IICETransport.h>

#include <ortc/internal/ortc_IRTPRouter.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportForRTPTransport)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPRouter
    #pragma mark
    
    class RTPRouter : public Noop,
                      public MessageQueueAssociator,
                      public SharedRecursiveLock,
                      public IRTPRouter,
                      public IWakeDelegate
    {
    public:
      friend interaction IRTPRouter;
      friend interaction IRTPRouterFactory;

      enum State
      {
      };

    protected:
      RTPRouter(
                IMessageQueuePtr queue,
                IRTPRouterDelegatePtr delegate
                );

      RTPRouter(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPRouter();

      static RTPRouterPtr convert(IRTPRouterPtr object);

    protected:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPRouter => IRTPRouter
      #pragma mark

      static ElementPtr toDebug(RTPRouterPtr transport);

      static RTPRouterPtr create(
                                 IRTPRouterDelegatePtr delegate
                                 );

      virtual PUID getID() const;

      virtual IRTPRouterSubscriptionPtr subscribe(IRTPRouterDelegatePtr delegate);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPRouter => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPRouter => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();

      void cancel();

      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPRouter => (data)
      #pragma mark

      AutoPUID mID;
      RTPRouterWeakPtr mThisWeak;
      RTPRouterPtr mGracefulShutdownReference;

      IRTPRouterDelegateSubscriptions mSubscriptions;
      IRTPRouterSubscriptionPtr mDefaultSubscription;

      WORD mLastError {};
      String mLastErrorReason;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPRouterFactory
    #pragma mark

    interaction IRTPRouterFactory
    {
      static IRTPRouterFactory &singleton();

      virtual RTPRouterPtr create(
                                  IRTPRouterDelegatePtr delegate
                                  );
    };

    class RTPRouterFactory : public IFactory<IRTPRouterFactory> {};
  }
}
