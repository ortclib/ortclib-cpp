
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDtmfSender.h"

#include <ortc/IDTMFSender.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDtmfSender : public wrapper::org::ortc::RTCDtmfSender,
                               public ::ortc::IDTMFSenderDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDTMFSender, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDTMFSender, IDTMFSender);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDTMFSenderSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(RTCDtmfSender, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDtmfSender, WrapperType);
          RTCDtmfSenderWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_{ true };
          NativeTypeSubscriptionPtr subscription_;

          RTCDtmfSender() noexcept;
          virtual ~RTCDtmfSender() noexcept;

          // methods RTCDtmfSender
          void wrapper_init_org_ortc_RTCDtmfSender(wrapper::org::ortc::RTCRtpSenderPtr sender) noexcept(false) override;
          bool canInsertDtmf() noexcept override;
          void insertDtmf(String tones) noexcept(false) override;
          void insertDtmf(
            String tones,
            ::zsLib::Milliseconds duration
            ) noexcept(false) override;
          void insertDtmf(
            String tones,
            ::zsLib::Milliseconds duration,
            ::zsLib::Milliseconds interToneGap
            ) noexcept(false) override;

          // properties RTCDtmfSender
          uint64_t get_objectId() noexcept override;
          wrapper::org::ortc::RTCRtpSenderPtr get_sender() noexcept override;
          String get_toneBuffer() noexcept override;
          ::zsLib::Milliseconds get_duration() noexcept override;
          ::zsLib::Milliseconds get_interToneGap() noexcept override;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          virtual void onDTMFSenderToneChanged(
            IDTMFSenderPtr sender,
            String tone
          ) noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

