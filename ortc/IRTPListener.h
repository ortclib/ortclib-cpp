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
#include <ortc/IRTPTypes.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPListenerTypes
  #pragma mark
  
  interaction IRTPListenerTypes : public IRTPTypes
  {
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPListener
  #pragma mark
  
  interaction IRTPListener : public IRTPListenerTypes
  {
    static ElementPtr toDebug(IRTPListenerPtr listener);

    static IRTPListenerPtr create(
                                  IRTPListenerDelegatePtr delegate,
                                  IDTLSTransportPtr transport
                                  );

    virtual PUID getID() const = 0;

    virtual IRTPListenerSubscriptionPtr subscribe(IRTPListenerDelegatePtr delegate) = 0;

    virtual IDTLSTransportPtr getTransport() const = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPListenerDelegate
  #pragma mark

  interaction IRTPListenerDelegate
  {
    typedef IRTPTypes::SSRCType SSRCType;
    typedef IRTPTypes::PayloadType PayloadType;

    virtual void onRTPListenerUnhandledRTP(
                                           IRTPListenerPtr listener,
                                           SSRCType ssrc,
                                           PayloadType payloadType,
                                           String mid
                                           ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPListenerSubscription
  #pragma mark

  interaction IRTPListenerSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IRTPListenerDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPListenerPtr, IRTPListenerPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPListenerDelegate::SSRCType, SSRCType)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPListenerDelegate::PayloadType, PayloadType)
ZS_DECLARE_PROXY_METHOD_4(onRTPListenerUnhandledRTP, IRTPListenerPtr, SSRCType, PayloadType, String)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IRTPListenerDelegate, ortc::IRTPListenerSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IRTPListenerPtr, IRTPListenerPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IRTPListenerDelegate::SSRCType, SSRCType)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IRTPListenerDelegate::PayloadType, PayloadType)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_4(onRTPListenerUnhandledRTP, IRTPListenerPtr, SSRCType, PayloadType, String)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
