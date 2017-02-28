
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

          ConstrainString();
          virtual ~ConstrainString();

          // methods ConstrainString
          virtual void wrapper_init_org_ortc_ConstrainString() override;
          virtual void wrapper_init_org_ortc_ConstrainString(wrapper::org::ortc::ConstrainStringPtr source) override;
          virtual void wrapper_init_org_ortc_ConstrainString(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static ConstrainStringPtr toWrapper(NativeConstrainStringPtr native);
          static ConstrainStringPtr toWrapper(const NativeConstrainString &native);
          static NativeConstrainStringPtr toNative(wrapper::org::ortc::ConstrainStringPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

