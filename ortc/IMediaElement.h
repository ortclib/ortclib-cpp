/*
 
 Copyright (c) 2017, Optical Tone Ltd.
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
  ZS_DECLARE_INTERACTION_PTR(IWindowSink);
  ZS_DECLARE_INTERACTION_PTR(IMediaElementTypes);

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IWindowSink
  #pragma mark

  interaction IWindowSink
  {
#ifdef _WIN32
#ifdef __cplusplus_winrt
    static IMediaElementPtr create();
#else
    static IMediaElementPtr create(HWND hwnd);
#endif // __cplusplus_winrt
#else
    static IMediaElementPtr create(void *window);
#endif // _WIN32

    virtual PUID getID() const = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaElementTypes
  #pragma mark
  
  interaction IMediaElementTypes
  {
    typedef std::list<IMediaStreamTrackPtr> MediaStreamTrackList;
    typedef IMediaStreamTrackTypes::Kinds Kinds;
    typedef IMediaStreamTrackTypes::States States;

    struct MediaSink
    {
      String mAudioSinkID;
      IWindowSinkPtr mVideoSinkWindow;
    };

    virtual ~IMediaElementTypes() {}
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrack
  #pragma mark
  
  interaction IMediaElement : public Any,
                              public IMediaElementTypes
  {
    static ElementPtr toDebug(IMediaElementPtr object);

    static IMediaElementPtr convert(AnyPtr any);

    static IMediaElementPtr create(MediaSink sink);

    virtual PUID getID() const = 0;

    virtual IMediaStreamTrackSubscriptionPtr subscribe(IMediaElementDelegatePtr delegate) = 0;

    virtual String sinkID() const = 0;  // unique audio device ID delivering audio output
    virtual IWindowSinkPtr sinkWindow() const = 0;  // window where media is being delivered

    virtual bool muted() const = 0;
    virtual void muted(bool muted) = 0;
    virtual bool paused() const = 0;
    virtual void paused(bool paused) = 0;
    virtual States readyState() const = 0;

    virtual void stop() = 0;

    virtual MediaStreamTrackList audioTracks() = 0;
    virtual MediaStreamTrackList videoTracks() = 0;

    virtual PromisePtr attach(IMediaStreamTrackPtr track) = 0;
    virtual void detach(IMediaStreamTrackPtr track) = 0;
  };

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaElementDelegate
  #pragma mark

  interaction IMediaElementDelegate
  {
    virtual ~IMediaElementDelegate() {}
  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaElementSubscription
  #pragma mark
  
  interaction IMediaElementSubscription
  {
    virtual PUID getID() const = 0;
    
    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaElementDelegate)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaElementDelegate, ortc::IMediaElementSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
