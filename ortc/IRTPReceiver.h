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

#include <list>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPReceiverTypes
  #pragma mark

  interaction IRTPReceiverTypes : public IRTPTypes
  {
    ZS_DECLARE_STRUCT_PTR(ContributingSource)

    ZS_DECLARE_TYPEDEF_PTR(std::list<ContributingSource>, ContributingSourceList)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverTypes::ContributingSource
    #pragma mark

    struct ContributingSource {
      Time      mTimestamp;
      SSRCType  mCSRC {};
      BYTE      mAudioLevel {};
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPReceiver
  #pragma mark

  interaction IRTPReceiver : public IRTPReceiverTypes,
                             public IStatsProvider
  {
    static ElementPtr toDebug(IRTPReceiverPtr receiver);

    static IRTPReceiverPtr create(
                                  IRTPReceiverDelegatePtr delegate,
                                  IDTLSTransportPtr transport,
                                  IDTLSTransportPtr rtcpTransport = IDTLSTransportPtr()
                                  );

    virtual PUID getID() const = 0;

    virtual IRTPReceiverSubscriptionPtr subscribe(IRTPReceiverDelegatePtr delegate) = 0;

    virtual IMediaStreamTrackPtr getTrack() const = 0;
    virtual IDTLSTransportPtr getTransport() const = 0;
    virtual IDTLSTransportPtr getRTCPTransport() const = 0;

    virtual void setTransport(
                              IDTLSTransportPtr transport,
                              IDTLSTransportPtr rtcpTransport = IDTLSTransportPtr()
                              ) = 0;

    virtual CapabilitiesPtr getCapabilities(const char *kind = NULL);

    virtual void receive(const Parameters &parameters) throw (InvalidParameters);
    virtual void stop() = 0;

    virtual ContributingSourceList getContributingSources() const = 0;

    virtual void requestSendCSRC(SSRCType csrc) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPReceiverDelegate
  #pragma mark

  interaction IRTPReceiverDelegate
  {
    typedef WORD ErrorCode;

    virtual void onRTPReceiverError(
                                    IRTPReceiverPtr sender,
                                    ErrorCode errorCode,
                                    String errorReason
                                    ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPReceiverSubscription
  #pragma mark

  interaction IRTPReceiverSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}


ZS_DECLARE_PROXY_BEGIN(ortc::IRTPReceiverDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPReceiverPtr, IRTPReceiverPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTPSenderDelegate::ErrorCode, ErrorCode)
ZS_DECLARE_PROXY_METHOD_3(onRTPReceiverError, IRTPReceiverPtr, ErrorCode, String)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IRTPReceiverDelegate, ortc::IRTPReceiverSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IRTPReceiverPtr, IRTPReceiverPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IRTPReceiverDelegate::ErrorCode, ErrorCode)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_3(onRTPReceiverError, IRTPReceiverPtr, ErrorCode, String)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
