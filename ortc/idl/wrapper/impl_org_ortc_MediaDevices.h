
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaDevices.h"

#include <ortc/IMediaDevices.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaDevices : public wrapper::org::ortc::MediaDevices,
                              public ::ortc::IMediaDevicesDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaDevices, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaDevices, IMediaDevices);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaDevicesSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(MediaDevices, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::MediaDevices, WrapperType);
          MediaDevicesWeakPtr thisWeak_;
          //NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          NativeTypeSubscriptionPtr subscription_;

          MediaDevices() noexcept;
          virtual ~MediaDevices() noexcept;

          virtual void wrapper_onObserverCountChanged(size_t count) noexcept override;

          virtual void onMediaDevicesChanged() noexcept override;

          void subscribe() noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

