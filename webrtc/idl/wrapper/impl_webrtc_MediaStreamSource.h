
#pragma once

#include "impl_webrtc_IMediaStreamSource.h"

#ifdef WINUWP
#ifdef CPPWINRT_VERSION

#include <wrapper/impl_org_webRtc_pre_include.h>
#include "api/mediastreaminterface.h"
#include "rtc_base/criticalsection.h"
#include <wrapper/impl_org_webRtc_post_include.h>

#include <zsLib/types.h>
#include <zsLib/Proxy.h>
#include <zsLib/ProxySubscriptions.h>

#include <atomic>
#include <wrl.h>
#include <mfidl.h>

namespace webrtc
{
  class MediaStreamSource : public IMediaStreamSource
  {
  private:
    typedef Microsoft::WRL::ComPtr<IMFSample> IMFSampleComPtr;
    struct make_private {};

    typedef uint32_t DimensionType;
    typedef int RotationType;
    typedef LONGLONG RenderTime;  // The presentation time, in 100-nanosecond units.

    ZS_DECLARE_STRUCT_PTR(SampleData);

    typedef std::queue< SampleDataUniPtr > SampleDataQueue;

    struct SampleData
    {
      std::unique_ptr<webrtc::VideoFrame> frame_;
      IMFSampleComPtr sample_;
      DimensionType width_ {};
      DimensionType height_ {};
      RotationType rotation_ {};
      RenderTime renderTime_ {};
      bool isIDR_ {};
    };

  private:
    void init(const CreationProperties &props) noexcept;

  public:

    MediaStreamSource(const make_private &);
    ~MediaStreamSource();

    static MediaStreamSourcePtr create(const CreationProperties &info) noexcept;

    IMediaStreamSourceSubscriptionPtr subscribe(IMediaStreamSourceDelegatePtr delegate) override;

    winrt::Windows::Media::Core::MediaStreamSource source() const noexcept override { return source_; }
    std::string id() const noexcept override { return id_; }

    uint32_t width() const noexcept override;
    uint32_t height() const noexcept override;
    int rotation() const noexcept override;

    void notifyFrame(const webrtc::VideoFrame &frame) noexcept override;

  private:
    void putInQueue(SampleDataUniPtr sample) noexcept;
    void flushQueueOfExcessiveIDRs() noexcept;
    void popFromQueue() noexcept;

    void pendingRequestRespondToRequestedFrame() noexcept;
    bool respondToRequest(const winrt::Windows::Media::Core::MediaStreamSourceSampleRequest &request) noexcept;
    SampleDataUniPtr dequeue() noexcept;
    bool makeI420Sample(SampleData &sample);

    bool updateFrameRate() noexcept;

    void fireResolutionChanged() noexcept;
    void fireRotationChanged() noexcept;
    void fireFrameRateChanged() noexcept;

    void notifyStartCompleteIfReady() noexcept;
    void stop() noexcept;

    void handleStarting(const winrt::Windows::Media::Core::MediaStreamSourceStartingEventArgs &args);
    void handleSampleRequested(const winrt::Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs &args);
    void handleStreamClosed(const winrt::Windows::Media::Core::MediaStreamSourceClosedEventArgs &args);

  private:
    MediaStreamSourceWeakPtr thisWeak_;
    mutable zsLib::RecursiveLock lock_;

    IMediaStreamSourceDelegateSubscriptions subscriptions_;
    IMediaStreamSourceSubscriptionPtr defaultSubscription_;

    VideoFrameType frameType_ {};
    std::string id_;

    winrt::Windows::Media::Core::MediaStreamSource source_ {nullptr};
    winrt::Windows::Media::Core::VideoStreamDescriptor descriptor_ {nullptr};

    std::atomic_bool started_ {};
    std::atomic_bool stopping_ {};
    std::atomic_bool stopped_ {};
    winrt::event_token startingToken_ {};
    winrt::Windows::Media::Core::MediaStreamSourceStartingRequestDeferral startingDeferral_ {nullptr};
    winrt::Windows::Media::Core::MediaStreamSourceStartingEventArgs startingArgs_ {nullptr};

    winrt::event_token requestingSampleToken_ {};
    winrt::Windows::Media::Core::MediaStreamSourceSampleRequest requestSample_ {nullptr};
    winrt::Windows::Media::Core::MediaStreamSourceSampleRequestDeferral requestingSampleDeferral_ {nullptr};

    winrt::event_token stoppingToken_ {};

    bool putIDRIntoQueue_{};

    size_t totalFrameCounted_ {};

    std::atomic_bool firedResolutionChange_{};
    DimensionType lastWidth_ {};
    DimensionType lastHeight_ {};

    std::atomic_bool firedRotationChange_{};
    RotationType lastRotation_{};

    std::atomic_bool firedFrameRateChange_{};
    float frameRateChangeTolerance_ {};
    float lastAverageFrameRate_ {};
    zsLib::Time lastTimeChecked_ {};

    RenderTime firstRenderTime_ {};

    SampleDataQueue queue_;
    size_t totalIDRFramesInQueue_ {};
  };

}

#endif //CPPWINRT_VERSION
#endif //WINUWP
