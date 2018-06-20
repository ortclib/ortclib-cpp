
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

          RTCRtpOpusCodecCapabilityOptions() noexcept;
          virtual ~RTCRtpOpusCodecCapabilityOptions() noexcept;

          // methods RTCRtpOpusCodecCapabilityOptions
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpOpusCodecCapabilityOptions(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(AnyPtr native) noexcept;
          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

