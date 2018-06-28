
#pragma once

#include "types.h"
#include "generated/org_webrtc_VideoTrackSource.h"

#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct VideoTrackSource : public wrapper::org::webrtc::VideoTrackSource
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::VideoTrackSource, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::VideoTrackSource, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::VideoTrackSourceInterface, NativeType);

          typedef rtc::scoped_refptr<::webrtc::VideoTrackSourceInterface> NativeScopedPtr;

          NativeScopedPtr native_;
          VideoTrackSourceWeakPtr thisWeak_;

          VideoTrackSource() noexcept;
          virtual ~VideoTrackSource() noexcept;

          // properties MediaTrackSource
          wrapper::org::webrtc::MediaSourceState get_state() noexcept override;
          bool get_remote() noexcept override;

          // properties VideoTrackSource
          bool get_isScreencast() noexcept override;
          Optional< bool > get_needsDenoising() noexcept override;
          Optional< wrapper::org::webrtc::VideoTrackSourceStatsPtr > get_stats() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static NativeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

