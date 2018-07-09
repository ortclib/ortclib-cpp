
#pragma once

#include "types.h"
#include "generated/org_webRtc_MediaSource.h"

#ifdef CPPWINRT_VERSION
#include <winrt/Windows.Media.Core.h>
#endif // CPPWINRT_VERSION

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct MediaSource : public wrapper::org::webRtc::MediaSource
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::MediaSource, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaSource, WrapperImplType);

          AnyPtr source_{};

          MediaSourceWeakPtr thisWeak_;

          MediaSource() noexcept;
          virtual ~MediaSource() noexcept;


          // methods MediaSource
          void wrapper_init_org_webRtc_MediaSource(AnyPtr source) noexcept override;

          // properties MediaSource
          AnyPtr get_source() noexcept override;

#ifdef WINUWP
#ifdef __cplusplus_winrt
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(Windows::Media::Core::IMediaSource^ source) noexcept;
          ZS_NO_DISCARD() static Windows::Media::Core::IMediaSource^ toNative_cx(wrapper::org::webRtc::MediaSourcePtr source) noexcept;
#endif //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(winrt::Windows::Media::Core::IMediaSource const & source) noexcept;
          ZS_NO_DISCARD() static winrt::Windows::Media::Core::IMediaSource toNative_winrt(wrapper::org::webRtc::MediaSourcePtr source) noexcept;
#endif // CPPWINRT_VERSION
#else
#ifdef _WIN32
#endif //_WIN32
#endif //WINUWP

        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

