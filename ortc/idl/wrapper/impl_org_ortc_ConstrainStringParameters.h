
#pragma once

#include "types.h"
#include "generated/org_ortc_ConstrainStringParameters.h"

#include <ortc/IConstraints.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ConstrainStringParameters : public wrapper::org::ortc::ConstrainStringParameters
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IConstraints::ConstrainStringParameters, NativeConstrainStringParameters);

          ConstrainStringParametersWeakPtr thisWeak_;

          ConstrainStringParameters() noexcept;
          virtual ~ConstrainStringParameters() noexcept;

          // methods ConstrainStringParameters
          void wrapper_init_org_ortc_ConstrainStringParameters() noexcept override;
          void wrapper_init_org_ortc_ConstrainStringParameters(wrapper::org::ortc::ConstrainStringParametersPtr source) noexcept override;
          void wrapper_init_org_ortc_ConstrainStringParameters(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static ConstrainStringParametersPtr toWrapper(NativeConstrainStringParametersPtr native) noexcept;
          static ConstrainStringParametersPtr toWrapper(const NativeConstrainStringParameters &native) noexcept;
          static NativeConstrainStringParametersPtr toNative(wrapper::org::ortc::ConstrainStringParametersPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

