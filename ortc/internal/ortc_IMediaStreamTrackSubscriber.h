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

#include <ortc/internal/types.h>

#include <ortc/IMediaStreamTrack.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriber);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriberMedia);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriberRTP);

    
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackSubscriberForMediaStreamTrack
    #pragma mark

    interaction IMediaStreamTrackSubscriberForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberForMediaStreamTrack, ForMediaStreamTrack);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackChannelForMediaStreamTrackSubscriber, UseMediaChannelBase);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::OverconstrainedError, OverconstrainedError);

      virtual PUID getID() const = 0;

      virtual void shutdown() = 0;

      virtual void notifyTrackMute(bool isMuted) = 0;
      virtual void notifyTrackEnded() = 0;
      virtual void notifyOverConstrained(OverconstrainedErrorPtr error) = 0;

      virtual void notifyChannel(
                                 UseMediaChannelBasePtr mediaChannel,
                                 ImmutableMediaChannelTracePtr trace
                                 ) = 0;
    };

    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackSubscriberMediaForMediaStreamTrack
    #pragma mark

    interaction IMediaStreamTrackSubscriberMediaForMediaStreamTrack : public IMediaStreamTrackSubscriberForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberMediaForMediaStreamTrack, ForMediaStreamTrack);

      static ForMediaStreamTrackPtr create(
                                           IMediaStreamTrackMediaSubscriptionPtr &outSubscription,
                                           IMediaStreamTrackSyncMediaDelegatePtr syncDelegate,
                                           IMediaStreamTrackAsyncMediaDelegatePtr asyncDelegate
                                           );

      virtual ~IMediaStreamTrackSubscriberMediaForMediaStreamTrack();
    };

    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackSubscriberRTPForMediaStreamTrack
    #pragma mark

    interaction IMediaStreamTrackSubscriberRTPForMediaStreamTrack : public IMediaStreamTrackSubscriberForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberRTPForMediaStreamTrack, ForMediaStreamTrack);

      static ForMediaStreamTrackPtr create(
                                           IMediaStreamTrackMediaSubscriptionPtr &outSubscription,
                                           IMediaStreamTrackRTPDelegatePtr delegate
                                           );

      virtual ~IMediaStreamTrackSubscriberRTPForMediaStreamTrack();
    };

    
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackSubscriberForMediaStreamTrackChannel
    #pragma mark

    interaction IMediaStreamTrackSubscriberForMediaStreamTrackChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberForMediaStreamTrackChannel, ForMediaStreamTrackChannel);

    };

    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackSubscriberMediaForMediaStreamTrackChannel
    #pragma mark

    interaction IMediaStreamTrackSubscriberMediaForMediaStreamTrackChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberMediaForMediaStreamTrackChannel, ForMediaStreamTrackChannel);
    };

    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackSubscriberRTPForMediaStreamTrackChannel
    #pragma mark

    interaction IMediaStreamTrackSubscriberRTPForMediaStreamTrackChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberRTPForMediaStreamTrackChannel, ForMediaStreamTrackChannel);
    };

  } // namespace internal
} // namespace ortc
