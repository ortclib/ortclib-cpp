
#pragma once

#include "types.h"
#include "generated/org_webRtc_MediaElementMaker.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct MediaElementMaker : public wrapper::org::webRtc::MediaElementMaker
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::MediaElementMaker, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::MediaElementMaker, WrapperImplType);
          MediaElementMaker() noexcept = delete;
          MediaElementMaker(const MediaElementMaker &) noexcept = delete;
          virtual ~MediaElementMaker() noexcept;

        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

