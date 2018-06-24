/*

 Copyright (c) 2016, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/adapter/types.h>
#include <ortc/IStatsProvider.h>

namespace ortc
{
  namespace adapter
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTypes
    //

    interaction IMediaStreamTypes
    {
      ZS_DECLARE_TYPEDEF_PTR(std::list<IMediaStreamPtr>, MediaStreamList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<IMediaStreamTrackPtr>, MediaStreamTrackList);
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStream
    //

    interface IMediaStream : public IMediaStreamTypes,
                             public IStatsProvider
    {
      static IMediaStreamPtr create(
                                    IMediaStreamDelegatePtr delegate,
                                    IMediaStreamPtr stream = IMediaStreamPtr()
                                    ) noexcept;
      static IMediaStreamPtr create(
                                    IMediaStreamDelegatePtr delegate,
                                    const MediaStreamTrackList &tracks
                                    ) noexcept;

      static ElementPtr toDebug(IMediaStreamPtr stream) noexcept;

      virtual PUID getID() const noexcept  = 0;

      virtual IMediaStreamSubscriptionPtr subscribe(IMediaStreamDelegatePtr delegate) noexcept  = 0;

      virtual String id() const noexcept = 0;
      virtual bool active() const noexcept = 0;

      virtual MediaStreamTrackListPtr getAudioTracks() const noexcept = 0;
      virtual MediaStreamTrackListPtr getVideoTracks() const noexcept = 0;
      virtual MediaStreamTrackListPtr getTracks() const noexcept = 0;
      virtual IMediaStreamTrackPtr getTrackByID(const char *id) const noexcept = 0;
      virtual size_t size() const noexcept = 0;

      virtual void addTrack(IMediaStreamTrackPtr track) noexcept = 0;
      virtual void removeTrack(IMediaStreamTrackPtr track) noexcept = 0;

      virtual IMediaStreamPtr clone() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaStreamDelegate
    //

    interface IMediaStreamDelegate
    {
      virtual void onMediaStreamAddTrack(
                                         IMediaStreamPtr stream,
                                         IMediaStreamTrackPtr track
                                         ) = 0;
      virtual void onMediaStreamRemoveTrack(
                                            IMediaStreamPtr stream,
                                            IMediaStreamTrackPtr track
                                            ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IPeerConnectionSubscription
    //

    interaction IMediaStreamSubscription
    {
      virtual PUID getID() const noexcept = 0;

      virtual void cancel() noexcept = 0;

      virtual void background() noexcept = 0;
    };
  }
}


ZS_DECLARE_PROXY_BEGIN(ortc::adapter::IMediaStreamDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::adapter::IMediaStreamPtr, IMediaStreamPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamAddTrack, IMediaStreamPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamRemoveTrack, IMediaStreamPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_END()


ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::adapter::IMediaStreamDelegate, ortc::adapter::IMediaStreamSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::adapter::IMediaStreamPtr, IMediaStreamPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamAddTrack, IMediaStreamPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamRemoveTrack, IMediaStreamPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
