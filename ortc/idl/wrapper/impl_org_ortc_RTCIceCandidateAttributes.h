
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceCandidateAttributes.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceCandidateAttributes : public wrapper::org::ortc::RTCIceCandidateAttributes
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::ICECandidateAttributes, NativeStats);
          RTCIceCandidateAttributesWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCIceCandidateAttributes() noexcept;
          virtual ~RTCIceCandidateAttributes() noexcept;

          // methods RTCStats
          wrapper::org::ortc::JsonPtr toJson() noexcept override;
          String hash() noexcept override;

          // properties RTCStats
          ::zsLib::Time get_timestamp() noexcept override;
          Optional< wrapper::org::ortc::RTCStatsType > get_statsType() noexcept override;
          String get_statsTypeOther() noexcept override;
          String get_id() noexcept override;

          // methods RTCIceCandidateAttributes
          void wrapper_init_org_ortc_RTCIceCandidateAttributes() noexcept override;
          void wrapper_init_org_ortc_RTCIceCandidateAttributes(wrapper::org::ortc::RTCIceCandidateAttributesPtr source) noexcept override;
          void wrapper_init_org_ortc_RTCIceCandidateAttributes(wrapper::org::ortc::JsonPtr json) noexcept override;

          // properties RTCIceCandidateAttributes
          String get_relatedId() noexcept override;
          String get_ipAddress() noexcept override;
          unsigned long get_portNumber() noexcept override;
          String get_transport() noexcept override;
          wrapper::org::ortc::RTCIceCandidateType get_candidateType() noexcept override;
          unsigned long get_priority() noexcept override;
          String get_addressSourceUrl() noexcept override;

          static RTCIceCandidateAttributesPtr toWrapper(NativeStatsPtr native) noexcept;
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceCandidateAttributesPtr wrapper) noexcept;
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

