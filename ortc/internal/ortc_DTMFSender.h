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
    //
    // IDTMFSenderForRTPSender
    //

    interaction IDTMFSenderForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTMFSenderForRTPSender, ForRTPSender);

      static ElementPtr toDebug(ForRTPSenderPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // DTMFSender
    //
    
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

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForDTMFSender, UseRTPSender);

    public:
      DTMFSender(
                 const make_private &,
                 IMessageQueuePtr queue,
                 IDTMFSenderDelegatePtr delegate,
                 IRTPSenderPtr sender
                 ) noexcept;

    protected:
      DTMFSender(Noop) noexcept :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

    public:
      virtual ~DTMFSender() noexcept;

      static DTMFSenderPtr convert(IDTMFSenderPtr object) noexcept;
      static DTMFSenderPtr convert(ForRTPSenderPtr object) noexcept;

    protected:

      //-----------------------------------------------------------------------
      //
      // DTMFSender => IDTMFSender
      //

      static ElementPtr toDebug(DTMFSenderPtr transport) noexcept;

      static DTMFSenderPtr create(
                                  IDTMFSenderDelegatePtr delegate,
                                  IRTPSenderPtr sender
                                  ) noexcept;

      PUID getID() const noexcept override {return mID;}

      IDTMFSenderSubscriptionPtr subscribe(IDTMFSenderDelegatePtr delegate) noexcept override;

      bool canInsertDTMF() const noexcept override;

      void insertDTMF(
                      const char *tones,
                      Milliseconds duration = Milliseconds(70),
                      Milliseconds interToneGap = Milliseconds(70)
                      ) noexcept(false) override; // throws InvalidStateError, InvalidCharacterError

      IRTPSenderPtr sender() const noexcept override;

      String toneBuffer() const noexcept override;
      Milliseconds duration() const noexcept override;
      Milliseconds interToneGap() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // DTMFSender => IDTMFSenderForRTPSender
      //

      // (duplicate) static ElementPtr toDebug(DTMFSenderPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      //
      // DTMFSender => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // DTMFSender => IDTMFSenderDelegate
      //

      void onDTMFSenderToneChanged(
                                   IDTMFSenderPtr sender,
                                   String tone
                                   ) override;

      //-----------------------------------------------------------------------
      //
      // DTMFSender => IDTMFSenderAsyncDelegate
      //


    protected:
      //-----------------------------------------------------------------------
      //
      // DTMFSender => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      static Log::Params slog(const char *message) noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isClosed() const noexcept { return isShuttingDown() || isShutdown(); }
      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;

      bool stepSubscribeSender() noexcept;

      void cancel() noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // DTMFSender => (data)
      //

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
    //
    // IDTMFSenderFactory
    //

    interaction IDTMFSenderFactory
    {
      static IDTMFSenderFactory &singleton() noexcept;

      virtual DTMFSenderPtr create(
                                   IDTMFSenderDelegatePtr delegate,
                                   IRTPSenderPtr sender
                                   ) noexcept;
    };

    class DTMFSenderFactory : public IFactory<IDTMFSenderFactory> {};
  }
}
