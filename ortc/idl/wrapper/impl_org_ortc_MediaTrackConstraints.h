
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaTrackConstraints.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaTrackConstraints : public wrapper::org::ortc::MediaTrackConstraints
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::TrackConstraints, NativeTrackConstraints);

          MediaTrackConstraintsWeakPtr thisWeak_;

          MediaTrackConstraints() noexcept;
          virtual ~MediaTrackConstraints() noexcept;

          // methods MediaTrackConstraintSet
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          // methods MediaTrackConstraints
          virtual void wrapper_init_org_ortc_MediaTrackConstraints() noexcept override;
          virtual void wrapper_init_org_ortc_MediaTrackConstraints(wrapper::org::ortc::MediaTrackConstraintsPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_MediaTrackConstraints(wrapper::org::ortc::JsonPtr json) noexcept override;

          static MediaTrackConstraintsPtr toWrapper(NativeTrackConstraintsPtr native) noexcept;
          static MediaTrackConstraintsPtr toWrapper(const NativeTrackConstraints &native) noexcept;
          static NativeTrackConstraintsPtr toNative(wrapper::org::ortc::MediaTrackConstraintsPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

