
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaTrackSupportedConstraints.h"

#include <ortc/IMediaDevices.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaTrackSupportedConstraints : public wrapper::org::ortc::MediaTrackSupportedConstraints
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaDevicesTypes::SupportedConstraints, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(MediaTrackSupportedConstraints, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::MediaTrackSupportedConstraints, WrapperType);
          MediaTrackSupportedConstraintsWeakPtr thisWeak_;

          MediaTrackSupportedConstraints() noexcept;
          virtual ~MediaTrackSupportedConstraints() noexcept;

          // methods MediaTrackSupportedConstraints
          void wrapper_init_org_ortc_MediaTrackSupportedConstraints() noexcept override;
          void wrapper_init_org_ortc_MediaTrackSupportedConstraints(wrapper::org::ortc::MediaTrackSupportedConstraintsPtr source) noexcept override;
          void wrapper_init_org_ortc_MediaTrackSupportedConstraints(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;
          static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

