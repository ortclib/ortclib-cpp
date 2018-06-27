
#ifdef WINUWP

#ifdef __cplusplus_winrt
#include <windows.media.core.h>
#endif //__cplusplus_winrt

#ifdef __has_include
#if __has_include(<winrt/Windows.Media.Core.h>)
#include <winrt/Windows.Media.Core.h>
#endif //__has_include(<winrt/Windows.Media.Core.h>)
#endif //__has_include

#else

#ifdef _WIN32
#endif //_WIN32

#endif //WINUWP

#include "impl_org_webrtc_MediaSource.h"

using ::zsLib::String;
using ::zsLib::Optional;
using ::zsLib::Any;
using ::zsLib::AnyPtr;
using ::zsLib::AnyHolder;
using ::zsLib::Promise;
using ::zsLib::PromisePtr;
using ::zsLib::PromiseWithHolder;
using ::zsLib::PromiseWithHolderPtr;
using ::zsLib::eventing::SecureByteBlock;
using ::zsLib::eventing::SecureByteBlockPtr;
using ::std::shared_ptr;
using ::std::weak_ptr;
using ::std::make_shared;
using ::std::list;
using ::std::set;
using ::std::map;



#ifdef WINUWP
#ifdef __cplusplus_winrt

namespace wrapper { namespace impl { namespace org { namespace webrtc {
        ZS_DECLARE_STRUCT_PTR(MediaSourceWrapperAnyCx);

        struct MediaSourceWrapperAnyCx : Any
        {
          Windows::Media::Core::IMediaSource^ source_ {nullptr};
        };
} } } }

#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

namespace wrapper { namespace impl { namespace org { namespace webrtc {
        ZS_DECLARE_STRUCT_PTR(MediaSourceWrapperAnyCx);

        struct MediaSourceWrapperAnyWinrt : Any
        {
          winrt::Windows::Media::Core::IMediaSource source_ {nullptr};
        };
} } } }

#endif // CPPWINRT_VERSION

#else


namespace wrapper { namespace impl { namespace org { namespace webrtc {
        ZS_DECLARE_STRUCT_PTR(MediaSourceWrapperAny);

        struct MediaSourceWrapperAny : Any
        {
          HWND source_ {};
        };
} } } }

#endif //WINUWP

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::MediaSource::MediaSource() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaSourcePtr wrapper::org::webrtc::MediaSource::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::MediaSource>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::MediaSource::~MediaSource()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaSource::wrapper_init_org_webrtc_MediaSource() noexcept
{
}

//------------------------------------------------------------------------------
AnyPtr wrapper::impl::org::webrtc::MediaSource::get_source() noexcept
{
  return source_;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaSource::set_source(AnyPtr value) noexcept
{
  source_ = value;
}

#ifdef WINUWP
#ifdef __cplusplus_winrt

wrapper::org::webrtc::MediaSourcePtr wrapper::impl::org::webrtc::MediaSource::toWrapper(Windows::Media::Core::IMediaSource^ source) noexcept
{
  auto any{ make_shared<wrapper::impl::org::webrtc::MediaSourceWrapperAnyCx>() };
  any->source_ = source;
  auto result = wrapper::org::webrtc::MediaSource::wrapper_create();
  result->source_ = source;
  return result;
}

Windows::Media::Core::IMediaSource^ wrapper::impl::org::webrtc::MediaSource::toNative_cx(wrapper::org::webrtc::MediaSourcePtr source) noexcept
{
  if (!source) return nullptr;
  AnyPtr any = source->get_source();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webrtc::MediaSourceWrapperAnyCx, any);
  if (!castedAny) return nullptr;
  return castedAny->source_;
}
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

wrapper::org::webrtc::MediaSourcePtr wrapper::impl::org::webrtc::MediaSource::toWrapper(winrt::Windows::Media::Core::IMediaSource source) noexcept
{
  auto any{ make_shared<wrapper::impl::org::webrtc::MediaSourceWrapperAnyWinrt>() };
  any->source_ = source;
  auto result = wrapper::org::webrtc::MediaSource::wrapper_create();
  any->source_ = source;
  return result;
}

winrt::Windows::Media::Core::IMediaSource wrapper::impl::org::webrtc::MediaSource::toNative_winrt(wrapper::org::webrtc::MediaSourcePtr source) noexcept
{
  if (!source) return nullptr;
  AnyPtr any = source->get_source();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webrtc::MediaSourceWrapperAnyWinrt, any);
  if (!castedAny) return nullptr;
  return castedAny->source_;
}

#endif // CPPWINRT_VERSION

#else

#ifdef _WIN32
#endif //_WIN32

#endif //WINUWP

