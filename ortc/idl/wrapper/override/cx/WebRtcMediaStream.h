
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#pragma once

//#include "webrtc/modules/video_render/video_render.h"
//#include "webrtc/system_wrappers/include/critical_section_wrapper.h"

#include <wrl.h>
#include <Mfidl.h>

#include <ortc/IMediaStreamTrack.h>

namespace Org
{
  namespace Ortc
  {
    using Microsoft::WRL::ComPtr;

    using Microsoft::WRL::RuntimeClass;
    using Microsoft::WRL::RuntimeClassFlags;
    using Microsoft::WRL::RuntimeClassType;

    using Windows::System::Threading::ThreadPoolTimer;

    class MediaSourceHelper;

    class WebRtcMediaSource;

    class WebRtcMediaStream :
      public RuntimeClass<RuntimeClassFlags<RuntimeClassType::WinRtClassicComMix>,
                          IMFMediaStream, IMFMediaEventGenerator,
                          IMFGetService>
    {
      InspectableClass(L"WebRtcMediaStream", BaseTrust);

      ZS_DECLARE_CLASS_PTR(RTCRenderer);
      ZS_DECLARE_STRUCT_PTR(OuterReferenceHolder);

    public:
      WebRtcMediaStream();
      virtual ~WebRtcMediaStream();
      HRESULT RuntimeClassInitialize(
        WebRtcMediaSource* source,
        zsLib::AnyPtr track,
        Platform::String^ id
      );

      // IMFMediaEventGenerator
      IFACEMETHOD(GetEvent)(DWORD dwFlags, IMFMediaEvent **ppEvent);
      IFACEMETHOD(BeginGetEvent)(IMFAsyncCallback *pCallback, IUnknown *punkState);
      IFACEMETHOD(EndGetEvent)(IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);
      IFACEMETHOD(QueueEvent)(MediaEventType met, const GUID& guidExtendedType,
        HRESULT hrStatus, const PROPVARIANT *pvValue);
      // IMFMediaStream
      IFACEMETHOD(GetMediaSource)(IMFMediaSource **ppMediaSource);
      IFACEMETHOD(GetStreamDescriptor)(IMFStreamDescriptor **ppStreamDescriptor);
      IFACEMETHOD(RequestSample)(IUnknown *pToken);
      // IMFGetService
      IFACEMETHOD(GetService)(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

      STDMETHOD(Start)(IMFPresentationDescriptor *pPresentationDescriptor,
        const GUID *pguidTimeFormat, const PROPVARIANT *pvarStartPosition);
      STDMETHOD(Stop)();
      STDMETHOD(Shutdown)();
      STDMETHOD(SetD3DManager)(ComPtr<IMFDXGIDeviceManager> manager);

      rtc::scoped_ptr<webrtc::CriticalSectionWrapper> _lock;

    private:
      struct OuterReferenceHolder
      {
        OuterReferenceHolder(WebRtcMediaStream* streamSource) : _streamSource(streamSource) {}

        WebRtcMediaStream* _streamSource;

        std::atomic<bool> _shuttingDown{};
        std::atomic<ULONG> _framesBeingQueued{};
      };

      class RTCRenderer : public ::ortc::IMediaStreamTrackRenderCallback,
                          public webrtc::VideoRenderCallback
      {
      public:
        explicit RTCRenderer(OuterReferenceHolderPtr outer);
        virtual ~RTCRenderer();
        virtual int32_t RenderFrame(const uint32_t streamId,
                                   const webrtc::VideoFrame& videoFrame) override;
        virtual bool CanApplyRotation() { return true; }
      private:
        // This object is owned by RTMediaStreamSource
        // so _streamSource must be a weak reference
        OuterReferenceHolderWeakPtr _outer;
      };

    private:
      ComPtr<IMFMediaEventQueue> _eventQueue;

      WebRtcMediaSource* _source;
      zsLib::AnyPtr _track;
      Platform::String^ _id;

      static HRESULT CreateMediaType(unsigned int width, unsigned int height,
        unsigned int rotation, IMFMediaType** ppType, bool isH264);
      HRESULT MakeSampleCallback(webrtc::VideoFrame* frame, IMFSample** sample);
      void FpsCallback(int fps);

      HRESULT ReplyToSampleRequest();

      rtc::scoped_ptr<MediaSourceHelper> _helper;
      RTCRendererPtr _rtcRenderer;
      OuterReferenceHolderPtr _selfReferenceHolder;

      ComPtr<IMFMediaType> _mediaType;
      ComPtr<IMFDXGIDeviceManager> _deviceManager;
      ComPtr<IMFStreamDescriptor> _streamDescriptor;
      ULONGLONG _startTickCount{};
      ULONGLONG _frameCount{};
      int _index{};
      ULONG _frameReady{};

      // From the sample manager.
      HRESULT ResetMediaBuffers();
      static const int BufferCount = 3;
      std::vector<ComPtr<IMFMediaBuffer>> _mediaBuffers;
      int _frameBufferIndex{};

      bool _gpuVideoBuffer{};
      bool _isH264{};
      bool _started{};
    };
  } // namespace ortc
}  // namespace org
