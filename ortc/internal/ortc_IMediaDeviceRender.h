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

#include <ortc/internal/ortc_IMediaDevice.h>

#include <ortc/IMediaStreamTrack.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaDeviceForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IMediaDeviceRenderForMediaEngine);
    ZS_DECLARE_INTERACTION_PTR(IMediaDeviceRenderAudioForMediaEngine);

    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDevice);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDeviceRender);
    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDeviceRenderAudio);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceRender
    #pragma mark

    interaction IMediaDeviceRender : public IMediaDevice
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceRender, ForMediaStreamTrack);

      typedef IMediaStreamTrackTypes::MediaChannelID MediaChannelID;
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::MediaDeviceObjectID, MediaDeviceObjectID);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Kinds, Kinds);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);

      typedef zsLib::PromiseWithHolderPtr<IMediaDeviceRenderPtr> MediaDeviceRenderPromise;
      ZS_DECLARE_PTR(MediaDeviceRenderPromise);

      static MediaDeviceRenderPromisePtr create(
                                                MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                const TrackConstraints &constraints,
                                                IMediaDeviceRenderDelegatePtr delegate
                                                );

      virtual void notifyAudioFrame(
                                    ImmutableMediaChannelTracePtr trace,
                                    AudioFramePtr frame
                                    ) = 0;
      virtual void notifyVideoFrame(
                                    ImmutableMediaChannelTracePtr trace,
                                    VideoFramePtr frame
                                    ) = 0;

      virtual ~IMediaDeviceRender() {}
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceRenderAudio
    #pragma mark

    interaction IMediaDeviceRenderAudio : public IMediaDeviceRender
    {
      virtual void notifyVideoFrame(
                                    ImmutableMediaChannelTracePtr trace,
                                    VideoFramePtr frame
                                    ) override {} // ignored
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceRenderDelegate
    #pragma mark

    interaction IMediaDeviceRenderDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::States, States);

      virtual void onMediaDeviceRenderStateChanged(
                                                   IMediaDeviceRenderPtr device,
                                                   States state
                                                   ) = 0;

      virtual ~IMediaDeviceRenderDelegate() {}
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceRenderSubscription
    #pragma mark

    interaction IMediaDeviceRenderSubscription
    {
      virtual PUID getID() const = 0;

      virtual void cancel() = 0;

      virtual void background() = 0;
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceRenderForMediaEngine
    #pragma mark

    interaction IMediaDeviceRenderForMediaEngine : public IMediaDeviceForMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceRenderForMediaEngine, ForMediaEngine);

      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::MediaDeviceObjectID, MediaDeviceObjectID);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);

      typedef zsLib::PromiseWithHolderPtr<IMediaDeviceRenderPtr> MediaDeviceRenderPromise;
      ZS_DECLARE_PTR(MediaDeviceRenderPromise);

      virtual void mediaDeviceRenderSubscribe(
                                              MediaDeviceRenderPromisePtr promise,
                                              MediaDeviceObjectID repaceExistingDeviceObjectID,
                                              TrackConstraintsPtr constraints,
                                              IMediaDeviceRenderDelegatePtr delegate
                                              ) = 0;
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceRenderAudioForMediaEngine
    #pragma mark

    interaction IMediaDeviceRenderAudioForMediaEngine : public IMediaDeviceRenderForMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceRenderAudioForMediaEngine, ForMediaEngine);

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceRenderAudio, UseMediaEngine);

      static ForMediaEnginePtr create(
                                      UseMediaEnginePtr mediaEngine,
                                      const String &deviceID
                                      );
    };

  }
}


ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaDeviceRenderDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaDeviceRenderPtr, IMediaDeviceRenderPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IMediaDevice::States, States)
ZS_DECLARE_PROXY_METHOD_2(onMediaDeviceRenderStateChanged, IMediaDeviceRenderPtr, States)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::internal::IMediaDeviceRenderDelegate, ortc::internal::IMediaDeviceRenderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::internal::IMediaDeviceRenderPtr, IMediaDeviceRenderPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::internal::IMediaDevice::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onMediaDeviceRenderStateChanged, IMediaDeviceRenderPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
