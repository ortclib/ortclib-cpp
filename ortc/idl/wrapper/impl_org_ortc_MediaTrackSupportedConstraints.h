
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

          MediaTrackSupportedConstraints();
          virtual ~MediaTrackSupportedConstraints();

          // methods MediaTrackSupportedConstraints
          virtual void wrapper_init_org_ortc_MediaTrackSupportedConstraints() override;
          virtual void wrapper_init_org_ortc_MediaTrackSupportedConstraints(wrapper::org::ortc::MediaTrackSupportedConstraintsPtr source) override;
          virtual void wrapper_init_org_ortc_MediaTrackSupportedConstraints(wrapper::org::ortc::JsonPtr json) override;
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

