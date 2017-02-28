
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

          RTCCertificate();
          virtual ~RTCCertificate();

          // properties RTCCertificate
          virtual uint64_t get_objectId() override;
          virtual ::zsLib::Time get_expires() override;
          virtual wrapper::org::ortc::RTCDtlsFingerprintPtr get_fingerprint() override;

          static PromiseWithTypePtr toWrapper(NativePromiseWithCertificatePtr promise);
          static RTCCertificatePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(wrapper::org::ortc::RTCCertificatePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

