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
#include <ortc/IStatsProvider.h>
#include <ortc/IMediaStreamTrack.h>

#include <list>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IRTPReceiverTypes
  //

  interaction IRTPReceiverTypes : public IRTPTypes
  {
    typedef IMediaStreamTrack::Kinds Kinds;

    ZS_DECLARE_STRUCT_PTR(ContributingSource);

    ZS_DECLARE_TYPEDEF_PTR(std::list<ContributingSource>, ContributingSourceList);

    //-------------------------------------------------------------------------
    //
    // IRTPReceiverTypes::ContributingSource
    //

    struct ContributingSource {
      Time           mTimestamp {};
      SSRCType       mCSRC {};
      BYTE           mAudioLevel {};
      Optional<bool> mVoiceActivityFlag {};

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IRTPReceiver
  //

  interaction IRTPReceiver : public IRTPReceiverTypes,
                             public IStatsProvider
  {
    static ElementPtr toDebug(IRTPReceiverPtr receiver) noexcept;

    static IRTPReceiverPtr create(
                                  IRTPReceiverDelegatePtr delegate,
                                  Kinds kind,
                                  IRTPTransportPtr transport,
                                  IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                                  ) noexcept;

    virtual PUID getID() const noexcept = 0;

    virtual IRTPReceiverSubscriptionPtr subscribe(IRTPReceiverDelegatePtr delegate) noexcept = 0;

    virtual IMediaStreamTrackPtr track() const noexcept = 0;
    virtual IRTPTransportPtr transport() const noexcept = 0;
    virtual IRTCPTransportPtr rtcpTransport() const noexcept = 0;

    virtual void setTransport(
                              IRTPTransportPtr transport,
                              IRTCPTransportPtr rtcpTransport = IRTCPTransportPtr()
                              ) noexcept(false) = 0; // throws InvalidParameters

    static CapabilitiesPtr getCapabilities(Optional<Kinds> kind = Optional<Kinds>()) noexcept;

    virtual PromisePtr receive(const Parameters &parameters) noexcept(false) = 0; // throws InvalidParameters
    virtual void stop() noexcept = 0;

    virtual ContributingSourceList getContributingSources() const noexcept = 0;

    virtual void requestSendCSRC(SSRCType csrc) noexcept(false) = 0; // throws NotImplemented
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IRTPReceiverDelegate
  //

  interaction IRTPReceiverDelegate
  {
    virtual ~IRTPReceiverDelegate() noexcept {}
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IRTPReceiverSubscription
  //

  interaction IRTPReceiverSubscription
  {
    virtual PUID getID() const noexcept = 0;

    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };
}


ZS_DECLARE_PROXY_BEGIN(ortc::IRTPReceiverDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPReceiverPtr, IRTPReceiverPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IRTPReceiverDelegate, ortc::IRTPReceiverSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IRTPReceiverPtr, IRTPReceiverPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
