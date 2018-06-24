
#pragma once

#include "types.h"
#include "generated/org_ortc_OverconstrainedErrorEvent.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct OverconstrainedErrorEvent : public wrapper::org::ortc::OverconstrainedErrorEvent
        {
          OverconstrainedErrorEventWeakPtr thisWeak_;
          OverconstrainedErrorPtr error_;

          OverconstrainedErrorEvent() noexcept;
          virtual ~OverconstrainedErrorEvent() noexcept;
          void wrapper_init_org_ortc_OverconstrainedErrorEvent() noexcept override;

          // properties OverconstrainedErrorEvent
          wrapper::org::ortc::OverconstrainedErrorPtr get_error() noexcept override;

          static OverconstrainedErrorEventPtr toWrapper(OverconstrainedErrorPtr error) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

