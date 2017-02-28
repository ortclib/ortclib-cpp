
#pragma once

#include "types.h"
#include "generated/org_ortc_ConstrainDouble.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ConstrainDouble : public wrapper::org::ortc::ConstrainDouble
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::ConstrainDouble, NativeConstrainDouble);

          ConstrainDoubleWeakPtr thisWeak_;

          ConstrainDouble();
          virtual ~ConstrainDouble();

          // methods ConstrainDouble
          virtual void wrapper_init_org_ortc_ConstrainDouble() override;
          virtual void wrapper_init_org_ortc_ConstrainDouble(wrapper::org::ortc::ConstrainDoublePtr source) override;
          virtual void wrapper_init_org_ortc_ConstrainDouble(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static ConstrainDoublePtr toWrapper(NativeConstrainDoublePtr native);
          static ConstrainDoublePtr toWrapper(const NativeConstrainDouble &native);
          static NativeConstrainDoublePtr toNative(wrapper::org::ortc::ConstrainDoublePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

