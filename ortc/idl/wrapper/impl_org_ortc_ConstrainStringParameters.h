
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

          ConstrainStringParameters();
          virtual ~ConstrainStringParameters();

          // methods ConstrainStringParameters
          virtual void wrapper_init_org_ortc_ConstrainStringParameters() override;
          virtual void wrapper_init_org_ortc_ConstrainStringParameters(wrapper::org::ortc::ConstrainStringParametersPtr source) override;
          virtual void wrapper_init_org_ortc_ConstrainStringParameters(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static ConstrainStringParametersPtr toWrapper(NativeConstrainStringParametersPtr native);
          static ConstrainStringParametersPtr toWrapper(const NativeConstrainStringParameters &native);
          static NativeConstrainStringParametersPtr toNative(wrapper::org::ortc::ConstrainStringParametersPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

