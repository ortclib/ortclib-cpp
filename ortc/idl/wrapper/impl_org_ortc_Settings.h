
#pragma once

#include "types.h"
#include "generated/org_ortc_Settings.h"

#include <zsLib/ISettings.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct Settings : public wrapper::org::ortc::Settings
        {
          ZS_DECLARE_TYPEDEF_PTR(::zsLib::ISettings, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(OrtcLib, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::OrtcLib, WrapperType);

          virtual ~Settings();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

