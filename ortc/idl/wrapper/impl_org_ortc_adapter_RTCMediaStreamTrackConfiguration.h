
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

            RTCMediaStreamTrackConfiguration() noexcept;
            virtual ~RTCMediaStreamTrackConfiguration() noexcept;

            // methods RTCMediaStreamTrackConfiguration
            void wrapper_init_org_ortc_adapter_RTCMediaStreamTrackConfiguration() noexcept override;
            void wrapper_init_org_ortc_adapter_RTCMediaStreamTrackConfiguration(wrapper::org::ortc::adapter::RTCMediaStreamTrackConfigurationPtr source) noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

