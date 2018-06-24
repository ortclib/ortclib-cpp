
#pragma once

#include "types.h"
#include "generated/org_ortc_ConstrainString.h"

#include <ortc/IConstraints.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ConstrainString : public wrapper::org::ortc::ConstrainString
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IConstraints::ConstrainString, NativeConstrainString);

          ConstrainStringWeakPtr thisWeak_;

          ConstrainString() noexcept;
          virtual ~ConstrainString() noexcept;

          // methods ConstrainString
          void wrapper_init_org_ortc_ConstrainString() noexcept override;
          void wrapper_init_org_ortc_ConstrainString(wrapper::org::ortc::ConstrainStringPtr source) noexcept override;
          void wrapper_init_org_ortc_ConstrainString(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static ConstrainStringPtr toWrapper(NativeConstrainStringPtr native) noexcept;
          static ConstrainStringPtr toWrapper(const NativeConstrainString &native) noexcept;
          static NativeConstrainStringPtr toNative(wrapper::org::ortc::ConstrainStringPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

