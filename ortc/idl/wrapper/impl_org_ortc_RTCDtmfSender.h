
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

          RTCDtmfSender();
          virtual ~RTCDtmfSender();

          // methods RTCDtmfSender
          virtual void wrapper_init_org_ortc_RTCDtmfSender(wrapper::org::ortc::RTCRtpSenderPtr sender) override;
          virtual bool canInsertDtmf() override;
          virtual void insertDtmf(String tones) override;
          virtual void insertDtmf(
            String tones,
            ::zsLib::Milliseconds duration
            ) override;
          virtual void insertDtmf(
            String tones,
            ::zsLib::Milliseconds duration,
            ::zsLib::Milliseconds interToneGap
            ) override;

          // properties RTCDtmfSender
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::RTCRtpSenderPtr get_sender() override;
          virtual String get_toneBuffer() override;
          virtual ::zsLib::Milliseconds get_duration() override;
          virtual ::zsLib::Milliseconds get_interToneGap() override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          virtual void onDTMFSenderToneChanged(
            IDTMFSenderPtr sender,
            String tone
          ) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

