
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

          ConstrainBooleanParameters();
          virtual ~ConstrainBooleanParameters();

          // methods ConstrainBooleanParameters
          virtual void wrapper_init_org_ortc_ConstrainBooleanParameters() override;
          virtual void wrapper_init_org_ortc_ConstrainBooleanParameters(wrapper::org::ortc::ConstrainBooleanParametersPtr source) override;
          virtual void wrapper_init_org_ortc_ConstrainBooleanParameters(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static ConstrainBooleanParametersPtr toWrapper(NativeConstrainBooleanParametersPtr native);
          static ConstrainBooleanParametersPtr toWrapper(const NativeConstrainBooleanParameters &native);
          static NativeConstrainBooleanParametersPtr toNative(wrapper::org::ortc::ConstrainBooleanParametersPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

