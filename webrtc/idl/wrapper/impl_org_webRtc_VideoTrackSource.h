
#pragma once

#include "types.h"
#include "generated/org_webRtc_VideoTrackSource.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct VideoTrackSource : public wrapper::org::webRtc::VideoTrackSource
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::VideoTrackSource, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoTrackSource, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::VideoTrackSourceInterface, NativeType);

          typedef rtc::scoped_refptr<NativeType> NativeTypeScopedPtr;

          NativeTypeScopedPtr native_;
          VideoTrackSourceWeakPtr thisWeak_;

          VideoTrackSource() noexcept;
          virtual ~VideoTrackSource() noexcept;

          // properties MediaTrackSource
          wrapper::org::webRtc::MediaSourceState get_state() noexcept override;
          bool get_remote() noexcept override;

          // properties VideoTrackSource
          bool get_isScreencast() noexcept override;
          Optional< bool > get_needsDenoising() noexcept override;
          Optional< wrapper::org::webRtc::VideoTrackSourceStatsPtr > get_stats() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypeScopedPtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

