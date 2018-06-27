
#pragma once

#include "types.h"

#include <zsLib/types.h>
#include <map>

#define WRAPPER_DEPROXIFY_CLASS(xClassNamespace, xClassType, xPtr) \
        ::wrapper::impl::org::webrtc::deproxifyClass<xClassNamespace::xClassType, xClassNamespace::xClassType##Proxy, xClassNamespace::xClassType##Interface>(xPtr)

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        template<class XClassType, class XProxyType, class XOriginalInterfaceType>
        [[nodiscard]] XOriginalInterfaceType *deproxifyClass(XClassType *possibleProxy) noexcept
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

        template<class XInterfaceType, class XWrapperImplType>
        class WrapperMapper
        {
        public:
          ZS_DECLARE_TYPEDEF_PTR(XWrapperImplType, WrapperImplType);
          typedef XInterfaceType NativeType;

          typedef std::map<NativeType *, WrapperImplTypeWeakPtr> NativeWrapperMap;

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

        private:
          zsLib::Lock lock_;
          NativeWrapperMap map_;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

