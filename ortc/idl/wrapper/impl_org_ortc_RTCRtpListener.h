
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

          RTCRtpListener() noexcept;
          virtual ~RTCRtpListener() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument::zsLib::Exceptions::BadState

          // methods RTCRtpListener
          void wrapper_init_org_ortc_RTCRtpListener(wrapper::org::ortc::RTCRtpTransportPtr transport) noexcept override;
          void wrapper_init_org_ortc_RTCRtpListener(
            wrapper::org::ortc::RTCRtpTransportPtr transport,
            shared_ptr< list< wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr > > headerExtensions
            ) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument
          void setHeaderExtensions(shared_ptr< list< wrapper::org::ortc::RTCRtpHeaderExtensionParametersPtr > > headerExtensions) noexcept(false) override; // throws ::zsLib::Exceptions::InvalidArgument

          // properties RTCRtpListener
          uint64_t get_objectId() noexcept override;
          wrapper::org::ortc::RTCRtpTransportPtr get_transport() noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          virtual void onRTPListenerUnhandledRTP(
            IRTPListenerPtr listener,
            SSRCType ssrc,
            PayloadType payloadType,
            const char *mid,
            const char *rid
          ) noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

