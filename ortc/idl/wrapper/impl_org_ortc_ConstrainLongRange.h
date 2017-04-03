
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

          ConstrainLongRange();
          virtual ~ConstrainLongRange();

          // methods ConstrainLongRange
          virtual void wrapper_init_org_ortc_ConstrainLongRange() override;
          virtual void wrapper_init_org_ortc_ConstrainLongRange(wrapper::org::ortc::ConstrainLongRangePtr source) override;
          virtual void wrapper_init_org_ortc_ConstrainLongRange(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static ConstrainLongRangePtr toWrapper(NativeConstrainLongRangePtr native);
          static ConstrainLongRangePtr toWrapper(const NativeConstrainLongRange &native);
          static NativeConstrainLongRangePtr toNative(wrapper::org::ortc::ConstrainLongRangePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

