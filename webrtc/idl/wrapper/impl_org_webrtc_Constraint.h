
#pragma once

#include "types.h"
#include "generated/org_webrtc_Constraint.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/mediaconstraintsinterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct Constraint : public wrapper::org::webrtc::Constraint
        {
          ZS_DECLARE_TYPEDEF_PTR(::webrtc::MediaConstraintsInterface::Constraint, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::Constraint, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::Constraint, WrapperType);

          NativeTypePtr native_;
          ConstraintWeakPtr thisWeak_;

          Constraint() noexcept;
          virtual ~Constraint() noexcept;

          // methods Constraint
          void wrapper_init_org_webrtc_Constraint() noexcept override;
          void wrapper_init_org_webrtc_Constraint(wrapper::org::webrtc::ConstraintPtr source) noexcept override;
          void wrapper_init_org_webrtc_Constraint(
            String key,
            String value
            ) noexcept override;

          // properties Constraint
          String get_key() noexcept override;
          void set_key(String value) noexcept override;
          String get_value() noexcept override;
          void set_value(String value) noexcept override;

          [[nodiscard]] static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          [[nodiscard]] static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          [[nodiscard]] static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

