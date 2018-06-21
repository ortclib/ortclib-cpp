
#pragma once

#include "types.h"
#include "generated/org_ortc_StringOrStringList.h"

#include <ortc/IConstraints.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct StringOrStringList : public wrapper::org::ortc::StringOrStringList
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IConstraints::StringOrStringList, NativeStringOrStringList);

          StringOrStringListWeakPtr thisWeak_;

          StringOrStringList() noexcept;
          virtual ~StringOrStringList() noexcept;

          // methods StringOrStringList
          void wrapper_init_org_ortc_StringOrStringList() noexcept override;
          void wrapper_init_org_ortc_StringOrStringList(wrapper::org::ortc::StringOrStringListPtr source) noexcept override;
          void wrapper_init_org_ortc_StringOrStringList(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static StringOrStringListPtr toWrapper(NativeStringOrStringListPtr native) noexcept;
          static StringOrStringListPtr toWrapper(const NativeStringOrStringList &native) noexcept;
          static NativeStringOrStringListPtr toNative(wrapper::org::ortc::StringOrStringListPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

