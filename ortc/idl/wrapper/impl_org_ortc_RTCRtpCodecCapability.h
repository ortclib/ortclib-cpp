
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpCodecCapability.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpCodecCapability : public wrapper::org::ortc::RTCRtpCodecCapability
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::CodecCapability, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpCodecCapability, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpCodecCapability, WrapperType);
          RTCRtpCodecCapabilityWeakPtr thisWeak_;

          RTCRtpCodecCapability() noexcept;
          virtual ~RTCRtpCodecCapability() noexcept;

          // methods RTCRtpCodecCapability
          void wrapper_init_org_ortc_RTCRtpCodecCapability() noexcept override;
          void wrapper_init_org_ortc_RTCRtpCodecCapability(wrapper::org::ortc::RTCRtpCodecCapabilityPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCRtpCodecCapability(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

