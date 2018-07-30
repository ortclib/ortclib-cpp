
#include "impl_webrtc_MediaStreamSource.h"

#ifdef WINUWP
#ifdef CPPWINRT_VERSION

#include <zsLib/String.h>
#include <zsLib/IMessageQueueThread.h>

#include <wrapper/impl_org_webRtc_pre_include.h>
#include "third_party/winuwp_h264/H264Decoder/H264Decoder.h"
#include "media/base/videocommon.h"
#include "libyuv/convert.h"
#include "rtc_base/logging.h"
#include <wrapper/impl_org_webRtc_post_include.h>

#include <winrt/Windows.Media.MediaProperties.h>

//#include <mfapi.h>
//#include <mfidl.h>

using namespace webrtc;
using zsLib::String;
using zsLib::Time;
using zsLib::Seconds;
using zsLib::Milliseconds;
using zsLib::AutoRecursiveLock;

//-----------------------------------------------------------------------------
static bool isSampleIDR(IMFSample* sample)
{
  ZS_ASSERT(nullptr != sample);

  ComPtr<IMFMediaBuffer> pBuffer;
  sample->GetBufferByIndex(0, &pBuffer);
  BYTE* pBytes{};
  DWORD maxLength{};
  DWORD curLength{};
  if (FAILED(pBuffer->Lock(&pBytes, &maxLength, &curLength))) return false;

  bool found{};

  // Search for the beginnings of nal units.
  for (DWORD i = 0; i < curLength - 5; ++i)
  {
    BYTE* ptr = pBytes + i;

    if ((0x00 != ptr[0]) || (0x00 != ptr[1])) continue;

    int prefixLengthFound = 0;

    if ((0x00 == ptr[2]) && (0x01 == ptr[3]))
      prefixLengthFound = 4;
    else if (0x01 == ptr[2])
      prefixLengthFound = 3;
    else continue;

    if (0x05 != (ptr[prefixLengthFound] & 0x1f)) continue;

    // Found IDR NAL unit
    pBuffer->Unlock();
    found = true;
    break;
  }
  pBuffer->Unlock();
  return found;
}

//-----------------------------------------------------------------------------
MediaStreamSource::MediaStreamSource(const make_private &) :
  subscriptions_(decltype(subscriptions_)::create())
{
}

//-----------------------------------------------------------------------------
MediaStreamSource::~MediaStreamSource()
{
  thisWeak_.reset();
  stop();
}

//-----------------------------------------------------------------------------
MediaStreamSourcePtr MediaStreamSource::create(const CreationProperties &info) noexcept
{
  auto result = std::make_shared<MediaStreamSource>(make_private{});
  result->thisWeak_ = result;
  result->init(info);
  return result;
}

//-----------------------------------------------------------------------------
void MediaStreamSource::init(const CreationProperties &props) noexcept
{
  frameType_ = props.frameType_;
  id_ = String(props.id_);
  frameRateChangeTolerance_ = props.frameRateChangeTolerance_;

  updateFrameRate();

  if (props.delegate_) {
    defaultSubscription_ = subscriptions_.subscribe(props.delegate_, zsLib::IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue());
  }

  winrt::Windows::Media::MediaProperties::VideoEncodingProperties properties {nullptr};
  switch (frameType_) {
    case VideoFrameType::VideoFrameType_H264: {
      properties = winrt::Windows::Media::MediaProperties::VideoEncodingProperties::CreateH264();
      break;
    }
    case VideoFrameType::VideoFrameType_I420: {
      properties = winrt::Windows::Media::MediaProperties::VideoEncodingProperties::CreateUncompressed(winrt::Windows::Media::MediaProperties::MediaEncodingSubtypes::Nv12(), 10, 10);
      break;
    }
  }

  lastWidth_ = 720;
  lastHeight_ = 1280;

  // Initial value, this will be override by incoming frame from webrtc.
  // This is needed since the UI element might request sample before webrtc has
  // incoming frame ready (ex.: remote stream), in this case, this initial value
  // will make sure we will at least create a small dummy frame.
  descriptor_ = winrt::Windows::Media::Core::VideoStreamDescriptor(properties);
  descriptor_.EncodingProperties().Width(lastWidth_);
  descriptor_.EncodingProperties().Height(lastHeight_);
  descriptor_.EncodingProperties().FrameRate().Numerator(30);
  descriptor_.EncodingProperties().FrameRate().Denominator(1);

  source_ = winrt::Windows::Media::Core::MediaStreamSource(descriptor_);

  auto thisWeak = thisWeak_;

  startingToken_ = source_.Starting([thisWeak](auto &&, auto &&args) { auto pThis = thisWeak.lock(); if (!pThis) return; pThis->handleStarting(args); });
  requestingSampleToken_ = source_.SampleRequested([thisWeak](auto &&, auto &&args) { auto pThis = thisWeak.lock(); if (!pThis) return; pThis->handleSampleRequested(args); } );
  stoppingToken_ = source_.Closed([thisWeak](auto &&, auto &&args) { auto pThis = thisWeak.lock(); if (!pThis) return; pThis->handleStreamClosed(args); });
}

//-----------------------------------------------------------------------------
IMediaStreamSourceSubscriptionPtr MediaStreamSource::subscribe(IMediaStreamSourceDelegatePtr originalDelegate)
{
  AutoRecursiveLock lock(lock_);
  if (!originalDelegate) return defaultSubscription_;

  auto subscription = subscriptions_.subscribe(originalDelegate, zsLib::IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue());

  auto delegate = subscriptions_.delegate(subscription, true);

  if (delegate) {
    auto pThis = thisWeak_.lock();

    if (firedResolutionChange_) {
      delegate->onMediaStreamSourceResolutionChanged(pThis, lastWidth_, lastHeight_);
    }
    if (firedRotationChange_) {
      delegate->onMediaStreamSourceRotationChanged(pThis, lastRotation_);
    }
    if (firedFrameRateChange_) {
      delegate->onMediaStreamSourceFrameRateChanged(pThis, lastAverageFrameRate_);
    }
  }

  return subscription;
}

//-----------------------------------------------------------------------------
uint32_t MediaStreamSource::width() const noexcept
{
  AutoRecursiveLock lock(lock_);
  return lastWidth_;
}

//-----------------------------------------------------------------------------
uint32_t MediaStreamSource::height() const noexcept
{
  AutoRecursiveLock lock(lock_);
  return lastHeight_;
}

//-----------------------------------------------------------------------------
int MediaStreamSource::rotation() const noexcept
{
  AutoRecursiveLock lock(lock_);
  return lastRotation_;
}

//-----------------------------------------------------------------------------
void MediaStreamSource::notifyFrame(const webrtc::VideoFrame &frame) noexcept
{
  auto dataPtr = std::make_unique<SampleData>();
  auto &data = *dataPtr;

  data.width_ = frame.width();
  data.height_ = frame.height();
  data.rotation_ = frame.rotation();
  data.renderTime_ = static_cast<decltype(data.renderTime_)>(frame.render_time_ms()) * decltype(data.renderTime_)(10000); // 1000 * 10 => 100s nanosecond

  switch (frameType_) {
    case VideoFrameType::VideoFrameType_I420:
    {
      data.frame_ = std::make_unique<webrtc::VideoFrame>(frame.video_frame_buffer(), frame.rotation(), frame.timestamp());
      data.isIDR_ = true;
      break;
    }
    case VideoFrameType::VideoFrameType_H264:
    {
      auto frameBuffer = static_cast<webrtc::NativeHandleBuffer*>(frame.video_frame_buffer().get());
      if (!frameBuffer) return;

      IMFSample* tmpSample = (IMFSample*)frameBuffer->native_handle();
      if (!tmpSample) return;

      tmpSample->AddRef();
      data.sample_.Attach(tmpSample);

      if (isSampleIDR(tmpSample)) {
        data.isIDR_ = true;

        ComPtr<IMFAttributes> sampleAttributes;
        data.sample_.As(&sampleAttributes);

        // sampleAttributes->SetUINT32(MFSampleExtension_Discontinuity, TRUE);
        sampleAttributes->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
      }
      break;
    }
  }

  putInQueue(std::move(dataPtr));

  notifyStartCompleteIfReady();
}

//-----------------------------------------------------------------------------
void MediaStreamSource::putInQueue(SampleDataUniPtr sample) noexcept
{
  {
    AutoRecursiveLock lock(lock_);

    if (sample->isIDR_) {
      ++totalIDRFramesInQueue_;
      putIDRIntoQueue_ = true;
    } else {
      // reject sample if it's not an IDR and no IDR frames have ever been put in
      if (!putIDRIntoQueue_) return;
    }
    queue_.push(std::move(sample));

    // if a deferral is installed then flush only after the deferral is resolved
    if (!requestingSampleDeferral_) {
      flushQueueOfExcessiveIDRs();
    }
  }

  pendingRequestRespondToRequestedFrame();
}

//-----------------------------------------------------------------------------
void MediaStreamSource::flushQueueOfExcessiveIDRs() noexcept
{
  // WARNING: MUST ALREADY BE IN A LOCK

  // no need to pop if there is only one IDR frame in the queue
  if (totalIDRFramesInQueue_ < 2) return;

  // cannot flush more than 2 frames as a "look ahead" frame is needed to calculates the sample duration
  if (queue_.size() < 3) return;

  // do not pop if the front sample is not an IDR
  if (!queue_.front()->isIDR_) return;

  size_t totalEndOfQueueIDRs = (queue_.back()->isIDR_ ? 1 : 0);

  // never allow more than one IDR frame in the queue at a time
  bool popping{};
  while (((totalIDRFramesInQueue_ - totalEndOfQueueIDRs) > 1) && (queue_.size() > 2)) {
    popping = true;
    popFromQueue();
  }

  if (popping) {
    // pop until the next IDR is found
    do {
      {
        auto &frontSample = queue_.front();
        if (frontSample->isIDR_) break;
      }
      popFromQueue();
    } while (true);
  }
}

//-----------------------------------------------------------------------------
void MediaStreamSource::popFromQueue() noexcept
{
  {
    auto &sample = queue_.front();
    if (sample->isIDR_) {
      --totalIDRFramesInQueue_;
    }
  }
  queue_.pop();
}

//-----------------------------------------------------------------------------
void MediaStreamSource::pendingRequestRespondToRequestedFrame() noexcept
{
  decltype(requestingSampleDeferral_) deferral {nullptr};
  decltype(requestSample_) request {nullptr};

  {
    AutoRecursiveLock lock(lock_);
    deferral = requestingSampleDeferral_;
    requestingSampleDeferral_ = nullptr;
    request = requestSample_;
    requestSample_ = nullptr;
  }

  if (!deferral) return;

  if (!respondToRequest(request)) {
    AutoRecursiveLock lock(lock_);
    requestingSampleDeferral_ = deferral;
    requestSample_ = request;
    return;
  }
  deferral.Complete();
  deferral = nullptr;
}

//-----------------------------------------------------------------------------
bool MediaStreamSource::respondToRequest(const winrt::Windows::Media::Core::MediaStreamSourceSampleRequest &request) noexcept
{
  ZS_ASSERT(request);

  auto data = dequeue();
  if (!data) return false;

  auto imfRequest = request.as<IMFMediaStreamSourceSampleRequest>();

  if (!imfRequest) return false;
  if (!data->sample_) return false;

  auto result = imfRequest->SetSample(data->sample_.Get());
  ZS_ASSERT(SUCCEEDED(result));

  return SUCCEEDED(result);
}

//-----------------------------------------------------------------------------
MediaStreamSource::SampleDataUniPtr MediaStreamSource::dequeue() noexcept
{
  SampleDataUniPtr result;

  bool frameRateChange{};
  bool resolutionChange{};
  bool rotationChange{};

  {
    AutoRecursiveLock lock(lock_);

    // must be at least 2 frames in the queue to be able to dequeue any single frame
    if (queue_.size() < 2) return result;

    result = std::move(queue_.front());
    queue_.pop();

    auto &sample = *result;
    auto &lookAheadSample = *(queue_.front());

    if (0 == firstRenderTime_) firstRenderTime_ = sample.renderTime_; // zero base the sample time
    sample.renderTime_ -= firstRenderTime_;
    sample.renderTime_ += 45;

    switch (frameType_) {
      case VideoFrameType::VideoFrameType_I420:
      {
        // Only make full I420 frame when actually dequeing to prevent
        // unneeded conversions where some I420 frames may end up being dropped.
        if (!makeI420Sample(sample)) {
          result.reset();
          return result;
        }

        ComPtr<IMFAttributes> sampleAttributes;
        sample.sample_.As(&sampleAttributes);
        if (sampleAttributes) {
          sampleAttributes->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
          sampleAttributes->SetUINT32(MFSampleExtension_Discontinuity, TRUE);
        }
        LONGLONG duration = (LONGLONG)((1.0 / 30) * 1000 * 1000 * 10);
        sample.sample_->SetSampleDuration(duration);
        break;
      }
      case VideoFrameType::VideoFrameType_H264:
      {
        sample.renderTime_ = 0;
        sample.sample_->SetSampleDuration(lookAheadSample.renderTime_ - sample.renderTime_);
        break;
      }
    }


    sample.sample_->SetSampleTime(sample.renderTime_);

    ++totalFrameCounted_;
    frameRateChange = updateFrameRate();
    if ((sample.width_ != lastWidth_) || (sample.height_ != lastHeight_)) {
      lastWidth_ = sample.width_;
      lastHeight_ = sample.height_;
      resolutionChange = true;
    }
    if (sample.rotation_ != lastRotation_) {
      lastRotation_ = sample.rotation_;
      rotationChange = true;
    }

    if (rotationChange) {
      auto props = descriptor_.EncodingProperties().Properties();
      props.Insert(MF_MT_VIDEO_ROTATION, winrt::box_value(static_cast<UINT32>(lastRotation_)));
    }
    if (resolutionChange) {
      auto props = descriptor_.EncodingProperties();
      props.Width(sample.width_);
      props.Height(sample.height_);
    }

    flushQueueOfExcessiveIDRs();
  }

  if (frameRateChange) fireFrameRateChanged();
  if (resolutionChange) fireResolutionChanged();
  if (rotationChange) fireRotationChanged();

  return result;
}


//-------------------------------------------------------------------------
bool MediaStreamSource::makeI420Sample(SampleData &sample)
{
  ZS_ASSERT(sample.frame_);

  ComPtr<IMFSample> &spSample = sample.sample_;
  HRESULT hr = MFCreateSample(spSample.GetAddressOf());
  if (FAILED(hr)) {
    return false;
  }

  ComPtr<IMFMediaBuffer> mediaBuffer;
  hr = MFCreate2DMediaBuffer(
    (DWORD)sample.frame_->width(),
    (DWORD)sample.frame_->height(),
    cricket::FOURCC_NV12,
    FALSE,
    mediaBuffer.GetAddressOf()
  );

  if (FAILED(hr)) {
    return false;
  }

  spSample->AddBuffer(mediaBuffer.Get());

  ComPtr<IMF2DBuffer2> imageBuffer;
  if (FAILED(mediaBuffer.As(&imageBuffer))) {
    return false;
  }

  BYTE* destRawData;
  BYTE* buffer;
  LONG pitch;
  DWORD destMediaBufferSize;

  if (FAILED(imageBuffer->Lock2DSize(MF2DBuffer_LockFlags_Write, &destRawData, &pitch, &buffer, &destMediaBufferSize))) {
    return false;
  }

  try {
    // Convert to NV12
    rtc::scoped_refptr<webrtc::PlanarYuvBuffer> frameBuffer = static_cast<webrtc::PlanarYuvBuffer*>(sample.frame_->video_frame_buffer().get());

    uint8* uvDest = destRawData + (pitch * sample.frame_->height());
    libyuv::I420ToNV12(
      frameBuffer->DataY(), frameBuffer->StrideY(),
      frameBuffer->DataU(), frameBuffer->StrideU(),
      frameBuffer->DataV(), frameBuffer->StrideV(),
      reinterpret_cast<uint8*>(destRawData),
      pitch,
      uvDest,
      pitch,
      static_cast<int>(sample.frame_->width()),
      static_cast<int>(sample.frame_->height())
    );
  } catch (...) {
    RTC_LOG(LS_ERROR) << "Exception caught in MediaStreamSource::ConvertFrame()";
  }
  imageBuffer->Unlock2D();

  return true;
}

//-----------------------------------------------------------------------------
bool MediaStreamSource::updateFrameRate() noexcept
{
  if (Time() == lastTimeChecked_) {
    lastTimeChecked_ = zsLib::now();
    totalFrameCounted_ = 0;
    return false;
  }

  auto tick = zsLib::now();
  auto diff = tick - lastTimeChecked_;

  if (diff < Seconds(1)) return false;

  lastTimeChecked_ = tick;
  float fps = (float(totalFrameCounted_) / float(zsLib::toMilliseconds(diff).count())) * 1000.0f;
  auto diffFps = abs(lastAverageFrameRate_ - fps);
  if (diffFps > frameRateChangeTolerance_) {
    lastAverageFrameRate_ = fps;
  }
  totalFrameCounted_ = 0;
  return false;
}

//-----------------------------------------------------------------------------
void MediaStreamSource::fireResolutionChanged() noexcept
{
  firedResolutionChange_ = true;
  subscriptions_.delegate()->onMediaStreamSourceResolutionChanged(thisWeak_.lock(), lastWidth_, lastHeight_);
}

//-----------------------------------------------------------------------------
void MediaStreamSource::fireRotationChanged() noexcept
{
  firedRotationChange_ = true;
  subscriptions_.delegate()->onMediaStreamSourceRotationChanged(thisWeak_.lock(), lastRotation_);
}

//-----------------------------------------------------------------------------
void MediaStreamSource::fireFrameRateChanged() noexcept
{
  firedFrameRateChange_ = true;
  subscriptions_.delegate()->onMediaStreamSourceFrameRateChanged(thisWeak_.lock(), lastAverageFrameRate_);
}

//-----------------------------------------------------------------------------
void MediaStreamSource::notifyStartCompleteIfReady() noexcept
{
  if (!started_) return;

  decltype(startingDeferral_) startingDeferral {nullptr};
  decltype(startingArgs_) startingArgs {nullptr};

  {
    AutoRecursiveLock lock(lock_);
    if (totalIDRFramesInQueue_ < 1) return; // not ready

    startingDeferral = startingDeferral_;
    startingDeferral_ = nullptr;

    startingArgs = startingArgs_;
    startingArgs_ = nullptr;
  }
  
  if (!startingDeferral) return;
  ZS_ASSERT(startingArgs);

  startingArgs.Request().SetActualStartPosition(winrt::Windows::Foundation::TimeSpan{});

  startingDeferral.Complete();
  startingDeferral = nullptr;
}

//-----------------------------------------------------------------------------
void MediaStreamSource::stop() noexcept
{
  if (!stopping_.exchange(true)) return;

  decltype(startingDeferral_) startingDeferral {nullptr};
  decltype(requestingSampleDeferral_) requestingSampleDeferral {nullptr};

  {
    AutoRecursiveLock lock(lock_);

    if (source_) {
      if (startingToken_) {
        source_.Starting(startingToken_);
        startingToken_ = winrt::event_token{};
      }

      if (requestingSampleToken_) {
        source_.SampleRequested(requestingSampleToken_);
        requestingSampleToken_ = decltype(requestingSampleToken_) {};
      }

      if (stoppingToken_) {
        source_.Closed(stoppingToken_);
        stoppingToken_ = decltype(stoppingToken_) {};
      }
    }

    startingDeferral = startingDeferral_;
    requestingSampleDeferral = requestingSampleDeferral_;
  }

  if (startingDeferral) {
    startingDeferral_.Complete();
    startingDeferral_ = nullptr;
  }

  if (requestingSampleDeferral) {
    requestingSampleDeferral.Complete();
    requestingSampleDeferral = nullptr;
  }

  stopped_ = true;
}

//-----------------------------------------------------------------------------
void MediaStreamSource::handleStarting(const winrt::Windows::Media::Core::MediaStreamSourceStartingEventArgs &args)
{
  {
    AutoRecursiveLock lock(lock_);
    startingDeferral_ = args.Request().GetDeferral();
    startingArgs_ = args;
    started_ = true;
  }

  notifyStartCompleteIfReady();
}

//-----------------------------------------------------------------------------
void MediaStreamSource::handleSampleRequested(const winrt::Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs &args)
{
  auto request = args.Request();
  bool responded = respondToRequest(request);
  if (responded) return;

  {
    AutoRecursiveLock lock(lock_);
    requestSample_ = request;
    requestingSampleDeferral_ = requestSample_.GetDeferral();
  }

  // possible race condition where frame was inserted while deferral was installed
  pendingRequestRespondToRequestedFrame();
}

//-----------------------------------------------------------------------------
void MediaStreamSource::handleStreamClosed(ZS_MAYBE_USED() const winrt::Windows::Media::Core::MediaStreamSourceClosedEventArgs &args)
{
  ZS_MAYBE_USED(args);

  stop();
}

//-----------------------------------------------------------------------------
IMediaStreamSourcePtr IMediaStreamSource::create(const CreationProperties &info) noexcept
{
  return MediaStreamSource::create(info);
}

#endif //CPPWINRT_VERSION
#endif //WINUWP
