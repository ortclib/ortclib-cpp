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
#include <ortc/IMediaStreamTrack.h>
#include <ortc/IStatsProvider.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPSenderTypes
  #pragma mark
  
  interaction IRTPSenderTypes : public IRTPTypes
  {
    ZS_DECLARE_STRUCT_PTR(IncompatibleMediaStreamTrackError)

    typedef IMediaStreamTrack::Kinds Kinds;

    struct IncompatibleMediaStreamTrackError : public Any
    {
      static IncompatibleMediaStreamTrackErrorPtr convert(AnyPtr any);
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPSender
  #pragma mark

  interaction IRTPSender : public Any,
                           public IRTPSenderTypes,
                           public IStatsProvider
  {
    static ElementPtr toDebug(IRTPSenderPtr sender);

    static IRTPSenderPtr create(
                                IRTPSenderDelegatePtr delegate,
                                IMediaStreamTrackPtr track,
                                IRTPTransportPtr transport,
                                IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                );

    virtual PUID getID() const = 0;

    virtual IRTPSenderSubscriptionPtr subscribe(IRTPSenderDelegatePtr delegate) = 0;

    virtual IMediaStreamTrackPtr track() const = 0;
    virtual IRTPTransportPtr transport() const = 0;
    virtual IRTCPTransportPtr rtcpTransport() const = 0;

    virtual void setTransport(
                              IRTPTransportPtr transport,
                              IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                              ) = 0;

    virtual PromisePtr setTrack(IMediaStreamTrackPtr track) = 0;

    static CapabilitiesPtr getCapabilities(Optional<Kinds> kind = Optional<Kinds>());

    virtual PromisePtr send(const Parameters &parameters) = 0;
    virtual void stop() = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPSenderDelegate
  #pragma mark

  interaction IRTPSenderDelegate
  {
    ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::SSRCType, SSRCType)

    virtual void onRTPSenderSSRCConflict(
                                         IRTPSenderPtr sender,
                                         SSRCType ssrc
                                         ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPSenderSubscription
  #pragma mark

  interaction IRTPSenderSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}


ZS_DECLARE_PROXY_BEGIN(ortc::IRTPSenderDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPSenderPtr, IRTPSenderPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPSenderDelegate::SSRCType, SSRCType)
ZS_DECLARE_PROXY_METHOD_2(onRTPSenderSSRCConflict, IRTPSenderPtr, SSRCType)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IRTPSenderDelegate, ortc::IRTPSenderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IRTPSenderPtr, IRTPSenderPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IRTPSenderDelegate::SSRCType, SSRCType)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onRTPSenderSSRCConflict, IRTPSenderPtr, SSRCType)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
