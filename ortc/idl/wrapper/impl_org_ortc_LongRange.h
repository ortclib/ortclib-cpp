
#pragma once

#include "types.h"
#include "generated/org_ortc_LongRange.h"


#include <ortc/ICapabilities.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct LongRange : public wrapper::org::ortc::LongRange
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICapabilities::CapabilityLong, NativeCapabilityLong);
          LongRangeWeakPtr thisWeak_;

          LongRange();
          virtual ~LongRange();

          // methods LongRange
          virtual void wrapper_init_org_ortc_LongRange() override;
          virtual void wrapper_init_org_ortc_LongRange(long value) override;
          virtual void wrapper_init_org_ortc_LongRange(
            long min,
            long max
            ) override;
          virtual void wrapper_init_org_ortc_LongRange(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static LongRangePtr toWrapper(NativeCapabilityLongPtr native);
          static LongRangePtr toWrapper(const NativeCapabilityLong &native);
          static NativeCapabilityLongPtr toNative(wrapper::org::ortc::LongRangePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

