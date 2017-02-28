
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCMediaStreamTrackConfiguration.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCMediaStreamTrackConfiguration : public wrapper::org::ortc::adapter::RTCMediaStreamTrackConfiguration
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnectionTypes::MediaStreamTrackConfiguration, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCMediaStreamTrackConfiguration, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCMediaStreamTrackConfiguration, WrapperType);
            RTCMediaStreamTrackConfigurationWeakPtr thisWeak_;
            NativeTypePtr native_;

            RTCMediaStreamTrackConfiguration();
            virtual ~RTCMediaStreamTrackConfiguration();

            // methods RTCMediaStreamTrackConfiguration
            virtual void wrapper_init_org_ortc_adapter_RTCMediaStreamTrackConfiguration() override;
            virtual void wrapper_init_org_ortc_adapter_RTCMediaStreamTrackConfiguration(wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr source) override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static WrapperImplTypePtr toWrapper(const NativeType &native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

