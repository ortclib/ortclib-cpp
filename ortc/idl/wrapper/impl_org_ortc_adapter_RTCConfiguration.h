
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCConfiguration.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCConfiguration : public wrapper::org::ortc::adapter::RTCConfiguration
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnectionTypes::Configuration, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCConfiguration, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCConfiguration, WrapperType);
            RTCConfigurationWeakPtr thisWeak_;

            RTCConfiguration();
            virtual ~RTCConfiguration();

            // methods RTCConfiguration
            virtual void wrapper_init_org_ortc_adapter_RTCConfiguration() override;
            virtual void wrapper_init_org_ortc_adapter_RTCConfiguration(wrapper::org::ortc::adapter::RTCConfigurationPtr source) override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static WrapperImplTypePtr toWrapper(const NativeType &native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

