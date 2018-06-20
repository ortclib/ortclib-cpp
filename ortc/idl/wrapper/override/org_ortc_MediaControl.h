
#ifndef WRAPPER_USE_GENERATED_ORG_ORTC_MEDIACONTROL

#pragma once

#include <wrapper/types.h>

namespace wrapper {
  namespace org {
    namespace ortc {

      struct MediaControl
      {
        virtual ~MediaControl() noexcept {}


        static AnyPtr get_displayOrientation() noexcept;
        static void set_displayOrientation(AnyPtr value) noexcept;
      };

    } // ortc
  } // org
} // namespace wrapper

#endif //ifndef WRAPPER_USE_GENERATED_ORG_ORTC_MEDIACONTROL
