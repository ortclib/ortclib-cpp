
#pragma once

#include "types.h"
#include "generated/org_ortc_adapter_RTCTrackEvent.h"

#include <ortc/adapter/IPeerConnection.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {

          struct RTCTrackEvent : public wrapper::org::ortc::adapter::RTCTrackEvent
          {
            ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IPeerConnection::MediaStreamTrackEvent, NativeType);
            ZS_DECLARE_TYPEDEF_PTR(RTCTrackEvent, WrapperImplType);
            ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::adapter::RTCTrackEvent, WrapperType);
            RTCTrackEventWeakPtr thisWeak_;
            NativeTypePtr native_;

            RTCTrackEvent() noexcept;
            virtual ~RTCTrackEvent() noexcept;

            // properties RTCTrackEvent
            virtual wrapper::org::ortc::RTCRtpReceiverPtr get_receiver() noexcept override;
            virtual wrapper::org::ortc::MediaStreamTrackPtr get_track() noexcept override;
            virtual shared_ptr< list< wrapper::org::ortc::adapter::MediaStreamPtr > > get_mediaStreams() noexcept override;

            static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
            static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
          };

        } // adapter
      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

