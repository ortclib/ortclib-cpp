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
  //
  // IWindowSink
  //

  interaction IWindowSink
  {
#ifdef _WIN32
#ifdef __cplusplus_winrt
    static IMediaElementPtr create() noexcept;
#else
    static IMediaElementPtr create(HWND hwnd) noexcept;
#endif // __cplusplus_winrt
#else
    static IMediaElementPtr create(void *window) noexcept;
#endif // _WIN32

    virtual PUID getID() const noexcept = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaElementTypes
  //
  
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

    virtual ~IMediaElementTypes() noexcept {}
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrack
  //
  
  interaction IMediaElement : public Any,
                              public IMediaElementTypes
  {
    static ElementPtr toDebug(IMediaElementPtr object) noexcept;

    static IMediaElementPtr convert(AnyPtr any) noexcept;

    static IMediaElementPtr create(MediaSink sink) noexcept;

    virtual PUID getID() const noexcept = 0;

    virtual IMediaStreamTrackSubscriptionPtr subscribe(IMediaElementDelegatePtr delegate) noexcept = 0;

    virtual String sinkID() const noexcept = 0;  // unique audio device ID delivering audio output
    virtual IWindowSinkPtr sinkWindow() const noexcept = 0;  // window where media is being delivered

    virtual bool muted() const noexcept = 0;
    virtual void muted(bool muted) noexcept = 0;
    virtual bool paused() const noexcept = 0;
    virtual void paused(bool paused) noexcept = 0;
    virtual States readyState() const noexcept = 0;

    virtual void stop() noexcept = 0;

    virtual MediaStreamTrackList audioTracks() noexcept = 0;
    virtual MediaStreamTrackList videoTracks() noexcept = 0;

    virtual PromisePtr attach(IMediaStreamTrackPtr track) noexcept = 0;
    virtual void detach(IMediaStreamTrackPtr track) noexcept = 0;
  };

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //
  // IMediaElementDelegate
  //

  interaction IMediaElementDelegate
  {
    virtual ~IMediaElementDelegate() noexcept {}
  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaElementSubscription
  //
  
  interaction IMediaElementSubscription
  {
    virtual PUID getID() const noexcept = 0;
    
    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaElementDelegate)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaElementDelegate, ortc::IMediaElementSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
