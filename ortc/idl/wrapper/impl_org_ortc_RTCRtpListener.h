
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCRtpListener.h"

#include <ortc/IRTPListener.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCRtpListener : public wrapper::org::ortc::RTCRtpListener,
                                public ::ortc::IRTPListenerDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPListener, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPListener, IRTPListener);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPListenerSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCRtpListener, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCRtpListener, WrapperType);

          RTCRtpListenerWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCRtpListener();
          virtual ~RTCRtpListener();

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

          // methods RTCRtpListener
          virtual void wrapper_init_org_ortc_RTCRtpListener(wrapper::org::ortc::RTCRtpTransportPtr transport) override;
          virtual void wrapper_init_org_ortc_RTCRtpListener(
            wrapper::org::ortc::RTCRtpTransportPtr transport,
            shared_ptr< list< wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr > > headerExtensions
            ) override;
          virtual void setHeaderExtensions(shared_ptr< list< wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr > > headerExtensions) override;

          // properties RTCRtpListener
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::RTCRtpTransportPtr get_transport() override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          virtual void onRTPListenerUnhandledRTP(
            IRTPListenerPtr listener,
            SSRCType ssrc,
            PayloadType payloadType,
            const char *mid,
            const char *rid
          ) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

