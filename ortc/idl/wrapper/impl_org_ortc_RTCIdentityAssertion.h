
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIdentityAssertion.h"

#include <ortc/IIdentity.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIdentityAssertion : public wrapper::org::ortc::RTCIdentityAssertion
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IIdentityTypes::Assertion, NativeAssertion);
          RTCIdentityAssertionWeakPtr thisWeak_;

          RTCIdentityAssertion() noexcept;
          virtual ~RTCIdentityAssertion() noexcept;
          void wrapper_init_org_ortc_RTCIdentityAssertion() noexcept override;

          static RTCIdentityAssertionPtr toWrapper(NativeAssertionPtr native) noexcept;
          static RTCIdentityAssertionPtr toWrapper(const NativeAssertion &native) noexcept;
          static NativeAssertionPtr toNative(wrapper::org::ortc::RTCIdentityAssertionPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

