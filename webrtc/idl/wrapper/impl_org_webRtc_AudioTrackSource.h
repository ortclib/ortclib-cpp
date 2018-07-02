
#pragma once

#include "types.h"
#include "generated/org_webRtc_AudioTrackSource.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/mediastreaminterface.h"
#include "impl_org_webRtc_post_include.h"

#include <zsLib/IMessageQueue.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct AudioTrackSource : public wrapper::org::webRtc::AudioTrackSource
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::AudioTrackSource, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::AudioTrackSource, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::AudioSourceInterface, NativeType);

          typedef rtc::scoped_refptr<NativeType> NativeScopedPtr;

          ZS_DECLARE_STRUCT_PTR(WebrtcObserver);

          struct WebrtcObserver : public NativeType::AudioObserver
          {
            WebrtcObserver(
                           WrapperImplTypePtr wrapper,
                           IMessageQueuePtr queue
                           ) noexcept : outer_(wrapper), queue_(queue) {}

            void OnSetVolume(double volume) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              queue_->postClosure([outer, volume]() { outer->onWebrtcObserverSetVolume(volume); });
            }

          private:
            WrapperImplTypeWeakPtr outer_;
            IMessageQueuePtr queue_;
          };

          std::atomic<double> lastVolume_;
          WebrtcObserverUniPtr observer_;
          NativeScopedPtr native_;
          AudioTrackSourceWeakPtr thisWeak_;

          AudioTrackSource() noexcept;
          virtual ~AudioTrackSource() noexcept;

          // properties MediaTrackSource
          wrapper::org::webRtc::MediaSourceState get_state() noexcept override;
          bool get_remote() noexcept override;

          // properties AudioTrackSource
          double get_volume() noexcept override;
          void set_volume(double value) noexcept override;

          void wrapper_onObserverCountChanged(size_t count) noexcept override;

          void setupObserver();
          void teardownObserver();

          // WebrtcObserver methods
          void onWebrtcObserverSetVolume(double volume) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static NativeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

