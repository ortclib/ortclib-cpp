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
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrack
  #pragma mark
  
  interaction IMediaStreamTrack
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrackStates
    #pragma mark
    
    enum MediaStreamTrackStates
    {
      MediaStreamTrackState_New,
      MediaStreamTrackState_Live,
      MediaStreamTrackState_Ended
    };
    
    virtual String kind() = 0;
    virtual String id() = 0;
    virtual String label() = 0;
    virtual bool enabled() = 0;
    virtual bool muted() = 0;
    virtual bool readonly() = 0;
    virtual bool remote() = 0;
    virtual MediaStreamTrackStates readyState() = 0;
    virtual IMediaStreamTrackPtr clone() = 0;
    virtual void stop() = 0;
  };
  
  interaction IMediaStreamTrackDelegate
  {
    virtual void onMediaStreamTrackMute() = 0;
    virtual void onMediaStreamTrackUnmute() = 0;
    virtual void onMediaStreamTrackStarted() = 0;
    virtual void onMediaStreamTrackEnded() = 0;
    
    virtual void onMediaStreamFaceDetected() = 0;
    virtual void onMediaStreamVideoCaptureRecordStopped() = 0;
  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackSubscription
  #pragma mark
  
  interaction IMediaStreamTrackSubscription
  {
    virtual PUID getID() const = 0;
    
    virtual void cancel() = 0;
    
    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaStreamTrackDelegate)
ZS_DECLARE_PROXY_METHOD_0(onMediaStreamTrackMute)
ZS_DECLARE_PROXY_METHOD_0(onMediaStreamTrackUnmute)
ZS_DECLARE_PROXY_METHOD_0(onMediaStreamTrackStarted)
ZS_DECLARE_PROXY_METHOD_0(onMediaStreamTrackEnded)
ZS_DECLARE_PROXY_METHOD_0(onMediaStreamFaceDetected)
ZS_DECLARE_PROXY_METHOD_0(onMediaStreamVideoCaptureRecordStopped)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaStreamTrackDelegate, ortc::IMediaStreamTrackSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaStreamTrackMute)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaStreamTrackUnmute)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaStreamTrackStarted)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaStreamTrackEnded)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaStreamFaceDetected)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_0(onMediaStreamVideoCaptureRecordStopped)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
