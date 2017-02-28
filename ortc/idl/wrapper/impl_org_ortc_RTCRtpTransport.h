
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

          RTCRtpTransport();
          virtual ~RTCRtpTransport();

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

