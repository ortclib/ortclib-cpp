
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCError.h"

#include "impl_org_webrtc_pre_include.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCError : public wrapper::org::webrtc::RTCError
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::RTCError, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCError, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RTCError, NativeType);

          RTCErrorWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCError() noexcept;
          virtual ~RTCError() noexcept;
          void wrapper_init_org_webrtc_RTCError() noexcept override;

          // properties RTCError
          wrapper::org::webrtc::RTCErrorType get_type() noexcept override;
          String get_message() noexcept override;
          bool get_ok() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

