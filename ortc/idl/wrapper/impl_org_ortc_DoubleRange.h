
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

          DoubleRange();
          virtual ~DoubleRange();

          // methods DoubleRange
          virtual void wrapper_init_org_ortc_DoubleRange() override;
          virtual void wrapper_init_org_ortc_DoubleRange(double value) override;
          virtual void wrapper_init_org_ortc_DoubleRange(
            double min,
            double max
            ) override;
          virtual void wrapper_init_org_ortc_DoubleRange(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;


          static DoubleRangePtr toWrapper(NativeCapabilityDoublePtr native);
          static DoubleRangePtr toWrapper(const NativeCapabilityDouble &native);
          static NativeCapabilityDoublePtr toNative(wrapper::org::ortc::DoubleRangePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

