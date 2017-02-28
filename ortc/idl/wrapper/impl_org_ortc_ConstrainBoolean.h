
#pragma once

#include "types.h"
#include "generated/org_ortc_ConstrainBoolean.h"

#include <ortc/IConstraints.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ConstrainBoolean : public wrapper::org::ortc::ConstrainBoolean
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IConstraints::ConstrainBoolean, NativeConstrainBoolean);

          ConstrainBooleanWeakPtr thisWeak_;

          ConstrainBoolean();
          virtual ~ConstrainBoolean();

          // methods ConstrainBoolean
          virtual void wrapper_init_org_ortc_ConstrainBoolean() override;
          virtual void wrapper_init_org_ortc_ConstrainBoolean(wrapper::org::ortc::ConstrainBooleanPtr source) override;
          virtual void wrapper_init_org_ortc_ConstrainBoolean(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static ConstrainBooleanPtr toWrapper(NativeConstrainBooleanPtr native);
          static ConstrainBooleanPtr toWrapper(const NativeConstrainBoolean &native);
          static NativeConstrainBooleanPtr toNative(wrapper::org::ortc::ConstrainBooleanPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

