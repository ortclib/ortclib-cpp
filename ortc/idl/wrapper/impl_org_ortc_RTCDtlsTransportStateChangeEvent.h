
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDtlsTransportStateChangeEvent.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDtlsTransportStateChangeEvent : public wrapper::org::ortc::RTCDtlsTransportStateChangeEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(RTCDtlsTransportStateChangeEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDtlsTransportStateChangeEvent, WrapperType);
          RTCDtlsTransportStateChangeEventWeakPtr thisWeak_;
          wrapper::org::ortc::RTCDtlsTransportState state_ {wrapper::org::ortc::RTCDtlsTransportState_new};

          RTCDtlsTransportStateChangeEvent();
          virtual ~RTCDtlsTransportStateChangeEvent();

          // properties RTCDtlsTransportStateChangeEvent
          virtual wrapper::org::ortc::RTCDtlsTransportState get_state() override;

          static WrapperImplTypePtr toWrapper(wrapper::org::ortc::RTCDtlsTransportState state);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

