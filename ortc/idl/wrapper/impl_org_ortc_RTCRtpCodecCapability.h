
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

          RTCRtpCodecCapability();
          virtual ~RTCRtpCodecCapability();

          // methods RTCRtpCodecCapability
          virtual void wrapper_init_org_ortc_RTCRtpCodecCapability() override;
          virtual void wrapper_init_org_ortc_RTCRtpCodecCapability(wrapper::org::ortc::RTCRtpCodecCapabilityPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpCodecCapability(wrapper::org::ortc::JsonPtr json) override;
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

