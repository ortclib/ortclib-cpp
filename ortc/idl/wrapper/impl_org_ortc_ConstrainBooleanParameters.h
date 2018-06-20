
#pragma once

#include "types.h"
#include "generated/org_ortc_ConstrainBooleanParameters.h"

#include <ortc/IConstraints.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ConstrainBooleanParameters : public wrapper::org::ortc::ConstrainBooleanParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IConstraints::ConstrainBooleanParameters, NativeConstrainBooleanParameters);

          ConstrainBooleanParametersWeakPtr thisWeak_;

          ConstrainBooleanParameters() noexcept;
          virtual ~ConstrainBooleanParameters() noexcept;

          // methods ConstrainBooleanParameters
          virtual void wrapper_init_org_ortc_ConstrainBooleanParameters() noexcept override;
          virtual void wrapper_init_org_ortc_ConstrainBooleanParameters(wrapper::org::ortc::ConstrainBooleanParametersPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_ConstrainBooleanParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static ConstrainBooleanParametersPtr toWrapper(NativeConstrainBooleanParametersPtr native) noexcept;
          static ConstrainBooleanParametersPtr toWrapper(const NativeConstrainBooleanParameters &native) noexcept;
          static NativeConstrainBooleanParametersPtr toNative(wrapper::org::ortc::ConstrainBooleanParametersPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

