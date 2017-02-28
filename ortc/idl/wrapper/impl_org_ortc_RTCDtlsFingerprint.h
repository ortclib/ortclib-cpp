
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

          RTCDtlsFingerprint();
          virtual ~RTCDtlsFingerprint();

          // methods RTCDtlsFingerprint
          virtual void wrapper_init_org_ortc_RTCDtlsFingerprint() override;
          virtual void wrapper_init_org_ortc_RTCDtlsFingerprint(wrapper::org::ortc::RTCDtlsFingerprintPtr source) override;
          virtual void wrapper_init_org_ortc_RTCDtlsFingerprint(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;
          
          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

