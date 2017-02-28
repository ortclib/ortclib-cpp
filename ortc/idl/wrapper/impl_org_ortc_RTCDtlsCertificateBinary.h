
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

          RTCDtlsCertificateBinary();
          virtual ~RTCDtlsCertificateBinary();

          // properties RTCDtlsCertificateBinary
          virtual SecureByteBlockPtr get_certificate() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

