
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCMessageEvent.h"

#include <ortc/IDataChannel.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCMessageEvent : public wrapper::org::ortc::RTCMessageEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataChannelDelegate::MessageEventData, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCMessageEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCMessageEvent, WrapperType);
          RTCMessageEventWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCMessageEvent() noexcept;
          virtual ~RTCMessageEvent() noexcept;

          // properties RTCMessageEvent
          SecureByteBlockPtr get_binary() noexcept override;
          String get_text() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

