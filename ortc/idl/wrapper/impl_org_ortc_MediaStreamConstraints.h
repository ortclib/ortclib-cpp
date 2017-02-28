
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

          MediaStreamConstraints();
          virtual ~MediaStreamConstraints();

          // methods MediaStreamConstraints
          virtual void wrapper_init_org_ortc_MediaStreamConstraints() override;
          virtual void wrapper_init_org_ortc_MediaStreamConstraints(wrapper::org::ortc::MediaStreamConstraintsPtr source) override;
          virtual void wrapper_init_org_ortc_MediaStreamConstraints(wrapper::org::ortc::JsonPtr json) override;
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr native);
          static WrapperImplTypePtr toWrapper(const NativeType &native);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

