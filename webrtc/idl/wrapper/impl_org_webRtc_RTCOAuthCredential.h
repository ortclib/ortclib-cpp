
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCOAuthCredential.h"

#include <utility>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCOAuthCredential : public wrapper::org::webRtc::RTCOAuthCredential
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCOAuthCredential, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCOAuthCredential, WrapperImplType);
          //ZS_DECLARE_TYPEDEF_PTR(SomeType, NativeType);

          RTCOAuthCredentialWeakPtr thisWeak_;

          RTCOAuthCredential() noexcept;
          virtual ~RTCOAuthCredential() noexcept;

          // methods RTCOAuthCredential
          void wrapper_init_org_webRtc_RTCOAuthCredential() noexcept override;
          void wrapper_init_org_webRtc_RTCOAuthCredential(wrapper::org::webRtc::RTCOAuthCredentialPtr source) noexcept override;

          //ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          //ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          //ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

