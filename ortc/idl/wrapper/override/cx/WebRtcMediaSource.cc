
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "WebRtcMediaSource.h"
#include "WebRtcMediaStream.h"

#include <mferror.h>
#include <Mfapi.h>

using Microsoft::WRL::MakeAndInitialize;

#define RETURN_ON_FAIL(code) { HRESULT hr = code; if (FAILED(hr)) {OutputDebugString(L"Failed"); return hr;} }

namespace Org
{
  namespace Ortc
  {

    WebRtcMediaSource::WebRtcMediaSource() :
      _rateControlThin(FALSE), _rate(1.0f), _started(false),
      _lock(webrtc::CriticalSectionWrapper::CreateCriticalSection()) {
    }

    WebRtcMediaSource::~WebRtcMediaSource() {
    }

    HRESULT WebRtcMediaSource::CreateMediaSource(
      IMediaSource** source,
      zsLib::AnyPtr track,
      Platform::String^ id
    )
    {
      *source = nullptr;
      ComPtr<WebRtcMediaSource> internalRet;
      RETURN_ON_FAIL(MakeAndInitialize<WebRtcMediaSource>(
        &internalRet, track, id));
      ComPtr<IMediaSource> ret;
      internalRet.As(&ret);
      *source = ret.Detach();
      return S_OK;
    }

    HRESULT WebRtcMediaSource::RuntimeClassInitialize(
      zsLib::AnyPtr track,
      Platform::String^ id
    )
    {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue != nullptr)
        return S_OK;

      RETURN_ON_FAIL(MFCreateEventQueue(&_eventQueue));

      // For now only allow one stream
      RETURN_ON_FAIL(MakeAndInitialize<WebRtcMediaStream>(
        &_stream, this, track, id));
      ComPtr<IMFStreamDescriptor> streamDescriptor;
      RETURN_ON_FAIL(_stream->GetStreamDescriptor(&streamDescriptor));
      RETURN_ON_FAIL(MFCreatePresentationDescriptor(
        1, streamDescriptor.GetAddressOf(), &_presDescriptor));
      RETURN_ON_FAIL(_presDescriptor->SelectStream(0));

      return S_OK;
    }

    // IMFMediaEventGenerator
    IFACEMETHODIMP WebRtcMediaSource::GetEvent(
      DWORD dwFlags, IMFMediaEvent **ppEvent) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      return _eventQueue->GetEvent(dwFlags, ppEvent);
    }

    IFACEMETHODIMP WebRtcMediaSource::BeginGetEvent(
      IMFAsyncCallback *pCallback, IUnknown *punkState) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      return _eventQueue->BeginGetEvent(pCallback, punkState);
    }

    IFACEMETHODIMP WebRtcMediaSource::EndGetEvent(
      IMFAsyncResult *pResult, IMFMediaEvent **ppEvent) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      return _eventQueue->EndGetEvent(pResult, ppEvent);
    }

    IFACEMETHODIMP WebRtcMediaSource::QueueEvent(
      MediaEventType met, const GUID& guidExtendedType,
      HRESULT hrStatus, const PROPVARIANT *pvValue) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      return _eventQueue->QueueEventParamVar(
        met, guidExtendedType, hrStatus, pvValue);
    }

    // IMFMediaSource
    IFACEMETHODIMP WebRtcMediaSource::GetCharacteristics(
      DWORD *pdwCharacteristics) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      *pdwCharacteristics = MFMEDIASOURCE_IS_LIVE;
      return S_OK;
    }

    IFACEMETHODIMP WebRtcMediaSource::CreatePresentationDescriptor(
      IMFPresentationDescriptor **ppPresentationDescriptor) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      return _presDescriptor->Clone(ppPresentationDescriptor);
    }

    IFACEMETHODIMP WebRtcMediaSource::Start(
      IMFPresentationDescriptor *pPresentationDescriptor,
      const GUID *pguidTimeFormat, const PROPVARIANT *pvarStartPosition) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      OutputDebugString(L"WebRtcMediaSource::Start\r\n");
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      if (!_started) {
        ComPtr<IUnknown> unk;
        _stream.As(&unk);
        RETURN_ON_FAIL(_eventQueue->QueueEventParamUnk(
          MENewStream, GUID_NULL, S_OK, unk.Get()));
      }
      // Start stream
      RETURN_ON_FAIL(_stream->Start(pPresentationDescriptor,
        pguidTimeFormat, pvarStartPosition));
      RETURN_ON_FAIL(QueueEvent(MESourceStarted,
        GUID_NULL, S_OK, pvarStartPosition));
      _started = true;
      return S_OK;
    }

    IFACEMETHODIMP WebRtcMediaSource::Stop() {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      OutputDebugString(L"WebRtcMediaSource::Stop\r\n");
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      RETURN_ON_FAIL(_stream->Stop());
      RETURN_ON_FAIL(QueueEvent(MESourceStopped, GUID_NULL, S_OK, nullptr));
      return S_OK;
    }

    IFACEMETHODIMP WebRtcMediaSource::Pause() {
      return MF_E_INVALID_STATE_TRANSITION;
    }

    IFACEMETHODIMP WebRtcMediaSource::Shutdown() {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      OutputDebugString(L"WebRtcMediaSource::Shutdown\r\n");
      if (_eventQueue != nullptr) {
        _eventQueue->Shutdown();
      }

      RETURN_ON_FAIL(_stream->Shutdown());

      _presDescriptor = nullptr;
      _deviceManager = nullptr;
      _eventQueue = nullptr;
      _stream.Reset();
      return S_OK;
    }

    // IMFMediaSourceEx
    IFACEMETHODIMP WebRtcMediaSource::GetSourceAttributes(
      IMFAttributes **ppAttributes) {
      return E_NOTIMPL;
    }

    IFACEMETHODIMP WebRtcMediaSource::GetStreamAttributes(
      DWORD dwStreamIdentifier, IMFAttributes **ppAttributes) {
      return E_NOTIMPL;
    }

    IFACEMETHODIMP WebRtcMediaSource::SetD3DManager(IUnknown *pManager) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      RETURN_ON_FAIL(pManager->QueryInterface(
        IID_IMFDXGIDeviceManager,
        reinterpret_cast<void**>(_deviceManager.ReleaseAndGetAddressOf())));

      RETURN_ON_FAIL(_stream->SetD3DManager(_deviceManager));

      return S_OK;
    }

    IFACEMETHODIMP WebRtcMediaSource::GetService(
      REFGUID guidService, REFIID riid, LPVOID *ppvObject) {
      if (guidService == MF_RATE_CONTROL_SERVICE || guidService == MF_MEDIASOURCE_SERVICE) {
        HRESULT hr = QueryInterface(riid, ppvObject);
        return hr;
      }
      return MF_E_UNSUPPORTED_SERVICE;
    }

    IFACEMETHODIMP WebRtcMediaSource::SetRate(BOOL fThin, float flRate) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      if (fThin) {
        return MF_E_THINNING_UNSUPPORTED;
      }

      PROPVARIANT pv;
      PropVariantInit(&pv);
      pv.vt = VT_R4;
      pv.fltVal = 1.0f;
      RETURN_ON_FAIL(QueueEvent(MESourceRateChanged, GUID_NULL, S_OK, &pv));

      return S_OK;
    }

    IFACEMETHODIMP WebRtcMediaSource::GetRate(BOOL *pfThin, float *pflRate) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      *pfThin = FALSE;
      *pflRate = 1.0f;
      return S_OK;
    }

    IFACEMETHODIMP WebRtcMediaSource::GetSlowestRate(
      MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      if (eDirection == MFRATE_REVERSE) {
        return MF_E_REVERSE_UNSUPPORTED;
      }

      if (fThin) {
        return MF_E_THINNING_UNSUPPORTED;
      }

      if (!pflRate) {
        return E_POINTER;
      }

      *pflRate = 1.0f;

      return S_OK;
    }

    IFACEMETHODIMP WebRtcMediaSource::GetFastestRate(
      MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      if (eDirection == MFRATE_REVERSE) {
        return MF_E_REVERSE_UNSUPPORTED;
      }

      if (fThin) {
        return MF_E_THINNING_UNSUPPORTED;
      }

      if (!pflRate) {
        return E_POINTER;
      }

      *pflRate = 1.0f;

      return S_OK;
    }

    IFACEMETHODIMP WebRtcMediaSource::IsRateSupported(
      BOOL fThin, float flRate, float *pflNearestSupportedRate) {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_eventQueue == nullptr) {
        return MF_E_SHUTDOWN;
      }
      if (fThin) {
        return MF_E_THINNING_UNSUPPORTED;
      }

      if (pflNearestSupportedRate) {
        *pflNearestSupportedRate = 1.0f;
      }

      if (flRate != 1.0f) {
        return MF_E_UNSUPPORTED_RATE;
      }

      return S_OK;
    }
  } // namespace ortc
}  // namespace org
