
#pragma once

#include "types.h"
#include "generated/org_ortc_MessageQueue.h"

#include <zsLib/IMessageQueue.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MessageQueue : public wrapper::org::ortc::MessageQueue
        {
          ZS_DECLARE_TYPEDEF_PTR(::zsLib::IMessageQueue, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(MessageQueue, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::MessageQueue, WrapperType);
          MessageQueueWeakPtr thisWeak_;
          NativeTypePtr native_;

          MessageQueue() noexcept;
          virtual ~MessageQueue() noexcept;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper
