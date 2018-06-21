
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaStreamConstraints.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaStreamConstraints : public wrapper::org::ortc::MediaStreamConstraints
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::Constraints, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(MediaStreamConstraints, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::MediaStreamConstraints, WrapperType);
          MediaStreamConstraintsWeakPtr thisWeak_;

          MediaStreamConstraints() noexcept;
          virtual ~MediaStreamConstraints() noexcept;

          // methods MediaStreamConstraints
          void wrapper_init_org_ortc_MediaStreamConstraints() noexcept override;
          void wrapper_init_org_ortc_MediaStreamConstraints(wrapper::org::ortc::MediaStreamConstraintsPtr source) noexcept override;
          void wrapper_init_org_ortc_MediaStreamConstraints(wrapper::org::ortc::JsonPtr json) noexcept override;
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

