
#ifndef WRAPPER_USE_GENERATED_ORG_ORTC_DISPATCHER

#pragma once

#include <wrapper/types.h>

namespace wrapper {
  namespace org {
    namespace ortc {

      struct Dispatcher
      {
        static DispatcherPtr wrapper_create() noexcept;
        virtual ~Dispatcher() noexcept {}


        virtual void wrapper_init_org_ortc_Dispatcher(AnyPtr source) noexcept { ZS_MAYBE_USED(source); }

        static AnyPtr get_source() noexcept;
        static void set_source(AnyPtr value) noexcept;
      };

    } // ortc
  } // org
} // namespace wrapper

#endif // WRAPPER_USE_GENERATED_ORG_ORTC_DISPATCHER
