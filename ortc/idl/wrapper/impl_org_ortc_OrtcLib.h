
#pragma once

#include "types.h"
#include "generated/org_ortc_OrtcLib.h"

#include <ortc/IORTC.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct OrtcLib : public wrapper::org::ortc::OrtcLib
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IORTC, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(OrtcLib, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::OrtcLib, WrapperType);

          virtual ~OrtcLib();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

