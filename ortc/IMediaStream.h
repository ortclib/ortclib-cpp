/*
 
 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
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
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStream
  #pragma mark
  
  interaction IMediaStream
  {
    virtual String id() = 0;
    virtual MediaStreamTrackListPtr getAudioTracks() = 0;
    virtual MediaStreamTrackListPtr getVideoTracks() = 0;
    virtual IMediaStreamTrackPtr getTrackById(String trackId) = 0;
    virtual void addTrack(IMediaStreamTrackPtr track) = 0;
    virtual void removeTrack(IMediaStreamTrackPtr track) = 0;
    virtual IMediaStreamPtr clone() = 0;
    virtual bool inactive() = 0;
  };
  
  interaction IMediaStreamDelegate
  {
    virtual void onMediaStreamActive() = 0;
    virtual void onMediaStreamInactive() = 0;
    virtual void onMediaStreamAddTrack(IMediaStreamTrackPtr track) = 0;
    virtual void onMediaStreamRemoveTrack(IMediaStreamTrackPtr track) = 0;
  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamSubscription
  #pragma mark
  
  interaction IMediaStreamSubscription
  {
    virtual PUID getID() const = 0;
    
    virtual void cancel() = 0;
    
    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaStreamDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD_0(onMediaStreamActive)
ZS_DECLARE_PROXY_METHOD_0(onMediaStreamInactive)
ZS_DECLARE_PROXY_METHOD_1(onMediaStreamAddTrack, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD_1(onMediaStreamRemoveTrack, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaStreamDelegate, ortc::IMediaStreamSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaStreamActive)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaStreamInactive)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaStreamAddTrack, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaStreamRemoveTrack, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
