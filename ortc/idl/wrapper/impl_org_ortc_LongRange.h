
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

          LongRange() noexcept;
          virtual ~LongRange() noexcept;

          // methods LongRange
          virtual void wrapper_init_org_ortc_LongRange() noexcept override;
          virtual void wrapper_init_org_ortc_LongRange(long value) noexcept override;
          virtual void wrapper_init_org_ortc_LongRange(
            long min,
            long max
            ) noexcept override;
          virtual void wrapper_init_org_ortc_LongRange(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static LongRangePtr toWrapper(NativeCapabilityLongPtr native) noexcept;
          static LongRangePtr toWrapper(const NativeCapabilityLong &native) noexcept;
          static NativeCapabilityLongPtr toNative(wrapper::org::ortc::LongRangePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

