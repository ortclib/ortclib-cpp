
#pragma once

#include "types.h"
#include "generated/org_ortc_Dispatcher.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct Dispatcher : public wrapper::org::ortc::Dispatcher
        {
          DispatcherWeakPtr thisWeak_;
          AnyPtr native_;

          Dispatcher() noexcept;
          virtual ~Dispatcher() noexcept;

          // methods Dispatcher
          void wrapper_init_org_ortc_Dispatcher(AnyPtr source) noexcept override;

          static AnyPtr toNative(wrapper::org::ortc::DispatcherPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

