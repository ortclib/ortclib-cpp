
#pragma once

#include "types.h"
#include "generated/org_ortc_ConstrainLongRange.h"

#include <ortc/IConstraints.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ConstrainLongRange : public wrapper::org::ortc::ConstrainLongRange
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IConstraints::ConstrainLongRange, NativeConstrainLongRange);

          ConstrainLongRangeWeakPtr thisWeak_;

          ConstrainLongRange() noexcept;
          virtual ~ConstrainLongRange() noexcept;

          // methods ConstrainLongRange
          virtual void wrapper_init_org_ortc_ConstrainLongRange() noexcept override;
          virtual void wrapper_init_org_ortc_ConstrainLongRange(wrapper::org::ortc::ConstrainLongRangePtr source) noexcept override;
          virtual void wrapper_init_org_ortc_ConstrainLongRange(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static ConstrainLongRangePtr toWrapper(NativeConstrainLongRangePtr native) noexcept;
          static ConstrainLongRangePtr toWrapper(const NativeConstrainLongRange &native) noexcept;
          static NativeConstrainLongRangePtr toNative(wrapper::org::ortc::ConstrainLongRangePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

