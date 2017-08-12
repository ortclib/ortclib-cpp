
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

          RTCIdentityAssertion();
          virtual ~RTCIdentityAssertion();
          virtual void wrapper_init_org_ortc_RTCIdentityAssertion() override;

          static RTCIdentityAssertionPtr toWrapper(NativeAssertionPtr native);
          static RTCIdentityAssertionPtr toWrapper(const NativeAssertion &native);
          static NativeAssertionPtr toNative(wrapper::org::ortc::RTCIdentityAssertionPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

