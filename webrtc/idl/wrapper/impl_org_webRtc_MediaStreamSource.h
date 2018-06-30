
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#pragma once

#include "impl_org_webRtc_MediaSourceHelper.h"

#include <wrapper/impl_org_webRtc_pre_include.h>
#include "api/mediastreaminterface.h"
#include "rtc_base/criticalsection.h"
#include <wrapper/impl_org_webRtc_post_include.h>


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        //using Windows::Media::Core::MediaStreamSource;
        //using Platform::WeakReference;
        //using Windows::System::Threading::ThreadPoolTimer;
        //using Windows::Media::Core::MediaStreamSourceSampleRequest;

        ref class MediaStreamSource sealed {
          typedef Windows::Media::Core::MediaStreamSource UseMediaStreamSource;
          typedef Windows::System::Threading::ThreadPoolTimer ThreadPoolTimer;
          typedef Windows::Media::Core::MediaStreamSourceSampleRequest MediaStreamSourceSampleRequest;
          typedef Platform::WeakReference WeakReference;

        public:
          virtual ~MediaStreamSource();
          void Teardown();

          void OnSampleRequested(
                                 UseMediaStreamSource ^sender,
                                 Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args
                                 );

        internal:
          static MediaStreamSource^ CreateMediaSource(
                                                      VideoFrameType frameType,
                                                      Platform::String^ id
                                                      );

          UseMediaStreamSource^ GetMediaStreamSource();

          void RenderFrame(const webrtc::VideoFrame *frame);

        private:
          class RTCRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
          public:
            explicit RTCRenderer(MediaStreamSource^ streamSource);
            virtual ~RTCRenderer();
            virtual void SetSize(
                                 uint32 width,
                                 uint32 height,
                                 uint32 reserved
                                 );
            virtual void RenderFrame(const webrtc::VideoFrame *frame);
            virtual bool CanApplyRotation() { return true; }
            void OnFrame(const webrtc::VideoFrame& frame) override
            {
              RenderFrame(&frame);
            }
          private:
            // This object is owned by MediaStreamSource
            // so _streamSource must be a weak reference
            WeakReference _streamSource;
          };

          MediaStreamSource(VideoFrameType frameType);
          void ProcessReceivedFrame(webrtc::VideoFrame *frame);
          bool ConvertFrame(
                            IMFMediaBuffer* mediaBuffer,
                            webrtc::VideoFrame* frame
                            );
          void ResizeSource(
                            uint32 width,
                            uint32 height
                            );

          HRESULT MakeSampleCallback(
                                     webrtc::VideoFrame* frame,
                                     IMFSample** sample
                                     );
          void FpsCallback(int fps);

          void ProgressTimerElapsedExecute(ThreadPoolTimer^ source);
          void FPSTimerElapsedExecute(ThreadPoolTimer^ source);
          void ReplyToSampleRequest();

          Platform::String^ _id;  // Provided by the calling API.
          std::string _idUtf8; // Provided by the calling API, same as _id

          UseMediaStreamSource^ _mediaStreamSource;
          std::unique_ptr<RTCRenderer> _rtcRenderer;
          rtc::CriticalSection _critSect;

          std::unique_ptr<MediaSourceHelper> _helper;

          ThreadPoolTimer^ _progressTimer;

          ThreadPoolTimer^ _fpsTimer;
          bool _frameSentThisTime {};

          Windows::Media::Core::VideoStreamDescriptor^ _videoDesc;

          MediaStreamSourceSampleRequest^ _request;
          Windows::Media::Core::MediaStreamSourceSampleRequestDeferral^ _deferral;
          Windows::Media::Core::MediaStreamSourceStartingRequestDeferral^ _startingDeferral;
          Windows::Media::Core::MediaStreamSourceStartingEventArgs^ _startingArgs;

          ULONG _frameBeingQueued {};
        };

      } // namespace webRtc
    } // namespace org
  } // namespace impl
}  // namespace wrapper
