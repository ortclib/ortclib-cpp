
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaDeviceInfo.h"

#include <ortc/IMediaDevices.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaDeviceInfo : public wrapper::org::ortc::MediaDeviceInfo
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaDevicesTypes::Device, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(MediaDeviceInfo, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::MediaDeviceInfo, WrapperType);
          MediaDeviceInfoWeakPtr thisWeak_;

          MediaDeviceInfo() noexcept;
          virtual ~MediaDeviceInfo() noexcept;

          // methods MediaDeviceInfo
          void wrapper_init_org_ortc_MediaDeviceInfo() noexcept override;
          void wrapper_init_org_ortc_MediaDeviceInfo(wrapper::org::ortc::MediaDeviceInfoPtr source) noexcept override;
          void wrapper_init_org_ortc_MediaDeviceInfo(wrapper::org::ortc::JsonPtr json) noexcept override;
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

