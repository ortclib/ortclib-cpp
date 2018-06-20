
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCOfferOptions.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCOfferOptions : public wrapper::org::ortc::adapter::RTCOfferOptions
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnectionTypes::OfferOptions, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCOfferOptions, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCOfferOptions, WrapperType);
            RTCOfferOptionsWeakPtr thisWeak_;

            RTCOfferOptions() noexcept;
            virtual ~RTCOfferOptions() noexcept;
            void wrapper_init_org_ortc_adapter_RTCOfferOptions() noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

