
#pragma once

#include "types.h"
#include "generated/org_webrtc_MediaConstraints.h"

#include "impl_org_webrtc_pre_include.h"
#include "api/mediaconstraintsinterface.h"
#include "impl_org_webrtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webrtc {

        struct MediaConstraints : public wrapper::org::webrtc::MediaConstraints
        {
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
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webrtc::MediaConstraints, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webrtc::MediaConstraints, WrapperType);

          NativeImplTypePtr native_;
          MediaConstraintsWeakPtr thisWeak_;

          MediaConstraints() noexcept;
          virtual ~MediaConstraints() noexcept;

          // methods MediaConstraints
          void wrapper_init_org_webrtc_MediaConstraints() noexcept override;
          void wrapper_init_org_webrtc_MediaConstraints(wrapper::org::webrtc::MediaConstraintsPtr source) noexcept override;
          void wrapper_init_org_webrtc_MediaConstraints(
            shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > mandatory,
            shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > optional
            ) noexcept override;

          // properties MediaConstraints
          shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > get_mandatory() noexcept override;
          void set_mandatory(shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > value) noexcept override;
          shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > get_optional() noexcept override;
          void set_optional(shared_ptr< list< wrapper::org::webrtc::ConstraintPtr > > value) noexcept override;
        };

      } // webrtc
    } // org
  } // namespace impl
} // namespace wrapper

