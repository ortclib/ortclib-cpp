
#pragma once

#include "types.h"
#include "generated/org_webRtc_AudioOptions.h"


#include "impl_org_webRtc_pre_include.h"
#include "api/audio_options.h"
#include "impl_org_webRtc_post_include.h"

namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct AudioOptions : public wrapper::org::webRtc::AudioOptions
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::AudioOptions, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::AudioOptions, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(::cricket::AudioOptions, NativeType);

          AudioOptionsWeakPtr thisWeak_;

          AudioOptions() noexcept;
          virtual ~AudioOptions() noexcept;
          void wrapper_init_org_webRtc_AudioOptions() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(const NativeType &native) noexcept;
          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(NativeTypePtr native) noexcept;

          ZS_NO_DISCARD() static NativeTypePtr toNative(WrapperTypePtr wrapper) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

