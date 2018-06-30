

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
#endif //_WIN32

#endif //WINUWP

#include "impl_org_webrtc_helpers.h"

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

namespace wrapper { namespace impl { namespace org { namespace webrtc { ZS_IMPLEMENT_SUBSYSTEM(wrapper_org_webrtc); } } } }
