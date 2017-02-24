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

#include <ortc/IDTMFSender.h>

#include <zsLib/MessageQueueAssociator.h>

//#define ORTC_SETTING_SRTP_TRANSPORT_WARN_OF_KEY_LIFETIME_EXHAUGSTION_WHEN_REACH_PERCENTAGE_USSED "ortc/srtp/warm-key-lifetime-exhaustion-when-reach-percentage-used"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForDTMFSender);

    ZS_DECLARE_INTERACTION_PTR(IDTMFSenderForRTPSender);
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTMFSenderForRTPSender
    #pragma mark

    interaction IDTMFSenderForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTMFSenderForRTPSender, ForRTPSender)

      static ElementPtr toDebug(ForRTPSenderPtr transport);

      virtual PUID getID() const = 0;

    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTMFSender
    #pragma mark
    
    class DTMFSender : public Noop,
                       public MessageQueueAssociator,
                       public SharedRecursiveLock,
                       public IDTMFSender,
                       public IDTMFSenderForRTPSender,
                       public IWakeDelegate,
                       public IDTMFSenderDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IDTMFSender;
      friend interaction IDTMFSenderFactory;
      friend interaction IDTMFSenderForRTPSender;

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForDTMFSender, UseRTPSender)

    public:
      DTMFSender(
                 const make_private &,
                 IMessageQueuePtr queue,
                 IDTMFSenderDelegatePtr delegate,
                 IRTPSenderPtr sender
                 );

    protected:
      DTMFSender(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~DTMFSender();

      static DTMFSenderPtr convert(IDTMFSenderPtr object);
      static DTMFSenderPtr convert(ForRTPSenderPtr object);

    protected:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTMFSender => IDTMFSender
      #pragma mark

      static ElementPtr toDebug(DTMFSenderPtr transport);

      static DTMFSenderPtr create(
                                  IDTMFSenderDelegatePtr delegate,
                                  IRTPSenderPtr sender
                                  );

      virtual PUID getID() const override {return mID;}

      virtual IDTMFSenderSubscriptionPtr subscribe(IDTMFSenderDelegatePtr delegate) override;

      virtual bool canInsertDTMF() const override;

      virtual void insertDTMF(
                              const char *tones,
                              Milliseconds duration = Milliseconds(70),
                              Milliseconds interToneGap = Milliseconds(70)
                              ) throw (
                                       InvalidStateError,
                                       InvalidCharacterError
                                       ) override;

      virtual IRTPSenderPtr sender() const override;

      virtual String toneBuffer() const override;
      virtual Milliseconds duration() const override;
      virtual Milliseconds interToneGap() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTMFSender => IDTMFSenderForRTPSender
      #pragma mark

      // (duplicate) static ElementPtr toDebug(DTMFSenderPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTMFSender => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTMFSender => IDTMFSenderDelegate
      #pragma mark

      virtual void onDTMFSenderToneChanged(
                                           IDTMFSenderPtr sender,
                                           String tone
                                           ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTMFSender => IDTMFSenderAsyncDelegate
      #pragma mark


    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTMFSender => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isClosed() const { return isShuttingDown() || isShutdown(); }
      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();

      bool stepSubscribeSender();

      void cancel();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTMFSender => (data)
      #pragma mark

      AutoPUID mID;
      DTMFSenderWeakPtr mThisWeak;
      DTMFSenderPtr mGracefulShutdownReference;

      IDTMFSenderDelegateSubscriptions mSubscriptions;
      IDTMFSenderSubscriptionPtr mDefaultSubscription;

      UseRTPSenderWeakPtr mRTPSender;
      IDTMFSenderSubscriptionPtr mRTPSenderSubscription;

      bool mShutdown {false};
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTMFSenderFactory
    #pragma mark

    interaction IDTMFSenderFactory
    {
      static IDTMFSenderFactory &singleton();

      virtual DTMFSenderPtr create(
                                   IDTMFSenderDelegatePtr delegate,
                                   IRTPSenderPtr sender
                                   );
    };

    class DTMFSenderFactory : public IFactory<IDTMFSenderFactory> {};
  }
}
