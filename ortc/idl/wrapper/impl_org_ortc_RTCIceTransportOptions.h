
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceTransportOptions.h"

#include <ortc/IICETransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceTransportOptions : public wrapper::org::ortc::RTCIceTransportOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IICETransportTypes::Options, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCIceTransportOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCIceTransportOptions, WrapperType);
          RTCIceTransportOptionsWeakPtr thisWeak_;

          RTCIceTransportOptions();
          virtual ~RTCIceTransportOptions();

          // methods RTCIceTransportOptions
          virtual void wrapper_init_org_ortc_RTCIceTransportOptions() override;
          virtual void wrapper_init_org_ortc_RTCIceTransportOptions(wrapper::org::ortc::RTCIceTransportOptionsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceTransportOptions(wrapper::org::ortc::JsonPtr json) override;
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

