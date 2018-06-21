
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDataChannel.h"

#include <ortc/IDataChannel.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDataChannel : public wrapper::org::ortc::RTCDataChannel,
                                public ::ortc::IDataChannelDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataChannel, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataChannel, IDataChannel);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataChannelSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCDataChannel, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDataChannel, WrapperType);

          RTCDataChannelWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCDataChannel() noexcept;
          virtual ~RTCDataChannel() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept(false) override;

          // methods RTCDataChannel
          void wrapper_init_org_ortc_RTCDataChannel(
            wrapper::org::ortc::RTCDataTransportPtr transport,
            wrapper::org::ortc::RTCDataChannelParametersPtr params
            ) noexcept(false) override;
          void close() noexcept override;
          void send(String text) noexcept(false) override;
          void send(SecureByteBlockPtr data) noexcept(false) override;

          // properties RTCDataChannel
          uint64_t get_objectId() noexcept override;
          wrapper::org::ortc::RTCDataTransportPtr get_transport() noexcept override;
          wrapper::org::ortc::RTCDataChannelParametersPtr get_parameters() noexcept override;
          wrapper::org::ortc::RTCDataChannelState get_readyState() noexcept override;
          uint64_t get_bufferedAmount() noexcept override;
          uint64_t get_bufferedAmountLowThreshold() noexcept override;
          void set_bufferedAmountLowThreshold(uint64_t value) noexcept override;
          String get_binaryType() noexcept override;
          void set_binaryType(String value) noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          virtual void onDataChannelStateChange(
            IDataChannelPtr channel,
            ::ortc::IDataChannelTypes::States state
          ) noexcept override;

          virtual void onDataChannelError(
            IDataChannelPtr channel,
            ::ortc::ErrorAnyPtr error
          ) noexcept override;

          virtual void onDataChannelBufferedAmountLow(IDataChannelPtr channel) noexcept override;

          virtual void onDataChannelMessage(
            IDataChannelPtr channel,
            MessageEventDataPtr data
          ) noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

