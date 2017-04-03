
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

          MediaDeviceInfo();
          virtual ~MediaDeviceInfo();

          // methods MediaDeviceInfo
          virtual void wrapper_init_org_ortc_MediaDeviceInfo() override;
          virtual void wrapper_init_org_ortc_MediaDeviceInfo(wrapper::org::ortc::MediaDeviceInfoPtr source) override;
          virtual void wrapper_init_org_ortc_MediaDeviceInfo(wrapper::org::ortc::JsonPtr json) override;
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

