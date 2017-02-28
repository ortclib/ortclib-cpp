
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDataTransport.h"

#include <ortc/types.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDataTransport : public wrapper::org::ortc::RTCDataTransport
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataTransport, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCDataTransport, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDataTransport, WrapperType);
          RTCDataTransportWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCDataTransport();
          virtual ~RTCDataTransport();

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

