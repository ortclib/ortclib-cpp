
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCDataChannel.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/datachannelinterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCDataChannel : public wrapper::org::webRtc::RTCDataChannel
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCDataChannel, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDataChannel, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::DataChannelInterface, NativeType);

          typedef rtc::scoped_refptr<NativeType> NativeScopedPtr;

          ZS_DECLARE_STRUCT_PTR(WebrtcObserver);

          struct WebrtcObserver : public ::webrtc::DataChannelObserver
          {
            WebrtcObserver(
              WrapperImplTypePtr wrapper,
              IMessageQueuePtr queue
            ) noexcept : outer_(wrapper), queue_(queue) {}

            void OnStateChange() final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              queue_->postClosure([outer]() { outer->onWebrtcObserverStateChange(); });
            }
            void OnMessage(const ::webrtc::DataBuffer& buffer) final
            {
              auto outer = outer_.lock();
              if (!outer) return;
              queue_->postClosure([outer, buffer]() { outer->onWebrtcObserverMessage(buffer); });
            }
            void OnBufferedAmountChange(uint64_t previous_amount)
            {
              auto outer = outer_.lock();
              if (!outer) return;
              queue_->postClosure([outer, previous_amount]() { outer->onWebrtcObserverBufferedAmountChange(previous_amount); });
            }

          private:
            WrapperImplTypeWeakPtr outer_;
            IMessageQueuePtr queue_;
          };

          std::atomic<uint64_t> bufferLowThreshold_ {};
          std::atomic<bool> bufferLowNotified_ {};
          std::atomic<bool> notifiedOpen_ {};
          std::atomic<bool> notifiedClosed_ {};
          zsLib::Lock lock_;
          String binaryType_ {};

          WebrtcObserverUniPtr observer_;
          NativeScopedPtr native_;
          RTCDataChannelWeakPtr thisWeak_;

          RTCDataChannel() noexcept;
          virtual ~RTCDataChannel() noexcept;

          // methods RTCStatsProvider
          shared_ptr< PromiseWithHolderPtr< wrapper::org::webRtc::RTCStatsReportPtr > > getStats(wrapper::org::webRtc::RTCStatsTypeSetPtr statTypes) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr

          // methods RTCDataChannel
          void close() noexcept override;
          void send(String text) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          void send(SecureByteBlockPtr data) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr

          // properties RTCDataChannel
          unsigned short get_id() noexcept override;
          String get_label() noexcept override;
          bool get_ordered() noexcept override;
          Optional< ::zsLib::Milliseconds > get_maxPacketLifeTime() noexcept override;
          Optional< unsigned short > get_maxRetransmits() noexcept override;
          String get_protocol() noexcept override;
          bool get_negotiated() noexcept override;
          wrapper::org::webRtc::RTCPriorityType get_priority() noexcept override;
          wrapper::org::webRtc::RTCDataChannelState get_readyState() noexcept override;
          uint64_t get_bufferedAmount() noexcept override;
          uint64_t get_bufferedAmountLowThreshold() noexcept override;
          void set_bufferedAmountLowThreshold(uint64_t value) noexcept override;
          String get_binaryType() noexcept override;
          void set_binaryType(String value) noexcept override;

          void wrapper_onObserverCountChanged(size_t count) noexcept override;

          void setupObserver();
          void teardownObserver();

          // WebrtcObserver methods
          void onWebrtcObserverStateChange() noexcept;
          void onWebrtcObserverMessage(const ::webrtc::DataBuffer& buffer) noexcept;
          void onWebrtcObserverBufferedAmountChange(uint64_t previous_amount) noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static NativeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

