// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "RTMediaStreamSource.h"
#include <mfapi.h>
#include <ppltasks.h>
#include <mfidl.h>

#include <wrapper/impl_org_webrtc_pre_include.h>
#include "api/videosourceinterface.h"
#include "libyuv/convert.h"
#include "common_video/video_common_winuwp.h"
#include "rtc_base/logging.h"
#include "media/base/videocommon.h"
#include <wrapper/impl_org_webrtc_post_include.h>

namespace Org {
  namespace WebRtc {
    /// <summary>
    /// Delegate used to notify an update of the frame per second on a video stream.
    /// </summary>
    public delegate void FramesPerSecondChangedEventHandler(Platform::String^ id, Platform::String^ fps);

    /// <summary>
    /// Delegate used to notify an update of the frame resolutions.
    /// </summary>
    public delegate void ResolutionChangedEventHandler(Platform::String^ id, unsigned int width, unsigned int height);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    /// <summary>
    /// Class used to get frame rate events from renderer.
    /// </summary>
    public ref class FrameCounterHelper sealed {
    public:
      /// <summary>
      /// Event fires when the frame rate changes.
      /// </summary>
      static event FramesPerSecondChangedEventHandler^ FramesPerSecondChanged;
    internal:
      static void FireEvent(Platform::String^ id, Platform::String^ str);
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    /// <summary>
    /// Class used to get frame size change events from renderer.
    /// </summary>
    public ref class ResolutionHelper sealed {
    public:
      /// <summary>
      /// Event fires when the resolution changes.
      /// </summary>
      static event ResolutionChangedEventHandler^ ResolutionChanged;
    internal:
      static void FireEvent(Platform::String^ id, unsigned int width, unsigned int height);
    };

    //-------------------------------------------------------------------------
    void Org::WebRtc::FrameCounterHelper::FireEvent(
                                                    Platform::String^ id,
                                                    Platform::String^ str
                                                    )
    {
      Windows::UI::Core::CoreDispatcher^ windowDispatcher = webrtc::VideoCommonWinUWP::GetCoreDispatcher();
      if (windowDispatcher != nullptr) {
        windowDispatcher->RunAsync(
          Windows::UI::Core::CoreDispatcherPriority::Normal,
          ref new Windows::UI::Core::DispatchedHandler([id, str] {
            FramesPerSecondChanged(id, str);
          })
        );
      } else {
        FramesPerSecondChanged(id, str);
      }
    }

    //-------------------------------------------------------------------------
    void Org::WebRtc::ResolutionHelper::FireEvent(
                                                  Platform::String^ id,
                                                  unsigned int width,
                                                  unsigned int heigth
                                                  )
    {
      Windows::UI::Core::CoreDispatcher^ windowDispatcher = webrtc::VideoCommonWinUWP::GetCoreDispatcher();
      if (windowDispatcher) {
        windowDispatcher->RunAsync(
          Windows::UI::Core::CoreDispatcherPriority::Normal,
          ref new Windows::UI::Core::DispatchedHandler([id, width, heigth] {
            ResolutionChanged(id, width, heigth);
          })
        );
      } else {
        ResolutionChanged(id, width, heigth);
      }
    }

    namespace Internal
    {
      using Windows::Media::MediaProperties::VideoEncodingProperties;
      using Windows::Media::MediaProperties::MediaEncodingSubtypes;
      using Windows::Media::Core::VideoStreamDescriptor;
      using Windows::Media::Core::MediaStreamSourceStartingEventArgs;
      using Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs;
      using Windows::Media::Core::MediaStreamSource;

      //-------------------------------------------------------------------------
      RTMediaStreamSource^ RTMediaStreamSource::CreateMediaSource(
                                                                  VideoFrameType frameType,
                                                                  Platform::String^ id
                                                                  )
      {
        auto streamState = ref new RTMediaStreamSource(frameType);
        streamState->_id = id;
        streamState->_idUtf8 = rtc::ToUtf8(streamState->_id->Data());
        streamState->_rtcRenderer = std::unique_ptr<RTCRenderer>(
          new RTCRenderer(streamState));
        VideoEncodingProperties^ videoProperties;
        if (frameType == FrameTypeH264) {
          videoProperties = VideoEncodingProperties::CreateH264();
          //videoProperties->ProfileId = Windows::Media::MediaProperties::H264ProfileIds::Baseline;
        }
        else {
          videoProperties =
            VideoEncodingProperties::CreateUncompressed(
              MediaEncodingSubtypes::Nv12, 10, 10);
        }
        
        streamState->_videoDesc = ref new VideoStreamDescriptor(videoProperties);

        // initial value, this will be override by incoming frame from webrtc.
        // this is needed since the UI element might request sample before webrtc has
        // incoming frame ready(ex.: remote stream), in this case, this initial value
        // will make sure we will at least create a small dummy frame.
        streamState->_videoDesc->EncodingProperties->Width = 720;
        streamState->_videoDesc->EncodingProperties->Height = 1280;

        Org::WebRtc::ResolutionHelper::FireEvent(id,
          streamState->_videoDesc->EncodingProperties->Width,
          streamState->_videoDesc->EncodingProperties->Height);

        streamState->_videoDesc->EncodingProperties->FrameRate->Numerator = 30;
        streamState->_videoDesc->EncodingProperties->FrameRate->Denominator = 1;
        auto streamSource = ref new MediaStreamSource(streamState->_videoDesc);

        auto startingCookie = streamSource->Starting +=
          ref new Windows::Foundation::TypedEventHandler<
          MediaStreamSource ^,
          MediaStreamSourceStartingEventArgs ^>([streamState](
            MediaStreamSource^ sender,
            MediaStreamSourceStartingEventArgs^ args) {
          // Get a deferall on the starting event so we can trigger it
          // when the first frame arrives.
          streamState->_startingDeferral = args->Request->GetDeferral();
          streamState->_startingArgs = args;
        });

        // Set buffertime to 0 for rtc
        auto timespan = Windows::Foundation::TimeSpan();
        timespan.Duration = 0;
        streamSource->BufferTime = timespan;
        streamState->_mediaStreamSource = streamSource;

        // Use a lambda to capture a strong reference to RTMediaStreamSource.
        // This is the only way to tie the lifetime of the RTMediaStreamSource
        // to that of the MediaStreamSource.
        auto sampleRequestedCookie = streamSource->SampleRequested +=
          ref new Windows::Foundation::TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>(
            [streamState](MediaStreamSource^ sender, MediaStreamSourceSampleRequestedEventArgs^ args) {
              streamState->OnSampleRequested(sender, args);
            }
          );

        streamSource->Closed +=
          ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource^, Windows::Media::Core::MediaStreamSourceClosedEventArgs ^>(
            [streamState, startingCookie, sampleRequestedCookie](Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceClosedEventArgs^ args) {
              RTC_LOG(LS_INFO) << "RTMediaStreamSource::OnClosed";
              streamState->Teardown();
              sender->Starting -= startingCookie;
              sender->SampleRequested -= sampleRequestedCookie;
            }
          );

        // Create a timer which sends request progress periodically.
        //{
        //  auto handler = ref new TimerElapsedHandler(streamState,
        //    &RTMediaStreamSource::ProgressTimerElapsedExecute);
        //  auto timespan = Windows::Foundation::TimeSpan();
        //  timespan.Duration = 500 * 1000 * 10;  // 500 ms in hns
        //  streamState->_progressTimer = ThreadPoolTimer::CreatePeriodicTimer(
        //    handler, timespan);
        //}

        //// Create a timer which ensures we don't display frames faster that expected.
        //// Required because Media Foundation sometimes requests samples in burst mode
        //// but we use the wall clock to drive timestamps.
        //{
        //  auto handler = ref new TimerElapsedHandler(streamState,
        //    &RTMediaStreamSource::FPSTimerElapsedExecute);
        //  auto timespan = Windows::Foundation::TimeSpan();
        //  timespan.Duration = 15 * 1000 * 10;
        //  streamState->_fpsTimer = ThreadPoolTimer::CreatePeriodicTimer(handler,
        //    timespan);
        //}

        return streamState;
      }

      //-------------------------------------------------------------------------
      MediaStreamSource^ RTMediaStreamSource::GetMediaStreamSource()
      {
        rtc::CritScope lock(&_critSect);
        return _mediaStreamSource;
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::RenderFrame(const webrtc::VideoFrame *frame)
      {
        auto frameCopy = new webrtc::VideoFrame(frame->video_frame_buffer(), frame->rotation(), 0);
        ProcessReceivedFrame(frameCopy);
      }

      //-------------------------------------------------------------------------
      RTMediaStreamSource::RTMediaStreamSource(VideoFrameType frameType) :
        _frameSentThisTime(false),
        _frameBeingQueued(0)
      {
        RTC_LOG(LS_INFO) << "RTMediaStreamSource::RTMediaStreamSource";

        // Create the helper with the callback functions.
        _helper.reset(new MediaSourceHelper(frameType,
          [this](webrtc::VideoFrame* frame, IMFSample** sample) -> HRESULT {
            return MakeSampleCallback(frame, sample);
          },
          [this](int fps) {
            return FpsCallback(fps);
          }
        ));
      }

      //-------------------------------------------------------------------------
      RTMediaStreamSource::~RTMediaStreamSource()
      {
        RTC_LOG(LS_INFO) << "RTMediaStreamSource::~RTMediaStreamSource ID=" << _idUtf8;
        Teardown();
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::Teardown()
      {
        RTC_LOG(LS_INFO) << "RTMediaStreamSource::Teardown() ID=" << _idUtf8;
        {
          rtc::CritScope lock(&_critSect);
          if (_progressTimer != nullptr) {
            _progressTimer->Cancel();
            _progressTimer = nullptr;
          }
          if (_fpsTimer != nullptr) {
            _fpsTimer->Cancel();
            _fpsTimer = nullptr;
          }

          _request = nullptr;
          if (_deferral != nullptr) {
            _deferral->Complete();
            _deferral = nullptr;
          }
          if (_startingDeferral != nullptr) {
            _startingDeferral->Complete();
            _startingDeferral = nullptr;
          }
          _helper.reset();
        }

        // Wait until no frames are being queued
        // from the webrtc callback.
        while (_frameBeingQueued > 0) {
          Sleep(1);
        }

        {
          rtc::CritScope lock(&_critSect);
          if (_rtcRenderer != nullptr) {
            _rtcRenderer.reset();
          }
        }
        RTC_LOG(LS_INFO) << "RTMediaStreamSource::Teardown() done ID=" << _idUtf8;
      }

      //-------------------------------------------------------------------------
      RTMediaStreamSource::RTCRenderer::RTCRenderer(RTMediaStreamSource^ streamSource) : _streamSource(streamSource)
      {
      }

      //-------------------------------------------------------------------------
      RTMediaStreamSource::RTCRenderer::~RTCRenderer()
      {
        RTC_LOG(LS_INFO) << "RTMediaStreamSource::RTCRenderer::~RTCRenderer";
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::RTCRenderer::SetSize(
                                                     uint32 width,
                                                     uint32 height,
                                                     uint32 reserved
                                                     )
      {
        auto stream = _streamSource.Resolve<RTMediaStreamSource>();
        if (stream != nullptr) {
          stream->ResizeSource(width, height);
        }
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::RTCRenderer::RenderFrame(const webrtc::VideoFrame *frame)
      {
        auto stream = _streamSource.Resolve<RTMediaStreamSource>();
        if (stream != nullptr) {
          auto frameCopy = new webrtc::VideoFrame(
            frame->video_frame_buffer(), frame->rotation(),
            0);

          stream->ProcessReceivedFrame(frameCopy);
        }
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::ProgressTimerElapsedExecute(ThreadPoolTimer^ source)
      {
        //rtc::CritScope lock(&_critSect);
        //if (_request != nullptr) {}
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::FPSTimerElapsedExecute(ThreadPoolTimer^ source)
      {
        rtc::CritScope lock(&_critSect);
        _frameSentThisTime = false;
        if (_request != nullptr) {
          ReplyToSampleRequest();
        }
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::ReplyToSampleRequest()
      {
        auto sampleData = _helper->DequeueFrame();
        if (sampleData == nullptr) return;

        // Update rotation property
        if (sampleData->rotationHasChanged) {
          auto props = _videoDesc->EncodingProperties->Properties;
          OutputDebugString((L"Video rotation changed: " + sampleData->rotation + "\r\n")->Data());
          props->Insert(MF_MT_VIDEO_ROTATION, sampleData->rotation);
        }

        // Frame size in EncodingProperties needs to be updated before completing
        // deferral, otherwise the MediaElement will receive a frame having different
        // size and application may crash.
        if (sampleData->sizeHasChanged) {
          auto props = _videoDesc->EncodingProperties;
          props->Width = (unsigned int)sampleData->size.cx;
          props->Height = (unsigned int)sampleData->size.cy;
          Org::WebRtc::ResolutionHelper::FireEvent(
            _id, props->Width, props->Height);
          OutputDebugString((L"Video frame size changed for " + _id +
            L" W=" + props->Width +
            L" H=" + props->Height + L"\r\n")->Data());
        }

        Microsoft::WRL::ComPtr<IMFMediaStreamSourceSampleRequest> spRequest;
        HRESULT hr = reinterpret_cast<IInspectable*>(_request)->QueryInterface(
          spRequest.ReleaseAndGetAddressOf());

        hr = spRequest->SetSample(sampleData->sample.Get());

        if (_deferral != nullptr) {
          _deferral->Complete();
        }

        _deferral = nullptr;
      }

      //-------------------------------------------------------------------------
      HRESULT RTMediaStreamSource::MakeSampleCallback(
                                                      webrtc::VideoFrame* frame,
                                                      IMFSample** sample
                                                      )
      {
        ComPtr<IMFSample> spSample;
        HRESULT hr = MFCreateSample(spSample.GetAddressOf());
        if (FAILED(hr)) {
          return E_FAIL;
        }
        ComPtr<IMFMediaBuffer> mediaBuffer;
        hr = MFCreate2DMediaBuffer(
          (DWORD)frame->width(), (DWORD)frame->height(),
          cricket::FOURCC_NV12, FALSE,
          mediaBuffer.GetAddressOf());
        if (FAILED(hr)) {
          return E_FAIL;
        }

        spSample->AddBuffer(mediaBuffer.Get());

        ComPtr<IMF2DBuffer2> imageBuffer;
        if (FAILED(mediaBuffer.As(&imageBuffer))) {
          return E_FAIL;
        }

        BYTE* destRawData;
        BYTE* buffer;
        LONG pitch;
        DWORD destMediaBufferSize;

        if (FAILED(imageBuffer->Lock2DSize(MF2DBuffer_LockFlags_Write,
          &destRawData, &pitch, &buffer, &destMediaBufferSize))) {
          return E_FAIL;
        }
        try {
          //TODO Check
          //frame->MakeExclusive();
          // Convert to NV12
          rtc::scoped_refptr<webrtc::PlanarYuvBuffer> frameBuffer =
            static_cast<webrtc::PlanarYuvBuffer*>(frame->video_frame_buffer().get());
          uint8* uvDest = destRawData + (pitch * frame->height());
          libyuv::I420ToNV12(frameBuffer->DataY(), frameBuffer->StrideY(),
            frameBuffer->DataU(), frameBuffer->StrideU(),
            frameBuffer->DataV(), frameBuffer->StrideV(),
            reinterpret_cast<uint8*>(destRawData), pitch,
            uvDest, pitch,
            static_cast<int>(frame->width()),
            static_cast<int>(frame->height()));
        } catch (...) {
          RTC_LOG(LS_ERROR) << "Exception caught in RTMediaStreamSource::ConvertFrame()";
        }
        imageBuffer->Unlock2D();

        *sample = spSample.Detach();
        return S_OK;
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::FpsCallback(int fps)
      {
        Org::WebRtc::FrameCounterHelper::FireEvent(_id, fps.ToString());
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::OnSampleRequested(
                                                  MediaStreamSource ^sender,
                                                  MediaStreamSourceSampleRequestedEventArgs ^args
                                                  )
      {
        try {

          if (_mediaStreamSource == nullptr)
            return;

          rtc::CritScope lock(&_critSect);

          _request = args->Request;
          if (_request == nullptr) {
            return;
          }
          if (_helper == nullptr) {  // may be null while tearing down
            return;
          }

          if (_helper->HasFrames()) {
            ReplyToSampleRequest();
            return;
          }
          else {
            // Save the request and referral for when a sample comes in.
            if (_deferral != nullptr) {
              RTC_LOG(LS_ERROR) << "Got deferral when another hasn't completed.";
            }

            _deferral = _request->GetDeferral();
            return;
          }
        } catch (...) {
          RTC_LOG(LS_ERROR) << "Exception in RTMediaStreamSource::OnSampleRequested.";
        }
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::ProcessReceivedFrame(webrtc::VideoFrame *frame)
      {
        rtc::CritScope lock(&_critSect);

        if (_startingDeferral) {
          auto timespan = Windows::Foundation::TimeSpan();
          timespan.Duration = 0;
          _startingArgs->Request->SetActualStartPosition(timespan);
          _startingDeferral->Complete();
          _startingDeferral = nullptr;
          _startingArgs = nullptr;

          //TODO: Request a keyframe from the server when the first frame is received.
        }

        if (!_helper) {  // May be null while tearing down the MSS
          return;
        }
        _helper->QueueFrame(frame);

        // If we have a pending request, reply to it now.
        if (_deferral != nullptr) {
          ReplyToSampleRequest();
        }
      }

      //-------------------------------------------------------------------------
      void RTMediaStreamSource::ResizeSource(uint32 width, uint32 height)
      {
      }
    } // namespace Internal
  } // namespace WebRtc
}  // namespace Org
