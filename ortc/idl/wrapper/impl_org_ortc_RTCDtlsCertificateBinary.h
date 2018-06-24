
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDtlsCertificateBinary.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDtlsCertificateBinary : public wrapper::org::ortc::RTCDtlsCertificateBinary
        {
          ZS_DECLARE_TYPEDEF_PTR(::zsLib::eventing::SecureByteBlock, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCDtlsCertificateBinary, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDtlsCertificateBinary, WrapperType);
          RTCDtlsCertificateBinaryWeakPtr thisWeak_;
          SecureByteBlockPtr native_;

          RTCDtlsCertificateBinary() noexcept;
          virtual ~RTCDtlsCertificateBinary() noexcept;

          // properties RTCDtlsCertificateBinary
          SecureByteBlockPtr get_certificate() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

