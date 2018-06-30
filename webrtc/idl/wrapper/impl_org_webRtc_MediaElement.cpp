
#ifdef WINUWP

#ifdef __cplusplus_winrt
#include <windows.ui.xaml.controls.h>
#endif //__cplusplus_winrt

#ifdef __has_include
#if __has_include(<winrt/Windows.UI.Xaml.Controls.h>)
#include <winrt/Windows.UI.Xaml.Controls.h>
#endif //__has_include(<winrt/Windows.UI.Xaml.Controls.h>)
#endif //__has_include

#else

#ifdef _WIN32
#include <Windows.h>
#endif //_WIN32

#endif //WINUWP

#include "impl_org_webrtc_MediaElement.h"

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

// borrow existing definitions from wrapper implementation
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaElement::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaElement::WrapperType, WrapperType);

#ifdef WINUWP
#ifdef __cplusplus_winrt

namespace wrapper { namespace impl { namespace org { namespace webrtc {
        ZS_DECLARE_STRUCT_PTR(MediaElementWrapperAnyCx);

        struct MediaElementWrapperAnyCx : Any
        {
          Windows::UI::Xaml::Controls::MediaElement^ element_ {nullptr};
        };
} } } }

#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

namespace wrapper { namespace impl { namespace org { namespace webrtc {
        ZS_DECLARE_STRUCT_PTR(MediaElementWrapperAnyCx);

        struct MediaElementWrapperAnyWinrt : Any
        {
          winrt::Windows::UI::Xaml::Controls::MediaElement element_ {nullptr};
        };
} } } }

#endif // CPPWINRT_VERSION

#else


namespace wrapper { namespace impl { namespace org { namespace webrtc {
        ZS_DECLARE_STRUCT_PTR(MediaElementWrapperAny);

        struct MediaElementWrapperAny : Any
        {
          HWND element_ {};
        };
} } } }

#endif //WINUWP

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::MediaElement::MediaElement() noexcept
{
}

//------------------------------------------------------------------------------
wrapper::org::webrtc::MediaElementPtr wrapper::org::webrtc::MediaElement::wrapper_create() noexcept
{
  auto pThis = make_shared<wrapper::impl::org::webrtc::MediaElement>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::webrtc::MediaElement::~MediaElement() noexcept
{
  thisWeak_.reset();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaElement::wrapper_init_org_webrtc_MediaElement() noexcept
{
}

//------------------------------------------------------------------------------
AnyPtr wrapper::impl::org::webrtc::MediaElement::get_element() noexcept
{
  return element_;
}

//------------------------------------------------------------------------------
void wrapper::impl::org::webrtc::MediaElement::set_element(AnyPtr value) noexcept
{
  element_ = value;
}


#ifdef WINUWP
#ifdef __cplusplus_winrt

wrapper::org::webrtc::MediaElementPtr wrapper::impl::org::webrtc::MediaElement::toWrapper(Windows::UI::Xaml::Controls::MediaElement^ element) noexcept
{
  auto any{ make_shared<wrapper::impl::org::webrtc::MediaElementWrapperAnyCx>() };
  any->element_ = element;
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->element_ = any;
  return result;
}

Windows::UI::Xaml::Controls::MediaElement^ wrapper::impl::org::webrtc::MediaElement::toNative_cx(wrapper::org::webrtc::MediaElementPtr element) noexcept
{
  if (!element) return nullptr;
  AnyPtr any = element->get_element();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webrtc::MediaElementWrapperAnyCx, any);
  if (!castedAny) return nullptr;
  return castedAny->element_;
}
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION

wrapper::org::webrtc::MediaElementPtr wrapper::impl::org::webrtc::MediaElement::toWrapper(winrt::Windows::UI::Xaml::Controls::MediaElement const & element) noexcept
{
  auto any{ make_shared<wrapper::impl::org::webrtc::MediaElementWrapperAnyWinrt>() };
  any->element_ = element;
  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->element_ = any;
  return result;
}

winrt::Windows::UI::Xaml::Controls::MediaElement wrapper::impl::org::webrtc::MediaElement::toNative_winrt(wrapper::org::webrtc::MediaElementPtr element) noexcept
{
  if (!element) return nullptr;
  AnyPtr any = element->get_element();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webrtc::MediaElementWrapperAnyWinrt, any);
  if (!castedAny) return nullptr;
  return castedAny->element_;
}

#endif // CPPWINRT_VERSION

#else

wrapper::org::webrtc::MediaElementPtr wrapper::impl::org::webrtc::MediaElement::toWrapper(HWND element) noexcept
{
  auto any{ make_shared<wrapper::impl::org::webrtc::MediaElementWrapperAny>() };
  any->element_ = element;
  auto result = wrapper::org::webrtc::MediaElement::wrapper_create();
  any->element_ = element;
  return result;
}

HWND wrapper::impl::org::webrtc::MediaElement::toNative(wrapper::org::webrtc::MediaElementPtr element) noexcept
{
  if (!element) return nullptr;
  AnyPtr any = element->get_element();
  if (!any) return nullptr;
  auto castedAny = ZS_DYNAMIC_PTR_CAST(wrapper::impl::org::webrtc::MediaElementWrapperAny, any);
  if (!castedAny) return nullptr;
  return castedAny->element_;
}

#endif //WINUWP

