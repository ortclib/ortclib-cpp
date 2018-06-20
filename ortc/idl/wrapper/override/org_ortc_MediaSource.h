
#ifndef WRAPPER_USE_GENERATED_ORG_ORTC_MEDIASOURCE

#pragma once

#include <wrapper/types.h>

namespace wrapper {
  namespace org {
    namespace ortc {

      struct MediaSource
      {
        static MediaSourcePtr wrapper_create() noexcept;
        virtual ~MediaSource() noexcept {}

        virtual void wrapper_init_org_ortc_MediaSource() noexcept {}

        virtual AnyPtr get_source() noexcept = 0;
        virtual void set_source(AnyPtr value) noexcept = 0;
        virtual AnyPtr get_track() noexcept = 0;
      };

    } // ortc
  } // org
} // namespace wrapper

#endif // WRAPPER_USE_GENERATED_ORG_ORTC_MEDIASOURCE
