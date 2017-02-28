
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

          OverconstrainedErrorEvent();
          virtual ~OverconstrainedErrorEvent();
          virtual void wrapper_init_org_ortc_OverconstrainedErrorEvent() override;

          // properties OverconstrainedErrorEvent
          virtual wrapper::org::ortc::OverconstrainedErrorPtr get_error() override;

          static OverconstrainedErrorEventPtr toWrapper(OverconstrainedErrorPtr error);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

