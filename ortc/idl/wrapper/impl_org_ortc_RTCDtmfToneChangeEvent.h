
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCDtmfToneChangeEvent.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCDtmfToneChangeEvent : public wrapper::org::ortc::RTCDtmfToneChangeEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(String, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(RTCDtmfToneChangeEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::RTCDtmfToneChangeEvent, WrapperType);
          RTCDtmfToneChangeEventWeakPtr thisWeak_;
          NativeType native_;

          RTCDtmfToneChangeEvent();
          virtual ~RTCDtmfToneChangeEvent();

          // properties RTCDtmfToneChangeEvent
          virtual String get_tone() override;

          static WrapperImplTypePtr toWrapper(NativeType native);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

