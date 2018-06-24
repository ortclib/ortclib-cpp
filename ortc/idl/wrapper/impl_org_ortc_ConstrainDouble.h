
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

          ConstrainDouble() noexcept;
          virtual ~ConstrainDouble() noexcept;

          // methods ConstrainDouble
          void wrapper_init_org_ortc_ConstrainDouble() noexcept override;
          void wrapper_init_org_ortc_ConstrainDouble(wrapper::org::ortc::ConstrainDoublePtr source) noexcept override;
          void wrapper_init_org_ortc_ConstrainDouble(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static ConstrainDoublePtr toWrapper(NativeConstrainDoublePtr native) noexcept;
          static ConstrainDoublePtr toWrapper(const NativeConstrainDouble &native) noexcept;
          static NativeConstrainDoublePtr toNative(wrapper::org::ortc::ConstrainDoublePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

