
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCAnswerOptions.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCAnswerOptions : public wrapper::org::ortc::adapter::RTCAnswerOptions
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnectionTypes::AnswerOptions, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCAnswerOptions, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCAnswerOptions, WrapperType);
            RTCAnswerOptionsWeakPtr thisWeak_;

            RTCAnswerOptions() noexcept;
            virtual ~RTCAnswerOptions() noexcept;
            void wrapper_init_org_ortc_adapter_RTCAnswerOptions() noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

