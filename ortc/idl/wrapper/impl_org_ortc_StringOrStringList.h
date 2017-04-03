
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

          StringOrStringList();
          virtual ~StringOrStringList();

          // methods StringOrStringList
          virtual void wrapper_init_org_ortc_StringOrStringList() override;
          virtual void wrapper_init_org_ortc_StringOrStringList(wrapper::org::ortc::StringOrStringListPtr source) override;
          virtual void wrapper_init_org_ortc_StringOrStringList(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static StringOrStringListPtr toWrapper(NativeStringOrStringListPtr native);
          static StringOrStringListPtr toWrapper(const NativeStringOrStringList &native);
          static NativeStringOrStringListPtr toNative(wrapper::org::ortc::StringOrStringListPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

