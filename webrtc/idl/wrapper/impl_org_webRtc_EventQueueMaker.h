
#pragma once

#include "types.h"
#include "generated/org_webRtc_EventQueueMaker.h"


namespace wrapper {
  namespace impl {
    namespace org {
      namespace webRtc {

        struct EventQueueMaker : public wrapper::org::webRtc::EventQueueMaker
        {
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::webRtc::EventQueueMaker, WrapperType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::webRtc::EventQueueMaker, WrapperImplType);
          EventQueueMaker() noexcept = delete;
          EventQueueMaker(const EventQueueMaker &) noexcept = delete;
          virtual ~EventQueueMaker() noexcept;

        };

      } // webRtc
    } // org
  } // namespace impl
} // namespace wrapper

