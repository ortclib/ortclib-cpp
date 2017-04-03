
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCSsrcConflictEvent.h"

#include <ortc/IRTPTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCSsrcConflictEvent : public wrapper::org::ortc::RTCSsrcConflictEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IRTPTypes::SSRCType, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCSsrcConflictEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCSsrcConflictEvent, WrapperType);
          RTCSsrcConflictEventWeakPtr thisWeak_;
          NativeType native_;

          RTCSsrcConflictEvent();
          virtual ~RTCSsrcConflictEvent();

          // properties RTCSsrcConflictEvent
          virtual uint32_t get_ssrc() override;

          static WrapperImplTypePtr toWrapper(NativeType native);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

