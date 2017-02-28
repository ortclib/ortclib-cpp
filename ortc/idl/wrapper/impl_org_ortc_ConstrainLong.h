
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

          ConstrainLong();
          virtual ~ConstrainLong();

          // methods ConstrainLong
          virtual void wrapper_init_org_ortc_ConstrainLong() override;
          virtual void wrapper_init_org_ortc_ConstrainLong(wrapper::org::ortc::ConstrainLongPtr source) override;
          virtual void wrapper_init_org_ortc_ConstrainLong(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static ConstrainLongPtr toWrapper(NativeConstrainLongPtr native);
          static ConstrainLongPtr toWrapper(const NativeConstrainLong &native);
          static NativeConstrainLongPtr toNative(wrapper::org::ortc::ConstrainLongPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

