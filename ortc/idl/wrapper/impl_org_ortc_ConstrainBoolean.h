
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

          ConstrainBoolean() noexcept;
          virtual ~ConstrainBoolean() noexcept;

          // methods ConstrainBoolean
          void wrapper_init_org_ortc_ConstrainBoolean() noexcept override;
          void wrapper_init_org_ortc_ConstrainBoolean(wrapper::org::ortc::ConstrainBooleanPtr source) noexcept override;
          void wrapper_init_org_ortc_ConstrainBoolean(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static ConstrainBooleanPtr toWrapper(NativeConstrainBooleanPtr native) noexcept;
          static ConstrainBooleanPtr toWrapper(const NativeConstrainBoolean &native) noexcept;
          static NativeConstrainBooleanPtr toNative(wrapper::org::ortc::ConstrainBooleanPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

