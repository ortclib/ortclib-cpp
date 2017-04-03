
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIdentityResult.h"

#include <ortc/IIdentity.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIdentityResult : public wrapper::org::ortc::RTCIdentityResult
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IIdentityTypes::Result, NativeResult);
          RTCIdentityResultWeakPtr thisWeak_;

          RTCIdentityResult();
          virtual ~RTCIdentityResult();
          virtual void wrapper_init_org_ortc_RTCIdentityResult() override;

          static RTCIdentityResultPtr toWrapper(NativeResultPtr native);
          static RTCIdentityResultPtr toWrapper(const NativeResult &native);
          static NativeResultPtr toNative(wrapper::org::ortc::RTCIdentityResultPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

