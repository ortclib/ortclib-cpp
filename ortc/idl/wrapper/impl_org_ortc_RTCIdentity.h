
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIdentity.h"

#include <ortc/IIdentity.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIdentity : public wrapper::org::ortc::RTCIdentity
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IIdentity, NativeIdentity);
          RTCIdentityWeakPtr thisWeak_;
          NativeIdentityPtr native_;

          RTCIdentity() noexcept;
          virtual ~RTCIdentity() noexcept;

          // methods RTCIdentity
          void wrapper_init_org_ortc_RTCIdentity(wrapper::org::ortc::RTCDtlsTransportPtr transport) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > getIdentityAssertion(String provider) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > getIdentityAssertion(
            String provider,
            String protocol
            ) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > getIdentityAssertion(
            String provider,
            String protocol,
            String username
            ) noexcept override;
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityAssertionPtr > > setIdentityAssertion(String assertion) noexcept override;

          // properties RTCIdentity
          uint64_t get_objectId() noexcept override;
          wrapper::org::ortc::RTCIdentityAssertionPtr get_peerIdentity() noexcept override;

          static RTCIdentityPtr toWrapper(NativeIdentityPtr native) noexcept;
          static NativeIdentityPtr toNative(wrapper::org::ortc::RTCIdentityPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

