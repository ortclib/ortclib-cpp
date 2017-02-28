
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpOpusCodecCapabilityOptions.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpOpusCodecCapabilityOptions : public wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::OpusCodecCapabilityOptions, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpOpusCodecCapabilityOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptions, WrapperType);
          RTCRtpOpusCodecCapabilityOptionsWeakPtr thisWeak_;

          RTCRtpOpusCodecCapabilityOptions();
          virtual ~RTCRtpOpusCodecCapabilityOptions();

          // methods RTCRtpOpusCodecCapabilityOptions
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions() override;
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsPtr source) override;
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static WrapperImplTypePtr toWrapper(AnyPtr native);
          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

