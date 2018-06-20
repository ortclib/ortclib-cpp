
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDtlsFingerprint.h"

#include <ortc/ICertificate.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDtlsFingerprint : public wrapper::org::ortc::RTCDtlsFingerprint
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICertificateTypes::Fingerprint, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCDtlsFingerprint, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDtlsFingerprint, WrapperType);
          RTCDtlsFingerprintWeakPtr thisWeak_;

          RTCDtlsFingerprint() noexcept;
          virtual ~RTCDtlsFingerprint() noexcept;

          // methods RTCDtlsFingerprint
          virtual void wrapper_init_org_ortc_RTCDtlsFingerprint() noexcept override;
          virtual void wrapper_init_org_ortc_RTCDtlsFingerprint(wrapper::org::ortc::RTCDtlsFingerprintPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCDtlsFingerprint(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;
          
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

