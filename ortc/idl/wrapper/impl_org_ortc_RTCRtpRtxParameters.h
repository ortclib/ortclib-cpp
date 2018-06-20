
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpRtxParameters.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpRtxParameters : public wrapper::org::ortc::RTCRtpRtxParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::RTXParameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpRtxParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpRtxParameters, WrapperType);
          RTCRtpRtxParametersWeakPtr thisWeak_;

          RTCRtpRtxParameters() noexcept;
          virtual ~RTCRtpRtxParameters() noexcept;

          // methods RTCRtpRtxParameters
          virtual void wrapper_init_org_ortc_RTCRtpRtxParameters() noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxParameters(wrapper::org::ortc::RTCRtpRtxParametersPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_RTCRtpRtxParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

