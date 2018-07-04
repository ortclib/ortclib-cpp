
#pragma once

#include "types.h"
#include "generated/org_webRtc_RTCDtmfSender.h"

#include "impl_org_webRtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "api/dtmfsenderinterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct RTCDtmfSender : public wrapper::org::webRtc::RTCDtmfSender
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::RTCDtmfSender, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::RTCDtmfSender, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::DtmfSenderInterface, NativeType);

          typedef rtc::scoped_refptr<NativeType> NativeTypeScopedPtr;


          ZS_DECLARE_STRUCT_PTR(WebrtcObserver);

          struct WebrtcObserver : public ::webrtc::DtmfSenderObserverInterface
          {
            WebrtcObserver(
              WrapperImplTypePtr wrapper,
              IMessageQueuePtr queue
            ) noexcept : outer_(wrapper), queue_(queue) {}

            void OnToneChange(const std::string& tone) override
            {
              auto outer = outer_.lock();
              if (!outer) return;
              queue_->postClosure([outer, tone]() { outer->onWebrtcObserverToneChange(tone); });
            }

          private:
            WrapperImplTypeWeakPtr outer_;
            IMessageQueuePtr queue_;
          };

          WebrtcObserverUniPtr observer_;
          NativeTypeScopedPtr native_;
          RTCDtmfSenderWeakPtr thisWeak_;

          RTCDtmfSender() noexcept;
          virtual ~RTCDtmfSender() noexcept;

          // methods RTCDtmfSender
          bool canInsertDtmf() noexcept override;
          void insertDtmf(String tones) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          void insertDtmf(
            String tones,
            ::zsLib::Milliseconds duration
            ) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr
          void insertDtmf(
            String tones,
            ::zsLib::Milliseconds duration,
            ::zsLib::Milliseconds interToneGap
            ) noexcept(false) override; // throws wrapper::org::webRtc::RTCErrorPtr

          // properties RTCDtmfSender
          String get_toneBuffer() noexcept override;
          ::zsLib::Milliseconds get_duration() noexcept override;
          ::zsLib::Milliseconds get_interToneGap() noexcept override;

          void wrapper_onObserverCountChanged(size_t count) noexcept override;


          void onWebrtcObserverToneChange(const String &tones) noexcept;

          void setupObserver() noexcept;
          void teardownObserver() noexcept;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeType *native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypeScopedPtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypeScopedPtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

