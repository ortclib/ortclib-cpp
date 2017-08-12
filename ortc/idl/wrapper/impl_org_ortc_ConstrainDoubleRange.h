
#pragma once

#include "types.h"
#include "generated/org_ortc_ConstrainDoubleRange.h"

#include <ortc/IConstraints.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ConstrainDoubleRange : public wrapper::org::ortc::ConstrainDoubleRange
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IConstraints::ConstrainDoubleRange, NativeConstrainDoubleRange);
          ConstrainDoubleRangeWeakPtr thisWeak_;

          ConstrainDoubleRange();
          virtual ~ConstrainDoubleRange();

          // methods ConstrainDoubleRange
          virtual void wrapper_init_org_ortc_ConstrainDoubleRange() override;
          virtual void wrapper_init_org_ortc_ConstrainDoubleRange(wrapper::org::ortc::ConstrainDoubleRangePtr source) override;
          virtual void wrapper_init_org_ortc_ConstrainDoubleRange(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static ConstrainDoubleRangePtr toWrapper(NativeConstrainDoubleRangePtr native);
          static ConstrainDoubleRangePtr toWrapper(const NativeConstrainDoubleRange &native);
          static NativeConstrainDoubleRangePtr toNative(wrapper::org::ortc::ConstrainDoubleRangePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

