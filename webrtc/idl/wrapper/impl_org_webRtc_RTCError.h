
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCError.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCError : public wrapper::org::webRtc::RTCError
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCError, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCError, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::RTCError, NativeType);

          RTCErrorWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCError() noexcept;
          virtual ~RTCError() noexcept;

          void wrapper_init_org_webRtc_RTCError() noexcept override;

          // properties RTCError
          wrapper::org::webRtc::RTCErrorType get_type() noexcept override;
          String get_message() noexcept override;
          bool get_ok() noexcept override;

          void reject(PromisePtr promise) noexcept;

          static void rejectPromise(PromisePtr promise, const NativeType &native) noexcept;
          static void rejectPromise(PromisePtr promise, NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static PromisePtr toPromise(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static PromisePtr toPromise(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

