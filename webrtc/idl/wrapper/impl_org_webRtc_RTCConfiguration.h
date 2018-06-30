
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCConfiguration.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCConfiguration : public wrapper::org::webRtc::RTCConfiguration
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCConfiguration, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCConfiguration, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::RTCConfiguration, NativeType);

          RTCConfigurationWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCConfiguration() noexcept;
          virtual ~RTCConfiguration() noexcept;

          // methods RTCConfiguration
          void wrapper_init_org_webRtc_RTCConfiguration() noexcept override;
          void wrapper_init_org_webRtc_RTCConfiguration(wrapper::org::webRtc::RTCConfigurationType type) noexcept override;
          void wrapper_init_org_webRtc_RTCConfiguration(wrapper::org::webRtc::RTCConfigurationPtr source) noexcept override;

          // properties RTCConfiguration
          bool get_dscp() noexcept override;
          void set_dscp(bool value) noexcept override;
          bool get_cpuAdaptation() noexcept override;
          void set_cpuAdaptation(bool value) noexcept override;
          bool get_suspendBelowMinBitrate() noexcept override;
          void set_suspendBelowMinBitrate(bool value) noexcept override;
          bool get_prerendererSmoothing() noexcept override;
          void set_prerendererSmoothing(bool value) noexcept override;
          bool get_experimentCpuLoadEstimator() noexcept override;
          void set_experimentCpuLoadEstimator(bool value) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

