
#pragma once

#include "types.h"
#include "generated/org_ortc_OrtcLibWithDispatcher.h"

#include <ortc/IORTC.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct OrtcLibWithDispatcher : public wrapper::org::ortc::OrtcLibWithDispatcher
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IORTC, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(OrtcLibWithDispatcher, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::OrtcLibWithDispatcher, WrapperType);

          virtual ~OrtcLibWithDispatcher();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

