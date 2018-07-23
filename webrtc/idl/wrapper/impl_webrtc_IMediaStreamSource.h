
#pragma once

#ifdef WINUWP

#ifdef __has_include
#if __has_include(<winrt/windows.media.core.h>)
#include <winrt/windows.media.core.h>
#endif //__has_include(<winrt/windows.media.core.h>)
#endif //__has_include

#ifdef CPPWINRT_VERSION

#include <wrapper/impl_org_webRtc_pre_include.h>
#include "api/mediastreaminterface.h"
#include <wrapper/impl_org_webRtc_post_include.h>

#include <zsLib/types.h>
#include <zsLib/Proxy.h>
#include <zsLib/ProxySubscriptions.h>

#include <atomic>
#include <wrl.h>
#include <mfidl.h>


namespace webrtc
{
  ZS_DECLARE_INTERACTION_PTR(IMediaStreamSource);
  ZS_DECLARE_INTERACTION_PROXY(IMediaStreamSourceDelegate);
  ZS_DECLARE_CLASS_PTR(MediaStreamSource);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IMediaStreamSourceSubscription, IMediaStreamSourceDelegate);


  interaction IMediaStreamSource
  {
    enum VideoFrameType
    {
      VideoFrameType_Default,

      VideoFrameType_I420 = VideoFrameType_Default,
      VideoFrameType_H264,

      VideoFrameType_Last = VideoFrameType_H264,
    };

    struct CreationProperties
    {
      VideoFrameType frameType_ {};
      IMediaStreamSourceDelegatePtr delegate_;

      const char *id_{};
      float frameRateChangeTolerance_ {0.1f};
    };

    static IMediaStreamSourcePtr create(const CreationProperties &info) noexcept;

    virtual IMediaStreamSourceSubscriptionPtr subscribe(IMediaStreamSourceDelegatePtr delegate) = 0;

    virtual winrt::Windows::Media::Core::MediaStreamSource source() const noexcept = 0;
    virtual std::string id() const noexcept = 0;

    virtual uint32_t width() const noexcept = 0;
    virtual uint32_t height() const noexcept = 0;
    virtual int rotation() const noexcept = 0;

    virtual void notifyFrame(const webrtc::VideoFrame &frame) noexcept = 0;
  };
  
  interaction IMediaStreamSourceDelegate
  {
    virtual void onMediaStreamSourceResolutionChanged(
                                                      IMediaStreamSourcePtr source,
                                                      uint32_t width,
                                                      uint32_t height
                                                      ) = 0;
    virtual void onMediaStreamSourceRotationChanged(
                                                    IMediaStreamSourcePtr source,
                                                    int rotation
                                                    ) = 0;

    virtual void onMediaStreamSourceFrameRateChanged(
                                                     IMediaStreamSourcePtr source,
                                                     float frameRate
                                                     ) = 0;
  };

  interaction IMediaStreamSourceSubscription
  {
    virtual zsLib::PUID getID() const noexcept = 0;
    virtual void cancel() noexcept = 0;
    virtual void background() noexcept = 0;
  };
} // namespace webrtc


ZS_DECLARE_PROXY_BEGIN(webrtc::IMediaStreamSourceDelegate)
ZS_DECLARE_PROXY_TYPEDEF(webrtc::IMediaStreamSourcePtr, IMediaStreamSourcePtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamSourceResolutionChanged, IMediaStreamSourcePtr, uint32_t, uint32_t)
ZS_DECLARE_PROXY_METHOD(onMediaStreamSourceRotationChanged, IMediaStreamSourcePtr, int)
ZS_DECLARE_PROXY_METHOD(onMediaStreamSourceFrameRateChanged, IMediaStreamSourcePtr, float)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(webrtc::IMediaStreamSourceDelegate, webrtc::IMediaStreamSourceSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(webrtc::IMediaStreamSourcePtr, IMediaStreamSourcePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamSourceResolutionChanged, IMediaStreamSourcePtr, uint32_t, uint32_t)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamSourceRotationChanged, IMediaStreamSourcePtr, int)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamSourceFrameRateChanged, IMediaStreamSourcePtr, float)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

#endif //CPPWINRT_VERSION
#endif //WINUWP
