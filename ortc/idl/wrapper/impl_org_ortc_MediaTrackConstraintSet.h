
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaTrackConstraintSet.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaTrackConstraintSet : public wrapper::org::ortc::MediaTrackConstraintSet
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::ConstraintSet, NativeConstraintSet);

          MediaTrackConstraintSetWeakPtr thisWeak_;

          MediaTrackConstraintSet() noexcept;
          virtual ~MediaTrackConstraintSet() noexcept;

          // methods MediaTrackConstraintSet
          void wrapper_init_org_ortc_MediaTrackConstraintSet() noexcept override;
          void wrapper_init_org_ortc_MediaTrackConstraintSet(wrapper::org::ortc::MediaTrackConstraintSetPtr source) noexcept override;
          void wrapper_init_org_ortc_MediaTrackConstraintSet(wrapper::org::ortc::JsonPtr json) noexcept override;
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          static MediaTrackConstraintSetPtr toWrapper(NativeConstraintSetPtr native) noexcept;
          static MediaTrackConstraintSetPtr toWrapper(const NativeConstraintSet &native) noexcept;
          static NativeConstraintSetPtr toNative(wrapper::org::ortc::MediaTrackConstraintSetPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

