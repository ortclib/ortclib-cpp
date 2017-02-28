
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSctpTransportListener.h"

#include <ortc/ISCTPTransport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSctpTransportListener : public wrapper::org::ortc::RTCSctpTransportListener,
                                          public ::ortc::ISCTPTransportListenerDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransportListenerSubscription, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransportListenerSubscription, ISCTPTransportListenerSubscription);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ISCTPTransportListenerSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCSctpTransportListener, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSctpTransportListener, WrapperType);

          RTCSctpTransportListenerWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          //bool defaultSubscription_{ true };  // always last a default subscription
          //NativeTypeSubscriptionPtr subscription_; // native type is a subscription

          RTCSctpTransportListener();
          virtual ~RTCSctpTransportListener();

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          virtual void onSCTPTransport(::ortc::ISCTPTransportPtr transport) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

