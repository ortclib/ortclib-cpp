
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCOAuthCredential.h"

#include <utility>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCOAuthCredential : public wrapper::org::webrtc::RTCOAuthCredential
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::RTCOAuthCredential, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCOAuthCredential, WrapperImplType);
          //ZS_DECLARE_TYPEDEF_PTR(SomeType, NativeType);

          RTCOAuthCredentialWeakPtr thisWeak_;

          RTCOAuthCredential() noexcept;
          virtual ~RTCOAuthCredential() noexcept;

          // methods RTCOAuthCredential
          void wrapper_init_org_webrtc_RTCOAuthCredential() noexcept override;
          void wrapper_init_org_webrtc_RTCOAuthCredential(wrapper::org::webrtc::RTCOAuthCredentialPtr source) noexcept override;

          //ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          //ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          //ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

