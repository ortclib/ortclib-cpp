
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDataChannelStateChangeEvent.h"

#include <ortc/IDataChannelTypes.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDataChannelStateChangeEvent : public wrapper::org::ortc::RTCDataChannelStateChangeEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IDataChannelTypes::States, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCDataChannelStateChangeEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDataChannelStateChangeEvent, WrapperType);
          RTCDataChannelStateChangeEventWeakPtr thisWeak_;
          NativeType native_;

          RTCDataChannelStateChangeEvent();
          virtual ~RTCDataChannelStateChangeEvent();

          // properties RTCDataChannelStateChangeEvent
          virtual wrapper::org::ortc::RTCDataChannelState get_state() override;

          static WrapperImplTypePtr toWrapper(NativeType native);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

