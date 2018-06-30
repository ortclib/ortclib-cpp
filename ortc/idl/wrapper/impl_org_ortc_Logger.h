
#pragma once

#include "types.h"
#include "generated/org_ortc_Logger.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct Logger : public wrapper::org::ortc::Logger
        {
          Logger() noexcept = delete;
          Logger(const Logger &) noexcept = delete;
          virtual ~Logger() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

