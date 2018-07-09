
#pragma once

#include "types.h"
#include "generated/org_webRtc_MediaConstraints.h"

#include "impl_org_webRtc_pre_include.h"
#include "api/mediaconstraintsinterface.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct MediaConstraints : public wrapper::org::webRtc::MediaConstraints
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::MediaConstraints, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaConstraints, WrapperImplType);

          struct MediaConstraintsImpl : public ::webrtc::MediaConstraintsInterface {
          public:
            virtual ~MediaConstraintsImpl() noexcept;
            MediaConstraintsImpl() noexcept;
            MediaConstraintsImpl(
              const MediaConstraintsInterface::Constraints& mandatory,
              const MediaConstraintsInterface::Constraints& optional) noexcept;
            const MediaConstraintsInterface::Constraints& GetMandatory() const override;
            const MediaConstraintsInterface::Constraints& GetOptional() const override;

            MediaConstraintsInterface::Constraints mandatory_;
            MediaConstraintsInterface::Constraints optional_;
          };

          ZS_DECLARE_TYPEDEF_PTR(::webrtc::MediaConstraintsInterface, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(MediaConstraintsImpl, NativeImplType);

          NativeImplTypePtr native_;
          MediaConstraintsWeakPtr thisWeak_;

          MediaConstraints() noexcept;
          virtual ~MediaConstraints() noexcept;


          // methods MediaConstraints
          void wrapper_init_org_webRtc_MediaConstraints() noexcept override;
          void wrapper_init_org_webRtc_MediaConstraints(wrapper::org::webRtc::MediaConstraintsPtr source) noexcept override;
          void wrapper_init_org_webRtc_MediaConstraints(
            shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > mandatory,
            shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > optional
            ) noexcept override;

          // properties MediaConstraints
          shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > get_mandatory() noexcept override;
          void set_mandatory(shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > value) noexcept override;
          shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > get_optional() noexcept override;
          void set_optional(shared_ptr< list< wrapper::org::webRtc::ConstraintPtr > > value) noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

