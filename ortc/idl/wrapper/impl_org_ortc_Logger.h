
#pragma once

#include "types.h"
#include "generated/org_ortc_Logger.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct Logger : public wrapper::org::ortc::Logger
        {
          virtual ~Logger() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

