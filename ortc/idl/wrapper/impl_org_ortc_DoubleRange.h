
#pragma once

#include "types.h"
#include "generated/org_ortc_DoubleRange.h"

#include <ortc/ICapabilities.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct DoubleRange : public wrapper::org::ortc::DoubleRange
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICapabilities::CapabilityDouble, NativeCapabilityDouble);
          DoubleRangeWeakPtr thisWeak_;

          DoubleRange() noexcept;
          virtual ~DoubleRange() noexcept;

          // methods DoubleRange
          void wrapper_init_org_ortc_DoubleRange() noexcept override;
          void wrapper_init_org_ortc_DoubleRange(double value) noexcept override;
          void wrapper_init_org_ortc_DoubleRange(
            double minimum,
            double maximum
            ) noexcept override;
          void wrapper_init_org_ortc_DoubleRange(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static DoubleRangePtr toWrapper(NativeCapabilityDoublePtr native) noexcept;
          static DoubleRangePtr toWrapper(const NativeCapabilityDouble &native) noexcept;
          static NativeCapabilityDoublePtr toNative(wrapper::org::ortc::DoubleRangePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

