
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCCapabilityOptions.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCCapabilityOptions : public wrapper::org::ortc::adapter::RTCCapabilityOptions
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnectionTypes::CapabilityOptions, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCCapabilityOptions, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCCapabilityOptions, WrapperType);
            RTCCapabilityOptionsWeakPtr thisWeak_;

            RTCCapabilityOptions();
            virtual ~RTCCapabilityOptions();
            virtual void wrapper_init_org_ortc_adapter_RTCCapabilityOptions() override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static WrapperImplTypePtr toWrapper(const NativeType &native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

