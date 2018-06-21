
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCCertificate.h"

#include <ortc/ICertificate.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCCertificate : public wrapper::org::ortc::RTCCertificate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICertificate, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(PromiseWithHolderPtr< wrapper::org::ortc::RTCCertificatePtr >, PromiseWithType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICertificateTypes::PromiseWithCertificate, NativePromiseWithCertificate);

          RTCCertificateWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCCertificate() noexcept;
          virtual ~RTCCertificate() noexcept;

          // properties RTCCertificate
          uint64_t get_objectId() noexcept override;
          ::zsLib::Time get_expires() noexcept override;
          wrapper::org::ortc::RTCDtlsFingerprintPtr get_fingerprint() noexcept override;

          static PromiseWithTypePtr toWrapper(NativePromiseWithCertificatePtr promise) noexcept;
          static RTCCertificatePtr toWrapper(NativeTypePtr native) noexcept;
          static NativeTypePtr toNative(wrapper::org::ortc::RTCCertificatePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

