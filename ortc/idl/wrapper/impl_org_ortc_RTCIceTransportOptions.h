
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

          RTCIceTransportOptions() noexcept;
          virtual ~RTCIceTransportOptions() noexcept;

          // methods RTCIceTransportOptions
          virtual void wrapper_init_org_ortc_RTCIceTransportOptions() noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceTransportOptions(wrapper::org::ortc::RTCIceTransportOptionsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCIceTransportOptions(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

