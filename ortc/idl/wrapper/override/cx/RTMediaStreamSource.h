
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#pragma once

namespace Org
{
  namespace Ortc
  {

    /// <summary>
    /// Delegate used to notify an update of the frame per second on a video stream.
    /// </summary>
    public delegate void FramesPerSecondChangedEventHandler(Platform::String^ id, Platform::String^ fps);

    /// <summary>
    /// Class used to get frame rate events from renderer.
    /// </summary>
    public ref class FrameCounterHelper sealed
    {
    public:
      /// <summary>
      /// Event fires when the frame rate changes.
      /// </summary>
      static event FramesPerSecondChangedEventHandler^ FramesPerSecondChanged;

    internal:
      static void FireEvent(Platform::String^ id, Platform::String^ str);
    };

    /// <summary>
    /// Delegate used to notify an update of the frame resolutions.
    /// </summary>
    public delegate void ResolutionChangedEventHandler(Platform::String^ id, unsigned int width, unsigned int height);

    /// <summary>
    /// Class used to get frame size change events from renderer.
    /// </summary>
    public ref class ResolutionHelper sealed
    {
    public:
      /// <summary>
      /// Event fires when the resolution changes.
      /// </summary>
      static event ResolutionChangedEventHandler^ ResolutionChanged;

    internal:
      static void FireEvent(
        Platform::String^ id,
        unsigned int width,
        unsigned int height
      );
    };

  } // namespace ortc
}  // namespace org

#ifdef USE_OLD_RENDERER

#include "webrtc/modules/video_render/video_render.h"

#include <Mfidl.h>

namespace org
{
  namespace ortc
  {
    using Windows::Media::Core::MediaStreamSourceSampleRequest;
    using Windows::System::Threading::ThreadPoolTimer;
    using Windows::Media::Core::MediaStreamSource;
    using Platform::WeakReference;
    using Platform::String;

    class MediaSourceHelper;

    ref class MediaStreamTrack;

    ref class RTMediaStreamSource sealed {
    public:
      virtual ~RTMediaStreamSource();
      void Teardown();

      void OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender,
        Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);

    internal:
      static MediaStreamSource^ CreateMediaSource(
        MediaStreamTrack^ track, uint32 frameRate, Platform::String^ id);
    private:
      class RTCRenderer : public webrtc::VideoRenderCallback {
      public:
        explicit RTCRenderer(RTMediaStreamSource^ streamSource);
        virtual ~RTCRenderer();
        virtual void SetSize(uint32 width, uint32 height, uint32 reserved);
        virtual int32_t RenderFrame(const uint32_t streamId,
          const webrtc::VideoFrame& videoFrame);
        virtual bool CanApplyRotation() { return true; }
      private:
        // This object is owned by RTMediaStreamSource
        // so _streamSource must be a weak reference
        WeakReference _streamSource;
      };

      RTMediaStreamSource(MediaStreamTrack^ videoTrack, bool isH264);
      void ProcessReceivedFrame(webrtc::VideoFrame *frame);
      bool ConvertFrame(IMFMediaBuffer* mediaBuffer, webrtc::VideoFrame* frame);
      void ResizeSource(uint32 width, uint32 height);

      HRESULT MakeSampleCallback(webrtc::VideoFrame* frame, IMFSample** sample);
      void FpsCallback(int fps);

      MediaStreamTrack^ _videoTrack;
      Platform::String^ _id;  // Provided by the calling API.

      // Keep a weak reference here.
      // Its _mediaStreamSource that keeps a reference to this object.
      WeakReference _mediaStreamSource;
      rtc::scoped_ptr<RTCRenderer> _rtcRenderer;
      rtc::scoped_ptr<webrtc::CriticalSectionWrapper> _lock;

      rtc::scoped_ptr<MediaSourceHelper> _helper;

      ThreadPoolTimer^ _progressTimer;
      void ProgressTimerElapsedExecute(ThreadPoolTimer^ source);

      ThreadPoolTimer^ _fpsTimer;
      void FPSTimerElapsedExecute(ThreadPoolTimer^ source);
      bool _frameSentThisTime;

      Windows::Media::Core::VideoStreamDescriptor^ _videoDesc;

      void ReplyToSampleRequest();

      MediaStreamSourceSampleRequest^ _request;
      Windows::Media::Core::MediaStreamSourceSampleRequestDeferral^ _deferral;
      Windows::Media::Core::MediaStreamSourceStartingRequestDeferral^ _startingDeferral;
    };
  } // namespace ortc
}  // namespace org

#endif //USE_OLD_RENDERER
