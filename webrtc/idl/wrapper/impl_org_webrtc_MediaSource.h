
#pragma once

#include "types.h"
#include "generated/org_webrtc_MediaSource.h"

#ifdef CPPWINRT_VERSION
#include <winrt/Windows.Media.Core.h>
#endif // CPPWINRT_VERSION

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct MediaSource : public wrapper::org::webrtc::MediaSource
        {
          AnyPtr source_{};
          MediaSourceWeakPtr thisWeak_;

          MediaSource() noexcept;
          virtual ~MediaSource() noexcept;
          void wrapper_init_org_webrtc_MediaSource() noexcept override;

          // properties MediaSource
          AnyPtr get_source() noexcept override;
          void set_source(AnyPtr value) noexcept override;


#ifdef WINUWP
#ifdef __cplusplus_winrt
          [[nodiscard]] static wrapper::org::webrtc::MediaSourcePtr toWrapper(Windows::Media::Core::IMediaSource^ source) noexcept;
          [[nodiscard]] static Windows::Media::Core::IMediaSource^ toNative_cx(wrapper::org::webrtc::MediaSourcePtr source) noexcept;
#endif //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
          [[nodiscard]] static wrapper::org::webrtc::MediaSourcePtr toWrapper(winrt::Windows::Media::Core::IMediaSource const & source) noexcept;
          [[nodiscard]] static winrt::Windows::Media::Core::IMediaSource toNative_winrt(wrapper::org::webrtc::MediaSourcePtr source) noexcept;
#endif // CPPWINRT_VERSION
#else
#ifdef _WIN32
#endif //_WIN32
#endif //WINUWP

        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

