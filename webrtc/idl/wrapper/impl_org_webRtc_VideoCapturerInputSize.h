
#pragma once

#include "types.h"
#include "generated/org_webRtc_VideoCapturerInputSize.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct VideoCapturerInputSize : public wrapper::org::webRtc::VideoCapturerInputSize
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::VideoCapturerInputSize, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::VideoCapturerInputSize, WrapperImplType);
          VideoCapturerInputSizeWeakPtr thisWeak_;

          VideoCapturerInputSize() noexcept;
          virtual ~VideoCapturerInputSize() noexcept;
          void wrapper_init_org_webRtc_VideoCapturerInputSize() noexcept override;

          ZS_NO_DISCARD() static WrapperImplTypePtr toWrapper(int width, int height) noexcept;
        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

