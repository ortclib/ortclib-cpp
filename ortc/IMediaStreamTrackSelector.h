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

#include <ortc/IMediaStreamTrack.h>

namespace ortc
{

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackSelectorTypes
  #pragma mark
  
  interaction IMediaStreamTrackSelectorTypes
  {
    typedef IMediaStreamTrackTypes::MediaChannelID MediaChannelID;
    typedef IMediaStreamTrackTypes::Kinds Kinds;
    ZS_DECLARE_TYPEDEF_PTR(std::list<IMediaStreamTrackPtr>, MediaStreamTrackList);
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackSelector
  #pragma mark
  
  interaction IMediaStreamTrackSelector : public Any,
                                          public IMediaStreamTrackSelectorTypes,
                                          public IStatsProvider
  {
    static IMediaStreamTrackSelectorPtr convert(AnyPtr any);

    static IMediaStreamTrackSelectorPtr create(
                                               Kinds kind,
                                               size_t totalOutputMediaStreamTracks = 1,
                                               IMediaStreamTrackSelectorDelegatePtr delegate = IMediaStreamTrackSelectorDelegatePtr()
                                               );

    virtual PUID getID() const = 0;

    virtual IMediaStreamTrackSelectorSubscriptionPtr subscribe(IMediaStreamTrackSelectorDelegatePtr delegate) = 0;

    virtual Kinds kind() const = 0;
    virtual IMediaStreamTrackPtr getOutputTrack(size_t index) const = 0;
    virtual MediaStreamTrackList getInputTracks() const = 0;

    virtual PromisePtr attachInputTrack(
                                        IMediaStreamTrackPtr inputTrack,
                                        IMediaStreamTrackPtr useThisTrackToDetermineMediaActivityState  // use this track to detect media activity
                                        ) throw (InvalidParameters);
    virtual void detachInputTrack(IMediaStreamTrackPtr track);

    virtual void stop() = 0;
  };

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackSelectorDelegate
  #pragma mark

  interaction IMediaStreamTrackSelectorDelegate
  {
    virtual ~IMediaStreamTrackSelectorDelegate() {}
  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackSelectorSubscription
  #pragma mark
  
  interaction IMediaStreamTrackSelectorSubscription
  {
    virtual PUID getID() const = 0;
    
    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaStreamTrackSelectorDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
//ZS_DECLARE_PROXY_METHOD_2(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
//ZS_DECLARE_PROXY_METHOD_1(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
//ZS_DECLARE_PROXY_METHOD_2(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaStreamTrackSelectorDelegate, ortc::IMediaStreamTrackSelectorSubscription)
//ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
//ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
//ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
//ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
//ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
