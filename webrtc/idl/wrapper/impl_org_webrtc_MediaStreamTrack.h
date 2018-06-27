
#pragma once

#include "types.h"
#include "generated/org_webrtc_MediaStreamTrack.h"

#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/video/video_frame.h"
#include "api/mediastreaminterface.h"
#include "impl_org_webrtc_post_include.h"

#include <set>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct MediaStreamTrack : public wrapper::org::webrtc::MediaStreamTrack
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::MediaStreamTrack, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaStreamTrack, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::MediaStreamTrackInterface, NativeType);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::MediaElement, UseMediaElement);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaElement, UseMediaElementImpl);

          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::MediaSource, UseMediaSource);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaSource, UseMediaSourceImpl);

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

          MediaStreamTrackWeakPtr thisWeak_;

          MediaStreamTrack() noexcept;
          virtual ~MediaStreamTrack() noexcept;
          void wrapper_init_org_webrtc_MediaStreamTrack() noexcept override;

          // properties MediaStreamTrack
          String get_kind() noexcept override;
          String get_id() noexcept override;
          bool get_enabled() noexcept override;
          void set_enabled(bool value) noexcept override;
          wrapper::org::webrtc::MediaStreamTrackState get_state() noexcept override;
          wrapper::org::webrtc::MediaSourcePtr get_source() noexcept override;
          wrapper::org::webrtc::MediaElementPtr get_element() noexcept override;
          void set_element(wrapper::org::webrtc::MediaElementPtr value) noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          void notifySourceChanged(UseMediaSourcePtr source);
          void autoAttachSourceToElement();

          void setupObserver();
          void teardownObserver();

          // WebrtcObserver methods
          void notifyFrame(const ::webrtc::VideoFrame& frame) noexcept;
          void notifyDiscardedFrame() noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static rtc::scoped_refptr<NativeType> toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

