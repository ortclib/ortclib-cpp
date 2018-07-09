
#pragma once

#include "types.h"
#include "generated/org_webRtc_MediaTrackSource.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct MediaTrackSource : public wrapper::org::webRtc::MediaTrackSource
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::MediaTrackSource, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaTrackSource, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::MediaSourceInterface, NativeType);

          typedef ::rtc::scoped_refptr<NativeType> NativeTypeScopedPtr;

          NativeTypeScopedPtr native_;
          MediaTrackSourceWeakPtr thisWeak_;

          MediaTrackSource() noexcept;
          virtual ~MediaTrackSource() noexcept;
          void wrapper_dispose() noexcept override;


          // properties MediaTrackSource
          wrapper::org::webRtc::MediaSourceState get_state() noexcept override;
          bool get_remote() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypeScopedPtr native) noexcept;
          ZS_NO_DISCARD() static NativeTypeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

