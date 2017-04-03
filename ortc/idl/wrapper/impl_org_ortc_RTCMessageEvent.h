
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

          RTCMessageEvent();
          virtual ~RTCMessageEvent();

          // properties RTCMessageEvent
          virtual SecureByteBlockPtr get_binary() override;
          virtual String get_text() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

