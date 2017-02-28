
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

          RTCIdentity();
          virtual ~RTCIdentity();

          // methods RTCIdentity
          virtual void wrapper_init_org_ortc_RTCIdentity(wrapper::org::ortc::RTCDtlsTransportPtr transport) override;
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > getIdentityAssertion(String provider) override;
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > getIdentityAssertion(
            String provider,
            String protocol
            ) override;
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityResultPtr > > getIdentityAssertion(
            String provider,
            String protocol,
            String username
            ) override;
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCIdentityAssertionPtr > > setIdentityAssertion(String assertion) override;

          // properties RTCIdentity
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::RTCIdentityAssertionPtr get_peerIdentity() override;

          static RTCIdentityPtr toWrapper(NativeIdentityPtr native);
          static NativeIdentityPtr toNative(wrapper::org::ortc::RTCIdentityPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

