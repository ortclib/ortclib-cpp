
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDataChannelParameters.h"

#include <ortc/IDataChannel.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDataChannelParameters : public wrapper::org::ortc::RTCDataChannelParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataChannelTypes::Parameters, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCDataChannelParameters, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDataChannelParameters, WrapperType);
          RTCDataChannelParametersWeakPtr thisWeak_;

          RTCDataChannelParameters() noexcept;
          virtual ~RTCDataChannelParameters() noexcept;

          // methods RTCDataChannelParameters
          void wrapper_init_org_ortc_RTCDataChannelParameters() noexcept override;
          void wrapper_init_org_ortc_RTCDataChannelParameters(wrapper::org::ortc::RTCDataChannelParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCDataChannelParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
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

