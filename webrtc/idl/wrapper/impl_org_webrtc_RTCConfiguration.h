
#pragma once

#include "types.h"
#include "generated/org_webrtc_RTCConfiguration.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/peerconnectioninterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct RTCConfiguration : public wrapper::org::webrtc::RTCConfiguration
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::RTCConfiguration, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::RTCConfiguration, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::PeerConnectionInterface::RTCConfiguration, NativeType);

          RTCConfigurationWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCConfiguration() noexcept;
          virtual ~RTCConfiguration() noexcept;

          // methods RTCConfiguration
          void wrapper_init_org_webrtc_RTCConfiguration() noexcept override;
          void wrapper_init_org_webrtc_RTCConfiguration(wrapper::org::webrtc::RTCConfigurationPtr source) noexcept override;

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

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

