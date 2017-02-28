
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDataChannelEvent.h"

#include <ortc/types.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDataChannelEvent : public wrapper::org::ortc::RTCDataChannelEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataChannel, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCDataChannelEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDataChannelEvent, WrapperType);
          RTCDataChannelEventWeakPtr thisWeak_;
          NativeTypePtr native_;

          RTCDataChannelEvent();
          virtual ~RTCDataChannelEvent();

          // properties RTCDataChannelEvent
          virtual wrapper::org::ortc::RTCDataChannelPtr get_dataChannel() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

