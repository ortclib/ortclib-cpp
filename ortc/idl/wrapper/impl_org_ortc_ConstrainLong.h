
#pragma once

#include "types.h"
#include "generated/org_ortc_ConstrainLong.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ConstrainLong : public wrapper::org::ortc::ConstrainLong
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::ConstrainLong, NativeConstrainLong);

          ConstrainLongWeakPtr thisWeak_;

          ConstrainLong() noexcept;
          virtual ~ConstrainLong() noexcept;

          // methods ConstrainLong
          virtual void wrapper_init_org_ortc_ConstrainLong() noexcept override;
          virtual void wrapper_init_org_ortc_ConstrainLong(wrapper::org::ortc::ConstrainLongPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_ConstrainLong(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static ConstrainLongPtr toWrapper(NativeConstrainLongPtr native) noexcept;
          static ConstrainLongPtr toWrapper(const NativeConstrainLong &native) noexcept;
          static NativeConstrainLongPtr toNative(wrapper::org::ortc::ConstrainLongPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

