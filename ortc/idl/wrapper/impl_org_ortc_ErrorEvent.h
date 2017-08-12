
#pragma once

#include "types.h"
#include "generated/org_ortc_ErrorEvent.h"

#include <ortc/types.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct ErrorEvent : public wrapper::org::ortc::ErrorEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(ErrorEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::ErrorEvent, WrapperType);
          ErrorEventWeakPtr thisWeak_;
          wrapper::org::ortc::ErrorPtr error_;

          ErrorEvent();
          virtual ~ErrorEvent();

          // properties ErrorEvent
          virtual wrapper::org::ortc::ErrorPtr get_error() override;

          static WrapperImplTypePtr toWrapper(::ortc::ErrorAnyPtr error);
          static WrapperImplTypePtr toWrapper(wrapper::org::ortc::ErrorPtr error);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

