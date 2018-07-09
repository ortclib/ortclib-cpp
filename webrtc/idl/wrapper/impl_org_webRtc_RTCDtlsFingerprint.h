
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCDtlsFingerprint.h"

#include "impl_org_webRtc_pre_include.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCDtlsFingerprint : public wrapper::org::webRtc::RTCDtlsFingerprint
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCDtlsFingerprint, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDtlsFingerprint, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::rtc::SSLFingerprint, NativeType);

          
          RTCDtlsFingerprintWeakPtr thisWeak_;

          RTCDtlsFingerprint() noexcept;
          virtual ~RTCDtlsFingerprint() noexcept;


          // methods RTCDtlsFingerprint
          void wrapper_init_org_webRtc_RTCDtlsFingerprint() noexcept override;
          void wrapper_init_org_webRtc_RTCDtlsFingerprint(wrapper::org::webRtc::RTCDtlsFingerprintPtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const std::string &alg, NativeTypeUniPtr native) noexcept;
          ZS_NO_DISCARD() static NativeTypeUniPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

