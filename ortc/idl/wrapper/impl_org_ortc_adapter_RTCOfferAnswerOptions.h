
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCOfferAnswerOptions.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCOfferAnswerOptions : public wrapper::org::ortc::adapter::RTCOfferAnswerOptions
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnectionTypes::OfferAnswerOptions, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCOfferAnswerOptions, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCOfferAnswerOptions, WrapperType);
            RTCOfferAnswerOptionsWeakPtr thisWeak_;

            RTCOfferAnswerOptions();
            virtual ~RTCOfferAnswerOptions();

            static WrapperImplTypePtr toWrapper(NativeTypePtr native);
            static WrapperImplTypePtr toWrapper(const NativeType &native);
            static NativeTypePtr toNative(WrapperTypePtr wrapper);
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

