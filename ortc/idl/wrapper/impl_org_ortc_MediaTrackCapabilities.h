
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaTrackCapabilities.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaTrackCapabilities : public wrapper::org::ortc::MediaTrackCapabilities
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes::Capabilities, NativeCapabilities);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICapabilities::CapabilityString, NativeCapabilityString);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::ICapabilities::CapabilityBoolean, NativeCapabilityBoolean);
          
          MediaTrackCapabilitiesWeakPtr thisWeak_;

          MediaTrackCapabilities() noexcept;
          virtual ~MediaTrackCapabilities() noexcept;

          // methods MediaTrackCapabilities
          virtual void wrapper_init_org_ortc_MediaTrackCapabilities() noexcept override;
          virtual void wrapper_init_org_ortc_MediaTrackCapabilities(wrapper::org::ortc::MediaTrackCapabilitiesPtr source) noexcept override;
          virtual void wrapper_init_org_ortc_MediaTrackCapabilities(wrapper::org::ortc::JsonPtr json) noexcept override;
          virtual wrapper::org::ortc::JsonPtr toJson() noexcept override;
          virtual String hash() noexcept override;

          static MediaTrackCapabilitiesPtr toWrapper(NativeCapabilitiesPtr native) noexcept;
          static MediaTrackCapabilitiesPtr toWrapper(const NativeCapabilities &native) noexcept;
          static NativeCapabilitiesPtr toNative(wrapper::org::ortc::MediaTrackCapabilitiesPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

