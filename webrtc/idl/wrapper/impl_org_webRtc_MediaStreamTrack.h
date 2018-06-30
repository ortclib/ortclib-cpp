
#pragma once

#include "types.h"
#include "generated/org_webRtc_MediaStreamTrack.h"

#include "impl_org_webRtc_MediaSourceHelper.h"
#include "impl_org_webRtc_MediaStreamSource.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/video/video_frame.h"
#include "api/mediastreaminterface.h"
#include "impl_org_webRtc_post_include.h"

#include <set>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct MediaStreamTrack : public wrapper::org::webRtc::MediaStreamTrack
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::MediaStreamTrack, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaStreamTrack, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::MediaStreamTrackInterface, NativeType);

          typedef rtc::scoped_refptr<NativeType> NativeTypeScopedPtr;

          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::MediaElement, UseMediaElement);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaElement, UseMediaElementImpl);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::MediaSource, UseMediaSource);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaSource, UseMediaSourceImpl);

          ZS_DECLARE_STRUCT_PTR(WebrtcObserver);

          struct WebrtcObserver : public rtc::VideoSinkInterface<::webrtc::VideoFrame>
          {
            WebrtcObserver(WrapperImplTypePtr wrapper) noexcept : outer_(wrapper) {}

            void OnFrame(const ::webrtc::VideoFrame& frame) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyFrame(frame);
            }

            // Should be called by the source when it discards the frame due to rate
            // limiting.
            void OnDiscardedFrame()
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyDiscardedFrame();
            }

          private:
            WrapperImplTypeWeakPtr outer_;
          };

          WebrtcObserverUniPtr observer_;
          rtc::scoped_refptr<NativeType> native_;

          mutable zsLib::Lock lock_;
          UseMediaElementPtr element_;
          UseMediaSourcePtr source_;
          MediaStreamSource^ mediaStreamSource_;
          VideoFrameType currentFrameType_;
          bool firstFrameReceived_ { false };

          MediaStreamTrackWeakPtr thisWeak_;

          MediaStreamTrack() noexcept;
          virtual ~MediaStreamTrack() noexcept;

          // properties MediaStreamTrack
          String get_kind() noexcept override;
          String get_id() noexcept override;
          bool get_enabled() noexcept override;
          void set_enabled(bool value) noexcept override;
          wrapper::org::webRtc::MediaStreamTrackState get_state() noexcept override;
          wrapper::org::webRtc::MediaSourcePtr get_source() noexcept override;
          wrapper::org::webRtc::MediaElementPtr get_element() noexcept override;
          void set_element(wrapper::org::webRtc::MediaElementPtr value) noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          void notifySourceChanged(UseMediaSourcePtr source);
          void autoAttachSourceToElement();

          void setupObserver();
          void teardownObserver();

          // WebrtcObserver methods
          void notifyFrame(const ::webrtc::VideoFrame& frame) noexcept;
          void notifyDiscardedFrame() noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static NativeTypeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

