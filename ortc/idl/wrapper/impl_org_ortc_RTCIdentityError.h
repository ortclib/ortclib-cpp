
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIdentityError.h"

#include <ortc/IIdentity.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIdentityError : public wrapper::org::ortc::RTCIdentityError
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IIdentityTypes::Error, NativeError);

          RTCIdentityErrorWeakPtr thisWeak_;

          RTCIdentityError() noexcept;
          virtual ~RTCIdentityError() noexcept;
          void wrapper_init_org_ortc_RTCIdentityError() noexcept override;

          static RTCIdentityErrorPtr toWrapper(NativeErrorPtr native) noexcept;
          static RTCIdentityErrorPtr toWrapper(const NativeError &native) noexcept;
          static NativeErrorPtr toWrapper(wrapper::org::ortc::RTCIdentityErrorPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

