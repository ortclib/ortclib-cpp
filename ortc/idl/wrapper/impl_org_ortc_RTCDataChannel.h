
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

          RTCDataChannel();
          virtual ~RTCDataChannel();

          // methods RTCStatsProvider
          virtual shared_ptr< PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > > getStats(wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) override;

          // methods RTCDataChannel
          virtual void wrapper_init_org_ortc_RTCDataChannel(
            wrapper::org::ortc::RTCDataTransportPtr transport,
            wrapper::org::ortc::RTCDataChannelParametersPtr params
            ) override;
          virtual void close() override;
          virtual void send(String text) override;
          virtual void send(SecureByteBlockPtr data) override;

          // properties RTCDataChannel
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::RTCDataTransportPtr get_transport() override;
          virtual wrapper::org::ortc::RTCDataChannelParametersPtr get_parameters() override;
          virtual wrapper::org::ortc::RTCDataChannelState get_readyState() override;
          virtual uint64_t get_bufferedAmount() override;
          virtual uint64_t get_bufferedAmountLowThreshold() override;
          virtual void set_bufferedAmountLowThreshold(uint64_t value) override;
          virtual String get_binaryType() override;
          virtual void set_binaryType(String value) override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          virtual void onDataChannelStateChange(
            IDataChannelPtr channel,
            ::ortc::IDataChannelTypes::States state
          ) override;

          virtual void onDataChannelError(
            IDataChannelPtr channel,
            ::ortc::ErrorAnyPtr error
          ) override;

          virtual void onDataChannelBufferedAmountLow(IDataChannelPtr channel) override;

          virtual void onDataChannelMessage(
            IDataChannelPtr channel,
            MessageEventDataPtr data
          ) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

