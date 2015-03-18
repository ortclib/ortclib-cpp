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
#include <ortc/IICETypes.h>
#include <ortc/IStatsProvider.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererTypes
  #pragma mark
  
  interaction IICEGathererTypes : public IICETypes
  {
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererTypes::States
    #pragma mark

    enum States
    {
      State_New,
      State_Gathering,
      State_Complete,
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGatherer
  #pragma mark
  
  interaction IICEGatherer : public IICEGathererTypes,
                             public IStatsProvider
  {
    static ElementPtr toDebug(IICETransportPtr gatherer);

    static IICETransportPtr create(IICETransportDelegatePtr delegate);

    virtual PUID getID() const = 0;

    virtual IICEGathererSubscriptionPtr subscribe(IICEGathererDelegatePtr delegate) = 0;

    virtual States getState() const = 0;

    virtual ParametersPtr getLocalParameters() const = 0;
    virtual ParametersPtr getRemoteParameters() const = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererDelegate
  #pragma mark

  interaction IICEGathererDelegate
  {
    ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate)
    typedef WORD ErrorCode;

    virtual void onICEGathererStateChanged(
                                           IICEGathererPtr gatherer,
                                           IICEGatherer::States state
                                           ) = 0;

    virtual void onICEGathererLocalCandidate(
                                             IICEGathererPtr gatherer,
                                             CandidatePtr candidate
                                             ) = 0;
    virtual void onICEGathererError(
                                     IICEGathererPtr gatherer,
                                     ErrorCode errorCode,
                                     String errorReason
                                     ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICEGathererSubscription
  #pragma mark

  interaction IICEGathererSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IICEGathererDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGathererPtr, IICEGathererPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGatherer::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGathererDelegate::CandidatePtr, CandidatePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICEGathererDelegate::ErrorCode, ErrorCode)
ZS_DECLARE_PROXY_METHOD_2(onICEGathererStateChanged, IICEGathererPtr, States)
ZS_DECLARE_PROXY_METHOD_2(onICEGathererLocalCandidate, IICEGathererPtr, CandidatePtr)
ZS_DECLARE_PROXY_METHOD_3(onICEGathererError, IICEGathererPtr, ErrorCode, String)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IICEGathererDelegate, ortc::IICEGathererSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGathererPtr, IICEGathererPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGatherer::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGathererDelegate::CandidatePtr, CandidatePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IICEGathererDelegate::ErrorCode, ErrorCode)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICEGathererStateChanged, IICEGathererPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onICEGathererLocalCandidate, IICEGathererPtr, CandidatePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_3(onICEGathererError, IICEGathererPtr, ErrorCode, String)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
