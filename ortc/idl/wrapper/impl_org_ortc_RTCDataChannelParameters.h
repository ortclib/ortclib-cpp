
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

          RTCDataChannelParameters();
          virtual ~RTCDataChannelParameters();

          // methods RTCDataChannelParameters
          virtual void wrapper_init_org_ortc_RTCDataChannelParameters() override;
          virtual void wrapper_init_org_ortc_RTCDataChannelParameters(wrapper::org::ortc::RTCDataChannelParametersPtr source) override;
          virtual void wrapper_init_org_ortc_RTCDataChannelParameters(wrapper::org::ortc::JsonPtr json) override;
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

