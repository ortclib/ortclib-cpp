
#pragma once

#include "types.h"
#include "generated/org_webrtc_MillisecondIntervalRange.h"


#include "impl_org_webrtc_pre_include.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct MillisecondIntervalRange : public wrapper::org::webrtc::MillisecondIntervalRange
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::MillisecondIntervalRange, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MillisecondIntervalRange, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::rtc::IntervalRange, NativeType);

          MillisecondIntervalRangeWeakPtr thisWeak_;

          MillisecondIntervalRange() noexcept;
          virtual ~MillisecondIntervalRange() noexcept;

          // methods MillisecondIntervalRange
          void wrapper_init_org_webrtc_MillisecondIntervalRange() noexcept override;
          void wrapper_init_org_webrtc_MillisecondIntervalRange(wrapper::org::webrtc::MillisecondIntervalRangePtr source) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

