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

#include <ortc/types.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IDTMFSenderTypes
  //
  
  interaction IDTMFSenderTypes
  {
    ZS_DECLARE_CUSTOM_EXCEPTION(InvalidCharacterError);
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IDTMFSender
  //
  
  interaction IDTMFSender : public IDTMFSenderTypes
  {
    static ElementPtr toDebug(IDTMFSenderPtr sender) noexcept;

    static IDTMFSenderPtr create(
                                 IDTMFSenderDelegatePtr delegate,
                                 IRTPSenderPtr sender
                                 ) noexcept;

    virtual PUID getID() const noexcept = 0;

    virtual IDTMFSenderSubscriptionPtr subscribe(IDTMFSenderDelegatePtr delegate) noexcept = 0;

    virtual bool canInsertDTMF() const noexcept = 0;

    virtual void insertDTMF(
                            const char *tones,
                            Milliseconds duration = Milliseconds(70),
                            Milliseconds interToneGap = Milliseconds(70)
                            ) noexcept(false) = 0; //throws InvalidStateError, InvalidCharacterError

    virtual IRTPSenderPtr sender() const noexcept = 0;

    virtual String toneBuffer() const noexcept = 0;
    virtual Milliseconds duration() const noexcept = 0;
    virtual Milliseconds interToneGap() const noexcept = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IDTMFSenderDelegate
  //

  interaction IDTMFSenderDelegate
  {
    virtual void onDTMFSenderToneChanged(
                                         IDTMFSenderPtr sender,
                                         String tone
                                         ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IDTMFSenderSubscription
  //

  interaction IDTMFSenderSubscription
  {
    virtual PUID getID() const noexcept = 0;

    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };
}


ZS_DECLARE_PROXY_BEGIN(ortc::IDTMFSenderDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDTMFSenderPtr, IDTMFSenderPtr)
ZS_DECLARE_PROXY_METHOD(onDTMFSenderToneChanged, IDTMFSenderPtr, String)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IDTMFSenderDelegate, ortc::IDTMFSenderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDTMFSenderPtr, IDTMFSenderPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onDTMFSenderToneChanged, IDTMFSenderPtr, String)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
