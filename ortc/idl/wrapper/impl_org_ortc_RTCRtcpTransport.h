
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtcpTransport.h"

#include <ortc/types.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtcpTransport : public wrapper::org::ortc::RTCRtcpTransport
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTCPTransport, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtcpTransport, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtcpTransport, WrapperType);
          RTCRtcpTransportWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCRtcpTransport();
          virtual ~RTCRtcpTransport();

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

