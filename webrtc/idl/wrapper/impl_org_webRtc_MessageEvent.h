
#pragma once

#include "types.h"
#include "generated/org_webRtc_MessageEvent.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/datachannelinterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct MessageEvent : public wrapper::org::webRtc::MessageEvent
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::MessageEvent, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MessageEvent, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::DataBuffer, NativeType);

          SecureByteBlockPtr native_;
          bool isBinary_ {};
          MessageEventWeakPtr thisWeak_;

          MessageEvent() noexcept;
          virtual ~MessageEvent() noexcept;


          // properties MessageEvent
          SecureByteBlockPtr get_binary() noexcept override;
          String get_text() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const SecureByteBlock &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(SecureByteBlockPtr native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const String &native) noexcept;
          ZS_NO_DISCARD() static NativeType toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

