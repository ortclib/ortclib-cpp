
#pragma once

#include "types.h"
#include "generated/org_webRtc_Constraint.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/mediaconstraintsinterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct Constraint : public wrapper::org::webRtc::Constraint
        {
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::MediaConstraintsInterface::Constraint, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::Constraint, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::Constraint, WrapperType);

          NativeTypePtr native_;
          ConstraintWeakPtr thisWeak_;

          Constraint() noexcept;
          virtual ~Constraint() noexcept;

          // methods Constraint
          void wrapper_init_org_webRtc_Constraint() noexcept override;
          void wrapper_init_org_webRtc_Constraint(wrapper::org::webRtc::ConstraintPtr source) noexcept override;
          void wrapper_init_org_webRtc_Constraint(
            String key,
            String value
            ) noexcept override;

          // properties Constraint
          String get_key() noexcept override;
          void set_key(String value) noexcept override;
          String get_value() noexcept override;
          void set_value(String value) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

