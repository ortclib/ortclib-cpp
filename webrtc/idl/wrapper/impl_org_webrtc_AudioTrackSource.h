
#pragma once

#include "types.h"
#include "generated/org_webrtc_AudioTrackSource.h"


#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/mediastreaminterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct AudioTrackSource : public wrapper::org::webrtc::AudioTrackSource
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::AudioTrackSource, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::AudioTrackSource, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::AudioSourceInterface, NativeType);

          typedef rtc::scoped_refptr<::webrtc::AudioSourceInterface> NativeScopedPtr;

          ZS_DECLARE_STRUCT_PTR(WebrtcObserver);

          struct WebrtcObserver : public NativeType::AudioObserver
          {
            WebrtcObserver(WrapperImplTypePtr wrapper) noexcept : outer_(wrapper) {}

            void OnSetVolume(double volume) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              outer->notifyVolume(volume);
            }

          private:
            WrapperImplTypeWeakPtr outer_;
          };

          WebrtcObserverUniPtr observer_;
          NativeScopedPtr native_;
          AudioTrackSourceWeakPtr thisWeak_;

          AudioTrackSource() noexcept;
          virtual ~AudioTrackSource() noexcept;

          // properties MediaTrackSource
          wrapper::org::webrtc::MediaSourceState get_state() noexcept override;
          bool get_remote() noexcept override;
          void wrapper_init_org_webrtc_AudioTrackSource() noexcept override;

          // properties AudioTrackSource
          void set_volume(double value) noexcept override;

          void wrapper_onObserverCountChanged(size_t count) noexcept override;

          void setupObserver();
          void teardownObserver();

          // WebrtcObserver methods
          void notifyVolume(double volume) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static rtc::scoped_refptr<NativeType> toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

