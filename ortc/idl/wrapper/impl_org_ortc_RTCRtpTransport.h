
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpTransport.h"

#include <ortc/types.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpTransport : public wrapper::org::ortc::RTCRtpTransport
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTransport, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpTransport, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpTransport, WrapperType);
          RTCRtpTransportWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCRtpTransport() noexcept;
          virtual ~RTCRtpTransport() noexcept;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

