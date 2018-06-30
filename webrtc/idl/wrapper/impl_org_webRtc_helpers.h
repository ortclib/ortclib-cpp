
#pragma once

#include "types.h"

#include <zsLib/types.h>
#include <map>

#define WRAPPER_DEPROXIFY_CLASS(xClassNamespace, xClassType, xPtr) \
        ::wrapper::impl::org::webRtc::deproxifyClass<xClassNamespace::xClassType, xClassNamespace::xClassType##Proxy, xClassNamespace::xClassType##Interface>(xPtr)

#ifdef WINUWP
#if defined(__cplusplus_winrt) && defined(CPPWINRT_VERSION)
#define WRAPPER_TO_CX(xCxType, xWinrtObject) \
        ::wrapper::impl::org::webRtc::to_cx<xCxType>(xWinrtObject)
#define WRAPPER_FROM_CX(xWinrtType, xCxObject) \
        ::wrapper::impl::org::webRtc::from_cx<xWinrtType>(xCxObject)
#else
#define WRAPPER_TO_CX(xCxType, xWinrtObject) \
        ERROR_USING_CONVERSION_ROUTING_TO_CX_WHEN_BOTH_CX_AND_CPPWINRT_ARE_NOT_AVAILABLE()
#endif //defined(__cplusplus_winrt) && defined(CPPWINRT_VERSION)
#endif //WINUWP


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // (helper functions)
        //

        //---------------------------------------------------------------------
        template<class XClassType, class XProxyType, class XOriginalInterfaceType>
        ZS_NO_DISCARD() XOriginalInterfaceType *deproxifyClass(XClassType *possibleProxy) noexcept
        {
          do
          {
            XProxyType *proxy = dynamic_cast<XProxyType *>(possibleProxy);
            if (!proxy) break;
            auto temp = proxy->internal();
            if (!temp) return possibleProxy;
            possibleProxy = dynamic_cast<XClassType *>(temp);
            if (!possibleProxy) return temp;
          } while (true);
          return possibleProxy;
        }

#ifdef WINUWP
#if defined(__cplusplus_winrt) && defined(CPPWINRT_VERSION)
        // see: https://github.com/Microsoft/cppwinrt/blob/master/Docs/Interoperability%20Helper%20Functions.md
        // from version: https://github.com/Microsoft/cppwinrt/blob/eaf572eafbc7506f977b9adcbdb7919751ef3e4b/Docs/Interoperability%20Helper%20Functions.md

        //---------------------------------------------------------------------
        template <typename T>
        T^ to_cx(winrt::Windows::Foundation::IUnknown const& from)
        {
          return safe_cast<T^>(reinterpret_cast<Platform::Object^>(winrt::get_abi(from)));
        }

        //---------------------------------------------------------------------
        template <typename T>
        T from_cx(Platform::Object^ from)
        {
          T to{ nullptr };

          winrt::check_hresult(reinterpret_cast<::IUnknown*>(from)->QueryInterface(winrt::guid_of<T>(),
            reinterpret_cast<void**>(winrt::put_abi(to))));

          return to;
        }
#endif //defined(__cplusplus_winrt) && defined(CPPWINRT_VERSION)
#endif //WINUWP

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // WrapperMapper
        //
        template<class XInterfaceType, class XWrapperImplType>
        class WrapperMapper
        {
        public:
          ZS_DECLARE_TYPEDEF_PTR(XWrapperImplType, WrapperImplType);
          typedef XInterfaceType NativeType;

          typedef std::map<NativeType *, WrapperImplTypeWeakPtr> NativeWrapperMap;

          //-------------------------------------------------------------------
          WrapperImplTypePtr getExistingOrCreateNew(
                                                    NativeType *native,
                                                    std::function<WrapperImplTypePtr()> creatorFunc
                                                    ) noexcept
          {
            zsLib::AutoLock lock(lock_);

            {
              auto found = map_.find(native);
              if (found != map_.end()) {
                auto &weak = found->second;
                auto strong = weak.lock();
                if (!strong) goto create_new_entry;
                return strong;
              }
            }

          create_new_entry:
            auto temp = creatorFunc();
            map_[native] = temp;
            return temp;
          }

          //-------------------------------------------------------------------
          void remove(NativeType *native)
          {
            if (!native) return;
            zsLib::AutoLock lock(lock_);
            map_.erase(native);
          }

        private:
          zsLib::Lock lock_;
          NativeWrapperMap map_;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

