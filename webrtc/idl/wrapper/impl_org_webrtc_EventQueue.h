
#pragma once

#include "types.h"
#include "generated/org_webrtc_EventQueue.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct EventQueue : public wrapper::org::webrtc::EventQueue
        {
          EventQueueWeakPtr thisWeak_;

          AnyPtr queue_{};

          EventQueue() noexcept;
          virtual ~EventQueue() noexcept;

          // methods EventQueue
          void wrapper_init_org_webrtc_EventQueue(AnyPtr queue) noexcept override { queue_ = queue; }

          // properties EventQueue
          AnyPtr get_queue() noexcept override { return queue_; }

#ifdef WINUWP
#ifdef __cplusplus_winrt
          [[nodiscard]] static wrapper::org::webrtc::EventQueuePtr toWrapper(Windows::UI::Core::CoreDispatcher^ queue) noexcept;
          [[nodiscard]] static Windows::UI::Core::CoreDispatcher^ toNative_cx(wrapper::org::webrtc::EventQueuePtr queue) noexcept;
#endif //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
          [[nodiscard]] static wrapper::org::webrtc::EventQueuePtr toWrapper(winrt::Windows::UI::Core::CoreDispatcher queue) noexcept;
          [[nodiscard]] static winrt::Windows::UI::Core::CoreDispatcher toNative_winrt(wrapper::org::webrtc::EventQueuePtr queue) noexcept;
#endif // CPPWINRT_VERSION
#else
          [[nodiscard]] static wrapper::org::webrtc::EventQueuePtr toWrapper(::zsLib::IMessageQueuePtr queue) noexcept;
          [[nodiscard]] static ::zsLib::IMessageQueuePtr toNative(wrapper::org::webrtc::EventQueuePtr queue) noexcept;
#endif //WINUWP

        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

