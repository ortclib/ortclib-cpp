
#pragma once

#include "types.h"
#include "generated/org_webRtc_EventQueue.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct EventQueue : public wrapper::org::webRtc::EventQueue
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::EventQueue, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::EventQueue, WrapperType);

          EventQueueWeakPtr thisWeak_;

          AnyPtr queue_{};

          EventQueue() noexcept;
          virtual ~EventQueue() noexcept;

          // methods EventQueue
          void wrapper_init_org_webRtc_EventQueue(AnyPtr queue) noexcept override { queue_ = queue; }

          // properties EventQueue
          AnyPtr get_queue() noexcept override { return queue_; }

#ifdef WINUWP
#ifdef __cplusplus_winrt
          ZS_NO_DISCARD() static wrapper::org::webRtc::EventQueuePtr toWrapper(Windows::UI::Core::CoreDispatcher^ queue) noexcept;
          ZS_NO_DISCARD() static Windows::UI::Core::CoreDispatcher^ toNative_cx(wrapper::org::webRtc::EventQueuePtr queue) noexcept;
#endif //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
          ZS_NO_DISCARD() static wrapper::org::webRtc::EventQueuePtr toWrapper(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept;
          ZS_NO_DISCARD() static winrt::Windows::UI::Core::CoreDispatcher toNative_winrt(wrapper::org::webRtc::EventQueuePtr queue) noexcept;
#endif // CPPWINRT_VERSION
#else
          ZS_NO_DISCARD() static wrapper::org::webRtc::EventQueuePtr toWrapper(::zsLib::IMessageQueuePtr queue) noexcept;
          ZS_NO_DISCARD() static ::zsLib::IMessageQueuePtr toNative(wrapper::org::webRtc::EventQueuePtr queue) noexcept;
#endif //WINUWP

        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

